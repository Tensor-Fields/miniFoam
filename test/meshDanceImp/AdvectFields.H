/// To match field names for being/not being advected
#include <regex>

template<class Type>
void AdvectFields
(
    const fvMesh& mesh,
    const bool flip
    //const surfaceScalarField& phi,
    //const vectorField& totalMove=vector::zero,
    //bool ddtTV=false,
    //const word& smoothMeshDir="smoothMesh"
)
{
    typedef GeometricField<Type, fvPatchField, volMesh> fieldType;

    HashTable
    <
        const GeometricField<Type, fvPatchField, volMesh>*
    > fields = mesh.thisDb().lookupClass
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
            //    mesh.movePoints
            //    (
            //        incrementPoints
            //        (
            //            mesh,
            //            totalMove
            //        )
            //    );

            //volScalarField& meshPhi = fvc::surfaceIntegrate(mesh.phi())();

            //fvMatrix<Type>& fieldEq // Error

            if (mesh.moving())
            {
                Info<< "    "
                    << "Mesh is moving"
                    << endl;
            }

            fvMatrix<Type> fieldEq
            (
                fluxSign(flip) * fvm::div(fluxSign(flip) * mesh.phi(), field)
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

            Info<< "    "
                << "fieldEq "
                << "created."
                << endl;

            //TEq.solve();
            fieldEq.solve();

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
                mesh.thisDb().foundObject<fieldType>
                (word(field.name() + "_0"))
            )
            {
                const_cast<objectRegistry&>(mesh.thisDb()).checkOut
                (
                    const_cast<fieldType&>
                    (
                        mesh.thisDb().lookupObject<fieldType>
                        (word(field.name() + "_0"))
                    )
                );
            }
            
            if 
            (
                mesh.thisDb().foundObject<fieldType>
                (word(field.name() + "_0_0"))
            )
            {
                const_cast<objectRegistry&>(mesh.thisDb()).checkOut
                (
                    const_cast<fieldType&>
                    (
                        mesh.thisDb().lookupObject<fieldType>
                        (word(field.name() + "_0_0"))
                    )
                );
            }
        }
    }
}
