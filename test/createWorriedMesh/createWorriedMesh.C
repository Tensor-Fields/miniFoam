/**
 * Get a mesh and move points randomly
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
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Jan 03 2025
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
// Added from autoPatch.C to see if double free error is resolved
//#include "OFstream.H"
//#include "ListOps.H"

using namespace Foam;

/// Driver
int main (int argc, char* argv[])
{
    argList args(argc, argv);

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

    runTime.functionObjects().off();

    //
    // Use boundaryMesh to reuse all the featureEdge stuff there.
    //

    //boundaryMesh bMesh;

    //scalar featureAngle(readScalar(IStringStream(args.additionalArgs()[0])()));
    //scalar featureAngle(89);
    //bool overwrite = args.optionFound("overwrite");

    //scalar minCos = Foam::cos(featureAngle * mathematicalConstant::pi/180.0);

    //Info<< "Feature:" << featureAngle << endl
    //    << "minCos :" << minCos << endl
    //    << endl;

    //bMesh.read(mesh);

    // Set feature angle (calculate feature edges)
    //bMesh.setFeatureEdges(minCos);

    // Collect all feature edges as edge labels
    //labelList markedEdges;

    //collectFeatureEdges(bMesh, markedEdges);

    //pointMesh pMesh(mesh);

    //forAll(mesh.points(), pointI)
    //{
    //    Info<< "mesh.pointI"
    //        << nl << mesh.allPoints()[pointI]
    //        << "    "
    //        //<< pMesh.points()[pointI] // Error: No points() member 
    //        //<< pMesh.allPoints()[pointI] // Error: pointMesh have only access
    //                                       // (reference) to polyMesh, not its
    //                                       // own copy of points
    //        << endl;
    //}

    //label nBoundaryPoints = 0;

    //forAll(pMesh.boundary(), patchI)
    //{
    //    /// boundary() returns pointBoundaryField
    //    /// boundary()[patchI] returns pointPatch
    //    nBoundaryPoints += pMesh.boundary()[patchI].size();
    //    //pMesh.boundary()[patchI].banana();
    //}

    /// Mark points on feature edges
    //Info<< "Feature edges' end vertices:"
    //    << nl << endl;

    //forAll(markedEdges, edgeI)
    //{
    //    //pointLocation[mesh.edges()[markedEdges[edgeI]].start()] 
    //    //  = word("featureEdge");
    //    //pointLocation[mesh.edges()[markedEdges[edgeI]].end()] 
    //    //  = word("featureEdge");
    //    Info<< mesh.edges()[markedEdges[edgeI]].start() 
    //        << " "
    //        << mesh.edges()[markedEdges[edgeI]].end()
    //        << " "
    //        << mesh.points()[mesh.edges()[markedEdges[edgeI]].start()]
    //        << " "
    //        << mesh.points()[mesh.edges()[markedEdges[edgeI]].end()]
    //        << endl;
    //}

    //Info<< "Feature edges':"
    //    << nl << endl;

    //forAll(markedEdges, edgeI)
    //{
    //    //pointLocation[mesh.edges()[markedEdges[edgeI]].start()] 
    //    //  = word("featureEdge");
    //    //pointLocation[mesh.edges()[markedEdges[edgeI]].end()] 
    //    //  = word("featureEdge");
    //    Info<< mesh.edges()[markedEdges[edgeI]]
    //        << endl;
    //}

    //Info<< "markedEdges:"
    //    << markedEdges
    //    << nl
    //    << endl;

    //wordList pointLocation(mesh.points().size(), word("internal"));

    //Info<< "After feature edge specification"
    //    << endl;

    //forAll(pointLocation, pointI)
    //{
    //    Info<< "pointLocation["
    //        << pointI
    //        << "] = "
    //        << pointLocation[pointI]
    //        << endl;
    //}

    pointMesh pMesh(mesh);

    /// Mark points NOT on feature edges, BUT on boundary
    //forAll(pMesh.boundary(), patchI)
    //{
    //    forAll(pMesh.boundary()[patchI].meshPoints(), pointI)
    //    {
    //        if 
    //        (
    //            pointLocation
    //            [
    //                pMesh.boundary()[patchI].meshPoints()[pointI]
    //            ] != word("featureEdge")
    //            //pointLocation
    //            //[
    //            //    pointI 
    //            //  + pMesh.boundary()[patchI].start()
    //            //] != word("featureEdge")
    //        )
    //        {
    //            pointLocation
    //            [
    //                pMesh.boundary()[patchI].meshPoints()[pointI]
    //            ] = word("boundary");
    //            //pointLocation
    //            //[
    //            //    pointI 
    //            //  + pMesh.boundary()[patchI].start()
    //            //] = word("boundary");
    //        }
    //    }
    //}

    //forAll(pointLocation, pointI)
    //{
    //    Info<< "pointLocation["
    //        << pointI
    //        << "] = "
    //        << pointLocation[pointI]
    //        << endl;
    //}

    ///
    /// Find internal points
    ///

    pointField pf = mesh.points();

    /// List of the labels of the boundary points
    labelList boundary(pf.size(), -1);

    Info<< "N boundary points "
        << boundary.size()
        << endl;

    labelList visited = boundary;
    
    //const labelList boundary = pMesh.boundary().globalPatch().meshPoints();
    //const labelList boundary = pMesh.boundary().meshPoints(); // Error: No
                                                                // member
    //const labelList boundary2 = pMesh.boundary().points(); // Error: NO member

    label pointIndex = -1;

    forAll(pMesh.boundary(), patchI)
    {
        const labelList& patchPoints = pMesh.boundary()[patchI].meshPoints();

        //Info<< pMesh.boundary()[patchI].name()
        //    << " points"
        //    << ":"<< patchPoints
        //    << endl;

        forAll(patchPoints, pointI)
        {
            /// To avoid counting a boundary point more than once.
            if (visited[patchPoints[pointI]] == -1)
            {
                visited[patchPoints[pointI]] = 1;

                boundary[++pointIndex] = patchPoints[pointI];
            }
        }
    }

    /// Shrink the list of boundary points
    boundary.setSize(++pointIndex);

    //Info<< "boundary points:"
    //    << nl << boundary
    //    << nl << endl;

    /// List of labels of internal points
    /*unallocLabelList*/ labelList internal_point(pf.size() - boundary.size());

    /// Internal filter, true if internal, false if on boundary
    boolList is_internal(pf.size(), true);

    forAll(boundary, pointI)
    {
        is_internal[boundary[pointI]] = false;
    }

    /// Initialize internal_point
    label index = -1;

    /// Traverse all mesh points
    forAll(is_internal, pointI)
    {
        if (is_internal[pointI])
        {
            internal_point[++index] = pointI;
        }
    }

    internal_point.setSize(++index);

    //Info<< "internal points:"
    //    << nl << internal_point
    //    << nl << endl;

    const labelListList pointPoints = mesh.pointPoints();

    const labelListList pointEdges = mesh.pointEdges();

    /// Move engine: internal points
    forAll(internal_point, index)
    {
        /// Scale factor
        scalar r = 0;

        /// Calculate a characteristic distance as the maximum displacement of 
        /// a point
        vector l(0, 0, 0);

        //forAll(pointPoints[internal_point[index]], pointI)
        //{
        //    l += pf[pointPoints[internal_point[index]][pointI]];
        //    
        //    Info<< l  << endl;
        //}

        //l /= pointPoints[internal_point[index]].size();

        //Info<< "l_avg = "
        //    << l  << endl;

        //l -= pf[internal_point[index]];

        //Info<< "l_rel = "
        //    << l  << endl;

        /// Avg length of all edges connected to this point
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
           r * Foam::sin(2 * mathematicalConstant::pi * phi)
        );

        //forAll(move, cmpt)
        //{
        //    move[cmpt] *= l[cmpt];
        //}

        //Info<< "move : "
        //    << move
        //    << endl;
        
        /// Access internal points through their indices
        scalar amp = .2;

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
