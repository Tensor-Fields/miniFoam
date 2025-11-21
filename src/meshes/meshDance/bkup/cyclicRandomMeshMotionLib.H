/**
  * A class for incrementally moving meshes, given the final mesh,
  * optionally cyclic mesh motion. Specifically useful for advective remap
  * of a discrete rezoning ALE procedure, DALE.
  */

class meshDance
{
    /// Private
    fvMesh mesh_;

    const pointField startingPoints_;

    const word meshTargetDir_ = "meshTarget";

    /// Public
    public:

        /// Constructor
        meshDance(const fvMesh& mesh);

        /// Members

        /// Incrementally change the mesh towards the final points
        stepTowards(const pointField& finalPoints);

        /// Advect fields, driven by the most recent mesh motion
        void template<class T>
        advect();
};

Foam::meshDance::meshDance(const fvMesh& mesh)
:
mesh_(mesh),
startingPoints_(mesh.points())
{}

/// Time-step adaptive point increment
void Foam::meshDance::stepTowards(const& pointField& finalPoints)
{
    mesh_.movePoints
    (
        mesh_.points() + mesh_.time().deltaT().value() /
            (
                mesh_.time().endTime().value() 
              - mesh_.time().startTime().value()
            )
    );      * (finalPoints - startingPoints_); // Total motion vector
}

/// To match field names for being/not being advected
#include <regex>

template<class T>
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

            fvMatrix<Type> fieldEq
            (
                fvm::div(mesh_.phi(), field)
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


/**
  * A driver for random, cyclic mesh motion + advective remap.
  */

#include "argList.H"
#include "meshDance.H"
#include "pointGenerator.H"

int main(int argc, char* argv[])
{
    argList::validOption().add("M", "Number of meshes");
    argList::validOption().add("alpha", "Amplitude");
    argList::validOption().add("nY", "N cells in Y");

    argList args(argc, argv);

    /// Number of intermediate random meshes is M-1
    /// The Mth deformed mesh is the first mesh
    const label M = readScalar(IStringStream(args.option["M"]))();
    const label alpha = readScalar(IStringStream(args.option["alpha"]))();
    const label nY = readScalar(IStringStream(args.option["nY"]))();

    /// Create the dancing mesh
    meshDance    mesh(/*write intermediate mesh/fields, default: false*/);

    /// Save the initial mesh before change
    const pointField& firstPoints = mesh.points();

    /// Loop over random meshes
    for (const label meshi= 0; meshi < M; ++meshi)
    {
        /// If !last_mesh
        /// Create next random mesh
        if (meshCounter < M)
        {
            /// Final random points of the mesh at the step meshi
            const pointField& rp = pointGenerator(mesh.points(), alpha/nY);

            /// Re-set time
            runTime.setTime(startTime, 0);

            /// Fictitious time loop for advective remap
            while (runTime.run())
            {
                #include "readTimeControls.H"

                #include "meshCourantNo.H"

                scalar& CoNum = meshCoNum;

                /// Re-set time step
                /// Place it inside time loop for online interaction with Co settings
                if (adjustTimeStep)
                {
                    #include "setDeltaT.H"
                }

                runTime++;

                Info<< "Time = "
                    << runTime.timeName()
                    << endl;

                /// Move the mesh incrementally, new timestep takes effect.
                mesh.stepTowards(rp);

                /// Advect Field
                mesh.advect<scalar>();
            }
        }
        /// Else
        /// Force last mesh to be the first mesh
        else if (meshCounter == M)
        {
            /// Move the mesh back to its initial config 
            mesh.cyclicStep();

            /// Advect Field
            mesh.advect<scalar>();
        }
        else
        {
            FoamFatalErrorIn("meshCounter")
                << endl;
        }

        ///
        /// Write final mesh and fields of each random step
        ///

        /// Re-set time
        runTime.setTime(M, 0);

        /// Mesh and fields will be written to caseDir/M/
        mesh.write();
    }
}
