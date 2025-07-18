#include "meshWire.H"
#include "Random.H"

/// Constructor
Foam::meshWire::meshWire
(
    const pointField& oldPoints, 
    const scalar& alpha,
    const boolList& isRandom,
    const boolList& isPatch,
    const fvMesh& mesh,
    const bool fixPatches
)
:
pointGenerator
(
     oldPoints, 
     alpha,
     isRandom,
     isPatch,
     fixPatches
),
mesh_(mesh),
meshPointEdges_(mesh_.pointEdges()),
/// If a direction isRandom, it is not fixed
/// So, if it is fixed, it is not random
fixedDir_
(
    scalar(!isRandom_[0]), 
    scalar(!isRandom_[1]), 
    scalar(!isRandom_[2])
),
meshEdges_(mesh_.edges()),
wire_(2)
{}

/// Is parallel
bool Foam::meshWire::isParallel
(
    const Foam::vector& a, 
    const Foam::vector& b
)
{
    if (mag(a ^ b) < SMALL) return true;
    
    return false;
}

/// Find the wire connected to pointI
void Foam::meshWire::wire(const label& pointI)
{
    bool wireEndFound = false;

    /// For all edges connected to pointI
    forAll(meshPointEdges_[pointI], edgeI)
    {
        //Info<<"a="<< meshEdges_[meshPointEdges_[pointI][edgeI]]
        //             .vec(newPoints_)
        //    <<"\n"<<"b="
        //    << fixedDir_
        //    << endl;

        /// If edge is parallel to fixedDir_, this edge is the wire
        if 
        (
            isParallel
            (
                meshEdges_[meshPointEdges_[pointI][edgeI]]
                .vec(newPoints_), 
                fixedDir_
            )
        )
        {
            /// Wire starting point label
            wire_[0] = meshEdges_[meshPointEdges_[pointI][edgeI]].start();

            /// Wire end point label
            wire_[1] = meshEdges_[meshPointEdges_[pointI][edgeI]].end();

            wireEndFound = true;

            break;
        }
    }

    if (!wireEndFound)
    {
        FatalErrorIn("wire(point&):\n")
            << "Could not find the corresponding point"
            << abort(FatalError);
            //<< endl;
    }
}

Foam::pointField Foam::meshWire::randomize
(const scalar& seed)
{
    ///
    /// Create the random vector
    ///

    /// Seed
    Random rand(seed);

    /// Create a list of "point increment"s, whose components are random,
    /// between -1 and 1.
    /// From Lipnikov and Shashkov, Journal of Computational Physics 474(2023)
    /// 111822

    /// Initialize F's size
    pointField F(newPoints_.size());

    /// Evaluate F
    forAll(F, pointI)
    {
        if (!(isPatch_[pointI]) || (isPatch_[pointI] && !fixPatches_))
        {
            ///
            /// Find the wire corresponding to pointI
            ///
            wire(pointI);

            //Info<< "\nwire created\n"
            //    << endl;

            forAll(isRandom_, dirI)
            {
                if (isRandom_[dirI])
                {
                    //F[pointI][dirI] = rand.scalar01() * 2 - 1;
                    const scalar d = rand.scalar01() * 2 - 1;

                    /// \note: To avoid duplicate search, the front and back
                    /// patches on which wire end points reside should have
                    /// different names. In other words, the two ends of a
                    /// wire should reside on two distinct patches.
                    F[wire_[0]][dirI] = d;
                    F[wire_[1]][dirI] = d;
                }
                else if (!isRandom_[dirI])
                {

                    /// F being zero, leads to dirI staying unchanged.
                    F[pointI][dirI] = 0;
                }
                else
                {
                    FatalErrorIn("randomize()")
                        << "isRandom_[dirI] is neither true nor false"
                        << endl;
                }

                //Info<< F[pointI][dirI]
                //    << endl;
            }
        }
        else if (isPatch_[pointI] && fixPatches_)
        {
            forAll(F[pointI], dirI)
            {
                F[pointI][dirI] = 0;
            }
        }
    }

    newPoints_ += alpha_ * F;

    return newPoints_;

    //return (newPoints_ += alpha_ * F); // Error: cannot convert void to the stuff
}
