#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "GeometricField.H"
#include "pointPatchField.H"
#include "scalar.H"
#include "pointMesh.H"
#include "fvMatrices.H"

using namespace Foam;

typedef GeometricField<scalar, pointPatchField, pointMesh> pointScalarField;

int main(int argc, char* argv[])
{
    argList args(argc, argv);

    Time runTime("controlDict", args.rootPath(), args.caseName());

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

    pointScalarField fixedPoints
    (
        IOobject
        (
            "fixed",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        pointMesh(mesh),
        /// 0 for non-fixed points (aka free points)
        dimensionedScalar("zero", dimless, 0)
    );

    //argList::validArgs.append("patchName");

    //const word& patchName = args.additionalArgs()[1];

    const word& patchName = "billetRight";
   
    const polyBoundaryMesh& bmesh = mesh.boundaryMesh();

    //forAll(bmesh, patchI)
    //{
        const label& patchI = bmesh.findPatchID(patchName);

        Info<< "Patch "
            << patchName
            << " found"
            << endl;

        const List<label>& isFixed = bmesh[patchI].meshPoints();

        forAll(isFixed, pointI)
        {
            /// 1 for fixed (boundary) points
            fixedPoints[isFixed[pointI]] = 1;
        }
    //}

    Info<< "fixed field initialized"
        << endl;

    forAll(fixedPoints, pointI)
    {
        Info<< fixedPoints[pointI]
            << endl;
    }

    runTime++;

    runTime.write();

    Info<< "Data written to "
        << runTime.timeName()
        << endl;
}
