template<class Type>
void CreateFields
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

    for
    (
        IOobjectList::iterator fieldIter = fields.begin();
        fieldIter !=fields.end();
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
            mesh.time().timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false // Do not register this IOobject
        );
        
        GeometricField<Type, fvPatchField, volMesh> field
        (
            //*fieldIter(),
            fieldIOobject,
            mesh
        );
        
        /// Create a list of patch types
        wordList patchTypes(field.boundaryField().size());
        
        //PtrList<fvPatchField<scalar> > ptrListPatchFieldType
        //(field.boundaryField().size());
        
        forAll(patchTypes, patchI)
        {
            if (field.boundaryField()[patchI].type() != word("calculated"))
            {
                patchTypes[patchI] = field.boundaryField()[patchI].type(); 
            }
            else
            {
                patchTypes[patchI] = word("zeroGradient");
            }
        }
        
        //fieldIOobject.readOpt()= IOobject::NO_READ;
        //
        //fieldIOobject.writeOpt() = IOobject::AUTO_WRITE;
        
        //GeometricField<Type, fvPatchField, volMesh>* fieldPtr
        //(
        //    new GeometricField<Type, fvPatchField, volMesh>
        //    (
        //        //*fieldIter(),
        //        fieldIOobject,
        //        field,
        //        //mesh
        //        patchTypes
        //    )
        //);

        GeometricField<Type, fvPatchField, volMesh>* fieldPtr = new 
	    GeometricField<Type, fvPatchField, volMesh>
            (
                //*fieldIter(),
                //fieldIOobject,
                IOobject
                (
                    fieldIter()->name(),
                    mesh.time().timeName(),
                    mesh,
                    IOobject::NO_READ,
                    IOobject::AUTO_WRITE
                ),
                field,
                //mesh
                patchTypes
            );

        //GeometricField<Type, fvPatchField, volMesh> tField = *fieldPtr;

        Info<< "mesh.toc() after create field pointer:"
            << mesh.toc()
            << endl;
           
	//const_cast<objectRegistry&>(mesh.thisDb()).checkIn(*fieldPtr);

        //Info<< "mesh.toc() after check in:"
        //    << mesh.toc()
        //    << endl;
          
    }
}
