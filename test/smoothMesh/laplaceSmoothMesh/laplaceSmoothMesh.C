/**
 * Smooth mesh points by solving a laplace equation for points as a field 
 *
 * Algorithm
 * - Create a volFiel out of initial points
 * - Solve laplace equation
 * - Interpolate internal volField to internal pointField
 * - Extrapolate boundary volField to boundary pointField (not implemented)
 * - Move mesh points
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Jan 02 2025
 */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "IOobject.H"
#include "pointMesh.H"
#include "GeometricField.H"
#include "vector.H"
#include "pointPatchField.H"
#include "pointMesh.H"
#include "dimensionedScalar.H"
#include "dimensionedVector.H"
#include "fvMatrices.H"
#include "fvm.H"
#include "volPointInterpolation.H"
#include "pointVolInterpolation.H"
//#include "zeroGradientFvPatchFields.H"
//#include "zeroGradientPointPatchFields.H"
//#include "ZeroGradientPointPatchField.H"
#include "leastSquaresVolPointInterpolation.H"

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;
typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

int main (int argc, char* argv[])
{
    argList args(argc, argv);

    Time runTime
    (
        "fictiousTimeControlDict",
        args.rootPath(),
        args.caseName()
    );

    //Time physicalTime
    //(
    //    "controlDict",
    //    args.rootPath(),
    //    args.caseName()
    //);

    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    pointMesh pMesh(mesh);

    /// Store point mesh
    pointField pf = mesh.points();

    pointVectorField D
    (
        IOobject
        (
            "meshPoint",
            runTime.timeName(),
            mesh, // pMesh give error, no constructor matches
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        pMesh,
        dimensionedVector("zero", dimLength, vector::zero)//,
        //fvPatchVectorField::zeroGradientType
    );

    Info<< D.name()
        << " Created"
        << endl;

    /// Re-set D values

    /// Error:
    /// LHS: pointVectorField, RHS: pointField, aka vectorField, aka
    /// Field<vector>
    //D = mesh.points();

    forAll(pf, pointI)
    {
        D.internalField()[pointI] = pf[pointI];
    }

    /// To see if values are correctly mapped from `pf`
    //D.rename("D_old");
    D.write();

    /// Create boundary conditions
    //wordList patchTypes(D.boundaryField().size());

    //PtrList<fvPatchField<vector> > ptrListPatchFieldType
    //(D.boundaryField().size());

    //forAll(patchTypes, patchI)
    //{
    //    patchTypes[patchI] = D.boundaryField()[patchI].type(); 

    //    ptrListPatchFieldType.set
    //    (
    //        patchI, 
    //        new fvPatchField<vector> 
    //        (
    //            mesh.boundary()[patchI],
    //            D
    //            //p.internalField()
    //        )
    //    ); 

    //    Info<< "patchTypes["
    //        << patchI
    //        << "] = "
    //        << patchTypes[patchI] << endl;
    //}

    /// Create a vol field to be interpolated from `D`
    volVectorField volD
    (
        IOobject
        (
            "meshPointVol",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh//,
        //dimensionedVector("zero", dimLength, vector::zero),
        //zeroGradientFvPatchField<vector>::typeName
        //patchTypes
    );

    pointVolInterpolation toVol(pMesh, mesh);

    /// Reset volD values
    toVol.interpolate(D, volD);

    Info<< "point field `"
        << D.name()
        << "` interpolated to vol field `"
        << volD.name()
        << "`"<<endl;

    // To investigate the correctness of point-to-vol map
    volD.write();

    //Info<< "`"<< volD.name()
    //    << "` written"
    //    << endl;

    dimensionedScalar mu
    (
        "mu",
        pow(dimLength,2) / dimTime,
        1e-3
    );

    Info<< "`mu` created"
        << endl;

    while (runTime.loop())
    {
        fvMatrix<vector> volDEq
        (
            fvm::laplacian(mu, volD)
         == fvm::ddt(volD)
        );
        
        Info<< volD.name()
            << " equation created"
            << endl;

        volDEq.relax();

        Info<< volD.name()
            << " equation relaxed"
            << endl;
        
        volDEq.solve();

        Info<< volD.name()
            << " equation solved"
            << endl;
        
        //volD.correctBoundaryConditions();

        /// Store D before updating
        D.storePrevIter();
        //pointVectorField Dold = D;

        /// volD to D
        //volPointInterpolation toPoint(mesh);
        leastSquaresVolPointInterpolation toPoint(mesh);

        toPoint.interpolate(volD, D);
        
        Info<< "vol field `"
            << volD.name()
            << "` interpolated to point field "
            << "`"<< D.name()
            << "`"
            << endl;

        /// Create displacement field for using wrap-by-vector in Paraview
        //pointVectorField DD = D-Dold;
        pointVectorField DD = D - D.prevIter();

        DD.rename("DD");

        /// Write temporary fields
        volD.write();
        D.write();
        DD.write();

        // Map D to pf
        forAll(pf, pointI)
        {
            //if (pointI < D.internalField().size())
            //{
                pf[pointI] = D.internalField()[pointI];
            //}
        }

        Info<< "New points created"
            << endl;

        //label boundaryPointCounter = D.internalField().size();

        //forAll(D.boundaryField(), patchI)
        //{
        //    forAll(D.boundaryField()[patchI], pointI)
        //    {
        //        pf[boundaryPointCounter] 
        //      = D.boundaryField()[patchI][pointI];

        //        ++boundaryPointCounter;
        //    }
        //}

        mesh.movePoints(pf);

        Info<< mesh.name()
            << " points moved to new points"
            << endl;
        
        if (runTime.write())
        {
            Info<< runTime.timeName()
                << " written"
                << endl;
        }
    }

    Info<< "End"
        << endl;
}
