/**
  * For a given field, do this operation:
  * $\int field dV$
  * and print the result.
  *
  * \author: Maalik (Ali Shayegh, Richmond Hill, Dublin, IE)
  * \date:   May 10 2025
  */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "fvMatrices.H"
#include "scalar.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "word.H"
#include "IOmanip.H"

namespace Foam
{
typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;

class Mesh
{
    /// Private
    const fvMesh mesh_;

    /// Public
    public:
        Mesh(const Time& runTime);
        bool volInt(const word& fieldName, const scalar& p);
};
}

/// Exception handling classes
class bad{};

class badField{};

/// Class definition

/// Constructor
Foam::Mesh::Mesh(const Time& runTime)
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
)
{}

/// Member function
bool Foam::Mesh::volInt(const word& fieldName, const scalar& p)
{
    /// Reference to time
    const Time& runTime = mesh_.db().time();

    /// Calculate cell volumes
    const DimensionedField<scalar, volMesh>& V = mesh_.V();
 
 
    ///
    /// Read the field
    ///

    /// Initialize the field to keep this scope happy

    /// Limited to volScalarField's for now
    autoPtr<volScalarField> fieldPtr
    (
        new 
        volScalarField
        (
            IOobject
            (
                //"dummyField",
                fieldName,
                runTime.timeName(),
                mesh_,
                //IOobject::NO_READ,
                IOobject::MUST_READ,
                IOobject::NO_WRITE
            ),
            mesh_//,
            //dimensionedScalar("dummy", dimless, 0)
        )
    );
 
    /// Actually access the field
    IOobject fieldIO
    (
        fieldName,
        runTime.timeName(),
        mesh_,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    //Info<< fieldName
    //    << " is a(n) "
    //    << fieldIO.type()
    //    << endl;
 
    //IOobjectList fields(mesh_, runTime.timeName(), fieldName);

    //const word& fieldType = fieldIO::typename;

    //const GeometricField<fieldIO::typeName, fvPatchField, volMesh>& // Error

    //const word& fieldTypeWord = fieldIO.headerClassName();

    //typedef fieldTypeWord fieldTyep; // Error

    //typedef fieldIO.headerClassName() fieldType; // Error
    
    //const fieldType& field(fieldIO);

    //if (fieldTypeWord == "volScalarField")
    //{
    //    //const volScalarField& field(fieldIO, mesh_); // Error
    //    //const volScalarField field(fieldIO, mesh_); // Already declared
    //    fieldPtr.reset(new volScalarField(fieldIO, mesh_)); // Already declared

    //    Info<< "Field "
    //        << fieldPtr().name()
    //        << "'s type: "
    //        << fieldTypeWord
    //        << "\n"
    //        << endl;
    //}
    //else
    //{
    //    Info<< "Field "
    //        << fieldPtr().name()
    //        << "'s type: "
    //        << fieldTypeWord
    //        << "\n"
    //        << endl;

    //    throw badField{};

    //    return false;
    //}
    
    /// Initialize the integral
    scalar sum = 0;
 
    forAll(mesh_.cells(), cellI)
    {
        sum += fieldPtr()[cellI] * V[cellI];
    }
 
    Info<< fixed
        << setprecision(p);

    Info<< "sum = "
        << sum
        << endl;

    return true;
}

using namespace Foam;

int main(int argc, char* argv[])
try
{
    argList::validArgs.append("field");

    argList::validOptions.insert("time","timeName");

    argList::validOptions.insert("p","precision");

    argList args(argc, argv);

    const word& field = args.additionalArgs()[0];

    const word& timeName = args.options()["time"];

    const scalar& precision = readScalar(IStringStream(args.options()["p"])());

    // Error when defined as a reference
    //const Time& runTime("controlDict", args.rootPath(), args.caseName());

    const Time runTime("controlDict", args.rootPath(), args.caseName());

    // Reset time
    if (args.optionFound("time"))
    {
        const_cast<Time&>(runTime)
        .setTime
        (
            readScalar
            (
                IStringStream
                (
                    //timeName
                    args.options()["time"]
                )()
            ),
            0 // Time index place holder
        );
    }

    Mesh mesh(runTime);   

    if(mesh.volInt(field, precision))
    {
        Info<< "\n\nEnd."
            << endl;

        return true;
    }
    else
    {
        throw bad{};
    }
}

catch(bad)
{
    Info<< "Volume integration was unsuccesful"
        << endl;

    // Do not return, maybe badField is also caught
    //return 2;
}

catch(badField)
{
    Info<< "Exception in field creation\n"
        << "Only volScalarField is supported\n"
        << endl;

    return 2;
}

catch(...)
{
    Info<< "Unknown Error"
        << endl;
    
    return 3;
}
