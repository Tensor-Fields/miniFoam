/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  Tensor         | miniFOAM: The Computational Continuum Playground
   ~~~~~~~   Fields         | Website:  https://tensorfields.com
    O   O                   | Copyright (C) 2025 Tensorfields
      O                     |
-------------------------------------------------------------------------------
License
    This file is part of miniFoam.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Application
    waveFoam

Description
    Solves the wave equation.

Author
    Maalik (ali@tensorfields.com), Blue Room, UCD, Dublin, Ireland

Date
    Mar 07 2025

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "Time.H"
#include "fvMesh.H"
#include "GeometricField.H"
#include "volMesh.H"
#include "fvPatchField.H"
#include "fvMatrix.H"
#include "fvm.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    argList args(argc, argv);

    // Create Time
    Time runTime("controlDict", args);

    // Create Mesh
    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    // Create Field
    GeometricField<scalar, fvPatchField, volMesh> u
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

    // Sound speed
    //- Option 1: Read from dictionary
    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    dimensionedScalar c2 = transportProperties.lookup("c2");

    //- Option 2: Hard-code
    //dimensionedScalar c2("c2", dimLength ** 2 / dimTime ** 2, 1.0);

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    while(runTime.loop())
    {
        fvMatrix<scalar> uEq
        (
            fvm::d2dt2(u) == fvm::laplacian(c2, u)
        );

        uEq.relax();

        uEq.solve();

        runTime.write();
    }

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
