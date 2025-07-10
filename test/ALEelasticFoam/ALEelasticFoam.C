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
#include "simpleControl.H"
#include "pimpleControl.H"
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
    volVectorField& w,
    const volVectorField& D
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
        /// Lagrangian
        w[cellI] = v[cellI];
    }

    /// Consistently
    // ...

    const volPointInterpolation volPointInterpolator(mesh);

    pointField oldPoints = mesh.points();

    pointField newPoints = oldPoints;

    const GeometricField<vector, pointPatchField, pointMesh>& pw
        = volPointInterpolator.interpolate(w);

    /// Point displacement field
    const GeometricField<vector, pointPatchField, pointMesh>& pD
        = volPointInterpolator.interpolate(D);

    //forAll(pw, pointI)
    //forAll(pD, pointI)
    //{
        //newPoints[pointI] += mesh.time().deltaT().value() * pw[pointI];
        //newPoints[pointI] += pD;

        /// Update mesh points
        newPoints += pD;
    //}

    /// Reset boundary points to old points
    //pointMesh pm(mesh);

    //forAll(pm.boundary(), patchI)
    //{
    //    labelList bp = pm.boundary()[patchI].meshPoints();

    //    forAll(bp, pointI)
    //    {
    //        newPoints[bp[pointI]] = oldPoints[bp[pointI]];
    //    }
    //}

    /// Move the mesh
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
    argList::validOptions.insert("res", "value");

    argList args
    (
        //"controlDict"
        argc, argv
    );

    scalar max_res_to_break = readScalar
    (IStringStream(args.options()["res"])());

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

    /// Requested on run-time (who?)
    volTensorField gradV = fvc::grad(v);
    //(
    //    IOobject
    //    (
    //        "grad(v)",
    //        runTime.timeName(),
    //        mesh,
    //        IOobject::READ_IF_PRESENT,
    //        IOobject::AUTO_WRITE
    //    ),
    //    mesh,
    //    //fvc::grad(v)()
    //);

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
 
    ///
    /// Stress
    ///

    /// Helper to Cauchy stress calc
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

    volSymmTensorField sigma
    (
        IOobject
        (
            "sigma",
            runTime.timeName(),
            mesh,
            IOobject::READ_IF_PRESENT,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensioned<symmTensor>
        (
            "zero", 
            dimMass / (pow(dimTime,2) * dimLength),
            symmTensor::zero
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

    dimensionedScalar ES(physicalProperties.lookup("E"));

    dimensionedScalar lambdaS(physicalProperties.lookup("lambda"));

    //dimensionedScalar lambda = mu * (2 * mu - E) / (E - 3 * mu); // Gives
                                                                   // negative

    Info<< "lambda = "
        << lambdaS
        << endl;

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
        mesh,
        muS
        //E/(2.0*(1.0 + 0.25))
        //E/(2.0*(1.0 + nu))
    );

    volScalarField E
    (
        IOobject
        (
            "E",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        ES
    );

    volScalarField lambda
    (
        IOobject
        (
            "lambda",
            mesh.time().timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        lambdaS
    );

    surfaceScalarField phi = rho * linearInterpolate(w - v) & mesh.Sf();

    /// Read solution controls
 
    //solutionControl simple(mesh, "SIMPLE");

    //simpleControl simple(mesh); // Forces stop, when Simple converged, so no
                                  // time marching

    pimpleControl pimple(mesh);

    const label nIterMax = 1000;

    //while (runTime.loop()) // Only first time step is written into
                             // the endTime directory 
    while (runTime.run())
    {
        Info<< "Time = "
            << runTime.timeName()
            << endl;

        ///
        /// Differred correction loop
        /// Alternatives:
        /// - for
        /// - simple
        /// - pimple
        /// - while-do
        ///

        for (label nIter = 0; nIter < nIterMax; ++nIter)
        //while (simple.loop()) // Forces stop when simple converged, no matter
                                // what time marching setting is.
        //while (pimple.loop())
        {
            /// DEq
            //fvVectorMatrix DEq
            //(
            //    rho * fvm::d2dt2(D) // D = x - x0, so x replaced by D
            // == fvm::laplacian(mu, D)
            //  + fvc::div(sigma)
            //  - fvc::laplacian(mu, D)
            //  //+ fvm::laplacian(rho * (mesh.phi() - phi), U)
            //  + 1/runTime.deltaT() * (fvm::div(phi, D) - fvc::div(phi, D))
            //);
    
            /// Force V allocation for V0 to be stored
            /*const scalar totalV = */gSum(mesh.V());

            ///
            /// vEq
            ///

            /// For now, the only spatially-implicit contribution comes from the
            /// convection term. 
            /// v = (D_new - D_old)/deltaT, for a single particle;
            /// This is the definition of v.
            /// If you are calculating
            /// (D_new - D_old)/deltaT at the same point of space, then
            /// (D_new - D_old)/deltaT = 0! (It is \partial_t x in Chadwick,
            /// p.52, eq. 12). 
            /// From
            /// Chadwich, p.52, eq. 12, the above formula is set in the Eulerian
            /// description for v, and alternatively can be written as
            /// v = x_dot, which in turn means that for Eulerian v,
            /// velocity at each points is the velocity of the particle
            /// occupying that point.
            /// Note that v in our formulation is not the material velocity,
            /// because we decided that \int_R \rho v dV is the momentum of the
            /// ARBITRARY region R.

            /// So, still not clear how to calculate D from v, and later, how to
            /// implicitely contribute sigma to the system.
            fvVectorMatrix vEq
            (
                rho * fvm::ddt(v) // Actually fvm::ddt(rho * v)
             == 
              //fvm::laplacian(mu, D) // consider ortho and non-ortho terms
                runTime.deltaT() / 2 * fvm::laplacian(muS, v)
              + runTime.deltaT() / 2 * fvc::laplacian(muS, v)
              + fvc::div(sigma)
              //- fvc::laplacian(muS, D)
              + fvm::div(phi, v) // Non-linearity
            );

            Info<< "vEq assembled."
                << endl;

            vEq.relax();
 
            //vEq.solve();

            lduSolverPerformance solverPerformance = vEq.solve();

            v.correctBoundaryConditions();

            /// Update copies
            U.internalField() = v.internalField();
            U.boundaryField() = v.boundaryField();

            gradV.internalField() = fvc::grad(v)().internalField();
            gradV.boundaryField() = fvc::grad(v)().boundaryField();
 
            /// Update phi
            phi = rho * linearInterpolate(w - v) & mesh.Sf(); // phi updated; return to
                                                        // vEq
            Info<< "Phi corrected."
                << endl;
 
            /// Update D, not Dold (old in time, not iter)
            D = Dold + 0.5 * (v + vOld) * runTime.deltaT();

            Info<< "D corrected."
                << endl;

            /// Stress, corrected in a differred manner
            stress = mu * (fvc::grad(D) + (fvc::grad(D))().T())
                + lambda * fvc::div(D) * I;

            /// Pour stress to sigma
            forAll(stress, cellI)
            {
                forAll(stress[cellI], cmpt)
                {
                    sigma[cellI][0] = stress[cellI][0];
                    sigma[cellI][1] = stress[cellI][1];
                    sigma[cellI][2] = stress[cellI][2];
                    sigma[cellI][3] = stress[cellI][4];
                    sigma[cellI][4] = stress[cellI][5];
                    sigma[cellI][5] = stress[cellI][8];
                }
            }

            Info<< "sigma corrected."
                << endl;

            ///*lduSolverPerformance*/ solverPerformance = DEq.solve();
 
            if
            (
                solverPerformance.initialResidual() < max_res_to_break
                //mesh.solutionDict().solverPerformanceDict()
            ) {break;}
 
            // Update material velocity
            //u = fvc::ddt(D); // ?
        }

        // Force allocation of V. Important for any mesh changes since otherwise
        // old time volumes are not stored
        //const scalar totalVol = gSum(mesh.V());
        mesh.setV0();

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
        update(mesh, v, w, D); // mesh.update()?
 
        runTime++; // When runTime.run() is used instead of .loop()

        if (runTime.write()) // Only writes the latest time
        //if (runTime.outputTime()) // Only writes the latest time
        {
            //runTime.write(); // Writes 'uniform' dir only
            mesh.write();
        }
    }
}
