/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.0
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Application
    smoothMesh

Description
    Smoothing mesh using Laplacian smoothing

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "timeSelector.H"
#include "objectRegistry.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "boolList.H"
#include "emptyPolyPatch.H"
#include "twoDPointCorrector.H"

// degToRad
#include "unitConversion.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    timeSelector::addOptions();

    argList::validOptions.insert("smoothPatches", "");

    //argList::validArgs.append("patchWithSlidingPoints", "patchName");
    //argList::validArgs.append("patchWithSlidingPoints");
    argList::validOptions.insert("patchWithSlidingPoints", "patch name");

    argList::validOptions.insert("freePatches", "list");

    argList::validOptions.insert("crossingPatches", "list");

    argList::validOptions.insert("fixCrossingPoints", "");

    argList::validOptions.insert("slideCrossingPoints", "");

    argList::validOptions.insert("maxIter", "<N>, or default to 1000");
    argList::validOptions.insert("maxRes", "maxRes or 1e-3");

    argList::validOptions.insert("noWedge", "");
    argList::validOptions.insert("wedgeDir", "0, 1, or 2");
    argList::validOptions.insert("emptyDir", "0, 1, or 2");
    argList::validOptions.insert("radialDir", "0, 1, or 2");
    argList::validOptions.insert("wedgeAperture", "in degrees");

#   include "setRootCase.H"
#   include "createTime.H"
    //instantList timeDirs = timeSelector::select0(runTime, args);
#   include "createMesh.H"

    // bool smoothPatches = args.optionFound("smoothPatches");
    // if(smoothPatches)
    //   Info << "Smoothing patches" << nl << endl;

    // Smooth internal points

    const vectorField& oldPoints = mesh.points();

    vectorField newPoints = oldPoints;

    const List<string> patchesWithFreePoints = 
        args.optionReadList<string>(word("freePatches"));

    const List<string> crossingPatches = 
        args.optionReadList<string>(word("crossingPatches"));

    bool fixCrossingPoints = args.optionFound("fixCrossingPoints");

    bool slideCrossingPoints = args.optionFound("slideCrossingPoints");

    label maxIter = 1000;

    if (args.optionFound("maxIter"))
    {
        maxIter = label(readScalar(IStringStream(args.options()["maxIter"])()));
    }

    scalar maxRes = 1e-3;

    if (args.optionFound("maxRes"))
    {
        maxRes = readScalar(IStringStream(args.options()["maxRes"])());
    }

    //if (mesh.nGeometricD() == 3)
    {
        Info << "3-D mesh" << endl;

        const labelListList& pointEdges = mesh.pointEdges();
        const edgeList& edges = mesh.edges();

        boolList fixedPoints(newPoints.size(), false);

        boolList slidingPoints(newPoints.size(), false);

        boolList freePoints(newPoints.size(), false);

        labelList crossingPoints(newPoints.size(), 0);

        //const word patchWithSlidingPoints = args.additionalArgs()[0];
        const word patchWithSlidingPoints = 
            IStringStream(args.options()["patchWithSlidingPoints"])();

        /// Flag points based on the patch they belong to
        forAll(mesh.boundaryMesh(), patchI)
        {
            const polyPatch& patch = mesh.boundaryMesh()[patchI];
 
            const labelList& meshPoints =
                mesh.boundaryMesh()[patchI].meshPoints();

            bool skip = false;

            if (patch.name() == patchWithSlidingPoints)
            {
                Info<< "Flag points on patch "
                    << patch.name()
                    << " as sliding."
                    << endl;

                forAll(meshPoints, pointI)
                {
                    slidingPoints[meshPoints[pointI]] = true;
                }

                skip = true;
            }

            if (!skip)
            {
                /// Points on front and back patches are free to move, so not fixed
                /// Is `patch` free, i.e., is it in patchesWithFreePoints?
                forAll(patchesWithFreePoints, freePatchI)
                {
                    if (patch.name() == patchesWithFreePoints[freePatchI])
                    {
                        Info<< "Flag points on patch "
                            << patch.name()
                            << " as free."
                            << endl;

                        forAll(meshPoints, pointI)
                        {
                            freePoints[meshPoints[pointI]] = true;
                        }

                        skip = true;

                        /// One patch at a time, as `patch` is a specific patch
                        break;
                    }
                }
                /// Other points on patches are fixed.
                /// Filter-out points on axis patch
                if (!skip)
                {
                    Info<< "Flag points on patch "
                        << patch.name()
                        << " as fixed."
                        << endl;

                    forAll(meshPoints, pointI)
                    {
                        /// Re-include corner points, where axis intersects other
                        /// patches
                        fixedPoints[meshPoints[pointI]] = true;
                    }
                }
            }
        }

        /// Flag points based on the crossing edges between two patches
        forAll(crossingPatches, crossI)
        {
            const label patchID = 
                mesh.boundaryMesh().findPatchID(crossingPatches[crossI]);

            //const polyPatch patch = mesh.boundaryMesh()[patchID];
            const labelList meshPoints =
                mesh.boundaryMesh()[patchID].meshPoints();

            forAll(meshPoints, pI)
            {
                /// crossingPoints[i] > 1 is a point on the common edge
                ++crossingPoints[meshPoints[pI]];
            }
        }

        /// Debug
        //forAll(crossingPoints, pointI)
        //{
        //    if (crossingPoints[pointI] > 1) 
        //    {
        //        Info<< "crossingPoints["
        //            << pointI
        //            << "] = "
        //            << crossingPoints[pointI] 
        //            << endl;
        //    }
        //}

        scalarField residual(newPoints.size(), 0);
        label counter = 0;

        do
        {
            counter++;

            Info<< "Iteration: "
                << counter
                << " out of "
                << maxIter
                << endl;

            forAll(newPoints, pointI)
            {
                /// Only move non-fixed points,
                /// i.e., points not flagged as fixed, or points not belonging
                /// to more that one patch
                //if 
                //(
                //    !fixedPoints[pointI] /*||*/ && 
                //    !(
                //         fixCrossingPoints &&
                //         crossingPoints[pointI] > 1
                //     )
                //)
                {
                    vector curNewPoint = vector::zero;

                    scalar sumW = 0;

                    forAll(pointEdges[pointI], eI)
                    {
                        label curEdgeIndex = pointEdges[pointI][eI];

                        const edge& curEdge = edges[curEdgeIndex];

                        vector d =
                            newPoints[curEdge.otherVertex(pointI)]
                          - newPoints[pointI];

                        scalar w = 1.0;

                        curNewPoint += w*d;

                        sumW += w;
                    }

                    curNewPoint /= sumW;

                    curNewPoint += newPoints[pointI];

                    residual[pointI] = mag(curNewPoint - newPoints[pointI]);

                    if (!fixedPoints[pointI])
                    {
                        if 
                        (
                            !slidingPoints[pointI]
                            &&
                            !(
                                slideCrossingPoints &&
                                crossingPoints[pointI] > 1
                             )/*&& !freePoints[pointI]*/
                        )
                        {
                            newPoints[pointI][0] = curNewPoint[0];
                            newPoints[pointI][1] = curNewPoint[1];
                            if 
                            (
                                !args.optionFound("noWedge") &&
                                args.optionFound("wedgeDir") &&
                                args.optionFound("radialDir") &&
                                args.optionFound("wedgeAperture")
                            )
                            {
                                label wedgeDir = label
                                (
                                    readScalar
                                    (IStringStream(args.options()["wedgeDir"])())
                                );

                                label radialDir = label
                                (
                                    readScalar
                                    (IStringStream(args.options()["radialDir"])())
                                );

                                for (label dir = 0; dir < 3; ++dir)
                                {
                                    if (dir != wedgeDir)
                                    newPoints[pointI][dir] = curNewPoint[dir];
                                }

                                // Safeguard denomirator from vanishing
                                if (curNewPoint[wedgeDir] > SMALL)
                                {
                                    newPoints[pointI][wedgeDir] =
                                    // Sign (+/-)
                                    curNewPoint[wedgeDir]
                                    /// abs(curNewPoint[wedgeDir]) // Error:
                                    // returns 0
                                    / Foam::sqrt(pow(curNewPoint[wedgeDir],2))
                                    // radialDirComponent * tan(aperture)
                                    * curNewPoint[radialDir]
                                    * Foam::tan
                                      (
                                          degToRad
                                          (
                                              readScalar
                                              (IStringStream(args
                                              .options()["wedgeAperture"])())
                                          ) / 2
                                      );
                                }
                            }
                            else if 
                            (
                                (
                                    !args.optionFound("noWedge") &&
                                    !args.optionFound("wedgeDir") &&
                                    args.optionFound("radialDir") &&
                                    args.optionFound("wedgeAperture")
                                )
                                ||
                                (
                                    !args.optionFound("noWedge") &&
                                    args.optionFound("wedgeDir") &&
                                    args.optionFound("radialDir") &&
                                    !args.optionFound("wedgeAperture")
                                )
                                ||
                                (
                                    !args.optionFound("noWedge") &&
                                    args.optionFound("wedgeDir") &&
                                    !args.optionFound("radialDir") &&
                                    args.optionFound("wedgeAperture")
                                )
                            )
                            {
                                FatalErrorIn("Smoother loop")
                                    << "-wedgeDir, -radialDir, and " 
                                    << "-wedgeAperture must be "
                                    << "specified together."
                                    << abort(FatalError);
                            }
                            else if
                            (
                                args.optionFound("noWedge") &&
                                args.optionFound("emptyDir")
                            )
                            {
                                label emptyDir = label
                                (
                                    readScalar
                                    (IStringStream(args.options()["emptyDir"])())
                                );

                                for (label dir = 0; dir < 3; ++dir)
                                {
                                    if (dir != emptyDir)
                                    newPoints[pointI][dir] = curNewPoint[dir];
                                }
                            }
                            else if
                            (
                                (
                                    !args.optionFound("noWedge") &&
                                    args.optionFound("emptyDir")
                                )
                                ||
                                (
                                    args.optionFound("noWedge") &&
                                    !args.optionFound("emptyDir")
                                )
                            )
                            {
                                FatalErrorIn("Smoother loop")
                                    << "-noWedge and -emptyDir must be "
                                    << "specified together."
                                    << abort(FatalError);
                            }

                        }

                        else if
                        (
                            //slidingPoints[pointI] ||
                            (
                                slideCrossingPoints &&
                                crossingPoints[pointI] > 1
                            )/*&& !freePoints[pointI]*/
                        )
                        {
                            newPoints[pointI][0] = curNewPoint[0];
                        }
                    }
                }
            }

            residual /= max(mag(newPoints - oldPoints) + SMALL);
        }
        while(max(residual) > maxRes && maxIter > counter);

        Info << "Internal points, max residual: " << max(residual)
            << ", num of iterations: " << counter << endl;

        twoDPointCorrector twoDCorrector(mesh);
        twoDCorrector.correctPoints(newPoints);
    }

    twoDPointCorrector twoDCorrector(mesh);
    twoDCorrector.correctPoints(newPoints);

    mesh.movePoints(newPoints);

    runTime++;

    // Does not work
    //runTime.write();

    mesh.write();

    Info<< "New mesh written to "
        << runTime.timeName()
        << endl;

    Info<< "End\n" << endl;

    return 0;
}


// ************************************************************************* //
