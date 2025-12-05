#include "meshDance.H"
#include "GeometricField.H"
#include "fvPatchField.H"
#include "volMesh.H"

// smooth()
#include "boolList.H"
#include "emptyPolyPatch.H"
#include "boundBox.H"
// degToRad
#include "unitConversion.H"

///
/// Constructors
///

//Foam::meshDance::meshDance
//(
//    const Time& runTime, 
//    const label maxIter, 
//    const List<string>& fixedPatches,
//    const bool flipFluxSign,
//    const label N_iter,
//    const List<string>& illegalToAdvect,
//)

//Foam::meshDance::meshDance
//(
//    const Time& runTime, 
//    const label maxIter, 
//    const List<string>& fixedPatches,
//    const IOdictionary& ALEDict,
//    const bool flipFluxSign,
//    const label N_iter,
//    const List<string>& illegalToAdvect,
//    const label rezoneFreq
//)

Foam::meshDance::meshDance
(
    const Time& runTime,
    const bool regFields
)
/// \note: Initializing mesh_
/// with an fvMesh parameter is private to fvMesh.
:
//mesh_
//(
//    IOobject
//    (
//        polyMesh::defaultRegion,
//        runTime,
//        IOobject::MUST_READ,
//        IOobject::NO_WRITE
//    )
//),
runTime_(runTime),
nowName_(runTime.timeName()),
registerFields_(regFields),
//pseudoTime_(),
dMeshPtr_
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
),
/// mesh_ remained from the previous design; ideally we need dMeshPtr_ only.
mesh_(dMeshPtr_()),
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
            dMeshPtr_(),
            IOobject::MUST_READ
        )
    )
),
/// Remap params
flipFluxSign_(),
nRemapSteps_(),
illegalToAdvect_(),
maxIter_(),
fixedPatches_(),
ifForceWriteAtRemapTime_(),
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
/// Laplacian smoother params
laplacianMeshSmoother_(),
//smoothPatches_(),
patchWithSlidingPoints_(),
crossingPatches_(),
slideCrossingPoints_(),
maxSmoothingIter_(),
maxSmoothingResidual_(),
wedge_(),
wedgeDir_(),
radialDir_(),
wedgeAperture_(),
emptyDir_(),
patchesWithFreePoints_(),
fixPointsInBoundBoxAnyway_(),
boundBoxMin_(),
boundBoxMax_()
{
    /// fvMesh reference ... Private!
    //mesh_ = dMeshPtr_();

    /// this->read<T> reads dictionary, not IOdict
    const dictionary& aleDict = ALEDict_;

    /// Read remap settings
    read<label>        (aleDict, word("nRemapSteps"), nRemapSteps_);

    read<bool>(aleDict, word("remapWrite"), writeRemappedSteps_);

    ifRead<label>
    (writeRemappedSteps_,
        aleDict, word("remapWriteFrequency"), remapWriteFreq_);

    /// Reciprocal of motionFraction_
    rMotionFraction_ = nRemapSteps_;

    read<label>        (aleDict, word("remapLoopPerStep"), maxIter_);

    read<List<string> > (aleDict, word("fixedPatches"), fixedPatches_);

    read<List<string> > (aleDict, word("illegalToAdvect"), illegalToAdvect_);

    read<bool>        (aleDict, word("flipFluxSign"), flipFluxSign_);

	read<bool>
	    (aleDict, word("forceWriteAtRemapTime"), ifForceWriteAtRemapTime_);
    /// How many timeSteps per one rezone
    read<label>        (aleDict, word("rezoneFrequency"), rezoneFreq_);

    /// Mandatory entry, can be false though, if you do not want to use the
    /// built-in smoother
    read<bool>(aleDict, word("laplacianMeshSmoother"), laplacianMeshSmoother_);

    if (laplacianMeshSmoother_)
    {
        const dictionary& smootherDict = ALEDict_.subDict("laplacianMeshSmootherDict");

        //read<bool>(smootherDict, word("smoothPatches"), smoothPatches_);
        read<word>(smootherDict, word("patchWithSlidingPoints"), patchWithSlidingPoints_);
        read<List<string> > (smootherDict, word("crossingPatches"), crossingPatches_);
        //read<bool>         (smootherDict, word("fixCrossingPoints"), fixCrossingPoints_);
        read<bool>    (smootherDict, word("slideCrossingPoints"), slideCrossingPoints_);
        read<label>   (smootherDict, word("maxRezoneIter"), maxSmoothingIter_);
        read<scalar>  (smootherDict, word("maxRezoneRes"), maxSmoothingResidual_);
        read<bool>    (smootherDict, word("wedge"), wedge_);

        ifRead<label> (wedge_, smootherDict, word("wedgeDir"), wedgeDir_);
        ifRead<label> (wedge_, smootherDict, word("radialDir"), radialDir_);
        ifRead<scalar>(wedge_, smootherDict, word("wedgeAperture"), wedgeAperture_);
        ifRead<label> (!wedge_, smootherDict, word("emptyDir"), emptyDir_);
        read<List<string> >(smootherDict, word("freePatches"), patchesWithFreePoints_);

        read<bool>(smootherDict, word("fixPointsInBoundBoxAnyway"), fixPointsInBoundBoxAnyway_);
        ifRead<point>(fixPointsInBoundBoxAnyway_, smootherDict, word("bbMin"), boundBoxMin_);
        ifRead<point>(fixPointsInBoundBoxAnyway_, smootherDict, word("bbMax"), boundBoxMax_);
    }

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
    const_cast<pointField&>(overallPointMotion_) =  (finish - start);
}

/// Time-step adaptive point increment
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
            mesh_.movePoints
            (
                /// New points
                mesh_.points() + motionFraction_ * overallPointMotion_
            );
        }

	return false;
    }
}

void Foam::meshDance::hitFinalMesh()
{
    /// Move the mesh, so that the overall motion
    /// hits
    mesh_.movePoints(overallPointMotion_ + updatedStartingPoints_);
}

/// Update motion fraction
void Foam::meshDance::updateMotionFraction()
{
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
    if (badMesh())
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

bool Foam::meshDance::badMesh()
{
    /// I check mesh quality.... later!
    return false;
}

/// Laplacian smoother
Foam::pointField Foam::meshDance::smooth()
{
    Info<< "    Lapalaian smoother starts.\n";

    /// Mesh reference
    /*dynamicF*/fvMesh& mesh = dMeshPtr_();

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
                }
            }
        }
    }

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

    for (label smoothI = 0; smoothI < maxSmoothingIter_; ++smoothI)
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

    return newPoints;

    Info<< "    Mesh smoothing done\n" << endl;
}

/// Obtains the rezoned state; mesh motion actually happens in
/// remap()
void Foam::meshDance::rezone(const pointField& targetPoints)
{
    this -> setOverallPointMotion(dMeshPtr_().points(), targetPoints);
}

void Foam::meshDance::remap(const bool write)
{
    const scalar now = runTime_.value();
    const label nowIndex = runTime_.timeIndex();

	/// To be accessed inside advect()
	nowName_ = runTime_.timeName();

    for (label iter = 0; iter < nRemapSteps_; ++iter)
    {
        Info<< "Remap step = "
            <<iter<<"\n";

        /// Rezone overshoot-proof
        /// Read fields from the database and advect
        if(this -> stepTowards())
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

            break;
        }

        /// Volume fields
        this -> advect<scalar>    (writeRemappedSteps_);
        this -> advect<vector>    (writeRemappedSteps_);
        this -> advect<tensor>    (writeRemappedSteps_);
        this -> advect<symmTensor>(writeRemappedSteps_);
        //this -> advect<sphericalTensor>(write);

        /// Move ahead; needs to be after advect()
        const_cast<Time&>(/*pseudo*/runTime_)++;

        /// Write -- for debugging

		/// Previous design: Write remap steps in physical time dirs.
		/// New design: Write remap steps "inside" the remap physical time

        /// Counting starts from 0, so iter + 1
        //if (writeRemappedSteps_ && ((iter+1) % remapWriteFreq_ < SMALL))
        //{
        //    this->write();
        //    Info<< "    Remapped field and rezoned mesh written to "
        //        << runTime_.timeName()
        //        <<"\n";
        //}
    }

    /// Restore time; physical time remains unchaged during remap
    //label nowIndex = runTime_.findClosestTimeIndex
    //    (
    //        runTime_.findTimes(runTime_.path()),
    //        now
    //    );

    const_cast<Time&>(runTime_).setTime
    (
        now,
        nowIndex
        //0
        //1
    );

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

    Info<<"    Remap done\n    Time reset to "
        << runTime_.timeName()
        <<"\n";
}
