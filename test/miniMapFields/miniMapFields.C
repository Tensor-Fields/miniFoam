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
#include "meshToMesh.H"
#include "patchManipulationFunctions.H"
#include "IOobjectList.H"

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
        Info<< "Create "
            << p.boundaryField()[patchI].type()
            << " ...";

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
            //)
            //)//.ptr()
        );

        Info<< "Created."
            << endl;
    }
}

int main(int argc, char* argv[])
{
    argList::validArgs.append("Accuracy: map|interpolate|cellPointInterpolate");

    argList::validOptions.insert("sourceOnly","");

    argList args(argc, argv);

    enum accuracy 
    {MAP,INTERPOLATE,CELL_POINT_INTERPOLATE, NONE};

    const word& accuracyTag = args.additionalArgs()[0];

    //if (argList::validOptions.found("meshToMesh"))
    //{

    //word mapMod; 
    //args.optionReadIfPresent("meshToMesh", mapMod);
    //if (args.optionReadIfPresent("meshToMesh", mapMod))
    //{
        //const word& mapMod = args.option("meshToMesh");
    //const word& mapMod = args.option("meshToMesh");

    //Info<< "mapMod:"
    //    << mapMod
    //    << endl;

    //}

    //}

    //Info<< "Set accuracy order ... ";

    //accuracy* accuracyKey;

    //(*accuracyKey) = NONE;

    //Info<< "accuracy order initialized to NONE."
    //    << endl;

    //if (accuracyTag == word("map"))
    //{
    //    Info<< "MAP is to be chosen ... ";

    //    (*accuracyKey) = MAP;
    //    
    //    Info<< "MAP chosen"
    //        << endl;
    //}
    //else if (accuracyTag == word("interpolate"))
    //{
    //    (*accuracyKey) = INTERPOLATE;

    //    Info<< "INTERPOLATE chosen"
    //        << endl;
    //}
    //else if (accuracyTag == word("cellPointInterpolate"))
    //{
    //    (*accuracyKey) = CELL_POINT_INTERPOLATE;

    //    Info<< "CELL_POINT_INTERPOLATE chosen"
    //        << endl;
    //}
    //else
    //{
    //    FatalErrorIn 
    //    (
    //        "Invalid accuracy option.\n"
    //        "Valid options are:\n"
    //        "map\n"
    //        "interpolate\n"
    //        "cellPointInterpolate"
    //    )   << exit(FatalError);
    //}

    Time runTime
    (
        "controlDict", 
        args.rootPath(),
        args.caseName()
    );

    Info<< "Time created."
        << endl;

    const polyMesh polyMeshSource
    (
        IOobject
        (
            //polyMesh::defaultRegion,
            "meshSource",
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );

    IOobject polyMeshTargetIO
    (
        "meshTarget",
        //polyMesh::defaultRegion,
        runTime.timeName(),
        runTime,
        IOobject::MUST_READ
    );

    polyMesh polyMeshTarget(polyMeshTargetIO);

    Info<< "polyMeshSource read from "
        << polyMeshSource.path()
        << endl;

    Info<< "polyMeshTarget read from "
        << polyMeshTarget.path()
        << endl;

    patchManipulationFunctions::reorderPatchesConsistently
        (polyMeshSource, polyMeshTarget);

    Info<< "polyMeshTarget"
        << "    "
        << "polyMeshSource"
        << endl;

    //forAll(polyMeshTarget.boundaryMesh(), patchI)
    //{
    //    Info<< polyMeshTarget.boundaryMesh()[patchI].name()
    //        << "    "
    //        << polyMeshSource.boundaryMesh()[patchI].name()
    //        << endl;
    //}

    const fvMesh& meshSource = fvMesh(polyMeshSource);

    /// If you want fvMesh&, it needs to be const (not tried)

    /// Assignment operator for const fvMesh falls in the private scope of 
    /// fvMesh, if the lhs is not a reference
    //const fvMesh meshTarget = fvMesh(polyMeshTarget); // Error
    //const fvMesh& meshTarget = fvMesh(polyMeshTarget); // OK

    //const fvMesh& meshTarget(polyMeshTarget);

    /// Changes the patch order

    polyMeshTargetIO.readOpt() = IOobject::NO_READ;

    //const fvMesh& meshTarget = fvMesh(polyMeshTarget);
    //const fvMesh& meshTarget = fvMesh(polyMeshSource);

    /// Create fvMesh target from elements to preserve patch order
    //labelList patchSizes(polyMeshTarget.boundaryMesh().size(), 0);
    //labelList patchStarts(polyMeshTarget.boundaryMesh().size(), 0);

    //forAll(patchSizes, patchI)
    //{
    //    patchSizes[patchI] = polyMeshTarget.boundaryMesh()[patchI].size();
    //    patchStarts[patchI] = polyMeshTarget.boundaryMesh()[patchI].start();
    //}

    /// Discards qualifiers
    //meshTarget.resetPrimitives
    //(
    //    xferCopy(polyMeshTarget.points()),
    //    xferCopy(polyMeshTarget.faces()),
    //    xferCopy(polyMeshTarget.faceOwner()),
    //    xferCopy(polyMeshTarget.faceNeighbour()),
    //    patchSizes,
    //    patchStarts,
    //    true // valid boundary
    //);

    IOobject fvMeshTargetIO
    (
        polyMesh::defaultRegion,
        runTime.timeName(),
        runTime,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    );

    fvMesh meshTarget
    (
        fvMeshTargetIO,
        xferCopy(polyMeshTarget.points()),
        xferCopy(polyMeshTarget.faces()),
        xferCopy(polyMeshTarget.faceOwner()),
        xferCopy(polyMeshTarget.faceNeighbour())
    );

    List<polyPatch*> patchPtrList(polyMeshTarget.boundaryMesh().size());

    forAll(patchPtrList, patchI)
    {
        patchPtrList[patchI] = 
            polyMeshTarget.boundaryMesh()[patchI].clone
            (
                polyMeshTarget.boundaryMesh()
            ).ptr();
    }

    meshTarget.addFvPatches(patchPtrList);

    //Info<< "boundary():"
    //    << endl;

    //Info<< "meshTarget"
    //    << "    "
    //    << "meshSource"
    //    << endl;

    //forAll(meshTarget.boundary(), patchI)
    //{
    //    Info<< meshTarget.boundary()[patchI].name()
    //        << "    "
    //        << meshSource.boundary()[patchI].name()
    //        << endl;
    //}

    //Info<< "boundaryMesh():"
    //    << endl;

    //Info<< "meshTarget"
    //    << "    "
    //    << "meshSource"
    //    << endl;

    //forAll(meshTarget.boundary(), patchI)
    //{
    //    Info<< meshTarget.boundaryMesh()[patchI].name()
    //        << "    "
    //        << meshSource.boundaryMesh()[patchI].name()
    //        << endl;
    //}

    /// Read and register fields with meshSource
    IOobjectList objects(meshSource, runTime.timeName());

    meshToMesh meshInterpolator(meshSource, meshTarget);

    Info<< nl << "meshInterpolator created."
        << nl << endl;

    // p is the generic field we manipulate here
    const volVectorField p // aka, fieldSource
    (
        IOobject
        (
            "DU",
            runTime.timeName(),
            meshSource,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        meshSource
    );

    Info<< "Field "
        << p.name()
        << " read from "
        << p.path()
        << endl;

    //wordList patchTypes(p.boundaryField().size());

    //forAll(patchTypes, patchI)
    //{
    //    patchTypes[patchI] = p.boundaryField()[patchI].type(); 

    //    Info<< "patchTypes[patchI] = "
    //        << patchTypes[patchI] << endl;
    //}

    IOobject fieldTargetIO
    (
        p.name(),
        runTime.timeName(),
        meshTarget,
        IOobject::NO_READ,
        IOobject::AUTO_WRITE
    );
    
    Info<< "fieldTargetIO created"
        << endl;

    /// Create fieldTarget `patches`
    //typedef typename p::cmptType fieldType;

    PtrList<fvPatchField<vector> >
             patches(p.boundaryField().size());

    createPatches
    (
        patches,
        p,
        meshTarget,
        fieldTargetIO
    );

    Info<< nl << "Patches created"
        << nl << endl;

    /// Create fieldTarget

    /// from components

    Info<< "Create "
        << fieldTargetIO.name()
        << " ... ";
        //<< endl;
        

    /// map
    //Info<< nl << "Create tmp target field"
    //    << endl;

    //GeometricField <vector, fvPatchField, volMesh>& tpxx
    ////=
    //(
    //    (
    //        meshInterpolator.interpolate
    //        (
    //            //fieldSource,
    //            p,
    //            meshToMesh::INTERPOLATE
    //            //meshToMesh::MAP
    //        )
    //    )()
    //);

    //Info<< "tmp target field created"
    //    << endl;

    //GeometricField
    //    <vector, fvPatchField, volMesh> pxx
    //(
    //    fieldTargetIO,
    //    tpxx/*()*/
    //);

    GeometricField
        <vector, fvPatchField, volMesh>* pMappedPtr
  = new GeometricField
        <vector, fvPatchField, volMesh>
    (
        fieldTargetIO,
        meshTarget,
        p.dimensions(),
        Field<vector>(meshTarget.nCells()),
        patches
    );

    //switch (*accuracyKey)
    //{}
    
    // Invalid non-const initialisation of a reference
    //word tmpMapMod = "sourceAndTarget";
    //const word& tmpMapMod = "sourceAndTarget";
    word mapMod = "sourceAndTarget";

    if(args.optionFound("sourceOnly"))
    {
        mapMod = "sourceOnly";
    }

    Info<< nl << "    Interpolator: "
        << mapMod
        << nl << endl;

    /// To test if `test` being changed in the scope of `if` remains the same
    /// after and outside that scope: yes.

    //word* test = new word("before, outside if");

    //Info<< *test
    //    << endl;

    if (accuracyTag == word("map"))
    {
        Info<< "    order: MAP"
            << endl;

        if (mapMod == word("sourceOnly"))
        {
            //GeometricField
            //    <vector, fvPatchField, volMesh>* pMappedPtr
            pMappedPtr
          = new GeometricField
                <vector, fvPatchField, volMesh>
            (
                fieldTargetIO,
                meshInterpolator.interpolate
                (
                    //pxx, // fieldTarget
                    p,   // fieldSource
                    meshToMesh::MAP
                )
            );

            //test = new word("inside if map sourceOnly");
        }
        else
        {
            //GeometricField
            //    <vector, fvPatchField, volMesh> pMapped

            meshInterpolator.interpolate
            (
                *pMappedPtr, // fieldTarget
                p,   // fieldSource
                meshToMesh::MAP
            );
        }

        //case MAP:

        //Info<< "MAP is to be chosen ... ";

        //(*accuracyKey) = MAP;
    }

    else if (accuracyTag == word("interpolate"))
    {
        Info<< "    order: INTERPOLATE"
            << endl;

        if (mapMod == word("sourceOnly"))
        {
            pMappedPtr
          = new GeometricField
                <vector, fvPatchField, volMesh>
            (
                fieldTargetIO,
                meshInterpolator.interpolate
                (
                    //pxx, // fieldTarget
                    p,   // fieldSource
                    meshToMesh::INTERPOLATE
                )
            );
        }
        else
        {
            //GeometricField
            //    <vector, fvPatchField, volMesh> pMapped

            meshInterpolator.interpolate
            (
                *pMappedPtr, // fieldTarget
                p,   // fieldSource
                meshToMesh::INTERPOLATE
            );
        }

        //case INTERPOLATE:

            //(*accuracyKey) = INTERPOLATE;

        //break;
    }

    else if (accuracyTag == word("cellPointInterpolate"))
    {
        Info<< "    order: CELL_POINT_INTERPOLATE"
            << endl;

        if (mapMod == word("sourceOnly"))
        {
            pMappedPtr
          = new GeometricField
                <vector, fvPatchField, volMesh>
                (
                    fieldTargetIO,
                    meshInterpolator.interpolate
                    (
                        //pxx, // fieldTarget
                        p,   // fieldSource
                        meshToMesh::CELL_POINT_INTERPOLATE
                    )
                );
        }
        else
        {
            //GeometricField
            //    <vector, fvPatchField, volMesh> pMapped

            meshInterpolator.interpolate
            (
                *pMappedPtr, // fieldTarget
                p,   // fieldSource
                meshToMesh::CELL_POINT_INTERPOLATE
            );
        }

        //case CELL_POINT_INTERPOLATE:

            //(*accuracyKey) = CELL_POINT_INTERPOLATE;

        //break;
    }

    else
    {
        //default:
        FatalErrorIn 
        (
            "Invalid accuracy option.\n"
            "Valid options are:\n"
            "map\n"
            "interpolate\n"
            "cellPointInterpolate"
        )   << exit(FatalError);
    }

    Info<< "Target "
        << pMappedPtr->name()
        << " created"
        << endl;

    //Info<< pMappedPtr->name()
    //    << ", correct boundaryConditions ... ";

    //pMappedPtr->correctBoundaryConditions();

    //Info<< pMappedPtr->name()
    //    << " boundaryConditions corrected"
    //    << endl;
    
    runTime++;
    
    Info<< "Write "
        << pMappedPtr->name()
        << " in "
        << pMappedPtr->path()
        << " ..."
        << endl;
        
    //Info<< *test
    //    << endl;

    meshTarget.write();
    //pMappedPtr->write();
    
    Info<< pMappedPtr->name()
        << " written to "
        << runTime.timeName()
        << endl;
    
    delete pMappedPtr;

    Info<< nl << "End."
        << endl;
}
