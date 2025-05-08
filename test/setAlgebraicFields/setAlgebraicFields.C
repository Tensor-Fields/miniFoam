/**
 * Set fields using a given formula
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Feb 22 2025
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
//#include "zeroGradientFvPatchFields.H"
//#include "zeroGradientPointPatchFields.H"
//#include "ZeroGradientPointPatchField.H"
#include "leastSquaresVolPointInterpolation.H"
#include "leastSquaresGrad.H"
#include "IOobjectList.H"

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;
typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

/// Calculate the exact (prescribed) field at the cell centers of the
/// target mesh
template<class Type>
void fieldExact
(
    GeometricField<Type, fvPatchField, volMesh>& f,
    const volVectorField& meshTargetCellCenters
    //const List<point>& p
)
{
    const volVectorField& p = meshTargetCellCenters;
 
    const scalar& pi = mathematicalConstant::pi;

    forAll(p, pointI)
    {
        const scalar& x = p[pointI][0];
        const scalar& y = p[pointI][1];
        const scalar& z = p[pointI][2];

        // Foam::sin, as calling sin out of the namespace is ambigious
        // pointI is equivalent to the cell index.
        f[pointI] = 1 + Foam::sin(2 * pi * x) * Foam::cos(2 * pi * y);
    }
}

template<class Type>
void setAlgebraicFields
(
    const fvMesh& meshSource,
    const argList& args
)
{
    const Time& runTime = meshSource.time();

    IOobjectList fieldsTarget(meshSource, runTime.timeName());

    typedef
    GeometricField
    <
        Type, fvPatchField, volMesh
    > fieldType;

    IOobjectList objectsTarget = fieldsTarget.lookupClass(word(fieldType::typeName));

    for 
    (
        IOobjectList::iterator iteratorTarget = objectsTarget.begin();
        iteratorTarget != objectsTarget.end();
        ++iteratorTarget
    )
    {
        //const IOobject fieldSourceIO
        //(
        //    iteratorTarget->name(),
        //    meshSource,
        //    IOobject::MUST_READ,
        //    IOobject::NO_WRITE
        //);

        /// Create fields
        GeometricField
        <
            Type, fvPatchField, volMesh
        //> fieldSource(fieldSourceIO, meshSource);
        > fieldSource(*iteratorTarget(), meshSource);

        Info<< "Source field "
            << fieldSource.name()
            << " found at "
            << fieldSource.path()
            << endl;

       /// Evaluate fields
       fieldExact(fieldSource, meshSource.C());

       /// Overwrite fields
       fieldSource.write();
    }
}

int main (int argc, char* argv[])
{
    /// N rand points per cell can be variable for each cell, however, the
    /// curren pass-by-argument accepts a fixed number. Alternative
    /// generalization is:
    /// - pass a list specifying N for each cell
    /// - hard code how many random points per cell
    //argList::validArgs.append("No. rand points per cell");

    // N is read as an option's argument, not a mandatory arg.
    argList::validOptions.insert("randomPoints","No. of random points");

    // Whether the reference is the correct answer on meshTarget or the
    // approximate answer on meshSource
    argList::validOptions.insert("atSourceCellCenters","");

    // Default behaviour is bounded, i.e., out-of-bound extrapolated values
    // will be replaced by fieldSource min/max.
    argList::validOptions.insert("unbounded","");

    argList args(argc, argv);

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );

    const word& startTime = runTime.startTime().name();

    fvMesh meshSource
    (
        IOobject
        (
            //polyMesh::defaultRegion,
            "meshSource",
            //runTime.timeName(),
            startTime,
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    /// Update (read) fields by initializing them
    setAlgebraicFields<scalar>
    (
        meshSource, 
        //args.optionFound("randomPoints")
        args
    );

    Info<< nl << "End."
        << endl;
}
