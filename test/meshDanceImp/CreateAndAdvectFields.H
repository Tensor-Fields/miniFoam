#include "IOobjectList.H"

template<class Type>
void CreateAndAdvectFields
(
    const IOobjectList& objects,
    const fvMesh& mesh
)
{
    word fieldClassName
    (
        GeometricField<Type, fvPatchField, volMesh>::typeName
    );

    Info<< "Find fields"
        << endl;

    IOobjectList fields = objects.lookupClass(fieldClassName);

    /// \note
    /// List of fields needs you to create all fields once, or one field N 
    /// times 
    /// and then overwrite them with
    /// their actual properties, i.e., their respective IO, through 
    /// `for loop`. The 
    /// alternatives are HashTable and PtrList.

    /// Create an empty list of fields
    //List
    //<
    //    GeometricField<Type, fvPatchField, volMesh> 
    //    // Error: The constructor GeometricField<>() does not exist
    //    GeometricField<Type, fvPatchField, volMesh>
    //      (*fieldStart(), mesh)       
    //    // Not tested; assuming that fieldStart is defined.
    //>

    /// Create an empty HashTable of fields
    //HashTable <GeometricField<Type, fvPatchField, volMesh> > 

    /// Create an empty HashTable of fields
    /// PtrList also can take the advantage of forAll
    /// But has the same redundancy as List, i.e., you need to construct a 
    /// fieldStart as a place holder when creating the list, and
    /// then overwrite, or you get `error: hanging pointer, cannot 
    /// dereference`
    /// PtrList can be used by `set` member function to avoid `[]` operator 
    /// when assigning an element of the list to avoid
    /// hanging pointr error (not checked).
    
    //PtrList< GeometricField<Type, fvPatchField, volMesh> > 

    //List< GeometricField<Type, fvPatchField, volMesh>* >  
    // Exchanged for List<autoPtr<...>>

    //Info<< "Create list of field pointers"
    //<< endl;

    //List< autoPtr<GeometricField<Type, fvPatchField, volMesh> > >  
    // Exchanged for List<autoPtr<...>>

    //List< GeometricField<Type, fvPatchField, volMesh>* >  
    //fieldPtrList(fields.size());
    //fieldList(fields.size());

    /// Create fields
    //label fieldI = 0;

    for
    (
        IOobjectList::iterator fieldIter = fields.begin();
        fieldIter !=fields.end();
        ++fieldIter
    )
    {
        //Info<< "*fieldIter():"
        //    << nl
        //<< *fieldIter()
        //<< endl;
        
        //Info<< "*fieldIter().readOpt:"
        //    << fieldIter()->readOpt()
        //<< nl
        //    << "*fieldIter().writeOpt:"
        //    << fieldIter()->writeOpt()
        //<< endl;
        
        //Info<< "fieldIOobject created"
        //    << endl;
            
        /// Create a temporary field, will be used to create the final 
        /// field by
        /// replacing calculated patches.
        IOobject fieldIOobject
        (
            fieldIter()->name(),
            mesh.time().timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );
        
        GeometricField<Type, fvPatchField, volMesh> field
        (
            *fieldIter(),
            //fieldIOobject,
            mesh
        );
        
        /// Create a list of patch types
        wordList patchTypes(field.boundaryField().size());
        
        //PtrList<fvPatchField<scalar> > ptrListPatchFieldType
        //(field.boundaryField().size());
        
        forAll(patchTypes, patchI)
        {
            //patchTypes[patchI] = mesh.boundaryMesh()[patchI].name(); 
            // Returns patch names, not 
            // types
            // E.g., for pressure, 
            // returns wall instead of zeroGradient
        
            //patchTypes[patchI] = mesh.boundaryMesh()[patchI].type(); 
        
            if (field.boundaryField()[patchI].type() != word("calculated"))
            {
                patchTypes[patchI] = field.boundaryField()[patchI].type(); 
            }
            else
            {
                patchTypes[patchI] = word("zeroGradient");
            }
        
            //ptrListPatchFieldType.set
            //(
            //    patchI, 
            //    new fvPatchField<Type> 
            //    (
            //mesh.boundary()[patchI],
            //field
            ////p.internalField()
            //    )
            //); 
        
            //Info<< "patchTypes[patchI] = "
            //    << patchTypes[patchI] << endl;
        }
        
        //if (fieldIOobject.headerOk())
        
        Info<< "    Reading "
            << fieldIter()->name()
            << endl;
        
        //fieldList[fieldI] = // if it is a List
        //fieldList.insert    // if it is a HashTable
        //(
        //    fieldIter()->name(),
        //    GeometricField<Type, fvPatchField, volMesh> //field
        //    (
        //*fieldIter(),
        //mesh
        //    )
        //);
        
        //fieldList[fieldI] = // 
        // if it is a PtrList 
        // (hanging pointer error); alternatively, fieldList.set(...)
        
        /// The resultant advected field needs not to be MUST_READ,
        /// so, we do not use *fieldIter() IOobject.
        fieldIOobject.readOpt()= IOobject::NO_READ;
        
        fieldIOobject.writeOpt() = IOobject::AUTO_WRITE;
        
        GeometricField<Type, fvPatchField, volMesh>* fieldPtr
        (
            new GeometricField<Type, fvPatchField, volMesh>
            (
                //*fieldIter(),
                fieldIOobject,
                field,
                //mesh
                patchTypes
            )
        );
        
        //fieldPtrList[fieldI] = new 
        //    GeometricField<Type, fvPatchField, volMesh> 
            //    (
            //*fieldIter(),
            //mesh
            //    );
        
        //autoPtr< GeometricField<Type, fvPatchField, volMesh> >
        //(
        //    new GeometricField<Type, fvPatchField, volMesh> 
        //    (
        //*fieldIter(),
        //mesh
        //    )
        //);
            
            //++fieldI;
        
            /// Advect fields
            //forAll(fieldList, fieldI)
            //forAll(fieldPtrList, fieldI)
            /// ddt(*fieldPtr) is not understandable for the compiler, so I
            /// create a copy
            //GeometricField<Type, fvPatchField, volMesh> 
            //fieldCopy = *fieldPtrList[fieldI];
        
            fvScalarMatrix fieldEq
            (
                fvm::div(mesh.phi(), *(fieldPtr))
             == fvm::ddt(*(fieldPtr))
            );
        
            Info<< "fieldEq "
        << "created."
        << endl;
        
            //TEq.solve();
            fieldEq.solve();
        
            Info<< "Field " 
        //<< field().name()
        << fieldPtr->name()
        << " advected."
        << endl;
        
            //TPtr().correctBoundaryConditions();
            //field.correctBoundaryConditions();
            fieldPtr->correctBoundaryConditions();
        
            Info<< "Boundary condition of " 
        //<< field().name()
        << fieldPtr->name()
        << " corrected."
        << endl;
        
        Info<< "fieldPtr->sizr():"
            << fieldPtr->size()
        << endl;
        
            if (mesh.time().write())
            {
        fieldPtr->write();
        
        Info<< fieldPtr->name()
            << " written to " 
            << mesh.time().timeName()
            << endl;
            }
    }
}
