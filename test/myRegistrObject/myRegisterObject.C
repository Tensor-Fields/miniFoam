/**
 * A code to register an object to a mesh
 *
 * Registration is done in a function call, and we check whether the object 
 * stays
 * in the toc outside the scope of the function
 * 
 * \author Maalik, Richmond Hill, Dublin 
 * \date, 17-12-2024 
 */

#include "polyMesh.H"
#include "argList.H"
#include "foamTime.H"
#include "GeometricField.H"
#include "scalar.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "fvMatrices.H"

using namespace Foam;

typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;

void registerMe(const fvMesh& mesh)
{
    Info<< "mesh.toc() before registring fields"
    << mesh.toc()
    << endl;
    
    /// Create a dummy field
    //volScalarField p
    //(
    //    IOobject
    //    (
    //        "pCreated",
    //        mesh.time().timeName(),
    //        mesh
    //    ),
    //    mesh,
    //    dimensionedScalar("zero",dimless,0)
    //);
    //
    //Info<<"Field p created; check size:"
    //    << p.size()
    //    << endl;
    //
    //Info<< "mesh.toc() after registring fields"
    //    << mesh.toc()
    //    << endl;
    //
    //
    //const_cast<objectRegistry&>(mesh.thisDb()).checkIn(p);
    //
    //Info<< "mesh.toc() after checking fields in"
    //    << mesh.toc()
    //    << endl;
    //
    //autoPtr<volScalarField> pAutoPtr
    //(
    //    new
    //    volScalarField
    //    (
    //        IOobject
    //        (
    //            "pAutoPtr",
    //            mesh.time().timeName(),
    //            mesh
    //        ),
    //        mesh,
    //        dimensionedScalar("zero",dimless,0)
    //    )
    //);
    //
    //Info<< "mesh.toc() after registering autoPtr"
    //    << mesh.toc()
    //    << endl;
    
    volScalarField pRead
    (
        IOobject
        (
            "p",
            mesh.time().timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::NO_WRITE,
            false
        ),
        mesh//,
        //dimensionedScalar("zero",dimless,0)
    );
    
    Info<<"Field "
        << pRead.name()
        << " created;"
        //<< " check size:"
        //<< pRead.size()
        << endl;
    
    Info<< "mesh.toc() after creating "
        << pRead.name()
        << nl
        << mesh.toc()
        << endl;

    IOobject fieldIOobject
    (
        pRead.name(),
        mesh.time().timeName(),
        mesh,
        IOobject::MUST_READ,
        IOobject::NO_WRITE
    );

    //const_cast<objectRegistry&>(mesh.thisDb()).checkOut(pRead);

    volScalarField* pPtr = new volScalarField
    (
        fieldIOobject
        //(
        //    "pPtr",
        //    mesh.time().timeName(),
        //    mesh
        //),
        ,
        pRead//,
        //dimensionedScalar("zero",dimless,0)
    );
    
    Info<< "mesh.toc() after registering ptr to "
        << pRead.name() 
        << " created by copy constructor "
        << mesh.toc()
        << endl;

    volScalarField* pPtr_initToZero = new volScalarField
    (
        IOobject
        (
            "pPtr_initToZero",
            mesh.time().timeName(),
            mesh
        ),
        mesh,
        dimensionedScalar("zero",dimless,0)
    );
    
    Info<< "mesh.toc() after registering init_to_zero ptr"
        << mesh.toc()
        << endl;
}

int main(int argc, char* argv[])
{
    argList args(argc, argv);
    
    Time runTime
    (
        //"controlDict",
        args.rootPath(), 
        args.caseName() 
    );
    
    fvMesh mesh    
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );
    
    Info<< "runTime.toc() after mesh creation, before field creation"
        << runTime.toc()
        << endl;
    
    registerMe(mesh);
    
    Info<< "runTime.toc() after field creation, outside function body"
        << runTime.toc()
        << endl;
    
    Info<< "mesh.toc() after registring fields, outside function body"
        << mesh.toc()
        << endl;
}
