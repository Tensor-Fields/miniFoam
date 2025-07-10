/**
 * Smooth the billet mesh in the Taylor impact problem.
 *
 * Assumes that
 * - Billet impact direction is -Y: 
 *
 *       ___
 *      |   |             V
 *      |   |             V impact direction
 *     /     \            V
 *    /       \
 * ---------------        ^
 *                       Y|
 *                        .--->
 *                         X
 *
 *
 *   Therefore, mesh will be squished, and so smoothed using this
 *   utility in that direction.
 *
 * Usage: smoothBilletMesh N \( "fixedPatch_1" "fixedPatch_2" ... \)
 *     where N is the number of cells in Y direction.
 *
 * \author: Maalik (ali@tensorfields.com), UCD, Dublin, Ireland
 * \date: Jan 31 2025
 */

#include "argList.H"
#include "label.H"
#include "List.H"
#include "foamTime.H"
#include "IOobject.H"
#include "polyMesh.H"
#include "fvMesh.H"
#include "polyPatch.H"
#include "polyBoundaryMesh.H"

#include "scalar.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "GeometricFields.H"
//#include "DimensionedField.H"

//#include "surfaceMesh.H"
//#include "fvsPatchField.H"
#include "fvMatrices.H"
#include "fvc.H"
#include "Random.H"
#include "meshToMesh.H"
#include "IOobjectList.H"

//#include "fvCFD.H"

using namespace Foam;

//typedef GeometricField<scalar,fvPatchField,volMesh> volScalarField;
typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;
//typedef GeometricField<scalar, fvPatchField, surfaceMesh> surfaceScalarField;

int main(int argc, char* argv[])
{
    argList::validArgs.append("No. cells in Y direction");

    //argList::validArgs.append("patchNames to be fixed, (...)");

    argList::validOptions.insert("fixedPatches", "(...)");

    argList::validOptions.insert("linear", "");

    argList::validOptions.insert("logarithmic", "scalar");

    argList::validOptions.insert("parabolic", "scalar");

    argList args(argc, argv);

    /// Error
    //const scalar& N = 
    //const scalar N = 
    const scalar N  
    (
        readScalar
        (
            IStringStream(args.additionalArgs()[0])
                () // KEY
        )
    );

    //const stringList& fixedPatches = args.additionalArgs()/*[0]*/;

    const stringList& fixedPatches =
        args.optionReadList<string>(word("fixedPatches"))/*[0]*/;

    forAll(fixedPatches, fixedPatchI)
    {
        Info<< "Patch "
            << fixedPatches[fixedPatchI]
            << " is fixed."
            << endl;
    }

    // Non-const, as we finally increment it

    /// Error
    //Time& runTime

    Time runTime
    (
        "controlDict",
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
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    pointField oldPoints = mesh.points();
    pointField newPoints = oldPoints;

    /// Flag fixed points
    boolList fixed(oldPoints.size(), false);

    //Info<< "fixed.size() = "
    //    << fixed.size()
    //    << endl;

    //Info<< "oldPoints.size() = "
    //    << oldPoints.size()
    //    << endl;

    //Info<< "newPoints.size() = "
    //    << newPoints.size()
    //    << endl;

    /// Use point mesh object to search points based on patch name
    const pointMesh pMesh(mesh);

    forAll(pMesh.boundary(), patchI)
    {
        const pointPatch& pPatch = pMesh.boundary()[patchI];

        forAll(fixedPatches, fixedPatchI)
        {
            if (pPatch.name() == fixedPatches[fixedPatchI])
            {
                const labelList pointsOnPatch = pPatch.meshPoints();

                //Info<< "patch is "
                //    << pMesh.boundary()[patchI].name()
                //    << ":"<< pointsOnPatch
                //    << endl;

                forAll(pointsOnPatch, pointI)
                {
                    /// Flag fixed points
                    fixed[pointsOnPatch[pointI]] = true;
                }

                Info<< "Fixed points on patch "
                    << fixedPatches[fixedPatchI]
                    << " are flagged"
                    << endl;

                break;
            }
        }
    }

    /// Find max Y in mesh
    scalar H = -VGREAT;

    Info<< "H (maximum Y) initialized: "
        << H  << endl;

    forAll(oldPoints, pointI)
    {
        // Error: gMax
        //H = gMax(H, oldPoints[pointI][1]);

        H = max(H, oldPoints[pointI][1]);
    }

    Info<< "H found: " 
        << H  << endl;

    //const scalar& a = args.optionRead<scalar>("logarithmic");

    /// Evaluate new points
    forAll(oldPoints, pointI)
    {
        if (!(fixed[pointI]))
        {
            scalar& y0 = oldPoints[pointI][1];

            /// Linear
            if (args.optionFound("linear"))
            {
                //newPoints[pointI][1] = 
                //    oldPoints[pointI][1] + (H - oldPoints[pointI][1]) / N;
                
                /// Attribute an index in range [0 N] to this point
                Info<< "point "
                    << pointI
                    << " " 
                    << "abs(/*Foam::*/ceil(y0 * N/H) - y0 * N/H) = "
                    << abs(/*Foam::*/ceil(y0 * N/H) - y0 * N/H)
                    << " abs(/*Foam::*/floor(y0 * N/H) - y0 * N/H) = "
                    << abs(/*Foam::*/floor(y0 * N/H) - y0 * N/H)
                    << " y0 = "
                    << y0
                    << endl;
                if 
                (
                    /*Foam::*/abs(/*Foam::*/ceil(y0 * N/H) - y0 * N/H) < 
                        /*Foam::*/abs(/*Foam::*/floor(y0 * N/H) - y0 * N/H)
                )
                {
                    Info<< "in ceil, newPointY = ";
                    newPoints[pointI][1] = /*Foam::*/ceil(y0 * N / H) * H / N;

                    Info<< newPoints[pointI][1]<< endl;
                }
                else
                {
                    Info<< "in floor, newPointY = ";
                    //newPoints[pointI][1] = /*Foam::*/floor(y0 * N / H) * H / N;
                    newPoints[pointI][1] = /*Foam::*/ceil(y0 * N / H) * H / N;

                    Info<< newPoints[pointI][1]<< endl;
                }
            }

            /// Logarithmic
            else if (args.optionFound("logarithmic"))
            {
                const scalar& m = args.optionRead<scalar>("logarithmic");

                newPoints[pointI][1] = y0 +
                    //(H/N) * 
                    //    (
                    //        1 + (N - 1) * 
                    //            (Foam::log(1 + y0 / (1 + a)) / 
                    //                Foam::log(1 + H / (1 + a)))
                    //    );

                    //(H * (1 - 1 / N) / m) *
                    //    (
                    //        Foam::log
                    //        (
                    //            1 + m * Foam::log(m) * y0 / 
                    //                (H * (1 - 1 / N))
                    //        )
                    //    ) + H / N;


                    (
                        (1 * (1 - 1 / N) / m) *
                            (
                                Foam::log
                                (
                                    1 + m * Foam::log(m) * y0 / 
                                        (H * (1 - 1 / N))
                                )
                            ) + 1 / N
                    ) * (H - y0) / N ;
            }

            /// Parabolic
            //else if
            //{
            //    newPoints[pointI][1]
            //}
        }
        else
        {
            //Info<< "Point "
            //    << pointI
            //    << " is fixed"
            //    << endl;
        }
    }

    Info<< "New points calculated"
        << endl;

    /// Move mesh
    mesh.movePoints(newPoints);

    Info<< "Mesh moved"
        << endl;

    /// Write new mesh at new time
    runTime++;

    mesh.write();
    
    Info<< "Mesh written."
        << endl;

    Info<< nl << "End."
        << endl;
}
