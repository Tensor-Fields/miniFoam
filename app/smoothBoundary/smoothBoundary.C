/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | cfMesh: A library for mesh generation
   \\    /   O peration     |
    \\  /    A nd           | Author: Franjo Juretic (franjo.juretic@c-fields.com)
     \\/     M anipulation  | Copyright (C) Creative Fields, Ltd.
-------------------------------------------------------------------------------
License
    This file is part of cfMesh.

    cfMesh is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 3 of the License, or (at your
    option) any later version.

    cfMesh is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with cfMesh.  If not, see <http://www.gnu.org/licenses/>.

Description
    Performs point relocations in the mesh (smoothing) in order to
    improve quality measures. It does not make the mesh invalied.

Autor
    Maalik (Ali Shayegh, ali@tensorfields.com), Sana Room, Shiraz

Date
    May 12 2026

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "objectRegistry.H"
#include "foamTime.H"
#include "polyMesh.H"
#include "pointMesh.H"
#include "polyMeshGenModifier.H"
#include "meshSurfaceEngine.H"
#include "meshSurfaceOptimizer.H"
#include "meshOptimizer.H"
#include "debug.H"
#include "cellSet.H"
#include "faceSet.H"
#include "SortableList.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/// N of dims
//label nDim(const polyMesh& mesh)
//{
//    // Check directions
//    //{
//    Vector<label> dirs = mesh.solutionD();
//
//    label nValidDirs = 0;
//
//    for (direction cmpt = 0; cmpt < Vector<label>::nComponents; cmpt++)
//    {
//        if (dirs[cmpt] > 0)
//        {
//            nValidDirs++;
//        }
//    }
//
//    if (nValidDirs == 0)
//    {
//        Info<< " This mesh has no valid solving directions.  dirs = "
//            << dirs << ".  Please check mesh definition for empty patches."
//            << endl;
//
//        //noFailedChecks++;
//    }
//
//    Info << "    This is a " << nValidDirs << "-D mesh" << endl;
//    //}
//
//    return nValidDirs;
//}

/// Return true if mesh is bad
bool badMesh
(
    polyMesh& mesh,
    /*const debug::tolerancesSwitch*/ scalar aspectThreshold = -1,
    /*const debug::tolerancesSwitch*/ scalar skewThreshold = -1,
    /*const debug::tolerancesSwitch*/ scalar nonOrthThreshold = -1
)
{
    const_cast<debug::tolerancesSwitch&>(mesh.aspectThreshold_) = aspectThreshold;
    const_cast<debug::tolerancesSwitch&>(mesh.skewThreshold_) = skewThreshold;
    const_cast<debug::tolerancesSwitch&>(mesh.nonOrthThreshold_) = nonOrthThreshold;

    /// Check aspect ratio

    cellSet cells(mesh, "nonClosedCells", mesh.nCells()/100 + 1);
    cellSet aspectCells
    (
        mesh,
        "highAspectRatioCells",
        mesh.nCells()/100 + 1
    );

    if (aspectThreshold != -1)
    {
        if (mesh.checkClosedCells(true, &cells, &aspectCells))
        {
            //noFailedChecks++;

            label nNonClosed = returnReduce(cells.size(), sumOp<label>());

            if (nNonClosed > 0)
            {
                Info<< "  Writing " << nNonClosed
                    << " non closed cells to set " << cells.name() << endl;
                cells.write();
            }
        }

        label nHighAspect = returnReduce(aspectCells.size(), sumOp<label>());

        if (nHighAspect > 0)
        {
            Info<< "  Writing " << nHighAspect
                << " cells with high aspect ratio to set "
                << aspectCells.name() << endl;
            aspectCells.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Aspect ratio quality will not be checked ..."
            << endl;
    }

    /// Check skewness
 
    faceSet faces(mesh, "skewFaces", mesh.nFaces()/100 + 1);
    if (skewThreshold != -1)
    {
        if (mesh.checkFaceSkewness(true, &faces))
        {
            //noFailedChecks++;

            label nFaces = returnReduce(faces.size(), sumOp<label>());

            if (nFaces > 0)
            {
                Info<< "  Writing " << nFaces
                    << " skew faces to set " << faces.name() << endl;
                faces.write();

                /// Mesh is bad
                return true;
            }
        }
    }
    else
    {
        Info<< "    Skewness quality will not be checked ..."
            << endl;
    }

    faceSet facesNonOrtho(mesh, "nonOrthoFaces", mesh.nFaces()/100 + 1);

    if (nonOrthThreshold != -1)
    {
        if (mesh.checkFaceOrthogonality(true, &facesNonOrtho))
        {
            //noFailedChecks++;
        }

        label nFaces = returnReduce(facesNonOrtho.size(), sumOp<label>());

        if (nFaces > 0)
        {
            Info<< "  Writing " << nFaces
                << " non-orthogonal faces to set " << faces.name() << endl;
            faces.write();

            /// Mesh is bad
            return true;
        }
    }
    else
    {
        Info<< "    Non-orthogonality will not be checked ..."
            << endl;
    }

    /// Mesh is OK
    return false;
}

void correctAxisPoints
(
    polyMeshGen& pmg,
    const polyMesh& mesh,
    const List<string>& wedgePatchNames
)
{
    //Info<< wedgePatchNames
    //    << endl;

    if
    (
        wedgePatchNames.size() - 2 > SMALL
    )
    {
        FatalErrorIn("correctAxisPoints()")
            << "wedgePatchNames must be exactly two names of the"
            << "\ngeometrically-same-wedge patches;\n"
            << abort(FatalError);
    }

    /// Create a local polyMesh

        // //- Cast listPMG into list
        // const pointField& points = pmg.points();
        // const faceList& faces = pmg.faces();
        // const cellList& cells = pmg.cells();

        // //- Create the final mesh
        // polyMesh mesh
        // (
        //    IOobject
        //    (
        //        word("region0"),
        //        pmg.returnTime().timeName(),
        //        pmg.returnTime()
        //    ),
        //    xferCopy(points),
        //    xferCopy(faces),
        //    xferCopy(cells)
        // );

        // //- Create patch ptr list
        // const PtrList<boundaryPatch>& bm = pmg.boundaries();

        // List<polyPatch*> pPtrList(bm.size());

        // forAll(pPtrList, patchI)
        // {
        //     pPtrList[patchI] = bm[patchI].clone(bm).ptr();
        // }

        // //- Add patches
        // finalMesh.addPatches(pPtrList);

        //Info<< "Local polyMesh generated"
        //    << endl;

    const polyBoundaryMesh& bm = mesh.boundaryMesh();
    //const PtrList<boundaryPatch>& bm = pmg.boundaries();
    const pointMesh pMesh(mesh);
    const pointBoundaryMesh& pbMesh = pMesh.boundary();
    //const pointBoundaryMesh& pbMesh = pMesh.boundary();

    //- Find axis points
    labelList wedgePatchLabels(wedgePatchNames.size());

    //Info<< "wedgePatchLabels size: "
    //    << wedgePatchLabels.size()
    //    << endl;

    SortableList<label> wedgePointLabels;

    forAll(wedgePatchNames, patchI)
    {
        //wedgePathLabels[patchI] = bm.findPatchID(wedgePatchNames[patchI]);
        wedgePointLabels
        .append(pbMesh[bm.findPatchID(wedgePatchNames[patchI])].meshPoints());
        //.append(pbMesh[pmg.getPatchID(wedgePatchNames[patchI])].meshPoints());
    }

    wedgePointLabels.sort();

    Info<< "wedge point labels saved."
        << endl;

    SortableList<label> axisPointLabels;

    /// Find duplicate labels, that is, axis point labels
    for
    ( 
        label pointI = 0;
        pointI < (wedgePointLabels.size() - 1);
        ++pointI
    )
    {
        if (wedgePointLabels[pointI] == wedgePointLabels[pointI + 1])
        {
            axisPointLabels.append(List<label>(1, wedgePointLabels[pointI]));
        }
    }

    Info<< "axis point labels saved:"
        //<< axisPointLabels
        << endl;

    /// Axis vector
    vector axis =
        mesh.points()[axisPointLabels[0]]
      - mesh.points()[axisPointLabels[axisPointLabels.size() - 1]];

    /// Project axis points onto axis
    forAll(axisPointLabels, pointI)
    {
        pmg.points()[axisPointLabels[pointI]] =
            //transform(axis, pmg.points()[axisPointLabels[pointI]]);
            (axis & pmg.points()[axisPointLabels[pointI]]) * axis
            / sqr(mag(axis));
    }
}

int main(int argc, char *argv[])
{
    argList::validArgs.clear();

    argList::validOptions.insert("nLoops", "int");
    argList::validOptions.insert("nIterations", "int");
    argList::validOptions.insert("nSurfaceIterations", "int");
    argList::validOptions.insert("qualityThreshold", "scalar");
    argList::validOptions.insert("constrainedCellsSet", "word");
    argList::validOptions.insert("lockedPatchNames", "( patch0 patch1 ...)");
    argList::validOptions.insert("skewThreshold", "<scalar>");
    argList::validOptions.insert("nonOrthThreshold", "0-90, the higher the worse");
    argList::validOptions.insert("iterPerQualityCheck", "<label>");
    argList::validOptions.insert("aspectRatioThreshold", "<scalar>");
    argList::validOptions.insert("wedgePatchNames", "( patch0 patch1 ...)");
    argList::validOptions.insert("wedge", "");
    //argList::validOptions.insert("meshMotionResidual", "<0-1>");

    #include "setRootCase.H"
    #include "createTime.H"

    //- read the settings
    label nIterations(50);
    label nLoops(10);
    label nSurfaceIterations(2);
    scalar qualityThreshold(0.1);
    //scalar meshMotionRes(0.1)

    //- Parse wedge patch names
    List<string> wedgePatchNames;

    if (args.optionFound("wedge"))
    {
        wedgePatchNames =
            args.optionReadList<string>("wedgePatchNames");
    }
    if (args.optionFound("wedge"))
    {
        wedgePatchNames =
            args.optionReadList<string>("wedgePatchNames");
    }

    if( args.options().found("nLoops") )
    {
        nLoops = readLabel(IStringStream(args.options()["nLoops"])());
    }
    else
    {
        Info << "Default number of loops is 10" << endl;
    }

    if( args.options().found("nIterations") )
    {
        nIterations =
            readLabel(IStringStream(args.options()["nIterations"])());
    }
    else
    {
        Info << "Default number of iterations is 50" << endl;
    }

    if( args.options().found("nSurfaceIterations") )
    {
        nSurfaceIterations =
            readLabel(IStringStream(args.options()["nSurfaceIterations"])());
    }
    else
    {
        Info << "Default number of surface iterations is 2" << endl;
    }

    /// By default, only ckeck quality after all iterations, that is, no effect
    /// on smoothing decission.
    label iterPerQualityCheck = nIterations;

    if( args.options().found("iterPerQualityCheck") )
    {
        iterPerQualityCheck =
            readLabel(IStringStream(args.options()["iterPerQualityCheck"])());
    }
    else
    {
        Info<< "By default, only ckeck quality after all iterations."
            << endl;
    }

    if( args.options().found("qualityThreshold") )
    {
        qualityThreshold =
            readScalar(IStringStream(args.options()["qualityThreshold"])());
    }
    else
    {
        Info << "Using default quality threshold 0.1" << endl;
    }

    word constrainedCellSet;

    if( args.options().found("constrainedCellsSet") )
    {
        constrainedCellSet = args.options()["constrainedCellsSet"];
    }
    else
    {
        Info << "No constraints applied on the smoothing procedure" << endl;
    }

    label aspectThreshold = -1;

    if( args.options().found("aspectRatioThreshold") )
    {
        aspectThreshold =
            readScalar(IStringStream(args.options()["aspectRatioThreshold"])());
    }
    else
    {
        Info << "    aspectRatioThreshold was not found, set to 100." << endl;
    }

    label skewThreshold = -1;

    if( args.options().found("skewThreshold") )
    {
        skewThreshold =
            readScalar(IStringStream(args.options()["skewThreshold"])());
    }
    else
    {
        Info << "    skewThreshold was not found, set to ..." << endl;
    }

    label  nonOrthThreshold = -1;

    if( args.options().found("nonOrthThreshold") )
    {
        nonOrthThreshold =
            readScalar(IStringStream(args.options()["nonOrthThreshold"])());
    }
    else
    {
        Info << "    nonOrthThreshold was not found, set to 70" << endl;
    }

    //- Optimize boundary mesh of a 2-D mesh

    //- load the mesh from disk
    /// Reads points and faces from constant
    //polyMeshGen pmg(runTime);
    /// Reads sets from constant
    //pmg.read();

    Info<< "Time: "
        << runTime.timeName()
        << endl;

    //- Read mesh
    polyMesh mesh
    (
        IOobject
        (
            //polyMesh::region0,
            word("region0"),
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );

    //- Read patch data
    const polyBoundaryMesh& bm = mesh.boundaryMesh();

    labelList patchStart(bm.size());
    labelList nFacesInPatch(bm.size());

    forAll(bm, patchI)
    {
        Info<< bm[patchI].name()
            << "    start:"
            << bm[patchI].start()
            << "    size:"
            << bm[patchI].size()
            << endl;

        patchStart[patchI] = bm[patchI].start();
        nFacesInPatch[patchI] = bm[patchI].size();
    }

    //- Avoid loading the mesh from disk
    polyMeshGen pmg
    (
        runTime,
        mesh.points(),
        mesh.faces(),
        mesh.cells(),
        mesh.boundaryMesh().names(),
        patchStart,
        nFacesInPatch
    );

    //- construct the smoother
    meshOptimizer    mOpt(pmg);
    meshSurfaceEngine      sEngine(pmg);
    meshSurfaceOptimizer   sOpt(sEngine);

    //- Re-set constraints
    sOpt.removeUserConstraints();

    //- Add constraints

        //- Parse locked patch names
        List<string> lockedPatchNames = args.optionReadList<string>("lockedPatchNames");

        //- Find locked patch IDs
        labelList lockedPatches(lockedPatchNames.size());

        forAll(lockedPatchNames, patchI)
        {
            lockedPatches[patchI] = bm.findPatchID(lockedPatchNames[patchI]);
        }

        //- Find locked point labels
        const pointMesh pMesh(mesh);
        const pointBoundaryMesh& pbMesh = pMesh.boundary();

        labelList lockedPointLabels(0);

        forAll(lockedPatches, patchI)
        {
            lockedPointLabels.append(pbMesh[lockedPatches[patchI]].meshPoints());
        }

        //- Actually lock points
        sOpt.lockBoundaryPoints<labelList>(lockedPointLabels);

    //if( !constrainedCellSet.empty() )
    //{
    //    //- lock cells in constrainedCellSet
    //    mOpt.lockCellsInSubset(constrainedCellSet);

    //    //- find boundary faces which shall be locked
    //    labelLongList lockedBndFaces, selectedCells;

    //    const label sId = pmg.cellSubsetIndex(constrainedCellSet);
    //    pmg.cellsInSubset(sId, selectedCells);

    //    boolList activeCell(pmg.cells().size(), false);
    //    forAll(selectedCells, i)
    //        activeCell[selectedCells[i]] = true;
    //}

    //- clear geometry information before volume smoothing
    pmg.clearAddressingData();

    //- perform optimisation using the laplace smoother and
    //mOpt.optimizeMeshFV
    //(
    //    nLoops,
    //    nLoops,
    //    nIterations,
    //    nSurfaceIterations
    //);

    /// Suitable for non-wedge 2-D
    //sOpt.optimizeSurface2D(nIterations);

    pointField oldPoints =  mesh.points();
    pointField oldOldPoints =  mesh.points();

    /// Mesh optimization loop
    for (label iter = 0; iter < nIterations; ++iter)
    {
        oldPoints = mesh.points();

        /// If true, mesh is bad and needs optimization
        if
        (
            badMesh
            (
                mesh,
                aspectThreshold,
                skewThreshold,
                nonOrthThreshold
            )
        )
        {
            Info<<"    \nBoundary optimization iteration "
                << iter + 1
                << " ..."
                << endl;

            sOpt.optimizeSurface(iterPerQualityCheck);
            sOpt.untangleSurface();

            /// If 3-D
            //if (nDim(mesh) == 3)
            if (mesh.nGeometricD() == 3)
            {
                Info<< "    Mesh is 3-D.\n    Optimize non-boundary faces."
                    << endl;

                //- perform optimisation of worst quality faces
                mOpt.optimizeMeshFVBestQuality(nLoops, qualityThreshold);

                //- check the mesh again and untangl bad regions if any of them exist
                mOpt.untangleMeshFV(nLoops, iterPerQualityCheck, nSurfaceIterations);
            }

            /// Correct axis points
            correctAxisPoints(pmg, mesh, wedgePatchNames);

            /// Actually move the mesh
            mesh.movePoints(pmg.points());
        }
        else
        {
            Info<< "\n    Mesh is OK after "
                << iter + 1
                << " iterations."
                << endl;

            break;
        }

        /// Stop if mesh is not changing anymore
        /// maxCurrentMotion / maxPrevMotion;
        scalar changeMeasure =
                /*cmptDivide*/
                max(mag(mesh.points() - oldPoints))
                /max(SMALL + mag(oldPoints - oldOldPoints));

        Info<< "    Relative mesh motion: "
            << changeMeasure
            << endl;

        if (changeMeasure < /*meshMotionRes*/qualityThreshold)
        {
            Info<< "\n    Mesh did not change within the threshold of "
                << qualityThreshold
                << ";\n    Stop smoothing."
                << endl;

            break;
        }

        //if (meshQualityOK())
        //{
            //polyMeshGenChecks::checkFaceDotProduct
            //(
            //    const polyMeshGen&,
            //    const bool report = false,
            //    const scalar nonOrthWarn = 70.0,
            //    labelHashSet* setPtr = NULL,
            //    const boolList* changedFacePtr = NULL
            //);

            //polyMeshGenChecks::checkFaceSkewness
            //(
            //    const polyMeshGen&,
            //    scalarField&,
            //    const boolList* changedFacePtr = NULL
            //);
 
            //break;
        //}

        oldOldPoints = oldPoints;
    }

    //sOpt.untangleSurface2D();

    //- perform optimisation of worst quality faces
    //mOpt.optimizeMeshFVBestQuality(nLoops, qualityThreshold);

    //- check the mesh again and untangl bad regions if any of them exist
    //mOpt.untangleMeshFV(nLoops, nIterations, nSurfaceIterations);

    /// Write the optimized mesh into the next time step.
    //- Create a polyMesh copy from polyMeshGen
    //- This is, because the latter only writes in constant
    //- However, the former is flexible as to where to write.

    //- Cast listPMG into list
    const pointField& points = pmg.points();
    const faceList& faces = pmg.faces();
    const cellList& cells = pmg.cells();

    //- Create the final mesh
    polyMesh finalMesh
    (
       IOobject
       (
           word("region0"),
           runTime.timeName(),
           runTime
       ),
       xferCopy(points),
       xferCopy(faces),
       xferCopy(cells)
    );

    //- Create patch ptr list
    List<polyPatch*> pPtrList(bm.size());

    forAll(pPtrList, patchI)
    {
        pPtrList[patchI] = bm[patchI].clone(bm).ptr();
    }

    //- Add patches
    finalMesh.addPatches(pPtrList);

    runTime++;

    Info<< "\nWriting new mesh to "
        << runTime.timeName()
        << endl;

    if (runTime.write())
    {
        finalMesh.write();
    }

    //pmg.write();

    Info << "\nEnd\n" << endl;
    return 0;
}

// ************************************************************************* //
