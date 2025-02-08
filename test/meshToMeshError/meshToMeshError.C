/**
 * Calculate error at cell centers of a given mesh compared to a reference mesh 
 *
 * The reference mesh and field(s) is read from `meshSource` subdirectory inside
 * time directories, while the final (target) mesh and fields whose error is to
 * be calculated compared to the former are simply inside time directories,
 * e.g., `1/fieldTarget`, `2/fieldTarget`, etc, and `1/polyMesh`, `2/polyMesh`,
 * etc. This is done by reading `meshTarget` from `polyMesh::defaultRegion`, and
 * registering `fieldTarget`s with that.
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Jan 30 2025
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
#include "IOobjectList.H"

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;
typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

template<class Type>
void calcErr
(
    const fvMesh& meshSource,
    const fvMesh& meshTarget,
    const word& startTime
)
{
    //IOobjectList fieldsSource(meshTarget, startTime);

    /// calcErr is being called in the time loop, and for times > startTime.
    const Time& runTime = meshTarget.time();

    IOobjectList fieldsTarget(meshTarget, runTime.timeName());

    typedef
    GeometricField
    <
        Type, fvPatchField, volMesh
    > fieldType;

    /// Error
    //IOobjectList objects = fields.lookupClass(word(typename fieldType));

    //IOobjectList objectsSource = fieldsSource.lookupClass(word(fieldType::typeName));
    IOobjectList objectsTarget = fieldsTarget.lookupClass(word(fieldType::typeName));

    for 
    (
        IOobjectList::iterator iteratorTarget = objectsTarget.begin();
        iteratorTarget != objectsTarget.end();
        ++iteratorTarget
    )
    {
        GeometricField
        <
            Type, fvPatchField, volMesh
        > fieldTarget(*iteratorTarget(), meshTarget);

        Info<< "Target field "
            << fieldTarget.name()
            << " found at "
            << fieldTarget.path()
            << endl;

        const IOobject fieldSourceIO
        (
            fieldTarget.name(),
            meshSource,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );

        GeometricField
        <
            Type, fvPatchField, volMesh
        > fieldSource(fieldSourceIO, meshSource);

        Info<< "Source field "
            << fieldSource.name()
            << " found at "
            << fieldSource.path()
            << endl;

        GeometricField
        <
            Type, fvPatchField, volMesh
        >
        Err
        (
            IOobject
            (
                "Err",
                runTime.timeName(),
                meshTarget,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            meshTarget,
            dimensionedScalar("zero", fieldTarget.dimensions(), 0)
        );

        /// Pseudo-code

        /// A location, (x,y,z), falls in cellI_orig in meshSource, and in cellI
        /// in meshTarget.
        
        /// The value of a field, e.g., T, at (x,y,z) is the same in both cases
        /// if the two cells enclosing this location in both cases have the same
        /// values.

        /// Calculate the error in other points than cell centers.
        bool randomPoints = false; // To be read from command line

        if(randomPoints)
        {
            /// Seed the random generator only once
            Random rand(meshSource.time().value());

            /// We need N weights for each random point, as the random point is
            /// being calculated from the vertices, where N is the number of
            /// vertices. We need to do so M times, where M is the number of
            /// random points we need inside the cell.
            label N = meshTarget.pointCells()[cellI].size();
            //N = 10; // Number of random points per cell; to be read from input
            /// args
            /// \note Not all random points fall in a given cell.

            /// Create a random point based on the weighted average of cell
            /// vertices.

            /// Calculate weights
            List<scalar> weights(N);

            scalar weightSum = 0;

            for (label i = 0; i<N; ++i)
            {
                weights[label] = rand.scalar01();

                weightSum += weights[label];
            }

            List<point> samplePoints(N);
            
            forAll(samplePoints, pointI)
            {
                /// Look into all vertices of a cell
                forAll(meshTarget.pointCells()[cellI])
                {
                    samplePoints[pointI] += weights[]; 
                }

                /// Calculate the overlapped volume between a given meshTarget
                /// cell and meshSource cells, possibly using Monte-Carlo

                /// Calculate the volume-average error
                /// This seems to be a reasonable measure;
                /// - If two cells perfectly match, but their value is
                /// different, at each point of the cell in meshTarget, the error
                /// is the difference between the values.
                /// - If a given meshTarget cell has the situation below
                ///     _________
                /// {  |  }   {  |   }   
                /// {  +  } o {  *   }   
                /// {  |  }   {  |   }   
                ///    1---------1
                /// 
                /// where {} shows the meshSource cells, `*` and `+`,
                /// their centers, and
                /// `o`, a meshTarget cell center, and if field_o == field_*,
                /// then the right half of the cell `o` is correctly evaluated,
                /// while the left half is wrong by (field_o - field_+); the
                /// weighted volume average returns the error of
                /// 0.5 * (field_o - field_+)
                /// which is reasonable; because the smaller the overlap with
                /// `+` is, the less the error becomes.
            }
        }

        /// At cellI in meshTarget
        forAll(meshTarget.C(), cellI)
        {
            const point& pointI = meshTarget.C()[cellI];
            ///    Read T
            ///    Find cellI_orig in meshSource that

            ///    - encloses meshTarget.C()[cellI]
            ///      label findCell(const point& location) const
            const label& sourceCellEnclosingThisPoint = 
                meshSource.findCell(pointI);

            ///    - OR is closest to meshTarget.C()[cellI]
            ///      label findNearestCell(const point& location) const;

            ///        Read T_source.value()
            ///        Err = (T_source - T_target).value()
            Err[cellI] = 
                -fieldSource[sourceCellEnclosingThisPoint] + fieldTarget[cellI];
        }

        /// Apparently because we do not time-step as normal, so we have to
        /// explicitly write Err 
        Err.write();

        //targetField.write();
        //stepField.write();
        //field.rename("field_advected");
        //field.write();
    }
}

int main (int argc, char* argv[])
{
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

    fvMesh meshTarget
    (
        IOobject
        (
            polyMesh::defaultRegion,
            //"meshTarget",
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    /// fieldName at startTime is regarded as the target solution, i.e., a
    /// reference in error calculation.
    /// Workflow: 
    ///  - We set the target solution by setFields;  

    instantList times = runTime.times();

    /// Evaluate the error for all times, skip constant and startTime
    for (label timeI = 2; timeI < times.size(); ++timeI)
    {
        /// Update runTime
        runTime.setTime(times[timeI], timeI);

        /// Update mesh, if it has changed
        meshTarget.readUpdate();

        /// Update (read) fields and calculate the error
        calcErr<scalar>(meshSource, meshTarget, startTime);
    }

    Info<< nl << "End."
        << endl;
}
