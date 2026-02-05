/**
 * Skeleton of an OpenFOAM app
 *
 * Includes time, mesh, and a dummy field
 *
 * \author: Maalik (Ali Shayegh, ali@tensorfields.com)
 * \date: 04-11-2025
 */

#include "argList.H"
#include "foamTime.H"

#include "fvMesh.H"
#include "volMesh.H"
#include "surfaceMesh.H"

#include "GeometricField.H"
#include "vector.H"
#include "fvPatchField.H"
#include "fvsPatchField.H"

// Important
#include "surfaceFields.H"
// end Important

#include "fvc.H"
#include "fvm.H"
#include "fvMatrices.H"

#include "snGradScheme.H"

// Set precision
#include "IOmanip.H"

using namespace Foam;

int main(int argc, char* argv[])
{
    argList::validOptions.insert("gamma","<scalar>");

    argList args(argc, argv);

    Time runTime("controlDict", args.rootPath(), args.caseName());

    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime,
            IOobject::MUST_READ
        )
    );

    GeometricField<scalar, fvPatchField, volMesh>
    scalar_field
    (
        IOobject
        (
            "scalar_field",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    return 0;
}
