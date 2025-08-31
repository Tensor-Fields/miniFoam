#include "fvc.H"
#include "fvm.H"
#include "fvMatrices.H"
#include "foamTime.H"

/// To match field names for being/not being advected
#include <regex>

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
            mesh_.time().timeName(),
            mesh_,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false // Do not register this IOobject
        );
        
        GeometricField<Type, fvPatchField, volMesh> field
        (
            //*fieldIter(),
            fieldIOobject,
            mesh_
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
                    mesh_.time().timeName(),
                    mesh_,
                    IOobject::NO_READ,
                    IOobject::AUTO_WRITE
                ),
                field,
                //mesh_
                patchTypes
            );

        //GeometricField<Type, fvPatchField, volMesh> tField = *fieldPtr;

        Info<< "mesh_.toc() after create field pointer:"
            << mesh_.toc()
            << endl;
           
	//const_cast<objectRegistry&>(mesh.thisDb()).checkIn(*fieldPtr);

        //Info<< "mesh.toc() after check in:"
        //    << mesh.toc()
        //    << endl;
          
    }
}

template<class Type>
void Foam::meshDance::advect()
{
    typedef GeometricField<Type, fvPatchField, volMesh> fieldType;

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

        /// Filter out field's name to not be advected
        /// Note: you can use Foam::reg(something) class as well
        std::smatch matches;
        std::regex fieldName {R"(ddt0+)"}; 

        if 
        (
            std::regex_search(field.name(), matches, fieldName)
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
            //tmp<scalarField>& sweptVol = 
            //    mesh_.movePoints
            //    (
            //        incrementPoints
            //        (
            //            mesh_,
            //            totalMove
            //        )
            //    );

            //volScalarField& meshPhi = fvc::surfaceIntegrate(mesh.phi())();

            //fvMatrix<Type>& fieldEq // Error

            if (mesh_.moving())
            {
                Info<< "    "
                    << "Mesh is moving"
                    << endl;
            }

	    for (label iter=0; iter < maxIter_; iter++)
	    {
                fvMatrix<Type> fieldEq
                (
                    fluxSign() * fvm::div(fluxSign() * mesh_.phi(), field)
                    //fvm::div(/*mesh.*/phi/*()*/, field)
                    //fvm::div
                    //(
                    //    mesh.updatePhi
                    //    (
                    //        sweptVol
                    //    ), 
                    //    field
                    //)
                 == fvm::ddt(field)
                );

                /// Found faulty, as fvm::ddt multiplies field by cell volume
                //if(ddtTV)
                //{

                //    fvScalarMatrix fieldEq
                //    (
                //        fvm::div(mesh.phi(), field)
                //        //fvm::div(/*mesh.*/phi/*()*/, field)
                //        //fvm::div
                //        //(
                //        //    mesh.updatePhi
                //        //    (
                //        //        sweptVol
                //        //    ), 
                //        //    field
                //        //)
                //     == fvm::ddt(mesh.V(), field)
                //    );

                //    //fieldEq = fieldEq2;
                //}

                //Info<< "    "
                //    << "fieldEq "
                //    << "created."
                //    << endl;

                //TEq.solve();
                Info<<"    "
                    << fieldEq.solve()
                    << endl;
	    }

            Info<< "    " 
                << "Field " 
                << field.name()
                //<< fieldPtr->name()
                << " advected."
                << endl;
            
            //TPtr().correctBoundaryConditions();
            field.correctBoundaryConditions();
            //fieldPtr->correctBoundaryConditions();
            
            Info<< "    " 
                << "Boundary condition of " 
                << field.name()
                //<< fieldPtr->name()
                << " corrected."
                << endl;

            /// De-register old time objects created by Crank-Nicklson
            if 
            (
                mesh_.thisDb().foundObject<fieldType>
                (word(field.name() + "_0"))
            )
            {
                const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
                (
                    const_cast<fieldType&>
                    (
                        mesh_.thisDb().lookupObject<fieldType>
                        (word(field.name() + "_0"))
                    )
                );
            }
            
            if 
            (
                mesh_.thisDb().foundObject<fieldType>
                (word(field.name() + "_0_0"))
            )
            {
                const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
                (
                    const_cast<fieldType&>
                    (
                        mesh_.thisDb().lookupObject<fieldType>
                        (word(field.name() + "_0_0"))
                    )
                );
            }
        }
    }
}
