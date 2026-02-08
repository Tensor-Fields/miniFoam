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

typedef GeometricField<tensor, fvPatchField, volMesh> volTensorField;

typedef GeometricField<symmTensor, fvPatchField, volMesh> volSymmTensorField;

typedef GeometricField<scalar, fvsPatchField, surfaceMesh> surfaceScalarField;

/// Post-solve system, obtain relaxed primary and also secondary fields
#include "postSolve.H"

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
    argList::validOptions.insert("stabilisationFactor", "value");
    argList::validOptions.insert("convergenceFactors", "values");
    //argList::validOptions.insert("formulaA", "values");
    argList::validOptions.insert("formula", "<name>");
    argList::validOptions.insert("formulaA", "");
    argList::validOptions.insert("formulaWave", "");
    argList::validOptions.insert("nIterMax", "value");
    argList::validOptions.insert("relaxField", "value");
    argList::validOptions.insert("relaxEq", "value");
    argList::validOptions.insert("Lagrangian", "");
    argList::validOptions.insert
        (
            "stabilisationTerm",
            "defaults to laplacian-divGrad. Option: divGrad-laplacian "
        );

    argList::validOptions.insert("explicitWeight", "<weight>");
    argList::validOptions.insert("vBased", "");
    argList::validOptions.insert("DBased", "");

    argList args
    (
        //"controlDict"
        argc, argv
    );

    scalar max_res_to_break = readScalar
    (IStringStream(args.options()["res"])());

    const label& nIterMax = 1000;

    if (args.optionFound("nIterMax"))
    {
        const_cast<label&>(nIterMax) =
            label
            (
                readScalar
                (
                    IStringStream(args.options()["nIterMax"])()
                )
            );
    }

    const scalar& fCoeff = 1;

    const scalar& eqRelaxCoeff = 1;

    if (args.optionFound("relaxField"))
    {
        const_cast<scalar&>(fCoeff) =
            readScalar
            (
                IStringStream(args.options()["relaxField"])()
            );
    }

    if (args.optionFound("relaxEq"))
    {
        const_cast<scalar&>(eqRelaxCoeff) =
            readScalar
            (
                IStringStream(args.options()["relaxEq"])()
            );
    }

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

    volSymmTensorField sigmaD = sigma;

    volSymmTensorField sigmaDot
    (
        IOobject
        (
            "sigmaDot",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        mesh,
        dimensioned<symmTensor>
        (
            "zero", 
            dimMass / (pow(dimTime,3) * dimLength),
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
            //"physicalProperties",
            "mechanicalProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );
 
    //dimensionedScalar rho("rho", dimMass/dimVolume, );
    dimensionedScalar rho(physicalProperties.lookup("rho"));

    dimensionedScalar ES(physicalProperties.lookup("E"));

    // From Demirdzic and Martinovic, p.333, Eq.(9), we use nu for lambda and mu
    // calculation
    dimensionedScalar nuS(physicalProperties.lookup("nu"));

    ///
    /// Plane stress check
    ///

    // No plane stress
    dimensionedScalar lambdaS = nuS * ES / ((1 + nuS) * (1 - 2 * nuS));

    // Plane stress
    if (physicalProperties.lookupOrDefault<bool>("planeStress", false))
    {
        lambdaS = nuS * ES / ((1 + nuS) * (1 - nuS));

        /// Registerd as a field with the mesh,
        /// to be accessed by db() in boundary condition codes
        /// Create it as a pointer, so it lives out of this scope
        Field<bool>* planeStress = new Field<bool>(mesh.C().size(), true);
    }

    Info<< "lambda = "
        << lambdaS
        << endl;

    /// Wrong!
    //dimensionedScalar muS = ES / (nuS * (1 + nuS));

    /// Jasak and Weller 2000, and Wikipedia, and Saad, Table 4.1
    dimensionedScalar muS = ES / (2 * (1 + nuS));

    Info<< "mu = "
        << muS
        << endl;

    //dimensionedScalar muS(physicalProperties.lookup("mu"));

    //dimensionedScalar lambdaS(physicalProperties.lookup("lambda"));

    //dimensionedScalar lambda = mu * (2 * mu - E) / (E - 3 * mu); // Gives

                                                                   // negative
                                                                   // negative

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

    //const label nIterMax = 1000;

    //while (runTime.loop()) // Only first time step is written into
                             // the endTime directory 
    scalar stabilisationFactor = 1;

    if (args.optionFound("stabilisationFactor"))
    {
        stabilisationFactor =
            readScalar(IStringStream(args.options()["stabilisationFactor"])());
    }

    dimensioned<scalar> convergenceFactor = 2 * muS + lambdaS;
    if (args.optionFound("convergenceFactors"))
    {
        List<string> convergenceFactors =
            args.optionReadList<string>("convergenceFactors");

        convergenceFactor =
            readScalar(IStringStream(convergenceFactors[0])()) * muS
         +  readScalar(IStringStream(convergenceFactors[1])()) * lambdaS;
    }

    scalar a=1, b=0;

    if (args.optionFound("formulaA"))
    {
        if(!args.optionFound("convergenceFactors"))
        {
            FatalErrorIn("ALEelasticFoam.C")
                << "formulaA goes with convergenceFactors option"
                << abort(FatalError);
        }

        List<string> convergenceFactors =
            //args.optionReadList<string>("formulaA");
            args.optionReadList<string>("convergenceFactors");

        a = readScalar(IStringStream(convergenceFactors[0])());
        b = readScalar(IStringStream(convergenceFactors[1])());

        Info<< "a = " 
            << a  << "\nb = "
            << b  << endl;
    }

    volVectorField vPrevIter = v;

    /// Reference to automatically get updated. Wrong! Does not update!
    //scalar& pseudoDeltaT = runTime.deltaT();

    bool steadyState = false;
    bool steadyStateConverged = false;

    if
    (
        mesh.schemesDict().ddtSchemes().lookup("ddt(v)")
     == "steadyState" 
     || mesh.schemesDict().ddtSchemes().lookup("default")
     == "steadyState" 
    )
    {
        steadyState = true;

        /// From the trapizoidal rule
        //pseudoDeltaT = /*min (*/2 * (D - Dold) / (SMALL + v + vOld)/*, 0)*/
    }

    lduSolverPerformance solverPerformance;

    while (runTime.run())
    {
        Info<< nl << "Time = "
            << runTime.timeName()
            << endl;

        /// We put this coeffs here for them to be able to change with time if
        /// needed
        dimensioned<scalar> au0("au0", dimless, 1);
        //dimensioned<scalar> av0("av0", dimTime, 0.5 * runTime.deltaT());
        dimensioned<scalar> av0 = 0.5 * runTime.deltaT();
        dimensioned<scalar> av = av0;

        if (args.optionFound("formulaA"))
        {
            au0 = 1;
            av0 = 0.5 * runTime.deltaT();
            av = av0;

            //Info<< "au0 = "
            //    << au0<< "av0 = "
            //    << av0<< "av = "
            //    << av << endl;
        }
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
            /// Force V allocation for V0 to be stored
            /*const scalar totalV = */gSum(mesh.V());

            autoPtr<fvMatrix<vector> > vEqPtr;
            autoPtr<fvMatrix<vector> > DEqPtr;

            bool vBased = args.optionFound("vBased");
            bool DBased = args.optionFound("DBased");

            if (vBased && DBased)
            {
                FatalErrorIn("")
                    << "Choose either '-vBased' or '-DBased'"
                    << abort(FatalError);
            }

            if (vBased)
            {
                #include "vEq.H"
            }
            else if (DBased)
            {
                #include "DEq.H"
            }
            else
            {
                notImplemented("Only v-based and D-based models are"
                " imlemented;\n"
                "Choose one using '-vBased' or '-DBased'.");
                 
            }

            Info<< "Eq assembled."
                << endl;

            /// Solve and update fileds
            postSolve
            (
                /// Relax equation
                eqRelaxCoeff, args.optionFound("relaxEq"),
                /// Relax field
                fCoeff,
                vEqPtr,
                DEqPtr,
                /// ldu solver
                solverPerformance,
                /// Mesh
                mesh,
                /// Primary fields
                v,
                D,
                /// Secondary fields
                gradV,
                stress,
                sigma, 
                sigmaD, 
                sigmaDot,
                phi,
                w,
                /// Properties
                rho,
                mu,
                lambda
            );

            if
            (
                solverPerformance.initialResidual() < max_res_to_break
                //mesh.solutionDict().solverPerformanceDict()
            )
            {
                steadyStateConverged = true;
                break;
            }
 
            // Update material velocity
            //u = fvc::ddt(D); // ?
        }

        // Force allocation of V. Important for any mesh changes since otherwise
        // old time volumes are not stored
        //const scalar totalVol = gSum(mesh.V());
        mesh.setV0();

        /// Update Dold
        /// 'Old' in time, not in iteration, so Dold != D.prevIter()
        //Dold = D;
        //vOld = v;

        //Info<< "Update old fields."
        //    << endl;
 
        //
        // Update mesh velocity
        //
 
        if (args.optionFound("Lagrangian"))
        {
            /// Lagrangian
            w = v;
        }
        else
        {
            /// ALE
            // Move points according to some arbitrary rule
            update(mesh, v, w, D); // mesh.update()?
        }
 
        runTime++; // When runTime.run() is used instead of .loop()

        if (runTime.write()) // Only writes the latest time
        //if (runTime.outputTime()) // Only writes the latest time
        {
            //runTime.write(); // Writes 'uniform' dir only
            mesh.write();
        }

        /// Stop in steady-state, when the max residual is hit
        if
        (
            steadyState
         && steadyStateConverged
        )
        {
            Info<< nl << "Converged to the steady state solution"
                << endl;

            break;
        }
    }
}
