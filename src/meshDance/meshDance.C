#include "meshDance.H"
#include "GeometricField.H"
#include "fvPatchField.H"
#include "volMesh.H"

///
/// Constructors
///

Foam::meshDance::meshDance(const Time& runTime) 
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
updatedStartingPoints_(mesh_.points()),
overallPointMotion_(pointField(mesh_.points().size(), vector::zero)),
motionFraction_(0),
sumMotionFraction_(0)
{
    //startingPoints_ = mesh_.points(); // Error

    /// Create IOobjects from existing field files
    //IOobjectList fields(mesh_, runTime.timeName());

    registerFields<scalar>();
    registerFields<vector>();
    registerFields<tensor>();
    registerFields<symmTensor>();
    registerFields<sphericalTensor>();
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
void Foam::meshDance::stepTowards()
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
    if (motionFraction_/*()*/ > 1)
    {
        FatalErrorIn("stepTowards()\n")
            << "Motion overshoots, so not applied\n"
            << "Seems like deltaT > (endTime - startTime)"
            << abort(FatalError);
    }
    else
    {
        if (sumMotionFraction_/*()*/ > 1)
        {
            WarningIn("stepTowards()\n")
                << "This incremental motion overshoots;\n"
                << "Mesh moves to the final points, ignoring the overshoot."
                << endl;

            hitFinalMesh();
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
    }
}

void Foam::meshDance::hitFinalMesh()
{
    /// Move the much that the final mesh implied by overall motion hits
    mesh_.movePoints(overallPointMotion_ - mesh_.points());
}

/// Update motion fraction
void Foam::meshDance::updateMotionFraction()
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

    setStartTime(runTime.endTime());
    resetStartingPoints(points());
    resetSumMotionFraction();

    /// Re-set endTime
    const dimensionedScalar timeSpan = runTime.endTime() - runTime.startTime();

    runTime.setEndTime(runTime.endTime() + timeSpan);
}
