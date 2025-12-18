/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  Tensor         | miniFOAM: The Computational Continuum Playground
   ~~~~~~~   Fields         | Website:  https://tensorfields.com
    O   O                   | Copyright (C) 2025 Tensorfields
      O                     |
-------------------------------------------------------------------------------

Application
    waveFoam

Description
    Minimal wave equation solver.

Author
    Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin

Date
    Dec 15 2025
\*---------------------------------------------------------------------------*/
#include"argList.H"
#include"Time.H"
#include"IOobject.H"
#include"polyMesh.H"
#include"fvMatrix.H"
#include"fvm.H"

using namespace Foam;

//int main()
int main(int argc, char* argv[])
{
    argList args(argc, argv);

    Time runTime("controlDict", args);

    polyMesh mesh0
    (
        IOobject
        (
            "region0",
            runTime.name(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    fvMesh mesh(mesh0);

    typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;
    //GeometricField<scalar, fvPatchField, volMesh> u
    volScalarField u
    (
        IOobject
        (
            "u",
            runTime.name(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    IOdictionary waveProperties
    (
        IOobject
        (
            "waveProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ
            //IOobject::AUTO_WRITE
        )
    );

    dimensionedScalar c2
    (
        "c2",
        pow(dimLength, 2)/pow(dimTime,2),
        waveProperties.lookup<scalar>("c2")
    );

    while(runTime.loop())
    {
        fvMatrix<scalar> uEq
        (
            fvm::d2dt2(u) == fvm::laplacian(c2, u)
        );

        uEq.solve();

        if (runTime.write())
        {
            mesh.write();
        }

        //runTime++;
    }
}
