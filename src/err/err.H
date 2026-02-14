#ifndef err_H
#define err_H

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "IOobject.H"
#include "pointMesh.H"
#include "GeometricField.H"
#include "vector.H"
#include "pointPatchField.H"
#include "pointMesh.H"
#include "dimensionedScalar.H"
#include "dimensionedVector.H"
#include "fvMatrices.H"
#include "fvm.H"
#include "volPointInterpolation.H"
#include "pointVolInterpolation.H"
#include "leastSquaresVolPointInterpolation.H"
#include "leastSquaresGrad.H"
#include "IOobjectList.H"

namespace Foam
{
/// Error class, handles error calculation and report
template<class T>
class err
{
    /// Private
        
        /// Private data
        //const argList& args_;

        autoPtr<GeometricField<T, fvPatchField, volMesh> > fieldExactPtr_;

        autoPtr<GeometricField<T, fvPatchField, volMesh> > errPtr_; 

        autoPtr<Field<T> > relErrPtr_; 

        const fvMesh& meshSource_;
        const fvMesh& meshTarget_;

        scalar minErr_;
        scalar maxErr_;
        scalar maxRelErr_;
        scalar avgErr_;
        autoPtr<scalar> L1ErrPtr_;

        /// The default is error calculation at target mesh cell centers
        bool atSourceCellCenters_;

        /// True by default
        bool bounded_;

        /// Private member functions
        scalar int_over_whole_mesh
        (
            const GeometricField<T, fvPatchField, volMesh>& integrandField
        );

        GeometricField<T, fvPatchField, volMesh>& fieldExact
        (const GeometricField<T, fvPatchField, volMesh>&);

        void errStats();

        scalar L1Err();

    /// Public
    public:

        /// Constructors
        err
        (
            //const argList& args,
            const fvMesh& meshSource,
            const fvMesh& meshTarget,
            bool atSourceCellCenters = false,
            bool bounded = true
        );

        /// Members
        void calcErr(bool writeErrorField = false);

        void calcErr_wrt_same_mesh
        (
            //const word& sourceCase,
            //const fileName& sourceTime
            const bool& writeErrField
        );

        void report();

        void updateMeshTarget();

        GeometricField<T, fvPatchField, volMesh>
        errField();
};
}

#include "errTemplates.C"

#endif
