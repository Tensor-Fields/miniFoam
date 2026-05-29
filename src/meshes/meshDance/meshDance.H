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
#include "polyMeshGen.H"

namespace Foam
{
class meshDance//: public fvMesh
{
    /// Now
    word nowName_ = "";

    /// Private
    const Time& runTime_;

    const bool registerFields_;

    ///*const*/ Time pseudoTime_;

    autoPtr<dynamicFvMesh> dMeshAutoPtr_;
    dynamicFvMesh* dMeshPtr_;

    //fvMesh& mesh_;

    dynamicFvMesh& mesh_;

    const IOobjectList fields_;

    /// ALE dictionary
    const IOdictionary ALEDict_;
    const dictionary remapDict_;
    const dictionary rezoneDict_;

    /// Dimensions
    bool wedge_;
    bool planar2D_;

    const word meshTargetDir_ = "meshTarget";

    /// Read remap settings
        bool flipFluxSign_;
        label nRemapSteps_;

        /// Run-time selected field names to not be advected
        List<string> illegalToAdvect_;

        label maxIter_;

        /// To pass to remap()
        bool ifForceWriteAtRemapTime_ = false;

    /// General rezone settings
    const pointField/*&*/ startingPoints_;

    const dimensionedScalar startTime_;

    const dimensionedScalar timeSpan_;

    const pointField/*&*/ updatedStartingPoints_;

    const pointField overallPointMotion_;

    /// Updates every time step (in the changing deltaT scenario)
    scalar motionFraction_;

    /// Updates every time motionFraction_ is updated
    scalar sumMotionFraction_;

    /// Whether the mesh velocity is calculated based on the controlDict or
    /// based on the forced N_iter's; 0 means based on controlDict.
    label rMotionFraction_;

    List<string> fixedPatches_;

    /// Rezone frequency
    /// Set it to a large number by default, meaning that never rezone!
    label rezoneFreq_;

    /// Must be initialized as used by advect()
    bool writeRemappedSteps_ = false;
    label remapWriteFreq_;

    /// Rezone only if the swept volume of at least one cell meets this fraction
    /// of the minimum-volume cell's volume
    scalar sweptVolThreshold_;
    scalar aboveThresholdFraction_;
    label  minCellCount_;

    bool finalMeshIsHit_;

    /// Smoother settings

        /// General smoothing params
        label iterPerQualityCheck_;
        scalar qualityThreshold_;
        word constrainedCellsSet_;

        /// Built-in Laplace smoother settings
        bool laplacianMeshSmoother_;
        bool cfMeshSmoother_;
        word patchWithSlidingPoints_;
        List<string> crossingPatches_;
        bool slideCrossingPoints_;
        label maxSmoothingIter_;
        scalar maxSmoothingResidual_;
        label wedgeDir_;
        label radialDir_;
        scalar wedgeAperture_;
        label emptyDir_;
        List<string> patchesWithFreePoints_;
        bool fixPointsInBoundBoxAnyway_;
        point boundBoxMin_;
        point boundBoxMax_;

        /// cfMesh Laplace smoother settings
        bool untangle_;
        //List<string> lockedPatchNames_;
        List<string> wedgePatchNames_;

        /// 3-D smoothing params
        label nLoops_;
        label nSurfaceIterations_;

        /// Mesh quality parameters
        scalar skewThreshold_;
        scalar nonOrthThreshold_;
        scalar aspectThreshold_;
        
        /// Good mesh quality parameters
        scalar goodSkewThreshold_;
        scalar goodNonOrthThreshold_;
        scalar goodAspectThreshold_;


        /// HashTable of fields
        //template<class Type>
        //HashTable<GeometricField<Type, fvPatchField, volMesh>* >    tof_;

        /// HashTable of old fields
        // template<class Type>
        // HashTable<GeometricField<Type, fvPatchField, volMesh>* >    oldFHT_;

        // /// HashTable of old-old fields
        // template<class Type>
        // HashTable<GeometricField<Type, fvPatchField, volMesh>* >    oldOldFHT_;

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

    bool hitFinalMesh();

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
        
        /// Destructor
        ~meshDance();

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
            const dynamicFvMesh& meshR = dMeshAutoPtr_();

            return meshR;
        }

        /// Non-const access the underlying dynamicFvMesh
        /// Non-const to align with the desing of the
        /// bekaert solver.
        dynamicFvMesh& dMesh()
        {
            //dynamicFvMesh& meshR = dMeshAutoPtr_();

            //return meshR;
            return *dMeshPtr_;
        }

        /// Return pointer to mesh
        inline dynamicFvMesh* dMeshPtr()
        {
            return dMeshPtr_;
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
        bool isOld(const word& fieldName);

        /// Rezone or not
        bool ifRezone(const label);

        /// Is rezone counter hit
        bool rezoneNow(const label);

        void defineBadMesh();

        /// Is mesh quality bad?
        bool badMesh
        (
            //const polyMesh&,
            //const scalar aspectThreshold,
            //const scalar skewThreshold,
            //const scalar nonOrthThreshold
        );

        void defineGoodMesh();

        bool goodMesh
        (
            //const polyMesh&,
            //const scalar aspectThreshold,
            //const scalar skewThreshold,
            //const scalar nonOrthThreshold
        );

        bool meshFluxMeetsThreshold();

        /// Laplacian mesh smoother
        /*const*/ pointField/*&*/ LaplaceSmooth(const label iter);
        pointField cfMeshSmooth(const pointField&);
        pointField smooth();

        void correctAxisPoints
        (
            polyMeshGen& pmg/*, mesh*/,
            const List<string>& wedgePatchNames
        );
 
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

        void resetTime
        (
            const scalar now,
            const label nowIndex,
            const scalar deltaT
        );


        template<class T>
        void resetFieldTimeIndex(const label nowIndex);


        template<class T>
        bool isSameAsOld
        (
            const word fieldName,
            const string text,
            const word oldFieldName,
            const bool writeOldField = true
        );

        //template<class Type>
        //void saveOldTimes();

        //template<class Type>
        //void saveOldOldTimes();

        //template<class Type>
        //void advectOldFields
        //(
        //    HashTable<GeometricField<Type, fvPatchField, volMesh>* >
        //);

        //void AdvectOldFields();

        //template<class Type>
        //void updateOldFields();

        //void UpdateOldFields();

    };
}

#include "meshDanceTemplates.C"

#endif
