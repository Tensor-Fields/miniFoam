/**
  * A class for incrementally moving meshes, given the final mesh,
  * optionally cyclic mesh motion. Specifically useful for advective remap
  * of a discrete rezoning ALE procedure, DALE.
  *
  * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dubiln, IE
  * \date: May 2025
  */

#ifndef meshDance_H
#define meshDance_H

#include "pointField.H"
#include "word.H"
#include "fvMesh.H"
#include "IOobjectList.H"

namespace Foam
{
class meshDance//: public fvMesh
{
    /// Private
    fvMesh mesh_;

    const IOobjectList fields_;

    const pointField/*&*/ startingPoints_;

    const dimensionedScalar startTime_;

    const dimensionedScalar timeSpan_;

    const pointField/*&*/ updatedStartingPoints_;

    const pointField overallPointMotion_;

    /// Updates every time step (in the changing deltaT scenario)
    scalar motionFraction_;

    /// Updates every time motionFraction_ is updated
    scalar sumMotionFraction_;

    /*static*/ const word meshTargetDir_ = "meshTarget";

    /// Register fields_ of type Type with mesh_
    template<class Type>
    void registerFields();

    void updateMotionFraction();

    void hitFinalMesh();

    /// Reset start time
    void setStartTime(const dimensionedScalar& st)
    {
        const_cast<dimensionedScalar&>(startTime_) = st;
    }

    /// Reset starting points
    void resetStartingPoints(const pointField&);

    /// Reset sumMotionFraction_
    inline void resetSumMotionFraction()
    {
        sumMotionFraction_ = 0;
    }

    /// Public
    public:

        /// Constructor
        meshDance(const Time& runTime);

        /// Members

        void setOverallPointMotion
        (
            const pointField& start,
            const pointField& finish
            // Error:
            //const pointField& start = (this -> updatedStartingPoints())
            //const pointField& start = updatedStartingPoints_
        );

        /// Incrementally change the mesh towards the final points
        bool stepTowards
        (/*const pointField& finalPoints, const dimensionedScalar& startTime*/);

        /// Move the mesh back to its initial config, incrementally
        void cyclicStep
        (
            //const pointField& penultimatePoints, 
            //const dimensionedScalar& startTime
        );

        /// Return mesh points
        const pointField& points() const
        {
            return mesh_.points();
        }

        /// Access the underlying fvMesh
        const fvMesh& mesh()
        {
            const fvMesh& meshR = mesh_;

            return meshR;
        }

        /// Advect fields, driven by the most recent mesh motion
        template<class Type>
        void advect();

        /// Write
        void write();
        
        /// Reset points to their initial positions
        void resetPoints();
        
        /// Return updatedStartingPoints_
        const pointField& updatedStartingPoints()
        {
            return updatedStartingPoints_;
        }

        /// Re-set mesh motion settings when a cycle is finished.
        /// - startTime
        /// - starting points
        /// - sumMotionFraction
	/// \note: It is the driver's responsibility to use it when needed.
	/// This class is not aware of when the cycle finishes as of now.
	void resetMotion();
};
}

#include "meshDanceTemplates.C"

#endif
