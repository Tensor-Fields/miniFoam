#include "meshDance.H"
#include "GeometricField.H"
#include "fvPatchField.H"
#include "volMesh.H"

// smooth()
#include "boolList.H"
#include "emptyPolyPatch.H"
#include "boundBox.H"

// smooth() from cfMesh
#include "polyMeshGenModifier.H"
#include "meshOptimizer.H"

// degToRad
#include "unitConversion.H"

#include "cellSet.H"
#include "faceSet.H"

/// Smoother
#include "meshSurfaceEngine.H"
#include "meshSurfaceOptimizer.H"
#include "pointMesh.H"
#include "pointBoundaryMesh.H"

/// Lists
#include "SortableList.H"

///
/// Constructors
///

Foam::meshDance::meshDance
(
    const Time& runTime,
    const bool regFields
)
/// \note: Initializing mesh_
/// with an fvMesh parameter is private to fvMesh.
:
nowName_(runTime.timeName()),
runTime_(runTime),
registerFields_(regFields),
dMeshPtr_
(
    (
        dynamicFvMesh::New
        (
            IOobject
            (
                dynamicFvMesh::defaultRegion,
                runTime.timeName(),
                runTime,
                IOobject::MUST_READ
            )
        )
    ).ptr()
),
dMeshAutoPtr_
(dMeshPtr_
    //dynamicFvMesh::New
    //(
    //    IOobject
    //    (
    //        dynamicFvMesh::defaultRegion,
    //        runTime.timeName(),
    //        runTime,
    //        IOobject::MUST_READ
    //    )
    //)
),
/// mesh_ remained from the previous design; ideally we need dMeshPtr_ only.
//mesh_(dMeshAutoPtr_()),
mesh_(*dMeshPtr_),
fields_(mesh_, runTime.timeName()),

/// ALEDict
ALEDict_
(
    IOdictionary
    (
        IOobject
        (
            word("ALEDict"),
            runTime.constant(),
            //dMeshAutoPtr_(),
            *dMeshPtr_,
            IOobject::MUST_READ
        )
    )
),

remapDict_(ALEDict_.subDict("remapDict")),
rezoneDict_(ALEDict_.subDict("rezoneDict")),

/// Dimensions
wedge_(ALEDict_.lookup("wedge")),
planar2D_(),

/// Remap params
flipFluxSign_(/*remapDict.lookupOrDefault<bool>("flipFluxSign", false)*/),
nRemapSteps_(),
illegalToAdvect_(remapDict_.lookup("illegalToAdvect")),
maxIter_(),
fixedPatches_(),
ifForceWriteAtRemapTime_(),
sweptVolThreshold_(/*0*/),
aboveThresholdFraction_(),
minCellCount_(),

/// General rezone settings
startingPoints_(mesh_.points()),
startTime_(runTime.startTime()),
timeSpan_(runTime.endTime() - runTime.startTime()),
updatedStartingPoints_(mesh_.points()),
overallPointMotion_(pointField(mesh_.points().size(), vector::zero)),
motionFraction_(0),
sumMotionFraction_(0),
rMotionFraction_(),
rezoneFreq_(),
finalMeshIsHit_(false),

/// General smoothing params
iterPerQualityCheck_(),
qualityThreshold_(),
constrainedCellsSet_(),

/// Laplacian smoother params
laplacianMeshSmoother_(),
cfMeshSmoother_(),
patchWithSlidingPoints_(),
crossingPatches_(),
slideCrossingPoints_(),
maxSmoothingIter_(),
maxSmoothingResidual_(),
wedgeDir_(),
radialDir_(),
wedgeAperture_(),
emptyDir_(),
patchesWithFreePoints_(),
fixPointsInBoundBoxAnyway_(),
boundBoxMin_(),
boundBoxMax_(),

/// cfMesh Laplace smoother settings
untangle_(),
//lockedPatchNames_(),
wedgePatchNames_(),

/// 3-D smoothing params
nLoops_(),
nSurfaceIterations_(),

/// Mesh quality parameters
skewThreshold_(),
nonOrthThreshold_(),
aspectThreshold_(),

/// Good mesh quality parameters
goodSkewThreshold_(),
goodNonOrthThreshold_(),
goodAspectThreshold_()
{
    /// this->read<T> reads dictionary, not IOdict
    const dictionary& aleDict = ALEDict_;

    /// Dimensions
    //read<bool>(aleDict, word("wedge"), wedge_);
    read<bool>(aleDict, word("planar2D"), planar2D_);

    /// Remap
        read<bool> (remapDict_, word("flipFluxSign"), flipFluxSign_);
        read<label>(remapDict_, word("nRemapSteps"), nRemapSteps_);

        //read<List<string> > (remapDict_, word("illegalToAdvect"), illegalToAdvect_);

        /// Debug
        Info<< "illegalToAdvect:\n" 
            <<  illegalToAdvect_
            << endl;

        read<label>         (remapDict_, word("remapLoopPerStep"), maxIter_);
        read<bool>
            (remapDict_, word("forceWriteAtRemapTime"), ifForceWriteAtRemapTime_);
        read<bool>(remapDict_, word("remapWrite"), writeRemappedSteps_);
        ifRead<label>
        (writeRemappedSteps_,
            remapDict_, word("remapWriteFrequency"), remapWriteFreq_);
        read<scalar>       (remapDict_, word("sweptVolumeThreshold"), sweptVolThreshold_);

        if (sweptVolThreshold_ < 0)
        {
            FatalErrorIn("Constructor")
                << "sweptVolumeThreshold must be non-negative."
                << abort(FatalError);
        }
        else if (sweptVolThreshold_ > 1)
        {
            FatalErrorIn("Constructor")
                << "sweptVolumeThreshold is in 0-1 range."
                << abort(FatalError);
        }

        read<scalar>       (remapDict_, word("cellCountFraction"), aboveThresholdFraction_);

        if (aboveThresholdFraction_ <= 0 || aboveThresholdFraction_ > 1)
        {
            FatalErrorIn("Constructor")
                << "cellCountFraction is in (0,1] range."
                << abort(FatalError);
        }

        read<label>       (remapDict_, word("minCellCount"), minCellCount_);

        if (minCellCount_ <= 0)
        {
            FatalErrorIn("Constructor")
                << "minCellCount is in (0,inf) range."
                << abort(FatalError);
        }

    /// Rezone settings
        read<List<string> > (rezoneDict_, word("fixedPatches"), fixedPatches_);
        /// Reciprocal of motionFraction_
        rMotionFraction_ = nRemapSteps_;

        /// How many timeSteps per one rezone
        read<label>(rezoneDict_, word("rezoneFrequency"), rezoneFreq_);

    /// Mandatory entry, can be false though, if you do not want to use the
    /// built-in smoother
    read<bool>(rezoneDict_, word("LaplaceSmoother"), laplacianMeshSmoother_);
    //ifRead<bool>(!laplacianMeshSmoother_, aleDict, word("cfMeshSmoother"), cfMeshSmoother_);
    read<bool>(rezoneDict_, word("cfMeshSmoother"), cfMeshSmoother_);

    //if (laplacianMeshSmoother_)
    //{
        const dictionary& smootherDict = rezoneDict_.subDict("meshSmootherDict");

        read<bool>(smootherDict, word("untangle"), untangle_);
        //read<List<string> >
        //    (smootherDict, word("lockedPatchNames"), lockedPatchNames_);

        read<bool>(smootherDict, word("fixPointsInBoundBoxAnyway"), fixPointsInBoundBoxAnyway_);
        ifRead<point>(fixPointsInBoundBoxAnyway_, smootherDict, word("bbMin"), boundBoxMin_);
        ifRead<point>(fixPointsInBoundBoxAnyway_, smootherDict, word("bbMax"), boundBoxMax_);

        //read<bool>(smootherDict, word("smoothPatches"), smoothPatches_);
        read<word>(smootherDict, word("patchWithSlidingPoints"), patchWithSlidingPoints_);
        read<List<string> > (smootherDict, word("crossingPatches"), crossingPatches_);
        ifRead<List<string> >
            (wedge_, smootherDict, word("wedgePatchNames"), wedgePatchNames_);
        //read<bool>         (smootherDict, word("fixCrossingPoints"), fixCrossingPoints_);
        read<bool>    (smootherDict, word("slideCrossingPoints"), slideCrossingPoints_);
        read<label>   (smootherDict, word("maxRezoneIter"), maxSmoothingIter_);
        read<scalar>  (smootherDict, word("maxRezoneRes"), maxSmoothingResidual_);

        /// 3-D smoothing params
        ifRead<label>(!(wedge_ ^ planar2D_), smootherDict, word("nLoops"), nLoops_);
        ifRead<label>
        (
            !(wedge_ ^ planar2D_),
            smootherDict,
            word("nSurfaceIterations"),
            nSurfaceIterations_
        );

        /// General smoothing params
        read<label>
            (smootherDict, word("iterPerQualityCheck"), iterPerQualityCheck_);

        if (iterPerQualityCheck_ <= 0)
        {
            WarningIn("Constructor")
                << "iterPerQualityCheck must be >= 1;\n" 
                << "it is re-set to '1'."
                << endl;

            iterPerQualityCheck_ = 1;
        }

        read<scalar>
            (smootherDict, word("qualityThreshold"), qualityThreshold_);
        read<word>
            (smootherDict, word("constrainedCellsSet"), constrainedCellsSet_);

        ifRead<label> (wedge_, smootherDict, word("wedgeDir"), wedgeDir_);
        ifRead<label> (wedge_, smootherDict, word("radialDir"), radialDir_);
        ifRead<scalar>(wedge_, smootherDict, word("wedgeAperture"), wedgeAperture_);
        ifRead<label> (!wedge_, smootherDict, word("emptyDir"), emptyDir_);
        read<List<string> >(smootherDict, word("freePatches"), patchesWithFreePoints_);

        read<scalar>(smootherDict, word("skewThreshold"), skewThreshold_);
        read<scalar>(smootherDict, word("nonOrthoThreshold"), nonOrthThreshold_);
        read<scalar>(smootherDict, word("aspectThreshold"), aspectThreshold_);

        read<scalar>(smootherDict, word("goodSkewThreshold"), goodSkewThreshold_);
        read<scalar>(smootherDict, word("goodNonOrthoThreshold"), goodNonOrthThreshold_);
        read<scalar>(smootherDict, word("goodAspectThreshold"), goodAspectThreshold_);
    //}
    //else if (cfMeshSmoother_)
    //{
    //    const dictionary& smootherDict = ALEDict_.subDict("meshSmootherDict");

        //read<label>   (smootherDict, word("maxRezoneIter"), maxSmoothingIter_);
        //read<word>   (smootherDict, word("constrainedCellsSet"),
        //constrainedCellsSet_);
    //}
    //else
    //{
    //    FatalErrorIn("Constructor")
    //        << "At least one smoother must be true."
    //        << abort(FatalError);
    //}
    //{
    //    WarningIn("Constructor")
    //        << "No smoother selected."
    //        << endl;
    //}

    /// If case/system/fvSchemes/ddtScheme.default == steadyState,
    /// advection equation gives SIGFPE, so we define it as an error.

    if (mesh_.schemesDict().ddtSchemes().lookup("default")=="steadyState")
    {
        FatalErrorIn("system/fvSchemes.ddtScheme.default")
            << "ddtScheme cannot be steadyState"
            << abort(FatalError);
    }

    if (registerFields_)
    {
        Info<< "Read Time = "
            << mesh_.time().timeName()
            << " and register fields"
            << endl;

        registerFields<scalar>();
        registerFields<vector>();
        registerFields<tensor>();
        registerFields<symmTensor>();
        registerFields<sphericalTensor>();
    }
}

/// Destructor
Foam::meshDance::~meshDance()
{
    delete dMeshPtr_;
}

///
/// Private members
///

///
/// Public members
///

void Foam::meshDance::setOverallPointMotion
(
    const pointField& start,
    const pointField& finish
)
{
    //Info<< "start.size(): "
    //    << start.size()
    //    << endl;

    //Info<< "finish.size(): "
    //    << finish.size()
    //    << endl;

    const_cast<pointField&>(overallPointMotion_) =  (finish - start);
}

/// Time-step adaptive point increment
/// Returns true if points moved
bool Foam::meshDance::stepTowards()
{
    /// Incremental motion, dx = (dt / Time) * totalMove
    /// This formula is correct when Time/dt = N, number of iterations,
    /// in which case 
    /// dx = totalMove / N.
    
    /// When N is not integer, e.g., Time = 1, dt = 0.3, 
    /// - Does Time class automatically adjust the last step to 0.1, to fit
    ///   time into `1' window? If it does, we are getting the right answer.
    /// - If dt is changing, not a constant, chances are:
    /// dt_1 = 0.5, Time = 1, dt_1/Time = 0.5 provides half of the motion.
    /// dt_2 = 0.6, dt_2/Time = 0.6 provides more than half which is
    /// the remaining motion.
    
    /// The first point above should be tested.
    
    /// As far as mesh motion, we should always check that the motion does
    /// not overshoot.
                
    /// First update motion fraction
    updateMotionFraction();
    
    /// Check if motionFraction <= 1, otherwise, motion overshoots. 
    //if (motionFraction_/*()*/ > 1) // returns true when motionFraction_ == 1
    if ((motionFraction_ - 1.0) > SMALL)
    {
        /*FatalErrorIn*/WarningIn("stepTowards()\n")
            << "Motion overshoots,"
            << "Seems like deltaT > (endTime - startTime)"
            << "Mesh moves to the final points, ignoring the overshoot.\n"
            << /*abort(FatalError)*/endl;

        hitFinalMesh();
        return true;
    }
    else
    {
        if ((sumMotionFraction_ - 1.0) > SMALL)
        {
            Info<<"\n"<< endl;

            WarningIn("stepTowards()")
                << "sumMotionFraction_ = "
                << sumMotionFraction_
                << "\n"
                << "This incremental motion overshoots;\n"
                << "Mesh moves to the final points, ignoring the overshoot.\n"
                << endl;

            hitFinalMesh();
            return true;
        }
        else
        {
            /// Motion is allowed, as it, at most, hits the final mesh
            /// (sumMotionFraction == 1).
 
            /// However, check is needed to see if it undershoots when it
            /// supposed not to? Possibly this is the driver's responsibility.

            /// Debug
            //Info<< "motionFraction = "
            //    << motionFraction_
            //    << endl;

            //forAll(mesh_.points(), pI)
            //{
            //    Info<< mesh_.points()[pI]
            //        << "    "
            //        << (overallPointMotion_)[pI]
            //        //<< (mesh_.points() + motionFraction_
            //        // * overallPointMotion_)()[pI]
            //        << endl;
            //}

            Field<point> oldPoints = mesh_.points();

            /// Incremental new points
            mesh_.movePoints
            (
                /// New points
                oldPoints + motionFraction_ * overallPointMotion_
            );

            /// Points moved.
            return true;
        }
    }
}

bool Foam::meshDance::hitFinalMesh()
{
    if (!finalMeshIsHit_)
    {
        /// Move the mesh, so that the overall motion
        /// hits
        mesh_.movePoints(overallPointMotion_ + updatedStartingPoints_);

        finalMeshIsHit_ = true;
    }

    return finalMeshIsHit_;
}

/// Update motion fraction
void Foam::meshDance::updateMotionFraction()
{
    /// If rMotionFraction_ is 0
    if (!rMotionFraction_)
    {
        /// dt / Time, or 1/N where N is #iter
        motionFraction_ = 
            mesh_.time().deltaT().value() /
            (
                mesh_.time().endTime().value() 
              //- mesh_.time().startTime().value()
              - startTime_.value()
            );

        sumMotionFraction_ += motionFraction_; 
    }
    else
    {
        motionFraction_ = 1 / ( SMALL + rMotionFraction_);
    }
}

/// Access motion fraction
//scalar Foam::meshDance::motionFraction()
//{
//    return motionFraction_;
//}
//
//scalar Foam::meshDance::sumMotionFraction()
//{
//    return sumMotionFraction_;
//}

/// Move the mesh back to its initial config, incrementally
void Foam::meshDance::cyclicStep
(/*const dimensionedScalar& startTime*/)
{
    mesh_.movePoints
    (
        mesh_.points() + 
        (
            mesh_.time().deltaT().value() 
            /
            (
                mesh_.time().endTime().value() 
              //- mesh_.time().startTime().value()
              - startTime_.value()
            ) 
            * 
            overallPointMotion_
        )
    );
}

void Foam::meshDance::write()
{
    mesh_.write();
}

void Foam::meshDance::writeMesh()
{
    Info<< "    Write "
        << mesh_.name()
        << endl;

    // Writes registered fields too

    // 1-
    //mesh_.write()

    // 2-
    //mesh_.writeObject
    //(
    //    mesh_.time().writeFormat(),
    //    IOstream::currentVersion,
    //    mesh_.time().writeCompression()
    //);

    // 3-
    // Copy the mesh and write it

    polyMesh tmesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            mesh_.time().timeName(),
            mesh_.time(),
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            false
        ),
        xferCopy(mesh_.points()),
        xferCopy(mesh_.faces()),
        xferCopy(mesh_.faceOwner()),
        xferCopy(mesh_.faceNeighbour()),
        true
    );

    /// Add patches
    const polyBoundaryMesh& patchList = mesh_.boundaryMesh();

    List<polyPatch*> patchPtrList(patchList.size());

    forAll(patchPtrList, patchI)
    {
        /** [index/name] returns polyPatch& (See polyBoundaryMesh.H) */
        patchPtrList[patchI] = patchList[patchI].clone(patchList).ptr();
    }

    /// Add patches
    tmesh.addPatches(patchPtrList);

    tmesh.write();

    Info<< "polyMesh written to "
        << mesh_.time().timeName()
        << endl;

    // 4- Only write points
    // Error
    //mesh_.points().write()
}

void Foam::meshDance::resetPoints()
{
    mesh_.movePoints(startingPoints_);
}

void Foam::meshDance::resetStartingPoints(const pointField& pf)
{
    const_cast<pointField&>(updatedStartingPoints_) = pf;
}

/// Re-set mesh motion settings when a cycle is finished.
/// - startTime
/// - starting points
/// - sumMotionFraction
/// \note: It is the driver's responsibility to use it when needed.
/// This class is not aware of when the cycle finishes as of now.
void Foam::meshDance::resetMotion()
{
    const Time& runTime = mesh_.time();

    /// Re-set startTime and endTime
    setStartTime(runTime.endTime());
    const_cast<Time&>(runTime)
    .setEndTime(runTime.endTime() + timeSpan_);

    //Info<< "New startTime = "
    //    << startTime_
    //    << "\n"
    //    << "New endTime = "
    //    << runTime.endTime()
    //    << endl;

    resetStartingPoints(points());
    resetSumMotionFraction();
}

Foam::scalar Foam::meshDance::fluxSign()
{
    if (flipFluxSign_)
    {
        return -1.0;
    }

    return 1.0;
}

bool Foam::meshDance::ifRezone(const label someCounter)
{
    /// First check rezone frequency; if it is hit, do not bother calculating
    /// mesh quality
    if (rezoneNow(someCounter))
    {
        //rezoneCount_++;

        return true;
    }
    
    /// Check mesh quality
    //if (badMesh())
    if
    (
        badMesh
        (
            //mesh_,
            //aspectThreshold_,
            //skewThreshold_,
            //nonOrthThreshold_
        )
    )
    {
        return true;
    }

    return false;
}

bool Foam::meshDance::rezoneNow(const label count)
{
    /// Rules:
    /// - Only positive count is meaningful
    /// - rezoneFreq_ = 0 means do not rezone at all
    /// - Rezone when count is divisible by rezoneFreq_
    if ((count > 0) && (rezoneFreq_ > 0))
    {
        if((count % rezoneFreq_) < SMALL) return true;
        else
        {
            Info<< "    count % rezoneFreq_ == "
                << count % rezoneFreq_
                << '\n';
        }
    }
    /// Debug
    else
    {
        Info<< "    count = "
            << count
            << " and rezoneFreq_ = "
            << rezoneFreq_
            << "\n";
    }

    return false;
}

bool Foam::meshDance::meshFluxMeetsThreshold()
{
    /// I check mesh quality.... later!
    /// Optional quality checks to be defined in another function.

    /// Is mesh motion large enough?
    /// No! (by default)
    bool bad = false;

    /// Do we need to rezone at all?
    if
    (
        /// Swept vol threshold criterion is active (non-zero)
        sweptVolThreshold_
    )
    {
        const Field<point> oldPoints = mesh_.points();
        const Field<scalar> oldV = mesh_.V();
        const label nCells = oldV.size();

        /// SweptVol
        Field<scalar> sweptVol = mesh_.movePoints
        (
            oldPoints + overallPointMotion_
        );

        label sweptCellsCounter = 0;
        forAll(oldV, cellI)
        {
            /// Swept vol is larger than the threshold
            if
            (
                sweptVol[cellI] >= (sweptVolThreshold_ * oldV[cellI])
                /*.value()*/
            )
            {
                ++sweptCellsCounter;
            }
        }

        Info<< "    Number of cells whose swept volume meet the minimum\n"
            << /*"    required for rezoning*/" is "
            << sweptCellsCounter
            << endl;

        label minCellCount =
            min(ceil(nCells * aboveThresholdFraction_), minCellCount_);

        /// No. of cells whose swept vol meets the threshold is less than enough
        if
        (
            sweptCellsCounter < minCellCount
        )
        {
            /// Move points back
            mesh_.movePoints(oldPoints);

           // Info<< "    Number of cells required for rezoning is "
            Info<< "    Number of cells required for recognizing a bad mesh is "
                << minCellCount
                << endl;

            Info<< "    Bad mesh criterion is NOT met.\n"
                //<< "    Mesh did not move.\n"
                //<< "    Remap will be skipped."
                << endl;

            bad = false;
        }
        else
        {
            //Info<< "    Number of cells required for rezoning is "
            Info<< "    Number of cells required for recognizing a bad mesh is "
                << minCellCount
                << endl;

            Info<< "    Bad mesh criterion is met.\n"
                //<< "    Mesh did not move.\n"
                //<< "    Remap will be skipped."
                << endl;

            bad = true;
        }
    }
    /// Zero means turn off this check.
    else
    {
        bad = true;
    }

    return bad;
}

/// Laplacian smoother
/*const*/ Foam::pointField/*&*/ Foam::meshDance::LaplaceSmooth(const label maxIter)
{
    Info<< "    Lapalaian smoother starts.\n";

    /// Mesh reference
    ///*dynamicF*/fvMesh& mesh = dMeshAutoPtr_();
    /*dynamicF*/fvMesh& mesh = *dMeshPtr_;

    //Info << "3-D mesh" << endl;

    const labelListList& pointEdges = mesh.pointEdges();
    const edgeList& edges = mesh.edges();

    // Smooth internal points

    const vectorField& oldPoints = mesh.points();

    pointField newPoints = oldPoints;

    boolList fixedPoints(newPoints.size(), false);

    boolList slidingPoints(newPoints.size(), false);

    boolList freePoints(newPoints.size(), false);

    labelList crossingPoints(newPoints.size(), 0);

    /// Flag points based on the patch they belong to
    forAll(mesh.boundaryMesh(), patchI)
    {
        const polyPatch& patch = mesh.boundaryMesh()[patchI];
 
        const labelList& meshPoints =
            mesh.boundaryMesh()[patchI].meshPoints();

        bool skip = false;

        if (patch.name() == patchWithSlidingPoints_)
        {
            Info<< "    Flag points on patch "
                << patch.name()
                << " as sliding."
                << endl;

            forAll(meshPoints, pointI)
            {
                slidingPoints[meshPoints[pointI]] = true;
            }

            skip = true;
        }

        if (!skip)
        {
            /// Points on front and back patches are free to move, so not fixed
            /// Is `patch` free, i.e., is it in patchesWithFreePoints?
            forAll(patchesWithFreePoints_, freePatchI)
            {
                if (patch.name() == patchesWithFreePoints_[freePatchI])
                {
                    Info<< "    Flag points on patch "
                        << patch.name()
                        << " as free."
                        << endl;

                    forAll(meshPoints, pointI)
                    {
                        freePoints[meshPoints[pointI]] = true;
                        slidingPoints[meshPoints[pointI]] = false;
                    }

                    skip = true;

                    /// One patch at a time, as `patch` is a specific patch
                    break;
                }
            }
            /// Other points on patches are fixed.
            /// Filter-out points on axis patch
            if (!skip)
            {
                Info<< "    Flag points on patch "
                    << patch.name()
                    << " as fixed."
                    << endl;

                forAll(meshPoints, pointI)
                {
                    /// Re-include corner points, where axis intersects other
                    /// patches
                    fixedPoints[meshPoints[pointI]] = true;
                    freePoints[meshPoints[pointI]] = false;
                    slidingPoints[meshPoints[pointI]] = false;
                }
            }
        }
    }

    if (fixPointsInBoundBoxAnyway_)
    {
        ///
        /// Re-fix points falling inside fixedRegion
        ///
        
        /// Create fixedRegion
        boundBox fixedRegion(boundBoxMin_, boundBoxMax_);

        /// Fix points
        forAll(newPoints, pointI)
        {
            if (fixedRegion.contains(newPoints[pointI]))
            {
                fixedPoints[pointI] = true;

                Info<< "    Point "
                    << newPoints[pointI]
                    << " falls inside the specified bounding box, and will be fixed"
                    << '\n';
            }
        }
    }

    /// Flag points based on the crossing edges between two patches
    forAll(crossingPatches_, crossI)
    {
        const label patchID = 
            mesh.boundaryMesh().findPatchID(crossingPatches_[crossI]);

        //const polyPatch patch = mesh.boundaryMesh()[patchID];
        const labelList meshPoints =
            mesh.boundaryMesh()[patchID].meshPoints();

        forAll(meshPoints, pI)
        {
            /// crossingPoints[i] > 1 is a point on the common edge
            ++crossingPoints[meshPoints[pI]];
        }
    }

    scalarField residual(newPoints.size(), 0);
    label counter = 0;

    for (label smoothI = 0; smoothI < /*maxSmoothingIter_*/maxIter; ++smoothI)
    {
        counter++;

        Info<< "    Iteration: "
            << counter
            << " out of "
            << maxSmoothingIter_
            << endl;

        forAll(newPoints, pointI)
        {
            /// Only move non-fixed points,
            /// i.e., points not flagged as fixed, or points not belonging
            /// to more than one patch
            vector curNewPoint = vector::zero;

            scalar sumW = 0;

            forAll(pointEdges[pointI], eI)
            {
                label curEdgeIndex = pointEdges[pointI][eI];

                const edge& curEdge = edges[curEdgeIndex];

                vector d =
                    newPoints[curEdge.otherVertex(pointI)]
                  - newPoints[pointI];

                scalar w = 1.0;

                curNewPoint += w*d;

                sumW += w;
            }

            curNewPoint /= sumW;

            curNewPoint += newPoints[pointI];

            residual[pointI] = mag(curNewPoint - newPoints[pointI]);

            if (!fixedPoints[pointI])
            {
                if 
                (
                    !slidingPoints[pointI]
                    &&
                    !(
                        slideCrossingPoints_ &&
                        crossingPoints[pointI] > 1
                     )/*&& !freePoints[pointI]*/
                )
                {
                    newPoints[pointI][0] = curNewPoint[0];
                    newPoints[pointI][1] = curNewPoint[1];
                    /// If mesh is wedge
                    if 
                    (
                        wedge_
                    )
                    {
                        for (label dir = 0; dir < 3; ++dir)
                        {
                            if (dir != wedgeDir_)
                            newPoints[pointI][dir] = curNewPoint[dir];
                        }

                        // Safeguard denomirator from vanishing
                        //if (curNewPoint[wedgeDir] > SMALL)
                        {
                            scalar sign = 1;

                            if (curNewPoint[wedgeDir_] < 0)
                            {
                                sign = -1;
                            }

                            newPoints[pointI][wedgeDir_] =

                            // Sign (+/-)
                            //curNewPoint[wedgeDir]

                            /// Foam::sqrt(pow(curNewPoint[wedgeDir],2))
                            /// abs(curNewPoint[wedgeDir]) // Error:
                            // returns 0

                            sign
                            // radialDirComponent * tan(aperture)
                            * curNewPoint[radialDir_]
                            * Foam::tan
                              (
                                  degToRad
                                  (
                                      wedgeAperture_
                                      / 2.0
                                  )
                              );
                        }
                    }
                    else if
                    (
                        !wedge_
                    )
                    {
                        for (label dir = 0; dir < 3; ++dir)
                        {
                            if (dir != emptyDir_)
                            newPoints[pointI][dir] = curNewPoint[dir];
                        }
                    }
                }
                else if
                (
                    //slidingPoints[pointI] ||
                    (
                        slideCrossingPoints_ &&
                        crossingPoints[pointI] > 1
                    )/*&& !freePoints[pointI]*/
                )
                {
                    newPoints[pointI][0] = curNewPoint[0];
                }
            }
        }

        residual /= max(mag(newPoints - oldPoints) + SMALL);
 
        //runTime++;

        //if (runTime.write())
        //{
        //    mesh.movePoints(newPoints);
        //    mesh.write();
        //}
        if (max(residual) < maxSmoothingResidual_)
        {
            break;
        }
    }

    Info << "    Internal points, max residual: " << max(residual)
        << ", num of iterations: " << counter << endl;

    //twoDPointCorrector twoDCorrector(mesh);
    //twoDCorrector.correctPoints(newPoints);

    //twoDPointCorrector twoDCorrector(mesh);
    //twoDCorrector.correctPoints(newPoints);

    //mesh.movePoints(newPoints);

    Info<< "    Mesh smoothing done\n" << endl;

    return newPoints;
}

/// cfMesh Laplacian smoother
Foam::pointField Foam::meshDance::smooth/*cfMeshSmooth*/
(
    //const pointField& pf
)
{
    //- A local mesh
    polyMesh& mesh = mesh_;

    /// Freeze points for recovery
    const pointField& freezedPoints = mesh.points();

    //- Read patch data
    const polyBoundaryMesh& bm = mesh.boundaryMesh();

    labelList patchStart(bm.size());
    labelList nFacesInPatch(bm.size());

    forAll(bm, patchI)
    {
        patchStart[patchI] = bm[patchI].start();
        nFacesInPatch[patchI] = bm[patchI].size();
    }

    //- Avoid loading the mesh from disk
    polyMeshGen pmg
    (
        runTime_,
        mesh.points(),
        //pf,
        mesh.faces(),
        mesh.cells(),
        mesh.boundaryMesh().names(),
        patchStart,
        nFacesInPatch
    );

    //- construct the smoother
    meshOptimizer    mOpt(pmg);
    meshSurfaceEngine      sEngine(pmg);
    meshSurfaceOptimizer   sOpt(sEngine);

    //- Re-set constraints
    sOpt.removeUserConstraints();

    //- Add constraints

        //- Parse locked patch names
        List<string>& lockedPatchNames = fixedPatches_;

        //- Find locked patch IDs
        labelList lockedPatches(lockedPatchNames.size());

        forAll(lockedPatchNames, patchI)
        {
            lockedPatches[patchI] = bm.findPatchID(lockedPatchNames[patchI]);

            if (lockedPatches[patchI] == -1)
            {
                FatalErrorIn("smooth()")
                    << "It seems like the patch names specified in\n"
                    << "ALEDict.fixedPatches do not match the actual patch\n"
                    << "names."
                    << abort(FatalError);
            }
        }

        //- Find locked point labels
        const pointMesh pMesh(mesh);
        const pointBoundaryMesh& pbMesh = pMesh.boundary();

        labelList lockedPointLabels(0);

        forAll(lockedPatches, patchI)
        {
            lockedPointLabels.append(pbMesh[lockedPatches[patchI]].meshPoints());
        }

        //- Actually lock points
        sOpt.lockBoundaryPoints<labelList>(lockedPointLabels);

    //- clear geometry information before volume smoothing
    pmg.clearAddressingData();

    pointField oldPoints =  mesh.points();
    pointField oldOldPoints =  mesh.points();

    scalar characteristicLength = min(Foam::pow(mesh.cellVolumes(), 1./3.));

    //{
        oldPoints = mesh.points();

        for
        (
            label iter = 0;
            iter < scalar(maxSmoothingIter_) / scalar(iterPerQualityCheck_);
            ++iter
        )
        {
            /// If true, mesh is bad and needs optimization
            if
            (
                /// False is good mesh
                this->goodMesh()
             || iter == 0
            )
            {
                Info<<"    \nBoundary optimization iteration "
                    << iter + 1
                    << " ..."
                    << endl;

                if (cfMeshSmoother_)
                {
                    sOpt.optimizeSurface(iterPerQualityCheck_);
                }
                else if (laplacianMeshSmoother_)
                {
                    //pointFieldPMG& pf = pmg.points();
                    //pf = LaplaceSmooth(iterPerQualityCheck_);
                    const_cast<pointFieldPMG&>(pmg.points()) = LaplaceSmooth(iterPerQualityCheck_);
                }

                if (untangle_)
                {
                    sOpt.untangleSurface();
                }

                /// If 3-D
                if (mesh.nGeometricD() == 3)
                {
                    Info<< "    Mesh is 3-D.\n    Optimize non-boundary faces."
                        << endl;

                    //- perform optimisation of worst quality faces
                    mOpt.optimizeMeshFVBestQuality(nLoops_, qualityThreshold_);

                    //- check the mesh again and untangl bad regions if any of them exist
                    mOpt
                    .untangleMeshFV(nLoops_, iterPerQualityCheck_, nSurfaceIterations_);
                }

                /// Correct axis points
                /// Required only when cfMesh smoother is used.
                if (wedge_ && cfMeshSmoother_)
                {
                    correctAxisPoints(pmg/*, mesh*/, wedgePatchNames_);
                }

                /// Actually move the mesh
                mesh.movePoints(pmg.points());
            }
            else
            {
                Info<< "\n    Mesh is OK after "
                    << iter + 1
                    << " iterations."
                    << endl;

                break;
            }

            scalar changeMeasure =

            /// Stop if mesh is not changing anymore
            /// maxCurrentMotion / maxPrevMotion;
            /// Note: Not the best measure; when mesh motion is very slow per
            /// iteration, the sudden drop in this measure never occurs.

                //max(mag(mesh.points() - oldPoints))
                ///max(SMALL + mag(oldPoints - oldOldPoints));

                //average(mag(mesh.points() - oldPoints))
                ///average(SMALL + mag(oldPoints - oldOldPoints));

            /// Stop when maximum mesh motion is below a threshold
                max(mag(mesh.points() - oldPoints)) / characteristicLength;

            Info<< "    Relative mesh motion: "
                << changeMeasure
                << endl;

            if (changeMeasure < qualityThreshold_)
            {
                Info<< "\n    Mesh did not change within the threshold of "
                    << qualityThreshold_
                    << ";\n    Stop smoothing."
                    << endl;

                break;
            }

            oldOldPoints = oldPoints;
        }
    //}

    //- perform optimisation of worst quality faces
    //mOpt.optimizeMeshFVBestQuality(nLoops, qualityThreshold);

    //- check the mesh again and untangl bad regions if any of them exist
    //mOpt.untangleMeshFV(nLoops, nIterations, nSurfaceIterations);

    /// Recover freezed points
    mesh.movePoints(freezedPoints);

    /// Write the optimized mesh into the next time step.
    //- Create a polyMesh copy from polyMeshGen
    //- This is, because the latter only writes in constant
    //- However, the former is flexible as to where to write.

    //Info<< "pmg.points().size() after smoothing: "
    //    << pmg.points().size()
    //    << endl;

    //Info<< "mesh.points().size() after smoothing: "
    //    << mesh.points().size()
    //    << endl;

    autoPtr<pointField> pf(new pointField(pmg.points().size()));

    forAll(pf(), pointI)
    {
        pf()[pointI] = pmg.points()[pointI];
    }

    return pf();
}

void Foam::meshDance::defineBadMesh()
{
    polyMesh& mesh = mesh_;

    const_cast<debug::tolerancesSwitch&>(mesh.aspectThreshold_) =
        aspectThreshold_;
    const_cast<debug::tolerancesSwitch&>(mesh.skewThreshold_) = skewThreshold_;
    const_cast<debug::tolerancesSwitch&>(mesh.nonOrthThreshold_) =
        nonOrthThreshold_;
}

/// Return true if mesh is bad
bool Foam::meshDance::badMesh()
{
    this->defineBadMesh();

    polyMesh& mesh = mesh_;

    /// Check aspect ratio

    cellSet cells(mesh, "nonClosedCells", mesh.nCells()/100 + 1);
    cellSet aspectCells
    (
        mesh,
        "highAspectRatioCells",
        mesh.nCells()/100 + 1
    );

    if (aspectThreshold_ != -1)
    {
        if (mesh.checkClosedCells(true, &cells, &aspectCells))
        {
            //noFailedChecks++;

            label nNonClosed = returnReduce(cells.size(), sumOp<label>());

            if (nNonClosed > 0)
            {
                Info<< "  Writing " << nNonClosed
                    << " non closed cells to set " << cells.name() << endl;
                cells.write();
            }
        }

        label nHighAspect = returnReduce(aspectCells.size(), sumOp<label>());

        if (nHighAspect > 0)
        {
            Info<< "  Writing " << nHighAspect
                << " cells with high aspect ratio to set "
                << aspectCells.name() << endl;
            aspectCells.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Aspect ratio quality will not be checked ..."
            << endl;
    }

    /// Check skewness
 
    faceSet faces(mesh, "skewFaces", mesh.nFaces()/100 + 1);
    if (skewThreshold_!= -1)
    {
        if (mesh.checkFaceSkewness(true, &faces))
        {
            //noFailedChecks++;

            label nFaces = returnReduce(faces.size(), sumOp<label>());

            if (nFaces > 0)
            {
                Info<< "  Writing " << nFaces
                    << " skew faces to set " << faces.name() << endl;
                faces.write();

                /// Mesh is bad
                return true;
            }
        }
    }
    else
    {
        Info<< "    Skewness quality will not be checked ..."
            << endl;
    }

    faceSet facesNonOrtho(mesh, "nonOrthoFaces", mesh.nFaces()/100 + 1);

    if (nonOrthThreshold_!= -1)
    {
        if (mesh.checkFaceOrthogonality(true, &facesNonOrtho))
        {
            //noFailedChecks++;
        }

        label nFaces = returnReduce(facesNonOrtho.size(), sumOp<label>());

        if (nFaces > 0)
        {
            Info<< "  Writing " << nFaces
                << " non-orthogonal faces to set " << faces.name() << endl;
            faces.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Non-orthogonality will not be checked ..."
            << endl;
    }

    /// Mesh is OK
    return false;
}

void Foam::meshDance::defineGoodMesh()
{
    polyMesh& mesh = mesh_;

    const_cast<debug::tolerancesSwitch&>(mesh.aspectThreshold_) =
        goodAspectThreshold_;
    const_cast<debug::tolerancesSwitch&>(mesh.skewThreshold_) = goodSkewThreshold_;
    const_cast<debug::tolerancesSwitch&>(mesh.nonOrthThreshold_) =
        goodNonOrthThreshold_;
}

/// Return false if mesh is good
/// Used to stop mesh smoothing
bool Foam::meshDance::goodMesh()
{
    this->defineGoodMesh();

    polyMesh& mesh = mesh_;

    /// Check aspect ratio
    cellSet cells(mesh, "nonClosedCells", mesh.nCells()/100 + 1);
    cellSet aspectCells
    (
        mesh,
        "highAspectRatioCells",
        mesh.nCells()/100 + 1
    );

    if (goodAspectThreshold_ != -1)
    {
        if (mesh.checkClosedCells(true, &cells, &aspectCells))
        {
            //noFailedChecks++;

            label nNonClosed = returnReduce(cells.size(), sumOp<label>());

            if (nNonClosed > 0)
            {
                //Info<< "  Writing " << nNonClosed
                //    << " non closed cells to set " << cells.name() << endl;
                //cells.write();
            }
        }

        label nHighAspect = returnReduce(aspectCells.size(), sumOp<label>());

        if (nHighAspect > 0)
        {
            //Info<< "  Writing " << nHighAspect
            //    << " cells with high aspect ratio to set "
            //    << aspectCells.name() << endl;
            //aspectCells.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Aspect ratio quality will not be checked ..."
            << endl;
    }

    /// Check skewness
 
    faceSet faces(mesh, "skewFaces", mesh.nFaces()/100 + 1);
    if (goodSkewThreshold_!= -1)
    {
        if (mesh.checkFaceSkewness(true, &faces))
        {
            //noFailedChecks++;

            label nFaces = returnReduce(faces.size(), sumOp<label>());

            if (nFaces > 0)
            {
                //Info<< "  Writing " << nFaces
                //    << " skew faces to set " << faces.name() << endl;
                //faces.write();

                /// Mesh is bad
                return true;
            }
        }
    }
    else
    {
        Info<< "    Skewness quality will not be checked ..."
            << endl;
    }

    faceSet facesNonOrtho(mesh, "nonOrthoFaces", mesh.nFaces()/100 + 1);

    if (goodNonOrthThreshold_!= -1)
    {
        if (mesh.checkFaceOrthogonality(true, &facesNonOrtho))
        {
            //noFailedChecks++;
        }

        label nFaces = returnReduce(facesNonOrtho.size(), sumOp<label>());

        if (nFaces > 0)
        {
            //Info<< "  Writing " << nFaces
            //    << " non-orthogonal faces to set " << faces.name() << endl;
            //faces.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Non-orthogonality will not be checked ..."
            << endl;
    }

    /// Mesh is OK
    return false;
}

/// Force axis points to remain on axis
void Foam::meshDance::correctAxisPoints
(
    polyMeshGen& pmg,
    const List<string>& wedgePatchNames
)
{
    if
    (
        wedgePatchNames.size() - 2 > SMALL
    )
    {
        FatalErrorIn("correctAxisPoints()")
            << "wedgePatchNames must be exactly two names of the"
            << "\ngeometrically-same-wedge patches;\n"
            << abort(FatalError);
    }

    const polyMesh& mesh = mesh_;

    const polyBoundaryMesh& bm = mesh.boundaryMesh();
    //const PtrList<boundaryPatch>& bm = pmg.boundaries();
    const pointMesh pMesh(mesh);
    const pointBoundaryMesh& pbMesh = pMesh.boundary();

    if (!bm.size())
    {
        FatalErrorIn("correctAxisPoints()")
            << "Boundary mesh is empty."
            << abort(FatalError);
    }

    if (!pMesh.size())
    {
        FatalErrorIn("correctAxisPoints()")
            << "Point mesh is empty."
            << abort(FatalError);
    }

    if (!pbMesh.size())
    {
        FatalErrorIn("correctAxisPoints()")
            << "Point boundary mesh is empty."
            << abort(FatalError);
    }

    //const pointBoundaryMesh& pbMesh = pMesh.boundary();

    //- Find axis points
    labelList wedgePatchLabels(wedgePatchNames.size());

    //Info<< "wedgePatchLabels size: "
    //    << wedgePatchLabels.size()
    //    << endl;

    SortableList<label> wedgePointLabels;

    forAll(wedgePatchNames, patchI)
    {
        //Info<< wedgePatchNames[patchI]
        //    << " patch number of points: "
        //    <<
        //    pbMesh[bm.findPatchID(wedgePatchNames[patchI])].meshPoints().size()
        //    << endl;

        //wedgePathLabels[patchI] = bm.findPatchID(wedgePatchNames[patchI]);
        wedgePointLabels
        .append(pbMesh[bm.findPatchID(wedgePatchNames[patchI])].meshPoints());
        //.append(pbMesh[pmg.getPatchID(wedgePatchNames[patchI])].meshPoints());
    }

    if (!wedgePointLabels.size())
    {
        FatalErrorIn("correctAxisPoints()")
            << "Failed to find point labels on wedge patches."
            << abort(FatalError);
    }

    wedgePointLabels.sort();

    //Info<< "wedge point labels saved."
    //    << endl;

    SortableList<label> axisPointLabels;

    /// Find duplicate labels, that is, axis point labels
    for
    ( 
        label pointI = 0;
        pointI < (wedgePointLabels.size() - 1);
        ++pointI
    )
    {
        if (wedgePointLabels[pointI] == wedgePointLabels[pointI + 1])
        {
            axisPointLabels.append(List<label>(1, wedgePointLabels[pointI]));
        }
    }

    if (!axisPointLabels.size())
    {
        FatalErrorIn("correctAxisPoints()")
            << "Failed to find axis point labels for the wedge geometry."
            << abort(FatalError);
    }

    //Info<< "axis point labels saved:"
        //<< axisPointLabels
        //<< endl;

    /// Axis vector
    vector axis =
        mesh.points()[axisPointLabels[0]]
      - mesh.points()[axisPointLabels[axisPointLabels.size() - 1]];

    /// Project axis points onto axis
    forAll(axisPointLabels, pointI)
    {
        const_cast<point&>(pmg.points()[axisPointLabels[pointI]]) =
            //transform(axis, pmg.points()[axisPointLabels[pointI]]);
            (axis & pmg.points()[axisPointLabels[pointI]]) * axis
            / sqr(mag(axis));
    }
}

//Foam::pointField Foam::meshDance::smooth()
//{
//    autoPtr<pointField> pf
//    (
//        new Field<point>(mesh_.points().size(),vector::zero)
//    );
//
//    if (laplacianMeshSmoother_)
//    {
//        /// Plain Laplace smooth, with point slide functionality
//        pf.reset(new pointField(LaplaceSmooth()));
//    //}
//    //else if (cfMeshSmoother_)
//    //{
//        /// Improve quality (untangle)
//        if (cfMeshSmoother_)
//        {
//            pf.reset(new pointField(cfMeshSmooth(pf)));
//        }
//    }
//    else
//    {
//        FatalErrorIn("smooth()")
//            << "No smoother selected."
//            << abort(FatalError);
//    }
//
//    return pf();
//}

/// Obtains the rezoned state; mesh motion actually happens in
/// remap()
void Foam::meshDance::rezone(const pointField& targetPoints)
{
    //Info<< "targetPoints.size() = "
    //    << targetPoints.size()
    //    << endl;

    //this -> setOverallPointMotion(dMeshAutoPtr_().points(), targetPoints);
    this -> setOverallPointMotion((*dMeshPtr_).points(), targetPoints);
}


//template<class Type>
//void Foam::meshDance::storeFieldCopies()
//{
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    > fields = mesh_.thisDb().lookupClass
//    <
//        GeometricField<Type, fvPatchField, volMesh>
//    >();
//
//    typename
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    >::iterator fieldIter;
//
//    for
//    (
//        fieldIter = fields.begin();
//        fieldIter !=fields.end();
//        ++fieldIter
//    )
//    {
//        GeometricField
//	    <
//	        Type, fvPatchField, volMesh
//	    >& field = const_cast
//	    <
//            GeometricField
//	    <
//	        Type, fvPatchField, volMesh
//	    >&
//	    >(*fieldIter());
//
//        //{
//        tof_<Type>.set[field.name()] = new GeometricField<Type, fvPatchFields, volMesh>
//            (
//                IOobject
//                (
//                    field.name(),
//                    nowName_,
//                    mesh_,
//                    IOobject::NO_READ,
//                    IOobject::NO_WRITE,
//                    /// Do not register
//                    false
//                ),
//                field,
//                "zeroGradient"
//            );
//        //}
//    }
//}


//template<class Type>
//void Foam::meshDance::saveOldOldTimes()
//{
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    > fields = mesh_.thisDb().lookupClass
//    <
//        GeometricField<Type, fvPatchField, volMesh>
//    >();
//
//    typename
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    >::iterator fieldIter;
//
//    for
//    (
//        fieldIter = fields.begin();
//        fieldIter !=fields.end();
//        ++fieldIter
//    )
//    {
//        GeometricField
//	    <
//	        Type, fvPatchField, volMesh
//	    >& field = const_cast
//	    <
//            GeometricField
//	    <
//	        Type, fvPatchField, volMesh
//	    >&
//	    >(*fieldIter());
//
//        if (isOldOld(field.name()))
//        {
//            oldOldFHT_<Type>.set[field.name()] =
//                new GeometricField<Type, fvPatchFields, volMesh>
//                (
//                    IOobject
//                    (
//                        field.name(),
//                        nowName_,
//                        mesh_,
//                        IOobject::NO_READ,
//                        IOobject::NO_WRITE,
//                        /// Do not register
//                        false
//                    ),
//                    field,
//                    "zeroGradient"
//                );
//        }
//    }
//}


void Foam::meshDance::remap(const bool write)
{
    const scalar now = runTime_.value();
    const label nowIndex = runTime_.timeIndex();
    const scalar deltaT = runTime_.deltaTValue();

    /// To be accessed inside advect()
    nowName_ = runTime_.timeName();
    const_cast<Time&>(runTime_).setDeltaT(1);

    /// Save old- and oldOld-physical-time fields
    //saveOldTimes<scalar>();
    //saveOldTimes<vector>();
    //saveOldTimes<tensor>();
    //saveOldTimes<symmTensor>();

    //this->storeFieldCopies<scalar>();
    //this->storeFieldCopies<vector>();
    //this->storeFieldCopies<tensor>();
    //this->storeFieldCopies<symmTensor>();

    /// Advect nowTime fields
    //if (meshFluxMeetsThreshold())
    //{

    //isSameAsOld<vector>(word("DU"), "Before remap loop:\n", word("Before_remap_loop"));

        for (label iter = 0; iter < nRemapSteps_; ++iter)
        {
            Info<< "Remap step = "
                <<iter<<"\n";

            /// Rezone overshoot-proof
            /// Read fields from the database and advect
            if (this -> stepTowards())
            {
                Info<< "\n    Final mesh in this cycle is hit; ignore next"
                    << "\n    time steps, advect fields and move to the" 
                    << "\n    next cycle."
                    << endl;

                /// VolFields
                this -> advect<scalar>    (writeRemappedSteps_);
                this -> advect<vector>    (writeRemappedSteps_);
                this -> advect<tensor>    (writeRemappedSteps_);
                this -> advect<symmTensor>(writeRemappedSteps_);
                //this -> advect<sphericalTensor>(write);

                if (finalMeshIsHit_)
                {
                    finalMeshIsHit_ = false;
                    break;
                }
            }

            /// Volume fields
            //this -> advect<scalar>    (writeRemappedSteps_);
            //this -> advect<vector>    (writeRemappedSteps_);
            //this -> advect<tensor>    (writeRemappedSteps_);
            //this -> advect<symmTensor>(writeRemappedSteps_);
            //this -> advect<sphericalTensor>(write);

            //isSameAsOld<vector>(word("DU"), "In advect loop, before time++\n",
            //word("In_advect_loop_before_timePP"));

            /// Move ahead; needs to be after advect()
            //const_cast<Time&>(/*pseudo*/runTime_)++;

            /// Does it prevent the re-creation of old fields?
            //resetFieldTimeIndex<scalar>    (runTime_.timeIndex());
            //resetFieldTimeIndex<vector>    (runTime_.timeIndex());
            //resetFieldTimeIndex<tensor>    (runTime_.timeIndex());
            //resetFieldTimeIndex<symmTensor>(runTime_.timeIndex());

            /// Store old fields
            //storeOldFields();
             
            /// Previous design: Write remap steps in physical time dirs.
            /// New design: Write remap steps "inside" the remap physical time

            //isSameAsOld<vector>(word("DU"), "In advect loop, after time++\n",
            //word("In_advect_loop_after_timePP"));
        }

        //isSameAsOld<vector>(word("DU"), "After advect loop",
        //word("After_advect_loop"));

        /// Correct physical time, and prevent fields from re-creating old
        /// fields
        this->resetTime(now, nowIndex, deltaT);

        //isSameAsOld<vector>(word("DU"), "After reset time",
        //word("After_reset_time"));

        /// write parameter is responsible for forcing mesh and fields being
        /// written, ignoring the controlDict write controls
        //if (write)
        if (ifForceWriteAtRemapTime_)
        {
            this->write();

            Info<< "    Mesh and fields written to "
                << runTime_.timeName()
                << "\n";
        }

        //isSameAsOld<vector>(word("DU"), "In advect loop, after force write\n",
        //word("DU_0"));

        Info<<"    Remap done\n    Time reset to "
            << runTime_.timeName()
            <<"\n";
    //}
    //else
    //{
    //    Info<<"    Rezone and Remap is skipped."
    //        << endl;
    //}

    /// Advect oldTime fields
    //AdvectOldFields();

    /// Update field.oldTime() and field.oldTime().oldTime()
    //UpdateOldFields();
}


void Foam::meshDance::resetTime
(
    const scalar now,
    const label nowIndex,
    const scalar deltaT
)
{
    /// Reset deltaT
    const_cast<Time&>(runTime_).setDeltaT(deltaT);

    /// Reset time
    //const_cast<Time&>(runTime_).setTime
    //(
    //    now,
    //    nowIndex
    //);

    /// Reset field time indices
    //resetFieldTimeIndex<scalar>    (nowIndex);
    //resetFieldTimeIndex<vector>    (nowIndex);
    //resetFieldTimeIndex<tensor>    (nowIndex);
    //resetFieldTimeIndex<symmTensor>(nowIndex);
}


//void Foam::meshDance::AdvectOldFields()
//{
//    advectOldFields<scalar>(oldFHT_);
//    advectOldFields<vector>(oldFHT_);
//    advectOldFields<tensor>(oldFHT_);
//    advectOldFields<symmTensor>(oldFHT_);
//
//    advectOldFields<scalar>(oldOldFHT_);
//    advectOldFields<vector>(oldOldFHT_);
//    advectOldFields<tensor>(oldOldFHT_);
//    advectOldFields<symmTensor>(oldOldFHT_);
//}


//void Foam::meshDance::UpdateOldFields()
//{
//    updateOldFields<scalar>();
//    updateOldFields<vector>();
//    updateOldFields<tensor>();
//    updateOldFields<symmTensor>();
//}


bool Foam::meshDance::permissibleToAdvect(const word& fieldName)
{
    std::smatch matches;
    std::regex illegal{R"(ddt0+)"};

    if
    (
        std::regex_search(fieldName, matches, illegal)
    )
    {
        Info<< nl
            << "    "
            << fieldName
            << " is hard-coded to not be advected"
            << '\n';

        return false;
    }

    /// Run-time selected names
    forAll(illegalToAdvect_, fieldI)
    {
        if (fieldName == word(illegalToAdvect_[fieldI]))
        {
            Info<< nl
                << "    "
                << fieldName
                << " was selected to not be advected"
                << '\n';

            return false;
        }
    }

    return true;
}

/// Return true if the fieldName ends with "_0"
bool Foam::meshDance::isOld(const word& fieldName)
{
    std::smatch matches;

    /// String ending with _0
    std::regex illegal{R"(_0$)"};

    if
    (
        std::regex_search(fieldName, matches, illegal)
    )
    {
        Info<< nl
            << "    "
            << fieldName
            << " is and old-time field."
            << '\n';

        return true;
    }

    return false;
}
