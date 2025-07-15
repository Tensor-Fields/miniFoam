#include "meshDance.H"
#include "GeometricField.H"
#include "fvPatchField.H"
#include "volMesh.H"

///
/// Constructors
///

Foam::meshDance::meshDance(const Time& runTime) 
/// \note: Initializing mesh_
/// with an fvMesh parameter is private to fvMesh.
:
mesh_
(
    IOobject
    (
        polyMesh::defaultRegion,
        runTime,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    )
),
fields_(mesh_, runTime.timeName()),
startingPoints_(mesh_.points()),
updatedStartingPoints_(mesh_.points()),
overallPointMotion_(pointField(mesh_.points().size(), vector::zero))
{
    //startingPoints_ = mesh_.points(); // Error

    /// Create IOobjects from existing field files
    //IOobjectList fields(mesh_, runTime.timeName());

    registerFields<scalar>();
    registerFields<vector>();
    registerFields<tensor>();
    registerFields<symmTensor>();
    registerFields<sphericalTensor>();
}

///
/// Members
///

void Foam::meshDance::setOverallPointMotion
(
    const pointField& start,
    const pointField& finish
)
{
    const_cast<pointField&>(overallPointMotion_) =  (finish - start);
}

/// Time-step adaptive point increment
void Foam::meshDance::stepTowards
(
    const dimensionedScalar& startTime
)
{
    mesh_.movePoints
    (
        mesh_.points() + 
        (
            mesh_.time().deltaT().value()
            /
            (
                mesh_.time().endTime().value() 
              //- mesh_.time().startTime().value()
              - startTime.value()
            )
            *
            overallPointMotion_
        )
    );      
}

/// Move the mesh back to its initial config, incrementally
void Foam::meshDance::cyclicStep
(const dimensionedScalar& startTime)
{
    mesh_.movePoints
    (
        mesh_.points() + 
        (
            mesh_.time().deltaT().value() 
            /
            (
                mesh_.time().endTime().value() 
              //- mesh_.time().startTime().value()
              - startTime.value()
            ) 
            * 
            overallPointMotion_
        )
    );
}

//template<class Type>
//void Foam::meshDance::advect()
//{
//    typedef GeometricField<Type, fvPatchField, volMesh> fieldType;
//
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    > fields = mesh_.thisDb().lookupClass
//    <
//        GeometricField<Type, fvPatchField, volMesh>
//    >();
//
//    typename
//    HashTable
//    <
//        const GeometricField<Type, fvPatchField, volMesh>*
//    >::iterator fieldIter;
//
//    for
//    (
//        fieldIter = fields.begin();
//        fieldIter !=fields.end();
//        ++fieldIter
//    )
//    {
//
//        GeometricField
//	<
//	    Type, fvPatchField, volMesh
//	>& field = const_cast
//	<
//        GeometricField
//	<
//	    Type, fvPatchField, volMesh
//	>&
//	>(*fieldIter());
//
//        /// Filter out field's name to not be advected
//        /// Note: you can use Foam::reg(something) class as well
//        std::smatch matches;
//        std::regex fieldName {R"(ddt0+)"}; 
//
//        if 
//        (
//            std::regex_search(field.name(), matches, fieldName)
//        )
//        {
//            Info<< "    "
//                << "Field "
//                << field.name()
//                << " will not be advected."
//                << endl;
//        }
//        else
//        {
//            //tmp<scalarField>& sweptVol = 
//            //    mesh_.movePoints
//            //    (
//            //        incrementPoints
//            //        (
//            //            mesh_,
//            //            totalMove
//            //        )
//            //    );
//
//            //volScalarField& meshPhi = fvc::surfaceIntegrate(mesh.phi())();
//
//            //fvMatrix<Type>& fieldEq // Error
//
//            if (mesh_.moving())
//            {
//                Info<< "    "
//                    << "Mesh is moving"
//                    << endl;
//            }
//
//            fvMatrix<Type> fieldEq
//            (
//                fvm::div(mesh_.phi(), field)
//                //fvm::div(/*mesh.*/phi/*()*/, field)
//                //fvm::div
//                //(
//                //    mesh.updatePhi
//                //    (
//                //        sweptVol
//                //    ), 
//                //    field
//                //)
//             == fvm::ddt(field)
//            );
//
//            /// Found faulty, as fvm::ddt multiplies field by cell volume
//            //if(ddtTV)
//            //{
//
//            //    fvScalarMatrix fieldEq
//            //    (
//            //        fvm::div(mesh.phi(), field)
//            //        //fvm::div(/*mesh.*/phi/*()*/, field)
//            //        //fvm::div
//            //        //(
//            //        //    mesh.updatePhi
//            //        //    (
//            //        //        sweptVol
//            //        //    ), 
//            //        //    field
//            //        //)
//            //     == fvm::ddt(mesh.V(), field)
//            //    );
//
//            //    //fieldEq = fieldEq2;
//            //}
//
//            Info<< "    "
//                << "fieldEq "
//                << "created."
//                << endl;
//
//            //TEq.solve();
//            fieldEq.solve();
//
//            Info<< "    " 
//                << "Field " 
//                << field.name()
//                //<< fieldPtr->name()
//                << " advected."
//                << endl;
//            
//            //TPtr().correctBoundaryConditions();
//            field.correctBoundaryConditions();
//            //fieldPtr->correctBoundaryConditions();
//            
//            Info<< "    " 
//                << "Boundary condition of " 
//                << field.name()
//                //<< fieldPtr->name()
//                << " corrected."
//                << endl;
//
//            /// De-register old time objects created by Crank-Nicklson
//            if 
//            (
//                mesh_.thisDb().foundObject<fieldType>
//                (word(field.name() + "_0"))
//            )
//            {
//                const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
//                (
//                    const_cast<fieldType&>
//                    (
//                        mesh_.thisDb().lookupObject<fieldType>
//                        (word(field.name() + "_0"))
//                    )
//                );
//            }
//            
//            if 
//            (
//                mesh_.thisDb().foundObject<fieldType>
//                (word(field.name() + "_0_0"))
//            )
//            {
//                const_cast<objectRegistry&>(mesh_.thisDb()).checkOut
//                (
//                    const_cast<fieldType&>
//                    (
//                        mesh_.thisDb().lookupObject<fieldType>
//                        (word(field.name() + "_0_0"))
//                    )
//                );
//            }
//        }
//    }
//}

void Foam::meshDance::write()
{
    mesh_.write();
}

void Foam::meshDance::resetPoints()
{
    mesh_.movePoints(startingPoints_);
}

void Foam::meshDance::resetStartingPoints(const pointField& pf)
{
    const_cast<pointField&>(updatedStartingPoints_) = pf;
}
