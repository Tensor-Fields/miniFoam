#include "meshDance.H"
#include "GeometricField.H"
#include "fvPatchField.H"
#include "volMesh.H"

///
/// Constructors
///

Foam::meshDance::meshDance
(
    const Time& runTime, 
    const label maxIter, 
    const List<string>& fixedPatches,
    const bool flipFluxSign
    const label N_iter
)
/// \note: Initializing mesh_
/// with an fvMesh parameter is private to fvMesh.
:
mesh_
(
    IOobject
    (
        polyMesh::defaultRegion,
        runTime,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    )
),
fields_(mesh_, runTime.timeName()),
startingPoints_(mesh_.points()),
startTime_(runTime.startTime()),
timeSpan_(runTime.endTime() - runTime.startTime()),
updatedStartingPoints_(mesh_.points()),
overallPointMotion_(pointField(mesh_.points().size(), vector::zero)),
motionFraction_(0),
sumMotionFraction_(0),
flipFluxSign_(flipFluxSign),
maxIter_(maxIter),
fixedPatches_(fixedPatches)
staticMotionFraction_(N_iter)
{
    //startingPoints_ = mesh_.points(); // Error

    /// Create IOobjects from existing field files
    //IOobjectList fields(mesh_, runTime.timeName());

    Info<< "Read Time = "
        << mesh_.time().timeName()
        << " and register fields"
        << endl;

    registerFields<scalar>();
    registerFields<vector>();
    registerFields<tensor>();
    registerFields<symmTensor>();
    registerFields<sphericalTensor>();

    //registerFields<scalar, fvsPatchField, surfaceMesh>();
    //registerFields<vector, fvsPatchField, surfaceMesh>();
    //registerFields<tensor, fvsPatchField, surfaceMesh>();
    //registerFields<symmTensor, fvsPatchField, surfaceMesh>();
    //registerFields<sphericalTensor, fvsPatchField, surfaceMesh>();

    /// Create grad(DD), needed when copying convected DD_tmp into DD
    //if 
    //(
    //    !mesh_
    //    .foundObject<GeometricField<tensor, fvPatchField, volMesh> >
    //    ("grad(DD)")
    //)
    //{
    //    Info<< "    Create grad(DD)"
    //        << endl;

    //    GeometricField<tensor, fvPatchField, volMesh>* dummyName = new
    //    GeometricField<tensor, fvPatchField, volMesh>
    //    (
    //        //IOobject
    //        //(
    //        //    "grad(DD)",
    //        //    mesh_.time().timeName(),
    //        //    mesh_,
    //        //    IOobject::NO_READ,
    //        //    IOobject::AUTO_WRITE
    //        //),
    //        fvc::grad
    //        (
    //            mesh_
    //            .thisDb()
    //            .lookupObject<GeometricField<vector, fvPatchField, volMesh> >("DD")
    //        )()
    //    );

    //    //GeometricField<tensor, fvPatchField, volMesh> gradDD
    //    //= fvc::grad
    //    //(
    //    //    mesh_
    //    //    .thisDb()
    //    //    .lookupObject<GeometricField<vector, fvPatchField, volMesh> >("DD"),
    //    //    "grad(DD)"
    //    //);
    //}
    //else
    //{
    //    Info<< "grad(DD) already exists"
    //        << endl;
    //}

    //Info<< "mesh_.toc() is "
    //    << mesh_.toc()
    //    << endl;
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
    if (!staticMotionFraction_)
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
        motionFraction_ = staticMotionFraction_;
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
