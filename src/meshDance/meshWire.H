/**
  * A mesh wire is composed of two points
  *
  * Assume a cube meshed with unit cells. The purpose is to
  * move a mesh wire connecting a point on the front face to
  * the point with the same location but on the back face,
  * to a random new position. To do so, we actually move both
  * points with the exact same displacement vector.
  *
  * \author: Maalik (Ali Shayegh), ali@tensorfields.com, RH, Dublin
  * \date: Jul 13 2025
  */

#ifndef meshWire_H
#define meshWire_H

#include "pointGenerator.H"
#include "fvMesh.H"
#include "vector.H"

namespace Foam
{
class meshWire: public pointGenerator
{
    /// Private
    private:

        const fvMesh& mesh_;
        const List<labelList> meshPointEdges_;
        const vector fixedDir_;
        const edgeList meshEdges_;

        /// A wire is a list of two points
        List<label> wire_;

        /// Private members
        void wire(const label&);

        bool isParallel(const vector& a, const vector& b);

    public:
        
        /// Constructor
        meshWire
        (
            const pointField& initialPoints,
            const scalar& randomizingAmplitude,
            const boolList& directionsToBeRandomized,
            const boolList& trueIfPatchPoint,
            const fvMesh& mesh,
            const bool fixPatches = true
        );

        /// Move wires randomely
        pointField randomize(const scalar& seed);

	/// Move wires sinusoidally
        pointField sinCycle
        (
            const scalar tau,
            const scalar maxTau,
            const scalar phi
        );
};
}

#endif
