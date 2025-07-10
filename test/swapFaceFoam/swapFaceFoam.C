/**
  * You have a mesh with a given format; it is desired that the skewnees off all
  * cells is above a threshold; only face swaping is allowed.
  *
  * \author: Maalik, ali@tensorfields.com, Richmond Hill, Dublin, Ireland
  * \date: May 3 2025
  */

/// Mesh class
namespace Foam
{
class Mesh
{
    /// Private
    fvMesh mesh_;

    /// Public
    public:

        /// Constructor
        Mesh(const Time&);

        /// Members
        void swapFaces();
};
}

Foam::Mesh::Mesh(const Time& runTime)
:
/// Read the mesh
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

/// Swap faces
void Foam::Mesh::swapFaces()
{
    const labelList& cells = mesh_.C();

    forAll(cells, cellI)
    {
        if (cell.quality_measure() < quality_threshold)
        {
            mesh[cellI].swapFaces();
        }
    }
}

int main(int argc, char* argv[])
try
{
    /// Process user inputs
    argList args(argc, argv);

    /// Create time (data base)
    Time runTime("controlDict", args.rootPath(), args.rootCase());

    /// Create a Mesh object
    Mesh mesh(runTime);

    /// Swap faces when necessary
    mesh.swapFaces();
}
catch()
catch(...)
{}
