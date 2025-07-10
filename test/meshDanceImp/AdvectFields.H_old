template<class Type>
void AdvectFields
(
    /*const*/ fvMesh& mesh
)
{
    //Info<< "AdvectFields started; mesh.toc() = "
    //    << mesh.toc()
    //    << endl;

    /** Read fields from the first time step */
    HashTable
    <
        const GeometricField<Type, fvPatchField, volMesh>*
    >
    fields
    (
        mesh.thisDb().objectRegistry::template lookupClass 
        <
            GeometricField<Type, fvPatchField, volMesh>
        > ()
    );

    typename 
    HashTable
    <
        const GeometricField<Type, fvPatchField, volMesh>*
    >::iterator fieldIter;

    ///** Read the first field */
    //GeometricField<Type, fvPatchField, volMesh>& field =
    //const_cast
    //<
    //    GeometricField<Type, fvPatchField, volMesh>&
    //>(*fields.begin()());

    ///** Read mesh from the first field */
    //fvMesh& mesh = field.parent();

    /** Read time from mesh */
    const Time& runTime = mesh.time(); // From objectRegistry.H

    for
    (
        fieldIter = fields.begin();
        fieldIter != fields.end();
        ++fieldIter
    )
    {
        //autoPtr<GeometricField<Type, fvPatchField, volMesh> >& field =
        autoPtr<GeometricField<Type, fvPatchField, volMesh> > fieldPtr
        (
            const_cast
            <
                GeometricField<Type, fvPatchField, volMesh>*
            >(fieldIter())
        );

        Info<< "Field "
            << fieldPtr().name()
            << " initialized."
            << endl;

        //fvScalarMatrix TEq
        fvScalarMatrix fieldEq
        (
            //fvm::ddt(TPtr()) == /*fvc::ddt(TPtr()) - fvm::div(phi, TPtr()) +*/ fvm::div(mesh.phi(), TPtr())
            //fvm::ddt(field) == /*fvc::ddt(TPtr()) - fvm::div(phi, TPtr()) +*/ fvm::div(mesh.phi(), field)
		    fvm::div(mesh.phi(), fieldPtr())
         == fvm::ddt(fieldPtr()) /*fvc::ddt(TPtr()) - fvm::div(phi, TPtr()) +*/ 
        );

        Info<< "fieldEq "
            << "created."
            << endl;

        //TEq.solve();
        fieldEq.solve();

        Info<< "Field " 
            //<< field().name()
            << fieldPtr().name()
            << " advected."
            << endl;

        //TPtr().correctBoundaryConditions();
        //field.correctBoundaryConditions();
        fieldPtr().correctBoundaryConditions();

        Info<< "Boundary condition of " 
            //<< field().name()
            << fieldPtr().name()
            << " corrected."
            << endl;

        //field.write();

        //Info<< field.name()
        //    << " written to " 
        //    << runTime.timeName()
        //    << endl;

        // Overwrite field
        mesh.set(fieldPtr().name(), fieldPtr.ptr());

        Info<< fieldPtr().name()
            << " registered with 'mesh'."
            << endl;
    }
}
