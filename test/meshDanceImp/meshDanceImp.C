/**
* @brief Description
* A code to read a scalar field on a particular mesh, move the mesh nodes while
* preserving topology, and calculate the field on the new mesh.
* 
* - Theory
* Writing the integral transport equation for a variable $T$ leads to
* 
* \begin{align}
* \frac{d}{dt}\int_{AR}\rho T \mathrm{d}V = 0 + 
* \int_{AR}\rho T n_i (u_i - {u_b}_i)\mathrm{d}S
* \end{align}
* 
* where AR is an arbitrary region, $u_i$ is the material velocity, {u_b}_i is
* the boundary velocity for the arbitrary region, and $0$ on the RHS stands for
* $\int_{AR} \rho \frac{\mathrm{d}T}{\mathrm{d}t}\mathrm{d}V$, which is $0$,
* because we are solving for a fixed time, i.e., $t$ does not exist in our
* problem.
* 
* - Dependencies
* foam-extend-4.0
* 
* - Roadmap
* For a given mesh associated with some fields, increase mesh quality,
* read all fields into a \c for loop,
* advect them implicitly (2nd-order accurate), and write the results.
*
* - Signature
* Maalik, Maxwell Corner, Cambridge, 040924
* ali@tensorfields.com
*/

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "IOobject.H"
#include "GeometricField.H"
#include "vector.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "pointField.H"
#include "scalar.H"
#include "faceList.H"
#include "labelList.H"
#include "Xfer.H"
#include "pointPatchField.H"
#include "pointMesh.H"
#include "dimensionedVector.H"
#include "dimensionSets.H"
#include "fvc.H"
#include "fvm.H"
//#include "boundaryMesh.H"
//#include "fvBoundaryMesh.H"
//#include "meshReader.H"
/** forAllIter definition */
//#include "UList.H"
#include "IOobjectList.H"

/**
 * Algorithm:
 *
 * Read fields every time step, advect them in the
 * memory; write to the disk every time step,
 * update fields by reading them again 
 * from the disk.
 *
 * For fields to get updated you need to write
 * every field to the disk and read it again,
 * because `fields` is a HashTable of const 
 * `field`s.
 * 
 * .--> Read fields from the disk into `fields`
 * |    container
 * |    |
 * |    V
 * |    for `field` in `fields`
 * |    |
 * |    .-> const-cast every single field into 
 * |        `field` container
 * |        |
 * |        V
 * |        Advect `field`
 * |        |
 * |        V
 * |    .-- Write `field`
 * |    |
 * |    V
 * .-<- Next timestep
 *
 * Possible alternative algorithms: 
 *
 * Read fields only once, and advect them in the
 * memory; update fields in the memory after
 * advection; write based on time controls.
 *
 *      Read fields from the disk into `fields`
 *      container
 *      |
 *      V
 * .--> for `field` in `fields`
 * |    |
 * |    .-> const-cast every single field into 
 * |        `field` container
 * |        |
 * |        V
 * |        Advect `field`
 * |        |
 * |        V
 * |    .-- Update `fields`
 * |    |
 * |    V
 * |    Write `fields`
 * |    |
 * |    V
 * .-<- Next timestep
 */

using namespace Foam;

/// A collection of point motion engines
#include "newPoints.H"

/// pointGenerator class
#include "Random.H"

class pointGenerator
{
    /// Private
    pointField newPoints_;
    const scalar alpha_;

    // Public
    public:
        pointGenerator(const pointField&, const scalar&);
        //pointField randomize();
        //const pointField operator++(pointGenerator&)
};

/// Constructor
pointGenerator::pointGenerator(const pointField& oldPoints, const scalar& alpha)
:
newPoints_(oldPoints),
alpha_(alpha)
{}

/// Member functions
//pointField pointGenerator::randomize()
//{
//    /// Create the random vector
//    Random rand(12345);
//
//    /// Create a list of "point increment"s, whose components are random,
//    /// between -1 and 1.
//    /// From Lipnikov and Shashkov, Journal of Computational Physics 474(2023)
//    /// 111822
//
//    /// Initialize F's size
//    pointField F(newPoints_.size());
//
//    /// Evaluate F
//    forAll(F, pointI)
//    {
//        F[pointI][0] = rand.scalar01() * 2 - 1;
//        F[pointI][1] = rand.scalar01() * 2 - 1;
//        F[pointI][2] = rand.scalar01() * 2 - 1;
//    }
//
//    //newPoints_ += alpha_ * F;
//
//    //return newPoints_;
//
//    return (newPoints_ += alpha_ * F);
//}

/// Member operator
/// \note: Is this a better idea, or randomize() directly returning pointField?
/// This seems to be better ... because, randomize, and return are separate,
/// it looks more modular. Does it ...?
//const pointField pointGenerator::operator++(pointGenerator& pGen)
//{
//    randomize();
//
//    return newPoints_;
//}

typedef GeometricField<scalar, fvPatchField, volMesh> volScalarField;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;

typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

#include "CreateFields.H"
#include "fluxSign.H"
#include "AdvectFields.H"

int main(int argc, char* argv[])
{
    argList::validArgs.append("Smooth mesh directory name (in constant)");
    //argList::validArgs.append
    //    ("true/false if temporal term is ddt(T,V)/ddt(T)");
    argList::validOptions.insert("flipFluxSign","");

    argList args(argc, argv);

    const word& smoothMeshDir = args.additionalArgs()[0];

    #include "createTime.H"
    #include "createMesh.H"

    vectorField totalMove = overallMotion
    (
        mesh,
        fvMesh
        (
            IOobject
            (
                smoothMeshDir,
                runTime.constant(),
                runTime,
                IOobject::MUST_READ
            )
        )
    );

    /// Create IOobjects from existing field files
    IOobjectList objects(mesh, runTime.timeName());

    CreateFields<scalar>(objects, mesh);

    #include "createTimeControls.H"

    while (runTime.run()/*runTime.loop()*/)
    {
        /// Defaults maxCo = 1, adjustTimeStep = false.
        #include "readTimeControls.H"

        #include "meshCourantNo.H"

        scalar& CoNum = meshCoNum;

        /// Re-set time step
        /// Place it inside time loop for online interaction with Co settings
        if (adjustTimeStep)
        {
            #include "setDeltaT.H"
        }

        /// Increment time after flux calculation: Error; segfault
        runTime++;

        Info<< "Time = "
            << runTime.timeName()
            << endl;

        // Force V allocation for V0 to be stored
        //const scalar totalVol = gSum(mesh.V());

        /// New points generator
        //pointGenerator    pGen(mesh.points(), 0.1);

        // Uses adjusted deltaT
        pointField newPoints = 
        incrementPoints
        (
            mesh,
            totalMove
        );

        mesh.movePoints
        (
            newPoints

            /// Increment points
            //pointGenerator::operator++(pGen)
            //pGen.randomize()
        );

        {
            //#include "meshCourantNo.H";
        }

        //const surfaceScalarField& phi = mesh.phi();

	AdvectFields<scalar>(mesh, args.optionFound("flipFluxSign"));

        if (runTime.write()) // Time dirs written without fields
        {
            mesh.write(); // Time dirs written without fields
            
            Info<< "    Fields written to "
                << runTime.timeName()
                << endl;
        }

        //mesh["T"]->write(); // Magic :]
    }

    Info<< "End."
        << endl;
}
