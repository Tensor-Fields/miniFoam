/**
 * Get a mesh, select a patch, and move patch internal points randomly
 *
 * New points are in the neighbourhood of the previous points, so that the new
 * mesh is not tangled (aka it does not have inverse cells).
 *
 * Roadmap:
 * - Enable for fixing corners; have a run-time switch to decide whether or not
 *   corners should be fixed.
 * - Enable for run-time decision as to whether boundary points move or not, and
 *   if they do, how (e.g., fixed vs slip, using command line args or from a 
 *   dic).
 * - To start, 
 *   -- points met by one face only are on boundary and are allowed to move
 *      on the boundary face only, not in its normal direction;
 *   -- points met by two faces only are on feature edges and can move along the
 *      feauture edge only;
 *   -- points met by more than two faces are corners and cannot move.
 * - Other assumptions is mentioned in the comments
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Jan 06 2025
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
#include "leastSquaresVolPointInterpolation.H"
#include "boundaryMesh.H"

using namespace Foam;

/// Driver
int main (int argc, char* argv[])
{
    argList::validArgs.append("Patch name");
    argList::validArgs.append("Distortion, 0..1");

    argList args(argc, argv);

    const word& patchName = args.additionalArgs()[0];
    const scalar& amp // Invalid: = args.additionalArgs()[1];
      (readScalar(IStringStream(args.additionalArgs()[1])()));

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

    //argList::noParallel();
    //argList::validOptions.insert("overwrite", "");

    runTime.functionObjects().off();

    //const word oldInstance = mesh.pointsInstance();

    //scalar featureAngle
    //  (readScalar(IStringStream(args.additionalArgs()[0])()));

    const pointMesh pMesh(mesh);

    // Non-const, as it will be updated when calculting new points
    pointField pf = mesh.points();
    
    labelList pointsOnPatchFrequency(pf.size(), 0);

    forAll(pMesh.boundary(), patchI)
    {
        const pointPatch& pPatch = pMesh.boundary()[patchI];

        const labelList pointsOnPatch = pPatch.meshPoints();

        Info<< "patch is "
            << pMesh.boundary()[patchI].name()
            << ":"<< pointsOnPatch
            << endl;

        forAll(pointsOnPatch, pointI)
        {
            ++pointsOnPatchFrequency[pointsOnPatch[pointI]];
        }
    }

    Info<< "pointsOnPatchFrequency:"
        << nl << pointsOnPatchFrequency
        << endl;

    // Was hard-coded, now from args
    //const word& patchName = "front";

    // Using mesh, as pMesh does not offer findPatchID
    const label& patchLabel = mesh.boundaryMesh().findPatchID(patchName);

    Info<< "patchLabel:"
        << patchLabel
        << endl;

    const pointPatch& pPatch = pMesh.boundary()[patchLabel];

    // Checked, correct.
    const labelList pointsOnPatch = pPatch.meshPoints();

    //Info<< "pointsOnPatch:"
    //    << nl << pointsOnPatch
    //    << endl;

    ///
    /// Create the move engine,
    /// - set movement of patch edges to zero
    /// - set movement in the direction normal to the patch zero
    ///
    /// Note: Only works for a flat patch
    ///

    /// List of labels of patch internal points

    labelList internal_point(pointsOnPatch.size());

    label index = -1;

    forAll(pointsOnPatch, pointI)
    {
        /// If point occurs to be on exatly one patch
        if (pointsOnPatchFrequency[pointsOnPatch[pointI]] == 1)
        {
            internal_point[++index] = pointsOnPatch[pointI];
        }
    }

    internal_point.setSize(++index);

    Info<< "internal_point:"
        << nl << internal_point
        << endl;

    const labelListList pointEdges = mesh.pointEdges();

    /// Move engine: patch internal points;
    /// - Assumes that patch normal is "z", so points only move in x and y dirs

    forAll(internal_point, index)
    {
        /// Scale factor
        scalar r = 0;

        /// Avg length of all edges connected to this point (incl. those edges
        /// that does not fall on the patch itself
        forAll(pointEdges[internal_point[index]], edgeI)
        {
            r += mesh.edges()[pointEdges[internal_point[index]][edgeI]].mag(pf);

            Info<<"r:"<< r  << endl;
        }

        r /= pointEdges[internal_point[index]].size();

        Info<<"r:"<< r  << endl;

        Info<< "internal point : " 
            << pf[internal_point[index]]
            << endl;

        // Arg must be a label
        Random rand(index);

        // Move the point to a random point on a sphere with the center to be
        // the point itself and the radius is r
        scalar theta = rand.scalar01();

        scalar phi = rand.scalar01();

        /// Random vector between (-1 -1 -1) to (1 1 1), severs as a scale
        /// factor

        /// Bug: if avg location of pointPoints is coincident with the point
        /// itself, you end up with a (0 0 0) move vector
        //vector move = 2 * (rand.vector01() - vector(0.5, 0.5, 0.5));

        vector move
        (
           //r * Foam::cos(rand.scalar01() * 2 * mathematicalConstant::pi)
           //  * Foam::sin(rand.scalar01() * 2 * mathematicalConstant::pi), 
           //r * Foam::cos(rand.scalar01() * 2 * mathematicalConstant::pi)
           //  * Foam::cos(rand.scalar01() * 2 * mathematicalConstant::pi), 
           //r * Foam::sin(rand.scalar01() * 2 * mathematicalConstant::pi)
           r * Foam::cos(2 * mathematicalConstant::pi * phi)
             * Foam::sin(2 * mathematicalConstant::pi * theta), 
           r * Foam::cos(2 * mathematicalConstant::pi * phi)
             * Foam::cos(2 * mathematicalConstant::pi * theta), 
           //r * Foam::sin(2 * mathematicalConstant::pi * phi)
           0
        );

        //forAll(move, cmpt)
        //{
        //    move[cmpt] *= l[cmpt];
        //}

        //Info<< "move : "
        //    << move
        //    << endl;
        
        /// Access internal points through their indices
        // Was hard-coded, now from args
        //scalar amp = .2;

        pf[internal_point[index]] += (amp * move);

        //vector& point = pf[internal_point[index]];

        //forAll(point, coord)
        //{
        //   point[coord] *= 
        //}
        
        Info<< "internal point after move : " 
            << pf[internal_point[index]]
            << endl;
    }

    mesh.movePoints(pf);

    Info<< mesh.name()
        << " points moved to new points"
        << endl;
    
    runTime++;

    mesh.write();

    Info<< "End"
        << endl;
}
