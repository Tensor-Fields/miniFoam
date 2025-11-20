#include "pointGenerator.H"
#include "Random.H"

/// Constructor
Foam::pointGenerator::pointGenerator
(
    const pointField& oldPoints, 
    const scalar& alpha,
    const boolList& isRandom,
    const boolList& isPatch,
    const bool fixPatches
)
:
newPoints_(oldPoints),
alpha_(alpha),
isRandom_(isRandom),
isPatch_(isPatch),
fixPatches_(fixPatches)
{}

/// Member functions
Foam::pointField Foam::pointGenerator::randomize(const scalar& seed)
{
    ///
    /// Create the random vector
    ///

    /// Seed
    //Random rand(seed_);
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
            forAll(isRandom_, dirI)
            {
                if (isRandom_[dirI])
                {
	            /// Generate a random number in [-1, 1]
                    F[pointI][dirI] = rand.scalar01() * 2 - 1;
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
