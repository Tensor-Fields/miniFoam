/**
 * Find the grad of a field
 *
 * \author: Maalik (ali@tendorfields.com), Dublin, Ireland
 * \date: Jan 07 2025
 */

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
//#include "zeroGradientFvPatchFields.H"
//#include "zeroGradientPointPatchFields.H"
//#include "ZeroGradientPointPatchField.H"
#include "leastSquaresVolPointInterpolation.H"
// For is_same<>:
//#include <type_traits>

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> 
    pointVectorField;
typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

template<class T>
void createGrad(const fvMesh& mesh, const word& fieldName)
{
    //if (std::is_same<T, scalar>::value)
    //if constexpr (std::is_same<T, scalar>::value)
    //{
        GeometricField<T, fvPatchField, volMesh>* pPtr =
            new GeometricField<T, fvPatchField, volMesh>
            (
                mesh.thisDb().lookupObject
                <
                    GeometricField<T, fvPatchField, volMesh>
                >(fieldName)
            );

        //if (typename T::T == typename vector)
        //if constexpr (std::is_same<T, vector>::value)

        //if (typeid(T)==typeid(vector))
        //{
        //    //Type = "vector"

        //    //GeometricField<Type, fvPatchField, volMesh> gradPP =
        //    GeometricField
        //    <
        //        tensor, 
        //        fvPatchField, 
        //        volMesh
        //    > 
        //    //volVectorField
        //    gradPP = fvc::grad(*pPtr);

        //    //(mesh.time())++;

        //    gradPP.write();

        //    Info<< "gradPP from pPtr calculated"
        //        << endl;
        //}
        ///*else*/ if (typeid(T)==typeid(scalar))
        ///*else*/ if (std::is_same<T, scalar>::value)
        //{
        //Type = "vector"

        //GeometricField<Type, fvPatchField, volMesh> gradPP =
        //GeometricField
        //<
        //    //vector, 
        //    typename outerProduct<vector, T>::type,
        //    fvPatchField, 
        //    volMesh
        //> 
        //}
        //volScalarField p = *pPtr;

        //volVectorField
        //gradPP = fvc::grad(*pPtr);
        //gradPP = fvc::grad(p);

        //(mesh.time())++;

        //for (direction cmpt = 0; cmpt < pTraits<Type>::nComponents; ++cmpt)
        //{
        //    /// Component field
        //    GeometricField
        //    <
        //        typename outerProduct
        //        <
        //            vector, 
        //            typename pTraits<Type>::cmptType
        //        >::type,
        //        fvPatchField,
        //        volMesh
        //    >
        //    gradCmpt = fvc::grad(pPtr->component(cmpt));
        //}

        //gradPP.write();

        //Info<< "gradPP from pPtr calculated"
        //    << endl;
    //}
}

int main (int argc, char* argv[])
{
    argList args(argc, argv);

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );

    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    volScalarField p
    (
        IOobject
        (
            "p",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    volVectorField U
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    volTensorField T
    (
        IOobject
        (
            "T",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    //createGrad<scalar>(mesh, "p");
    //createGrad<vector>(mesh, "U");
    //createGrad<tensor>(mesh);

    GeometricField
    <
        typename outerProduct<vector, scalar>::type,
        fvPatchField,
        volMesh
    > gradP = fvc::grad(p);

    GeometricField
    <
        typename outerProduct<vector, vector>::type,
        fvPatchField,
        volMesh
    > gradU = fvc::grad(U);

    GeometricField
    <
        typename outerProduct<vector, tensor>::type,
        fvPatchField,
        volMesh
    > gradT = fvc::grad(T);

    //tmp
    //<
    //    GeometricField
    //    <typename outerProduct<vector, tensor>::type, fvPatchField, volMesh>
    //>
    //GeometricField
    //<
    //    typename outerProduct
    //    <
    //        vector,
    //        typename outerProduct<vector, vector>::type
    //    >::type,
    //    fvPatchField,
    //    volMesh
    //> 
    Tensor<vector> gGradU = fvc::grad(gradU);

    runTime++;

    // Does not work
    //runTime.write();
    //gradP.write();
    //gradU.write();
    //gradT.write();
    //gGradU.write();

    mesh.write();

    Info<< "End"
        << endl;
}
