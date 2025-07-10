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
    const volVectorField& meshTargetCellCenters,
    const argList args
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
        const scalar a = readScalar(IStringStream(args.options()["a"])());
        const scalar b = readScalar(IStringStream(args.options()["b"])());
        const scalar c = readScalar(IStringStream(args.options()["c"])());
        const scalar d = readScalar(IStringStream(args.options()["d"])());

        if (args.optionFound("sinsin"))
        {
            f[pointI] = a + b * Foam::sin(c * pi * x) * Foam::sin(d * pi * y);
        }
        else
        {
            f[pointI] = a + b * Foam::sin(c * pi * x) * Foam::cos(d * pi * y);
        }
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
       fieldExact(fieldSource, meshSource.C(), args);

       /// Overwrite fields
       fieldSource.write();
    }
}

int main (int argc, char* argv[])
{
    argList::validOptions.insert("a","value");
    argList::validOptions.insert("b","value");
    argList::validOptions.insert("c","value");
    argList::validOptions.insert("d","value");
    argList::validOptions.insert("h","");
    argList::validOptions.insert("sinsin","");

    argList args(argc, argv);

    if (args.optionFound("h"))
    {
        Info<< "Creates the field fieldName,\n"
            << "fieldName = a + b sin(c * pi * x) cos(d * pi * y)\n\n"
            << "of if `-sinsin` is specified,\n\n"
            << "fieldName = a + b sin(c * pi * x) sin(d * pi * y)\n\n"
            << "Usage:\n\n"
            << "setAlgebraicFields \\\n"
            << "    -a <value> -b <value> -c <value> -d <value> \\\n"
            << "    -region <name>\n\n"
            << "All options are \"optional\". If values not provided, all\n"
            << "coefficients are `1`."
            << endl;

        return 0;
    }

    word regionName = polyMesh::defaultRegion;

    if (args.optionFound("region"))
    {
        regionName = args.options()["region"];
    }

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );

    const word& startTime = runTime.startTime().name();

    fvMesh mesh
    (
        IOobject
        (
            regionName,
            startTime,
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    /// Update (read) fields by initializing them
    setAlgebraicFields<scalar>
    (
        mesh, 
        args
    );

    Info<< nl << "End."
        << endl;
}
