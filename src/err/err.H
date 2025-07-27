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
        const argList& args_;

        autoPtr<GeometricField<T, fvPatchField, volMesh> > fieldExactPtr_;

        autoPtr<GeometricField<T, fvPatchField, volMesh> > errPtr_; 

        const fvMesh& meshSource_;
        const fvMesh& meshTarget_;

        scalar minErr_;
        scalar maxErr_;
        scalar avgErr_;
        autoPtr<scalar> L1ErrPtr_;

        /// Private member functions
        scalar int_over_whole_mesh
        (
            GeometricField<T, fvPatchField, volMesh> integrandField
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
            const argList& args,
            const fvMesh& meshSource,
            const fvMesh& meshTarget
        );

        /// Members
        void calcErr();

        void calcErr_wrt_same_mesh(const word& sourceCase);

        void report();

        void updateMeshTarget();
};
}

#include "errTemplates.C"

#endif
