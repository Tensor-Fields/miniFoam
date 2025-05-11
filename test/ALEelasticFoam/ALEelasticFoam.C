/**
 * ALE formulation of the momentum equation
 *
 * This is an implementation of the integral form of the momentum equation for
 * an arbitrary region, so not necessarily a material region.
 *
 * v: Material velocity
 * w: Region's boundary velocity
 *
 * \author: Maalik, Sky, on my way to Dubai, ali@tensorfields.com
 * \date: Mar 16 2025
 */

/// Headers
#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "GeometricField.H"
#include "dimensionedScalar.H"
#include "IOdictionary.H"

/// Crucial, though non-trivial line
#include "fvMatrices.H"

/// Headers, continued
#include "linear.H"
#include "solutionControl.H"
//#include "simpleControl.H"
#include "fvm.H"
#include "fvc.H"
#include "volPointInterpolation.H"

using namespace Foam;

typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;

typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

typedef GeometricField<scalar, fvsPatchField, surfaceMesh> surfaceScalarField;

///
/// updateMesh()
/// Move mesh points according to some arbitrary rule
///
void update
(
    /// Mesh points will move, so non-const
    fvMesh& mesh, 
    const volVectorField v, 
    volVectorField& w
)
{
    //const pointField& oldPoints = mesh.points();

    /// Stick boundary faces to the material (aka Lagrangian description)

    forAll(mesh.boundaryMesh(), patchI)
    {
        //const polyPatch& patch  = mesh.boundaryMesh()[patchI];

        w.boundaryField()[patchI] 
            = v.boundaryField()[patchI];

        //const labelList& patchFaces
        //(
        //    (mesh.boundaryMesh()[patchI]).meshFaces()
        //);

        //forAll(patchFaces, faceI)
        //{
        //    w.boundaryField()[patchFaces[faceI]] 
        //        = v.boundaryField()[patchFaces[faceI]];
        //}
    }

    /// Slide boundary points
    // ...

    /// Move the internal points consistently wrt the boundary
    forAll(w, cellI)
    {
        w[cellI] = v[cellI];
    }

    /// Consistently
    // ...

    const volPointInterpolation volPointInterpolator(mesh);

    pointField newPoints = mesh.points();

    const GeometricField<vector, pointPatchField, pointMesh>& pw
        = volPointInterpolator.interpolate(w);

    forAll(pw, pointI)
    {
        newPoints[pointI] += mesh.time().deltaT().value() * pw[pointI];
    }

    mesh.movePoints
    (
        newPoints
    );
//
//    /// Calculate mesh velocity
//    w = (newPoints - oldPoints) / runTime.deltaT(); // Lagrangian mesh motion
}

int main(int argc, char* argv[])
{
    argList args
    (
        //"controlDict"
        argc, argv
    );

    /// Create time
    Time runTime
    (
        "controlDict", 
        args.rootPath(),
        //args.casePath()
        args.caseName()
    );
    
    /// Create mesh
    autoPtr<fvMesh> meshPtr
    (
        new fvMesh
        (
            IOobject
            (
                polyMesh::defaultRegion,
                runTime,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE
            )
        )
    );
 
    fvMesh& mesh = meshPtr();
 
    /// Material velocity
    //surfaceVectorField ?    We need them on surfaces
    volVectorField v
    (
        IOobject
        (
            "v",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensioned<vector>("zero", dimLength/dimTime, vector::zero)
    );

    /// Alias of v, called U, needed by bekaert contact BC's
    /// Edit: A reference did not work, we work with a copy
    volVectorField U = v;
    U.rename("U");

    /// D field: x - x_ref
    volVectorField D
    (
        IOobject
        (
            "D",
            runTime.timeName(),
            mesh,
            //IOobject::MUST_READ,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensioned<vector>("D", dimLength, vector::zero)
    );
 
    volVectorField Dold = D;

    volVectorField vOld = v;
    
    /// Mesh velocity
    //surfaceVectorField ?    We need them on surfaces
    volVectorField w = v;
    
    w.rename("w");
 
    /// Stress
    volTensorField stress
    (
        IOobject
        (
            "stress",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensioned<tensor>
        (
            "zero", 
            dimMass / (pow(dimTime,2) * dimLength),
            tensor::zero
        )
    );

    /// Identity matrix (also Kronecker delta)
    //volTensorField I
    //(
    //    IOobject
    //    (
    //        "I",
    //        runTime.timeName(),
    //        mesh,
    //        IOobject::NO_READ
    //        IOobject::NO_WRITE
    //    ),
    //    mesh,
    //    dimensioned<tensor>
    //    (
    //        "zero", 
    //        dimless,
    //        tensor::zero
    //    )
    //);

    //forAll(I)
    symmTensor I(1, 0, 0, 1, 0, 1);
 
    /// Material constants
    IOdictionary physicalProperties
    (
        IOobject
        (
            "physicalProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );
 
    //dimensionedScalar rho("rho", dimMass/dimVolume, );
    dimensionedScalar rho(physicalProperties.lookup("rho"));
 
    dimensionedScalar muS(physicalProperties.lookup("mu"));

    dimensionedScalar E(physicalProperties.lookup("E"));

    volScalarField mu
    (
        IOobject
        (
            "mu",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        E/(2.0*(1.0 + 0.25))
        //E/(2.0*(1.0 + nu))
    );

    dimensionedScalar lambda(physicalProperties.lookup("lambda"));

    //dimensionedScalar lambda = mu * (2 * mu - E) / (E - 3 * mu); // Gives
                                                                   // negative

    Info<< "lambda = "
        << lambda
        << endl;

    surfaceScalarField phi = rho * linearInterpolate(w - v) & mesh.Sf();

    /// Read solution controls
 
    //solutionControl simple(mesh, "SIMPLE");
    //simpleControl simple(mesh);

    const label nIterMax = 1000;

    while (runTime.loop())
    {
        Info<< "Time = "
            << runTime.timeName()
            << endl;

        /// Differred correction loop
        for (label nIter = 0; nIter < nIterMax; ++nIter)
        //while (simple.loop())
        {
            /// DEq
            //fvVectorMatrix DEq
            //(
            //    rho * fvm::d2dt2(D) // D = x - x0, so x replaced by D
            // == fvm::laplacian(mu, D)
            //  + fvc::div(stress)
            //  - fvc::laplacian(mu, D)
            //  //+ fvm::laplacian(rho * (mesh.phi() - phi), U)
            //  + 1/runTime.deltaT() * (fvm::div(phi, D) - fvc::div(phi, D))
            //);
    
            /// vEq
            fvVectorMatrix vEq
            (
                rho * fvm::ddt(v)
             == runTime.deltaT() / 2 * fvm::laplacian(muS, v)
              + runTime.deltaT() / 2 * fvc::laplacian(muS, v)
              + fvc::div(stress)
              - fvc::laplacian(muS, D)
              + fvm::div(phi, v) // Non-linearity
            );

            Info<< "vEq assembled."
                << endl;

            vEq.relax();
 
            //vEq.solve();

            lduSolverPerformance solverPerformance = vEq.solve();

            /// Update v copy
            U = v;
 
            /// Update phi
            phi = rho * linearInterpolate(w - v) & mesh.Sf(); // phi updated; return to
                                                        // vEq
            Info<< "Phi corrected."
                << endl;
 
            /// Update D, not Dold
            D = Dold + 0.5 * (v + vOld) * runTime.deltaT();

            Info<< "D corrected."
                << endl;

            /// Stress, corrected in a differred manner
            stress = mu * (fvc::grad(D) + (fvc::grad(D))().T())
                + lambda * fvc::div(D) * I;

            Info<< "Stress corrected."
                << endl;

            ///*lduSolverPerformance*/ solverPerformance = DEq.solve();
 
            if 
            (
                solverPerformance.initialResidual() < 1e-4
                //mesh.solutionDict().solverPerformanceDict()
            ) {break;}
 
            // Update material velocity
            //u = fvc::ddt(D); // ?
        }

        // Force allocation of V. Important for any mesh changes since otherwise
        // old time volumes are not stored
        //const scalar totalVol = gSum(mesh.V());

        /// Update Dold
        /// 'Old' in time, not in iteration, so Dold != D.prevIter()
        Dold = D;
        vOld = v;

        Info<< "Update old fields."
            << endl;
 
        //
        // Update mesh velocity
        //
 
        // Move points according to some arbitrary rule
        update(mesh, v, w); // mesh.update()?
 
        //if (runTime.write()) // Only writes the latest time
        if (runTime.outputTime()) // Only writes the latest time
        {
            //runTime.write(); // Writes 'uniform' dir only
            mesh.write();
        }
    }
}
