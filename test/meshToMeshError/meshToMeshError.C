/**
 * Calculate error at cell centers, and optinoally random ponits of a given mesh
 * compared to a reference mesh 
 *
 * The reference mesh and field(s) is read from `meshSource` subdirectory inside
 * time directories, while the final (target) mesh and fields whose error is to
 * be calculated compared to the former are simply inside time directories,
 * e.g., `1/fieldTarget`, `2/fieldTarget`, etc, and `1/polyMesh`, `2/polyMesh`,
 * etc. This is done by reading `meshTarget` from `polyMesh::defaultRegion`, and
 * registering `fieldTarget`s with that.
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Started at Jan 30 2025
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
#include "leastSquaresVolPointInterpolation.H"
#include "leastSquaresGrad.H"
#include "IOobjectList.H"
#include "err.H"

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;

typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

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

    // Source case
    argList::validOptions.insert("sourceCase","dir");

    // Whether the reference is the correct answer on meshTarget or the
    // approximate answer on meshSource
    argList::validOptions.insert("atSourceCellCenters","");

    // Default behaviour is bounded, i.e., out-of-bound extrapolated values 
    // will be replaced by fieldSource min/max.
    argList::validOptions.insert("unbounded","");

    argList::validOptions.insert("a","value");
    argList::validOptions.insert("b","value");
    argList::validOptions.insert("c","value");
    argList::validOptions.insert("d","value");

    argList::validOptions.insert("noPrescribedFields","");

    argList args(argc, argv);

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );


    /// meshSource
    fvMesh meshSource
    (
        IOobject
        (
            polyMesh::defaultRegion,
            args.options()["sourceCase"],
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    /// meshTarget
    fvMesh meshTarget
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        )
    );

    /// Create the error object
    /// Make sure every argument recognised by err<> is passed to it
    err<scalar> scalarFieldError
    (
        args,
        meshSource,
        meshTarget
    );

    /// fieldName at startTime is regarded as the target solution, i.e., a
    /// reference in error calculation.
    /// Workflow: 
    ///  - We set the target solution by setFields, algebraicSetFields, etc;  

    instantList times = runTime.times();

    /// Evaluate the error for all times, skip constant and startTime
    for (label timeI = 2; timeI < times.size(); ++timeI)
    {
        /// Update runTime
        runTime.setTime(times[timeI], timeI);

        Info<< "Time: "
            << runTime.value()
            << endl;

        /// Update mesh, if it has changed
        //meshTarget.readUpdate();
        scalarFieldError.updateMeshTarget();

        Info<< "Mesh updated"
            << endl;

        //Info<< "Mesh target updated."
        //    << endl;

        /// Update (read) fields and calculate the error
        //calcErr<scalar>
        //(
        //    meshSource, 
        //    meshTarget, 
        //    //args.optionFound("randomPoints")
        //    args
        //);

        //scalarFieldError.calcErr();
        scalarFieldError.calcErr_2();

        Info<< "meshToMeshError: Error calculated"
            << endl;

        /// Report error
        scalarFieldError.report();

        //Info<< "Error reported."
        //    << endl;
    }

    Info<< nl << "End."
        << endl;
}
