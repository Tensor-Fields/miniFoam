#include "fvc.H"
#include "fvm.H"
#include "fam.H"
#include "fac.H"
#include "IOdictionary.H"
#include "word.H"
#include "fvMatrices.H"
#include "foamTime.H"
#include "fixedValueFvPatchField.H"

/// To match field names for being/not being advected
//#include <regex>

bool Foam::meshDance::permissibleToAdvect(const word& fieldName)
{
    std::smatch matches;
    std::regex illegal{R"(ddt0+)"};

    if
    (
        std::regex_search(fieldName, matches, illegal)
    )
    {
        Info<< nl
            << "    "
            << fieldName
            << " is hard-coded to not be advected"
            << '\n';

        return false;
    }

    /// Run-time selected names
    forAll(illegalToAdvect_, fieldI)
    {
        if (fieldName == illegalToAdvect_[fieldI])
        {
            Info<< nl
                << "    "
                << fieldName
                << " was selected to not be advected"
                << '\n';

            return false;
        }
    }

    return true;
}

template<class Type>
void Foam::meshDance::registerFields()
{
    word fieldClassName
    (
        GeometricField<Type, fvPatchField, volMesh>::typeName
    );

    //Info<< "Find fields"
    //    << endl;

    IOobjectList fields = fields_.lookupClass(fieldClassName);

    for
    (
        IOobjectList::iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        Info<< "    Reading "
            << fieldIter()->name()
            << endl;
        
        /// Create a temporary field, will be used to create the final 
        /// field by
        /// replacing calculated patches.
        IOobject fieldIOobject
        (
            fieldIter()->name(),
            mesh_.time().timeName(),
            mesh_,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE,
            true // Do not register this IOobject
        );
        
        //GeometricField<Type, fvPatchField, volMesh>* fieldPtr = new
        // Error:
            // Fields with calculated patches struggle to convect
            // The workaround is to read from the disk then change the
            // boundary types

        new GeometricField<Type, fvPatchField, volMesh>
        (
            //*fieldIter(),
            fieldIOobject,
            mesh_
        );
    }
}

template< typename Type, template<typename> typename PatchType, typename MeshType >
void Foam::meshDance::registerFields()
{
    word fieldClassName
    (
        GeometricField<Type, PatchType, MeshType>::typeName
    );

    //Info<< "Find fields"
    //    << endl;

    IOobjectList fields = fields_.lookupClass(fieldClassName);

    for
    (
        IOobjectList::iterator fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        /// Filter out field's name to not be advected
        /// Note: you can use Foam::reg(something) class as well
        //std::smatch matches;
        //std::regex fieldName3 {R"(.*_0.*)"}; 

        //if 
        //(
        //    std::regex_search(fieldIter()-> name(), matches, fieldName3)
        //)
        //{
        //    /// Pass this field, do not advect

        //    Info<< "    "
        //        << "Field "
        //        << fieldIter() -> name()
        //        << " will not be advected."
        //        << endl;
        //}
        //else
        {
            Info<< "    Reading "
                << fieldIter()->name()
                << endl;
            
            /// Create a temporary field, will be used to create the final 
            /// field by
            /// replacing calculated patches.
            IOobject fieldIOobject
            (
                fieldIter()->name(),
                mesh_.time().timeName(),
                mesh_,
                IOobject::MUST_READ,
                IOobject::AUTO_WRITE,
                true // Do not register this IOobject
            );
            
            //GeometricField<Type, fvPatchField, volMesh>* fieldPtr = new
            // Error:
                // Fields with calculated patches struggle to convect
                // The workaround is to read from the disk then change the
                // boundary types

            new GeometricField<Type, PatchType, MeshType>
            (
                //*fieldIter(),
                fieldIOobject,
                mesh_
            );

            //--------------------------------------------------------------
            // Start test block
            //--------------------------------------------------------------
            /// See if the field being read consists of the BC's on the disk
            /// Copy fieldPtr(), resetting IO
            //word dummyFileName = "1000";
            //word dummyFileName = mesh_.time().timeName();
            //
            ////mkDir(dummyFileName);

            //GeometricField<Type, fvPatchField, volMesh> fieldToWrite
            //(
            //    IOobject
            //    (
            //        (fieldPtr -> name() + "_copy"),
            //        dummyFileName,
            //        mesh_,
            //        IOobject::NO_READ,
            //        IOobject::AUTO_WRITE
            //    ),
            //    *fieldPtr
            //);

            //if (fieldToWritePtr -> write())
            //{
            //    Info<< "    The read "
            //        << fieldPtr -> name()
            //        << " written to "
            //        << dummyFileName
            //        << endl;
            //}
            //else
            //{
            //    FatalErrorIn("")
            //        << fieldToWritePtr->name()
            //        << " is not written to "
            //        << dummyFileName
            //        << endl;
            //}
            //--------------------------------------------------------------
            // End test block
            //--------------------------------------------------------------

            //--------------------------------------------------------------
            // Start edit_patches
            //--------------------------------------------------------------

            //Info<< "    Temporary "
            //    << fieldIter() -> name()
            //    << " created."
            //    << endl;

            ////
            //// Create patches
            ////

            //// Method 1:

            //// Errors:
            //    //- For a field like DD, error below is due to the construction of
            //        // some BC's being failed:

            //        // --> FOAM FATAL ERROR: 
            //        // Could not find solidModel_region0
            //        // 
            //        // solidModels in the objectRegistry: 
            //        // 0
            //        // (
            //        // )
            //        // 
            //        // 
            //        // solidModels in the parent objectRegistry:
            //        // 0
            //        // (
            //        // )
            //        // 
            //        // 
            //        //     From function const solidModel& lookupSolidModel(const objectRegistry& obReg)
            //        //     in file solidModels/solidModel/lookupSolidModel.C at line 66.

            ///// Create a list of patch types
            //wordList patchTypes(fieldPtr->boundaryField().size());
            //
            //bool patchTypeChanged = false;

            //forAll(patchTypes, patchI)
            //{
            //    Info<< "        patch "
            //        << patchI 
            //        << " is "
            //        << fieldPtr->boundaryField()[patchI].type()
            //        << endl;

            //    // To work around the error above, force all patch types to be
            //    // fixedVaule
            //    if 
            //    (
            //        fieldPtr->boundaryField()[patchI].type() != word("calculated")
            //     && fieldPtr->boundaryField()[patchI].type() != word("solidContact")
            //     && fieldPtr->boundaryField()[patchI].type() != word("solidTraction")
            //     //&& field.boundaryField()[patchI].type() != word("solidWedge")
            //     //&& field.boundaryField()[patchI].type() 
            //     //!= word("fixedDisplacement")
            //    )
            //    {
            //        patchTypes[patchI] = fieldPtr->boundaryField()[patchI].type(); 
            //    }
            //    else
            //    {
            //        patchTypes[patchI] = word("fixedValue");
            //        patchTypeChanged = true;

            //        Info<< "        "
            //            << fieldPtr->boundaryField()[patchI].type()
            //            << " changed to fixedValue"
            //            << endl;
            //    }
            //}


            ///// Create/Edit field
            //if (patchTypeChanged)
            //{
            //    // Check-out the field with calculated patches, check-in the
            //    // editted field
            //    const_cast<fvMesh&>(mesh_).thisDb().checkOut(*fieldPtr);

            //    // Method 1:
            //    /*GeometricField<Type, fvPatchField, volMesh>**/ fieldPtr = new 
	    //        GeometricField<Type, fvPatchField, volMesh>
            //        (
            //            IOobject
            //            (
            //                fieldIter()->name(),
            //                mesh_.time().timeName(),
            //                mesh_,
            //                IOobject::NO_READ,
            //                IOobject::AUTO_WRITE
            //            ),
            //            *fieldPtr,
            //            patchTypes

            //            // Force fixedValue
            //            //"fixedValue"
            //        );

            //    Info<< "        Check out "
            //        << fieldIter()->name()
            //        << " with with old patch types, check in the editted"
            //        << " version"
            //        << endl;
            //}
            ////else
            ////{
            ////    // Method 2:
            ////    
            ////    ///
            ////    /// Map boundary conditions from `field`
            ////    /// 

            ////    PtrList<fvPatchField<Type> >
            ////        patches(field.boundaryField().size());

            ////    forAll(patches, patchI)
            ////    {
            ////        patches.set
            ////        (
            ////            patchI,
            ////            field.boundaryField()[patchI].clone().ptr()
            ////        );
            ////    }

            ////    // Errors:
            ////        //- For a field like DD, error below is due to the construction of
            ////            // some BC's being failed:

            ////            // --> FOAM FATAL ERROR: 
            ////            // Could not find solidModel_region0
            ////            // 
            ////            // solidModels in the objectRegistry: 
            ////            // 0
            ////            // (
            ////            // )
            ////            // 
            ////            // 
            ////            // solidModels in the parent objectRegistry:
            ////            // 0
            ////            // (
            ////            // )
            ////            // 
            ////            // 
            ////            //     From function const solidModel& lookupSolidModel(const objectRegistry& obReg)
            ////            //     in file solidModels/solidModel/lookupSolidModel.C at line 66.

            ////        //- For fields with calculated patches, e.g., U, 

            ////            // --> FOAM FATAL ERROR: 

            ////            // valueInternalCoeffs cannot be called for a calculatedFvPatchField
            ////            // on patch billetTop of field U in file "/root/foam/root-4.0/run_host/miniFoam/test/meshDanceImp/Test-meshDanceImp/impactBar/impactBar_DALE/4e-05/U"
            ////            // You are probably trying to solve for a field with a default boundary condition.

            ////            // From function calculatedFvPatchField<Type>::valueInternalCoeffs(const tmp<scalarField>&) const
            ////            // in file fields/fvPatchFields/basic/calculated/calculatedFvPatchField.C at line 144.

            ////    // Method 2:
            ////    GeometricField
            ////    <
            ////        Type, fvPatchField, volMesh
            ////    >*
            ////    fieldPtr = new GeometricField
            ////    <
            ////        Type, fvPatchField, volMesh
            ////    >
            ////    (
            ////        IOobject
            ////        (
            ////            fieldIter()->name(),
            ////            mesh_.time().timeName(),
            ////            mesh_,
            ////            IOobject::NO_READ,
            ////            IOobject::AUTO_WRITE
            ////        ),
            ////        mesh_,
            ////        field.dimensions(),
            ////        Field<Type>(mesh_.nCells()),
            ////        patches
            ////    );
            ////}
            //--------------------------------------------------------------
            // End edit_patches
            //--------------------------------------------------------------
        }
    }
}

template<class Type>
void Foam::meshDance::advect(bool writeAdvectedFields)
{
    //typedef GeometricField<Type, fvPatchField, volMesh> fieldType;

    HashTable
    <
        const GeometricField<Type, fvPatchField, volMesh>*
    > fields = mesh_.thisDb().lookupClass
    <
        GeometricField<Type, fvPatchField, volMesh>
    >();

    typename
    HashTable
    <
        const GeometricField<Type, fvPatchField, volMesh>*
    >::iterator fieldIter;

    for
    (
        fieldIter = fields.begin();
        fieldIter !=fields.end();
        ++fieldIter
    )
    {
        GeometricField
	<
	    Type, fvPatchField, volMesh
	>& field = const_cast
	<
        GeometricField
	<
	    Type, fvPatchField, volMesh
	>&
	>(*fieldIter());

        if 
        (
            permissibleToAdvect(field.name())
        )
        {
            /// Create the copy of the field to be convected

	    GeometricField<Type, fvPatchField, volMesh> tfield
            (
                IOobject
                (
                    field.name()+"_tmp",
                    //mesh_.time().timeName(),
					nowName_,
					//"remapSteps/"+mesh_.time().timeName(),
					fileName("remapSteps/"+runTime_.timeName()),
                    //pseudoTime_.timeName(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                field,
                "zeroGradient"
            );

            Info<< "    Temporary copy of "
                << field.name()
                << " created"
                << endl;

	    for (label iter=0; iter < maxIter_; iter++)
	    {
                fvMatrix<Type> fieldEq
                (
                    fluxSign() * fvm::div(fluxSign() * mesh_.phi(), tfield)
                 == fvm::ddt(tfield)
                );

                //TEq.solve();
                Info<<"    "
                    << fieldEq.solve()
                    << endl;
	    }

            Info<< "    " 
                << "Field " 
                << tfield.name()
                //<< fieldPtr->name()
                << " advected."
                << endl;

            /// Do not correct BC's for the temporary copy; it has
            /// not-necessarily-correct zeroGradient on all boundaries
            //tfield.correctBoundaryConditions();

            /// Edit field
            if (field.name() == "D")
            {
                // Answer the run-time request

                // Pointer to live outside this scope
                //GeometricField<tensor, fvPatchField, volMesh>* gradDPtr
                /*= */new GeometricField<tensor, fvPatchField, volMesh> 
                (
                    fvc::grad
                    (
                        mesh_.thisDb()
                        .lookupObject
                        <GeometricField<vector, fvPatchField, volMesh> >("D"),
                        "grad(D)"
                    )
                );
            }
            else if (field.name() == "DD")
            {
                // Answer the run-time request

                // Pointer to live outside this scope
                new GeometricField<tensor, fvPatchField, volMesh> 
                (
                    fvc::grad
                    (
                        mesh_.thisDb()
                        .lookupObject
                        <GeometricField<vector, fvPatchField, volMesh> >("DD"),
                        "grad(DD)"
                    )
                );
            }

            field.internalField() = tfield.internalField();

            Info<< "    "
                << field.name()
                << " internal values updated from "
                << tfield.name()
                << endl;

            // Extrapolate the boundary field from the internal field
            boolList isFixed(field.boundaryField().size(), false);

            forAll(field.boundaryField(), patchI)
            {
                forAll(fixedPatches_, fixedPatchI)
                {
                    if 
                    (
                        mesh_.boundaryMesh()[patchI].name()
                     == fixedPatches_[fixedPatchI]
                    )
                    {
                        isFixed[patchI] = true;

                        break;
                    }

                }
            }

            forAll(field.boundaryField(), patchI)
            {
                if (isFixed[patchI])
                {
                    // Do nothing, i.e., keep the existing boundary values. 
                }
                else
                {
                    field.boundaryField()[patchI] 
                  = field.boundaryField()[patchI].patchInternalField()();
                }
            }

            Info<< "    "
                << field.name()
                << " non-fixed boundary values updated from "
                << "internal values" 
                << "\n"
                << endl;
            
            if (writeAdvectedFields)
            {
                Info<< "    Write "
                    << tfield.name()
                    //<< field.name()
                    << "\n"
                    << endl;

                tfield.write();
                //field.write();
            }
        }
        else
        {
            Info<< "    "
                << "Field "
                << field.name()
                << " will not be advected."
                << nl << endl;
        }
    }
}

template<class Type>
void Foam::meshDance::advects(bool writeAdvectedFields)
{
    //typedef GeometricField<Type, fvsPatchField, surfaceMesh> fieldType;

    HashTable
    <
        const GeometricField<Type, fvsPatchField, surfaceMesh>*
    > fields = mesh_.thisDb().lookupClass
    <
        GeometricField<Type, fvsPatchField, surfaceMesh>
    >();

    typename
    HashTable
    <
        const GeometricField<Type, fvsPatchField, surfaceMesh>*
    >::iterator fieldIter;

    for
    (
        fieldIter = fields.begin();
        fieldIter !=fields.end();
        ++fieldIter
    )
    {
        GeometricField
	<
	    Type, fvsPatchField, surfaceMesh
	>& field = const_cast
	<
        GeometricField
	<
	    Type, fvsPatchField, surfaceMesh
	>&
	>(*fieldIter());

        /// Filter out field's name to not be advected
        /// Note: you can use Foam::reg(something) class as well
        std::smatch matches;
        std::regex fieldName0 {R"(ddt0+)"}; 
        std::regex fieldName1 {R"(grad(D))"}; 
        std::regex fieldName2 {R"(grad(DD))"}; 

        // This pattern matches things like rho_0_0_0 which are created on the
        // fly, not read from the disk, who has calculated patches, for which
        // the advection eq cannot
        // be solved.
        //std::regex fieldName3 {R"(.*_0.*)"}; 

        if 
        (
            std::regex_search(field.name(), matches, fieldName0)
         || std::regex_search(field.name(), matches, fieldName1)
         || std::regex_search(field.name(), matches, fieldName2)
         //|| std::regex_search(field.name(), matches, fieldName3)
        )
        {
            Info<< "    "
                << "Field "
                << field.name()
                << " will not be advected."
                << endl;
        }
        else
        {
            /// Create the copy of the field to be convected

	    GeometricField<Type, fvsPatchField, surfaceMesh> tfield
            (
                IOobject
                (
                    field.name()+"_tmp",
                    mesh_.time().timeName(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::NO_WRITE
                ),
                field,
                "zeroGradient"
            );

            Info<< "    Temporary copy of "
                << field.name()
                << " created"
                << endl;

            //if (mesh_.moving())
            //{
            //    Info<< "    "
            //        << "Mesh is moving"
            //        << endl;
            //}

            GeometricField<Type, fvPatchField, volMesh> tvfield =
                fvc::reconstruct(tfield);

	    for (label iter=0; iter < maxIter_; iter++)
	    {
                fvMatrix<Type> fieldEq
                (
                    fluxSign() * fvm::div(fluxSign() * mesh_.phi(), tvfield)
                 == fvm::ddt(tvfield)
                );

                //TEq.solve();
                Info<<"    "
                    << fieldEq.solve()
                    << endl;
	    }

            Info<< "    " 
                << "Field " 
                << tfield.name()
                //<< fieldPtr->name()
                << " advected."
                << endl;

            /// Do not correct BC's for the temporary copy; it has
            /// not-necessarily-correct zeroGradient on all boundaries
            //tfield.correctBoundaryConditions();

            //Info<< "    " 
            //    << "Boundary condition of " 
            //    << tfield.name()
            //    //<< fieldPtr->name()
            //    << " corrected."
            //    << endl;

            //field.internalField() = tfield.internalField();
            field.internalField() = fvc::interpolate(tvfield).internalField();

            Info<< "    "
                << field.name()
                << " internal values updated from "
                << tfield.name()
                << endl;

            // Extrapolate the boundary field from the internal field
            boolList isFixed(field.boundaryField().size(), false);

            forAll(field.boundaryField(), patchI)
            {
                forAll(fixedPatches_, fixedPatchI)
                {
                    if 
                    (
                        mesh_.boundaryMesh()[patchI].name()
                     == fixedPatches_[fixedPatchI]
                    )
                    {
                        isFixed[patchI] = true;

                        break;
                    }
                }
            }

            forAll(field.boundaryField(), patchI)
            {
                if (isFixed[patchI])
                {
                    // Do nothing, i.e., keep the existing boundary values. 
                }
                else
                {
                    field.boundaryField()[patchI] 
                  = field.boundaryField()[patchI].patchInternalField()();
                }
            }

            Info<< "    "
                << field.name()
                << " non-fixed boundary values updated from "
                << "internal values" 
                << "\n"
                << endl;
            
            //TPtr().correctBoundaryConditions();

            // For solidContact, errors: solidModel not found
            //field.correctBoundaryConditions();
            //fieldPtr->correctBoundaryConditions();
            
            /// De-register old time objects created by Crank-Nicklson
            //if 
            //(
            //    mesh_.thisDb().foundObject<fieldType>
            //    (word(field.name() + "_0"))
            //)
            //{
            //    const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
            //    (
            //        const_cast<fieldType&>
            //        (
            //            mesh_.thisDb().lookupObject<fieldType>
            //            (word(field.name() + "_0"))
            //        )
            //    );
            //}
            
            //if 
            //(
            //    mesh_.thisDb().foundObject<fieldType>
            //    (word(field.name() + "_0_0"))
            //)
            //{
            //    const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
            //    (
            //        const_cast<fieldType&>
            //        (
            //            mesh_.thisDb().lookupObject<fieldType>
            //            (word(field.name() + "_0_0"))
            //        )
            //    );
            //}

            if (writeAdvectedFields)
            {
                Info<< "    Write "
                    << field.name()
                    << "\n"
                    << endl;

                field.write();
            }
        }
    }
}

/// Read ALEDict or throw error if key not found.
template<class T>
void Foam::meshDance::read
(
    const dictionary& dict,
    const word key,
    T& value
)
{
    if (dict.found(key))
    {
        dict.readIfPresent<T>(key, value);
    }
    else
    {
        FatalErrorIn("")
            << "Keyword "
            << key
            << " was not found in "
            << dict.name()
            << '\n';
    }
}

/// Conitional key-reading
template<class T>
void Foam::meshDance::ifRead
(
    const bool condition,
    const dictionary& dict,
    const word key,
    T& value
)
{
    if (condition)
    {
        read(dict, key, value);
    }
}

        //--------------------------------------------------------------
        // Error documentation: registerFields<T>
        //--------------------------------------------------------------
        // Start test block
        //--------------------------------------------------------------
        /// See if the field being read consists of the BC's on the disk
        /// Copy fieldPtr(), resetting IO
        //word dummyFileName = "1000";
        //word dummyFileName = mesh_.time().timeName();
        //
        ////mkDir(dummyFileName);

        //GeometricField<Type, fvPatchField, volMesh> fieldToWrite
        //(
        //    IOobject
        //    (
        //        (fieldPtr -> name() + "_copy"),
        //        dummyFileName,
        //        mesh_,
        //        IOobject::NO_READ,
        //        IOobject::AUTO_WRITE
        //    ),
        //    *fieldPtr
        //);

        //if (fieldToWritePtr -> write())
        //{
        //    Info<< "    The read "
        //        << fieldPtr -> name()
        //        << " written to "
        //        << dummyFileName
        //        << endl;
        //}
        //else
        //{
        //    FatalErrorIn("")
        //        << fieldToWritePtr->name()
        //        << " is not written to "
        //        << dummyFileName
        //        << endl;
        //}
        //--------------------------------------------------------------
        // End test block
        //--------------------------------------------------------------

        //--------------------------------------------------------------
        // Start edit_patches
        //--------------------------------------------------------------

        //Info<< "    Temporary "
        //    << fieldIter() -> name()
        //    << " created."
        //    << endl;

        ////
        //// Create patches
        ////

        //// Method 1:

        //// Errors:
        //    //- For a field like DD, error below is due to the construction of
        //        // some BC's being failed:

        //        // --> FOAM FATAL ERROR: 
        //        // Could not find solidModel_region0
        //        // 
        //        // solidModels in the objectRegistry: 
        //        // 0
        //        // (
        //        // )
        //        // 
        //        // 
        //        // solidModels in the parent objectRegistry:
        //        // 0
        //        // (
        //        // )
        //        // 
        //        // 
        //        //     From function const solidModel& lookupSolidModel(const objectRegistry& obReg)
        //        //     in file solidModels/solidModel/lookupSolidModel.C at line 66.

        ///// Create a list of patch types
        //wordList patchTypes(fieldPtr->boundaryField().size());
        //
        //bool patchTypeChanged = false;

        //forAll(patchTypes, patchI)
        //{
        //    Info<< "        patch "
        //        << patchI 
        //        << " is "
        //        << fieldPtr->boundaryField()[patchI].type()
        //        << endl;

        //    // To work around the error above, force all patch types to be
        //    // fixedVaule
        //    if 
        //    (
        //        fieldPtr->boundaryField()[patchI].type() != word("calculated")
        //     && fieldPtr->boundaryField()[patchI].type() != word("solidContact")
        //     && fieldPtr->boundaryField()[patchI].type() != word("solidTraction")
        //     //&& field.boundaryField()[patchI].type() != word("solidWedge")
        //     //&& field.boundaryField()[patchI].type() 
        //     //!= word("fixedDisplacement")
        //    )
        //    {
        //        patchTypes[patchI] = fieldPtr->boundaryField()[patchI].type(); 
        //    }
        //    else
        //    {
        //        patchTypes[patchI] = word("fixedValue");
        //        patchTypeChanged = true;

        //        Info<< "        "
        //            << fieldPtr->boundaryField()[patchI].type()
        //            << " changed to fixedValue"
        //            << endl;
        //    }
        //}


        ///// Create/Edit field
        //if (patchTypeChanged)
        //{
        //    // Check-out the field with calculated patches, check-in the
        //    // editted field
        //    const_cast<fvMesh&>(mesh_).thisDb().checkOut(*fieldPtr);

        //    // Method 1:
        //    /*GeometricField<Type, fvPatchField, volMesh>**/ fieldPtr = new 
        //        GeometricField<Type, fvPatchField, volMesh>
        //        (
        //            IOobject
        //            (
        //                fieldIter()->name(),
        //                mesh_.time().timeName(),
        //                mesh_,
        //                IOobject::NO_READ,
        //                IOobject::AUTO_WRITE
        //            ),
        //            *fieldPtr,
        //            patchTypes

        //            // Force fixedValue
        //            //"fixedValue"
        //        );

        //    Info<< "        Check out "
        //        << fieldIter()->name()
        //        << " with with old patch types, check in the editted"
        //        << " version"
        //        << endl;
        //}
        ////else
        ////{
        ////    // Method 2:
        ////    
        ////    ///
        ////    /// Map boundary conditions from `field`
        ////    /// 

        ////    PtrList<fvPatchField<Type> >
        ////        patches(field.boundaryField().size());

        ////    forAll(patches, patchI)
        ////    {
        ////        patches.set
        ////        (
        ////            patchI,
        ////            field.boundaryField()[patchI].clone().ptr()
        ////        );
        ////    }

        ////    // Errors:
        ////        //- For a field like DD, error below is due to the construction of
        ////            // some BC's being failed:

        ////            // --> FOAM FATAL ERROR: 
        ////            // Could not find solidModel_region0
        ////            // 
        ////            // solidModels in the objectRegistry: 
        ////            // 0
        ////            // (
        ////            // )
        ////            // 
        ////            // 
        ////            // solidModels in the parent objectRegistry:
        ////            // 0
        ////            // (
        ////            // )
        ////            // 
        ////            // 
        ////            //     From function const solidModel& lookupSolidModel(const objectRegistry& obReg)
        ////            //     in file solidModels/solidModel/lookupSolidModel.C at line 66.

        ////        //- For fields with calculated patches, e.g., U, 

        ////            // --> FOAM FATAL ERROR: 

        ////            // valueInternalCoeffs cannot be called for a calculatedFvPatchField
        ////            // on patch billetTop of field U in file "/root/foam/root-4.0/run_host/miniFoam/test/meshDanceImp/Test-meshDanceImp/impactBar/impactBar_DALE/4e-05/U"
        ////            // You are probably trying to solve for a field with a default boundary condition.

        ////            // From function calculatedFvPatchField<Type>::valueInternalCoeffs(const tmp<scalarField>&) const
        ////            // in file fields/fvPatchFields/basic/calculated/calculatedFvPatchField.C at line 144.

        ////    // Method 2:
        ////    GeometricField
        ////    <
        ////        Type, fvPatchField, volMesh
        ////    >*
        ////    fieldPtr = new GeometricField
        ////    <
        ////        Type, fvPatchField, volMesh
        ////    >
        ////    (
        ////        IOobject
        ////        (
        ////            fieldIter()->name(),
        ////            mesh_.time().timeName(),
        ////            mesh_,
        ////            IOobject::NO_READ,
        ////            IOobject::AUTO_WRITE
        ////        ),
        ////        mesh_,
        ////        field.dimensions(),
        ////        Field<Type>(mesh_.nCells()),
        ////        patches
        ////    );
        ////}
        //--------------------------------------------------------------
        // End edit_patches
        //--------------------------------------------------------------
