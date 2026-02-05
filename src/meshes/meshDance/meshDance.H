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
#include "dynamicFvMesh.H"
#include "IOobjectList.H"
#include <regex>

namespace Foam
{
class meshDance//: public fvMesh
{
    /// Private
    const Time& runTime_;

    const bool registerFields_;

    ///*const*/ Time pseudoTime_;

    autoPtr<dynamicFvMesh> dMeshPtr_;

    //fvMesh& mesh_;

    dynamicFvMesh& mesh_;

    const IOobjectList fields_;

    const pointField/*&*/ startingPoints_;

    const dimensionedScalar startTime_;

    const dimensionedScalar timeSpan_;

    const pointField/*&*/ updatedStartingPoints_;

    const pointField overallPointMotion_;

    /// Updates every time step (in the changing deltaT scenario)
    scalar motionFraction_;

    /// Whether the mesh velocity is calculated based on the controlDict or
    /// based on the forced N_iter's; 0 means based on controlDict.
    label rMotionFraction_;

    /// Updates every time motionFraction_ is updated
    scalar sumMotionFraction_;

    /*static*/ const word meshTargetDir_ = "meshTarget";

    /*const*/ bool flipFluxSign_;

    /*const*/ label maxIter_;

    /*const*/ List<string> fixedPatches_;

	/// To pass to remap()
	/*const*/ bool ifForceWriteAtRemapTime_ = false;

    /// Run-time selected field names to not be advected
    List<string> illegalToAdvect_;

    /// Rezone frequency
    /// Set it to a large number by default, meaning that never rezone!
    /*const*/ label rezoneFreq_;

    /// ALE dictionary
    const IOdictionary ALEDict_;

    /// Read remap settings
    label nRemapSteps_;

	/// Must be initialized as used by advect()
    bool writeRemappedSteps_ = false;

    label remapWriteFreq_;

    /// Laplacian smoother settings
    bool laplacianMeshSmoother_;

	/// Now
	word nowName_ = "";

    //bool smoothPatches_;
    word patchWithSlidingPoints_;
    List<string> crossingPatches_;
    bool slideCrossingPoints_;
    label maxSmoothingIter_;
    scalar maxSmoothingResidual_;
    bool wedge_;
    label wedgeDir_;
    label radialDir_;
    scalar wedgeAperture_;
    label emptyDir_;
    List<string> patchesWithFreePoints_;
    bool fixPointsInBoundBoxAnyway_;
    point boundBoxMin_;
    point boundBoxMax_;

    /// Private members
 
    /// Register fields_ of type Type with mesh_.
    /// If reading from the disk directly whenever we need fields (e.g., for
    /// solving the advection Eq.), we are uncesserily interacting with the disk,
    /// which slows the process down.
    template<class Type>
    void registerFields();

    template<class Type, template<class> class PatchType, class Mesh>
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

    scalar fluxSign();

    /// Public
    public:

        /// Constructor
        meshDance
        (
            const Time&,
            const bool registerFields = false
            /// const label,
            /// const List<string>&,
            /// const IOdictionary& ALEDict,
            /// const bool flipFluxSign = false,
            /// const label N_iter = 0,
            /// /// non-const does not allow this initialization
            /// const List<string>& illegalToAdvect = List<string>(0),
            /// const label rezoneFreq_ = label(GREAT)
        );

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
        //const fvMesh& mesh()
        const dynamicFvMesh& mesh()
        {
            const dynamicFvMesh& meshR = dMesh();

            return meshR;
        }

        /// Non-const access the underlying dynamicFvMesh
        /// Non-const to align with the desing of the
        /// bekaert solver.
        dynamicFvMesh& dMesh()
        {
            dynamicFvMesh& meshR = dMeshPtr_();

            return meshR;
        }

        /// Advect fields, driven by the most recent mesh motion
        template<class Type>
        void advect(bool writeAdvectedFields = false);

        /// Advect surfaceFields
        template<class Type>
        void advects(bool writeAdvectedFields);

        /// Write
        void write();

        void writeMesh();
        
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

        /// Check if this field is allowed to be advected
        bool permissibleToAdvect(const word& fieldName);

        /// Rezone or not
        bool ifRezone(const label);

        /// Is rezone counter hit
        bool rezoneNow(const label);

        /// Is mesh quality bad?
        bool badMesh();

        /// Laplacian mesh smoother
        pointField smooth();
        
        /// Read smoothing dictionary
        template<class T>
        void read
        (
            const dictionary& dict,
            const word key,
            T& value
        );

        template<class T>
        void ifRead
        (
            const bool condition,
            const dictionary& dict,
            const word key,
            T& value
        );

        /// Understand the rezone
        void rezone(const pointField& targetPoints);

        /// Do the remap. Includes the pseudo-time loop
        /// Pass write decision to advect()
        void remap(const bool writeAdvectedFields = false);
    };
}

#include "meshDanceTemplates.C"

#endif
