/**
 * Calculate error at cell centers, and optionally random ponits of a given mesh
 * compared to a reference mesh 
 *
 * Usage 1:
 * readlink -f <source> | xargs dirname | xargs meshToMeshError <options> \
 *     -sourceAbsolutePath
 *
 * Usage 2:
 * meshToMeshError <options> -sourceCase <sourceCaseName>
 *
 * Compares all dirs of the targetDir with the first available timeDir in
 * <source>.
 * In the alternative usage, sourceAbsolutePath is automatically extracted.
 *
 * Old \note:
 * We could have used `fileName::path()`, to use one option only:
 * `-sourceCase ...`
 * instead of:
 * `-sourceCase ... -sourceAbsolutePath ...`.
 * However, in that case, if we pass `-sourceCase foo/bar/`,
 * `path()` fails to output the path `foo/`, instead it
 * returns `foo/bar/`, unless user be careful to not include the last slash.
 *
 * Old Description:
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
    argList::validOptions.insert("sourceCase","caseName");
    argList::validOptions.insert("sourceAbsolutePath","dir");

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

    argList::validOptions.insert("latestTargetTime", "");
    argList::validOptions.insert("latestSourceTime", "");

    /// Call error calculation between same meshes
    argList::validOptions.insert("consistent", "");

    argList args(argc, argv);

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );

    /// Init same as runTime. Copy is not implemented.
    autoPtr<Time> runTimeSourcePtr /*runTime;*/
    (
        new Time
        (
            "controlDict",
            args.rootPath(),
            args.caseName()
        )
    );

    if
    (
        args.optionFound("sourceAbsolutePath")
     && args.optionFound("sourceCase")
    )
    {
        runTimeSourcePtr.reset
        (
            new Time
            (
                word("controlDict"),
                fileName(args.options()["sourceAbsolutePath"]),
                fileName(args.options()["sourceCase"])
            )
        );
    }
    else if
    (
        !args.optionFound("sourceAbsolutePath")
     && args.optionFound("sourceCase")
    )
    {
        fileName sourceCase(args.options()["sourceCase"]);

        runTimeSourcePtr.reset
        (
            new Time
            (
                word("controlDict"),
                sourceCase.path(),
                fileName(sourceCase.name())
            )
        );
    }
    else if
    (
        args.optionFound("sourceAbsolutePath")
     && !args.optionFound("sourceCase")
    )
    {
        FatalErrorIn("")
            << "-sourceAbsolutePath should go with -sourceCase"
            << abort(FatalError);
    }

    /// meshSource
    fvMesh meshSource
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTimeSourcePtr(),
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
    const bool bounded = args.optionFound("unbounded");

    err<scalar> scalarFieldError
    (
        //args,
        meshSource,
        meshTarget,
        args.optionFound("atSourceCellCenters"),
        bounded
    );

    //err<vector> vectorFieldError
    //(
    //    args,
    //    meshSource,
    //    meshTarget
    //);

    /// fieldName at startTime is regarded as the target solution, i.e., a
    /// reference in error calculation.
    /// Workflow: 
    ///  - We set the target solution by setFields, algebraicSetFields, etc;  

    /// Target times
    instantList times = runTime.times();
    /// Source times
    instantList sourceTimes = runTimeSourcePtr().times();
    
    bool latestTargetTime = args.optionFound("latestTargetTime");
    bool latestSourceTime = args.optionFound("latestSourceTime");

    label targetStartTimeIndex = 2;

    if (latestTargetTime)
    {
        targetStartTimeIndex = times.size() - 1;
    }

    label sourceTimeIndex = 0;

    if (latestSourceTime)
    {
        sourceTimeIndex = sourceTimes.size() - 1;
    }

    /// Evaluate the error for all target times, skip constant and startTime
    for (label timeI = targetStartTimeIndex; timeI < times.size(); ++timeI)
    //while (runTime.loop())
    {
        /// Update runTime (targetTime) and sourceTime
        runTime.setTime(times[timeI], timeI);
        runTimeSourcePtr().setTime(times[timeI], timeI);

        Info<< "Time: "
            << runTime.value()
            << endl;

        /// Update mesh, if it has changed
        //meshTarget.readUpdate();
        scalarFieldError.updateMeshTarget();
        //vectorFieldError.updateMeshTarget();

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

        if (args.optionFound("consistent"))
        {
            scalarFieldError
            .calcErr_wrt_same_mesh
            (
                args.options()["sourceCase"],
                sourceTimes[sourceTimeIndex].name()
            );
        }
        else
        {
			/// true: Write error field at meshTarget time dirs
			/// Default to false
            scalarFieldError.calcErr(true);
            //vectorFieldError.calcErr();
        }

        //Info<< "meshToMeshError: Error calculated"
        //    << endl;

        /// Report error
        //scalarFieldError.report();
        //vectorFieldError.report();

        //Info<< "Error reported."
        //    << endl;

		//if (runTime.write())
		//{
		//  scalarFieldError.errField().write();
		//}
    }

    Info<< nl << "End."
        << endl;
}
