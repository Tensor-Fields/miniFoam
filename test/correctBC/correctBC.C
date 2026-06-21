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

template<class Type>
void createPatches
(
    PtrList<fvPatchField<Type> >& patches,
    const GeometricField<Type, fvPatchField, volMesh>& p,
    const fvMesh& meshTarget,
    const IOobject& fieldTargetIO
)
{
    forAll(patches, patchI)
    {
        patches.set
        (
            patchI,
    
            ///
            /// clone constructor
            ///
    
            //// Gives the wrong patch size
            //p.boundaryField()[patchI].clone().ptr()
    
            /// Impossible; mapper is fvPatchFieldMapper object, the piece of info
            /// which is private to a meshToMesh object
            //p.boundaryField()[patchI].clone().ptr()->autoMap(mapper)
    
            ///
            /// from patch and internal field
            /// ??? does not work
            ///
    
            //new fvPatchField<scalar>
            //(
            //    mesh_2.boundary()[patchI],
            //    p.internalField()
            //)
    
            ///
            /// using New, from patch and internal field
            ///
    
            fvPatchField<Type>::New
            (
                //mesh_2.boundary()[patchI].name(), // Gives, eg, movingWall
                                                    // instead of patch
                p.boundaryField()[patchI].type(),
                meshTarget.boundary()[patchI],
                DimensionedField<Type, volMesh>
                  //(p.internalField()) // Error: conversion form
                                        // InternalField to
                                        // DimensionedField
                (
                    fieldTargetIO,
                    meshTarget,
                    p.dimensions()//,
                    //p.internalField()// Run-time error: different
                                       // patch sizes between p and
                                       // mesh_2
                )
            )
    
            //fvPatchField<Type>::New
            //(
            //    fieldThis.boundaryField()[patchI].type(),
            //    this->boundary()[patchI],
            //new fvPatchField<Type> 
            //(
            //    fieldThis.boundaryField()[patchI]
            //)
    
            //fieldThis.boundaryField()[patchI].clone
            //(
            //    DimensionedField<Type, volMesh>
            //    (
            //        IOobject
            //        (
            //            fieldThis.name(),
            //            this->time().timeName(),
            //            *this
            //        ),
            //        *this,
            //        dimensioned<Type>
            //        (
            //            "zero", 
            //            fieldThis.dimensions(),
            //            pTraits<Type>::zero 
            //        )
            //    )//,
            //    // Error: cast to abstract
            //    // class:
            //    //fvPatchFieldMapper() 
            //)//.ptr()
        );
    }
}

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
            "targetMesh",
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );

    // p is the generic field we manipulate here
    const volVectorField p
    (
        IOobject
        (
            "DU",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    //volScalarField& pp = const_cast<volScalarField&>(p);

    /// Use the constructor that gets dimensionedField
    //volScalarField px
    //(
    //    IOobject
    //    (
    //        "p",
    //        runTime.timeName(),
    //        mesh,
    //        IOobject::NO_READ,
    //        IOobject::AUTO_WRITE
    //    ),
    //    mesh,
    //    dimensionedScalar
    //    (
    //        "banana",  
    //        pp.dimensions(), 
    //        pTraits<scalar>::zero
    //    )
    //);

    /// Construct from components

    //const polyBoundaryMesh& bMesh = mesh.boundaryMesh(); // Disallowed; assignment

    //const polyBoundaryMesh bMesh(mesh.boundaryMesh()); // Disallowed; copy constructor
    
    //wordList& patchNames(mesh.boundaryMesh().size()); // Invalid init of non-const ref
    
    //wordList patchTypes(mesh.boundaryMesh().size()); // e.g., gives you wall
                                                       //(set in polyMesh/boundary)
                                                       // rather than zeroGradient 
                                                       //(set in 0/p)

    wordList patchTypes(p.boundaryField().size());

    //PtrList<fvPatchField<scalar> > ptrListPatchFieldType
    //(p.boundaryField().size());

    forAll(patchTypes, patchI)
    {
        patchTypes[patchI] = p.boundaryField()[patchI].type(); 

        Info<< "patchTypes[patchI] = "
            << patchTypes[patchI]
            << endl;

        Info<< "patchType = "
            << p.boundaryField()[patchI].patchType()
            << endl;
    }

IOobject fieldTargetIO
(
    "pxx",
    runTime.timeName(),
    mesh,
    IOobject::NO_READ,
    IOobject::AUTO_WRITE
);

Info<< "fieldTargetIO created"
    << endl;

    /// Create `patches`
    PtrList<fvPatchField<Type> >
             patches(p.boundaryField().size());

    createPatches
    (
        patches,
        p,
        meshTarget,
        fieldTargetIO
    );

    /// Create fieldTarget
    volScalarField pxx
    (
        fieldTargetIO,
        //mesh,
        meshTarget,

        p.dimensions(),
        Field<scalar>(meshTarget.nCells()),

        //zeroGradientFvPatchField
        //<
        //    typename outerProduct<vector, scalar>::type
        //>::typeName

        //p.internalField(),
        //ptrListPatchFieldType

        //dimensionedScalar
        //(
        //    "banana",  
        //    p.dimensions(), 
        //    pTraits<scalar>::zero
        //),

        //patchTypes,

        patches
    );

    //Info<< "mesh.boundaryMesh().name() = "
    //    << mesh.boundaryMesh().name() 
    //    << endl;

    /// Change cell value
    //forAll(mesh.C(), cellI)
    //{
    //    //px[cellI] = 1000;
    //    pxx[cellI] = 1000;
    //}

    //pxx.boundaryField() = 2000;
    
    //pxx.boundaryField() = (fvc::interpolate(pxx)()).boundaryField();
    /// volScalarField p and fvc::interpolate(p) have the same
    /// size on each patch
    //for (label i=0; i<3; ++i)
    //forAll(pxx.boundaryField(), patchI)
    //{
    //    //Info<< "pxx.boundaryField()[i].size() = "
    //    //    << pxx.boundaryField()[i].size()
    //    //    << endl;
    //    
    //    //Info<< "(fvc::interpolate(pxx)()).bf[i].size() = "
    //    //    << (fvc::interpolate(pxx)()).boundaryField()[i].size()
    //    //    << endl;

    //    /// Works fine!
    //    //forAll(pxx.boundaryField()[patchI], faceI)
    //    //{
    //    //    pxx.boundaryField()[patchI][faceI]
    //    //  = (fvc::interpolate(p)()).boundaryField()[patchI][faceI];
    //    //}

    //    ///
    //    /// Copy owner value, works fine
    //    ///
    //    
    //    /// list of owners of a patch
    //    //const unallocLabelList patchOwners = mesh.boundaryMesh()[patchI].faceCells();

    //    //label faceI = 0;

    //    //forAll(patchOwners, cellI)
    //    //{
    //    //    /// Map cell value to face value

    //    //    Info<< "Map pxx from cell "
    //    //        << cellI
    //    //        << " to face "
    //    //        << faceI
    //    //        << ". Owner label: "
    //    //        << patchOwners[cellI]
    //    //        << endl;

    //    //    //pxx.boundaryField()[patchI][faceI] = pxx[cellI];
    //    //    pxx.boundaryField()[patchI][faceI]
    //    //  = p.boundaryField()[patchI][faceI];

    //    //    ++faceI;
    //    //}

    //    /// Different value aqusition by merged boundaries
    //    //forAll(pxx.boundaryField()[patchI], faceI)
    //    //{
    //    //    //pxx.boundaryField()[patchI][faceI] = 800;
    //    //    //pxx.boundaryField()[patchI][faceI] = Random(faceI).scalar01();
    //    //  //= (fvc::interpolate(pxx)()).internalField()[faceI];
    //    //  //= (fvc::interpolate(pxx)()).boundaryField()[patchI][faceI];
    //    //}

    //    /// Extrapolate from the internal field
    //    					/// This internalField() has the same size as nCells.
    //  //  pxx.boundaryField()[patchI]
    //  //= pxx.boundaryField()[patchI].internalField();
    //  Info<<"pxx.boundaryField()[patchI].internalField().size() = "
    //      << pxx.boundaryField()[patchI].internalField().size()
    //    									<< endl;

    //  //Info<<"pxx.boundaryField()[patchI].patch().internalField().size() = "
    //   Info<< "pxx.boundaryField()[patchI].patchInternalField()().size() = "
    //      //<< pxx.boundaryField()[patchI].patch().internalField().size()
    //    									// Error: pxx.boundaryField()[patchI].patch() is fvPatch and does
    //    									//        not have internalField().
    //      //<< pxx.boundaryField()[patchI].banana()
    //      << pxx.boundaryField()[patchI].patchInternalField()().size()
    //    									<< endl;
    //}

    // Works fine for empty and zeroGradient, not fixedValue; the latter stays
    // zero.
    //pxx.boundaryField() = p.boundaryField();

    /// Write results
    runTime++;
    p.write();
    //px.write();
    Info<< "Write pxx after creation in "
        << runTime.timeName()
        << endl;
    pxx.write();

    /// update coeffs 
    runTime++;
    //p.boundaryField().updateCoeffs();
    //px.boundaryField().updateCoeffs();
    pxx.boundaryField().updateCoeffs();
    //px.write();
    p.write();

    Info<< "Write pxx after updateCoeffs in "
        << runTime.timeName()
        << endl;

    pxx.write();

    /// evaluate
    runTime++;
    //p.boundaryField().evaluate();
    //px.boundaryField().evaluate();
    pxx.boundaryField().evaluate();
    p.write();
    //px.write();

    Info<< "Write pxx after evaluate in "
        << runTime.timeName()
        << endl;

    pxx.write();

    /// Correct BC
    runTime++;
    //p.correctBoundaryConditions();
    //px.correctBoundaryConditions();
    pxx.correctBoundaryConditions();
    p.write();
    //px.write();

    Info<< "Write pxx after correctBC in "
        << runTime.timeName()
        << endl;

    pxx.write();

    /// Rename/retype tha last patch
    //const polyBoundaryMesh& oldBoundary = mesh.boundaryMesh(); 

    //Info<< "oldBoundary: "
    //    << oldBoundary
    //    << endl;

    //const label nPatches = oldBoundary.size();

    //List<polyPatch*> newBoundary(nPatches);

    //forAll(newBoundary, patchI)
    //{
    //    newBoundary[patchI] = new polyPatch(oldBoundary[patchI]);

    //    if (patchI == nPatches - 1)
    //    {
    //        newBoundary[patchI] = polyPatch::New
    //        (
    //           "symmetryPlane",
    //           "newPatch",
    //           oldBoundary[patchI].size(), // patch size
    //           oldBoundary[patchI].start(), // start face
    //           patchI,
    //           oldBoundary
    //        ).ptr();
    //    }
    //}

    //Info<< "newBoundary: "
    //    << endl;

    //forAll(newBoundary,patchI)
    //{
    //    Info<< *newBoundary[patchI]
    //        << endl;
    //}

    ///// Remove old boundary
    //mesh.removeFvBoundary();

    ///// Add new boundary
    //mesh.addFvPatches(newBoundary);
    ////mesh.addPatches(newBoundary);

    //runTime++;

    //mesh.write();
    //p.write();
    //Info<< "p:"
    //    << p
    //    << endl;
}
