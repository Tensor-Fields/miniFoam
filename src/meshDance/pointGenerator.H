/// pointGenerator class

#ifndef pointGenerator_H
#define pointGenerator_H

#include "Random.H"
#include "pointField.H"
#include "scalar.H"
#include "boolList.H"

namespace Foam
{
class pointGenerator
{
    /// Private

    /// Protected
    protected:
        pointField newPoints_;
        const scalar alpha_;
        const boolList isRandom_;
        const boolList isPatch_;
        const bool fixPatches_;

    // Public
    public:
        
        pointGenerator
        (
            const pointField& initialPoints,
            const scalar& randomizingAmplitude,
            const boolList& directionsToBeRandomized,
            const boolList& trueIfPatchPoint,
            const bool fixPatches = true
        );

        pointField randomize(const scalar&);
        //const pointField operator++(pointGenerator&)
};
}

#endif
