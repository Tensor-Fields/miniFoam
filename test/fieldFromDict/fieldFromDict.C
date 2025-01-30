/**
 * Create a GeometricField form a dictionary
 *
 * \author: Ali (ali@tensorfields.com), Richmond Hill
 * \date: Jan 03 2025
 */

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

    /// polyMesh at constant
    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            //runTime.constant(),
            runTime.timeName(0),
            runTime,
            IOobject::MUST_READ
        )
    );

    /// polyMesh at latestTime
    fvMesh mesh2
    (
        IOobject
        (
            //polyMesh::defaultRegion,
            "mesh2",
            //"mesh2",
            runTime,
            IOobject::MUST_READ
        )
    );

    Info<< "mesh.nCells() = "
        << mesh.nCells()
        << endl;

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

    //dictionary dic;

    //Ostream os;

    //Istream is;
    //
    //p.boundaryField().writeEntry("boundaryField", os);

    //is  << os;

    //is  >> dic;

    ///
    /// Map boundaries
    ///

    wordList patchTypes(p.boundaryField().size());

    PtrList<fvPatchField<scalar> > patches(p.boundaryField().size());

    forAll(patchTypes, patchI)
    {
        patchTypes[patchI] = p.boundaryField()[patchI].type(); 
        Info<< patchTypes[patchI] 
            << endl;
    //}

    //forAll(patches, patchI)
    //{
        //p.boundaryField()[patchI].banana();
        //patches[patchI] = p.boundaryField()[patchI];
        patches.set
        (
            patchI,
            //new (patchTypes[patchI]) (p.boundaryField()[patchI]) // Error
            //new fvPatchField<scalar>(p.boundaryField()[patchI])
            p.boundaryField()[patchI].clone
            (
                // Error: Field is not DimensionedField
                //Field<scalar>(mesh2.boundaryMesh()[patchI].size()) 
                DimensionedField<scalar, volMesh>
                (
                    IOobject
                    (
                        "pxx",
                        runTime.timeName(),
                        mesh2
                    ),
                    mesh2,
                    dimensionedScalar("zero", p.dimensions(), 0 )
                )
            ).ptr()
            //p.boundaryField()[patchI].clone().ptr()
        );
    }

    /// pxx from components
    volScalarField pxx
    (
        IOobject
        (
            "pxx",
            runTime.timeName(),
            /// OK: Although p.internalField() have a different size from
            /// mesh2, it does not matter for being the registry
            //mesh,
            mesh2,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        /// Error: mesh2 and p.internalField() have different sizes
        mesh2,
        p.dimensions(),
        //p.internalField(),
        Field<scalar>(mesh2.nCells()),
        patches
    );

    Info<< "p.size() = "
        << p.size()
        << endl;

    Info<< "p.internalField().size() = "
        << p.internalField().size()
        << endl;

    Info<< "mesh2.nCells() = "
        << mesh2.nCells()
        << endl;


    /// pxx copy from p, reset IO; unsuccessful for copying boundaries while
    /// mesh corresponding to the new field (pxx) changes

    /// Another pxx constructor
    //volScalarField pxx
    //(
    //    IOobject
    //    (
    //        "pxx",
    //        runTime.timeName(),
    //        mesh,
    //        IOobject::NO_READ,
    //        IOobject::AUTO_WRITE
    //    ),
    //    mesh2,
    //    IOdictionary(p)
    //    //p.dimensions(),
    //    //zeroGradientFvPatchField
    //    //<
    //    //    typename outerProduct<vector, scalar>::type
    //    //>::typeName

    //    //p.internalField(),
    //    //ptrListPatchFieldType

    //    //dimensionedScalar
    //    //(
    //    //    "banana",  
    //    //    pp.dimensions(), 
    //    //    pTraits<scalar>::zero
    //    //),
    //    //patchTypes
    //);

    /// Write results
    runTime++;
    p.write();

    //Info<< "Write pxx after creation in "
    //    << runTime.timeName()
    //    << endl;

    //pxx.write();

    ///// update coeffs 
    //runTime++;
    //pxx.boundaryField().updateCoeffs();

    //Info<< "Write pxx after updateCoeffs in "
    //    << runTime.timeName()
    //    << endl;

    //pxx.write();

    ///// evaluate
    //runTime++;
    //pxx.boundaryField().evaluate();

    //Info<< "Write pxx after evaluate in "
    //    << runTime.timeName()
    //    << endl;

    //pxx.write();

    /// Correct BC
    //runTime++;
    pxx.correctBoundaryConditions();

    Info<< "Write pxx after correctBC in "
        << runTime.timeName()
        << endl;

    pxx.write();
}
