/**
 * Flatten a specific symmetry boundary
 *
 * \author: Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin
 * \date: May 04 2025
 */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
//#include "fvMartices.H"
#include "pointMesh.H"
#include "pointPatch.H"
#include "pointBoundaryMesh.H"
#include "scalar.H"

namespace Foam
{
    class meshToBeFlattened
    {
        /// Private
        const fvMesh/*&*/     mesh_;
    
        public:
            /// Constructor
            meshToBeFlattened(const Time&);
    
            /// Flatten
            void flatten(const word&);

            /// Write
            void write();
    };
}

Foam::meshToBeFlattened::meshToBeFlattened(const Time& runTime)
:
mesh_
(
    IOobject
    (
        polyMesh::defaultRegion,
        runTime,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    )
)
{
    Info<< "Mesh read from "
        << runTime.timeName()
        << endl;
}

void Foam::meshToBeFlattened::flatten(const word& patchName)
{
    /// Save mesh points
    const pointField& pf = mesh_.points();

    /// Create new points
    pointField npf = pf;

    /// Create the point mesh
    const pointMesh pm(mesh_);

    /// Read point patches
    const pointBoundaryMesh& pbm = pm.boundary();

    /// Find the patch
    forAll(pbm, patchI)
    {
        if (pbm[patchI].name() == patchName)
        {
            Info<< "Patch "
                << patchName
                << " found."
                << endl;

            const pointPatch& pp = pbm[patchI];

            const labelList& plabels = pp.meshPoints();

            /// To be read from args
            const scalar tol = 1e-9;

            /// To be read from args
            const scalar zCoord = 0.0;

            forAll(plabels, pointI)
            {
                /// Check the z coordinates of points on
                /// patchName
                //if 
                //(
                //    abs
                //    (
                //        pf[plabels[pointI]][2] 
                //      - zCoord
                //    ) > tol
                //)
                {
                    /// Reset npf, to make sure correct
                    /// initialisation
                    npf[plabels[pointI]] 
                  = pf[plabels[pointI]];

                    /// Change z coordinates
                    npf[plabels[pointI]][2] = 0;

                    //Info<< "Point "
                    //    << plabels[pointI]
                    //    << " spotted for flattening ..."
                    //    << endl;
                }
                //else
                //{
                //    /// Only reset npf, to make sure correct
                //    /// initialisation
                //    npf[plabels[pointI]] 
                //  = pf[plabels[pointI]];
                //}
            }

            break;
        }
    }

    const_cast<fvMesh&>(mesh_).movePoints(npf);
}

void Foam::meshToBeFlattened::write()
{
    mesh_.write();
}

using namespace Foam;

int main(int argc, char* argv[])
{
    /// Process user arguments
    argList::validArgs.append("patchName");

    argList args(argc, argv);

    /// Create time
    const Time runTime("controlDict", args.rootPath(), args.caseName());

    Info<< "Time = "
        << runTime.timeName()
        << endl;

    /// Create the mesh object
    meshToBeFlattened mesh(runTime);

    /// Flatten
    mesh.flatten(args.additionalArgs()[0]);

    /// Write
    const_cast<Time&>(runTime)++; mesh.write();

    Info<< "Flattened mesh written to "
        << runTime.timeName()
        << "\n\n"
        << "End"
        << endl;
}
