#include "argList.H"
#include "label.H"
#include "List.H"
#include "foamTime.H"
#include "IOobject.H"
#include "polyMesh.H"
#include "fvMesh.H"
#include "polyPatch.H"
#include "polyBoundaryMesh.H"

#include "scalar.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "GeometricFields.H"
//#include "DimensionedField.H"

//#include "surfaceMesh.H"
//#include "fvsPatchField.H"
#include "fvMatrices.H"
#include "fvc.H"
#include "Random.H"

//#include "fvCFD.H"

using namespace Foam;

//typedef GeometricField<scalar,fvPatchField,volMesh> volScalarField;
typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;
//typedef GeometricField<scalar, fvPatchField, surfaceMesh> surfaceScalarField;

int main(int argc, char* argv[])
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
            IOobject::MUST_READ
        )
    );

    fvMesh mesh_2
    (
        IOobject
        (
            "mesh_2",
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );

    const volScalarField p
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
}
