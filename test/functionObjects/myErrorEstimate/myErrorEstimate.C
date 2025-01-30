/**
 * Smooth mesh points by solving a laplace equation for points as a field 
 *
 * Algorithm
 * - Create a volFiel out of initial points
 * - Solve laplace equation
 * - Interpolate internal volField to internal pointField
 * - Extrapolate boundary volField to boundary pointField (not implemented)
 * - Move mesh points
 *
 * \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, Ireland 
 * \date: Jan 02 2025
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
void initTargetSolution
(
    /// Pass by reference, as it will be modified in the function body
    GeometricField
      <Type, fvPatchField, volMesh>& solution,
    const fvMesh& mesh,
    const scalar&  xMin,
    const scalar&  yMin,
    const scalar&  xMax,
    const scalar&  yMax
)
{
    const List<vector>& C = mesh.C();

    // Error: does not retreive mesh, but another type.
    //const List<vector>& C = solution.db().C();

    forAll(C, cellI)
    {
        if 
        (
            C[cellI][0] < xMin
         || C[cellI][0] > xMax
         || C[cellI][1] < yMin
         || C[cellI][1] > yMax
        )
        {
           solution[cellI] = 0;
        }
        else if
        (
            C[cellI][0] >= xMin
         && C[cellI][0] <= xMax
         && C[cellI][1] >= yMin
         && C[cellI][1] <= yMax
        )
        {
           solution[cellI] = 1;
        }
        else
        {
            Info<< FatalErrorIn("Solution cannot be evaluated")
                << endl;
        }
    }
}

template<class Type>
void calcErr(const fvMesh& mesh/*, const word& startTime*/)
{
    const Time& runTime = mesh.time();

    IOobjectList fields(mesh, runTime.timeName());

    typedef
    GeometricField
    <
        Type, fvPatchField, volMesh
    > fieldType;

    /// Error
    //IOobjectList objects = fields.lookupClass(word(typename fieldType));

    IOobjectList objects = fields.lookupClass(word(fieldType::typeName));

    for 
    (
        IOobjectList::iterator iterator = objects.begin();
        iterator != objects.end();
        ++iterator
    )
    {
        GeometricField
        <
            Type, fvPatchField, volMesh
        > field(*iterator(), mesh);

        Info<< "Field "
            << field.name()
            << " found at "
            << runTime.timeName()
            << endl;

        /// Error: Field-cell addressing does not retrieve the exact solution
        /// due to mesh motion
        //GeometricField
        //<
        //    Type, fvPatchField, volMesh
        //> targetField
        //(
        //    IOobject
        //    (
        //        word(field.name()+"_target"),
        //        //runTime.startTime().name(), // not tested, might work
        //        startTime,
        //        mesh,
        //        IOobject::NO_READ,
        //        IOobject::AUTO_WRITE
        //    ),
        //    field
        //);

        GeometricField<Type, fvPatchField, volMesh> stepField 
        (
            IOobject
            (
                field.name()+"_exact",
                runTime.timeName(),
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh,
            dimensionedScalar("solution", field.dimensions(), 0)
        );

        initTargetSolution<scalar>
        (
            stepField, 
            mesh,
            0.025,//xMin
            0.025,//yMin
            0.075,//xMax
            0.075 //yMax
        );

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
                mesh,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            mesh,
            dimensionedScalar("zero", field.dimensions(), 0)
        );

        forAll(mesh.C(), cellI)
        {
            Err[cellI] = (-stepField[cellI] + field[cellI]);
            //Err[cellI] = 
            //  (
            //    field[cellI] 
            //  - targetField[cellI]
            //  );
        }

        /// Does not work
        //if (runTime.write())
        //{
        //    Info<< "Results written to "
        //        << runTime.timeName()
        //        << endl;
        //}

        /// Apparently because we do not time-step as normal, so we have to
        /// explicitly write Err 
        Err.write();
        //targetField.write();
        stepField.write();
        
        field.rename("field_advected");
        field.write();
    }
}

int main (int argc, char* argv[])
{
    //argList::validArgs.append
    //(
    //    "Scalar field for which error field is to be calculated"
    //);

    argList args(argc, argv);

    //const word& fieldName = args.additionalArgs()[0];

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
            polyMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        )
    );

    //volScalarField field
    //(
    //    IOobject
    //    (
    //        fieldName,
    //        runTime.timeName(),
    //        mesh,
    //        IOobject::MUST_READ
    //    ),
    //    mesh
    //);

    /// fieldName at startTime is the target solution which is regarded as a
    /// reference in error calculation.
    /// Advantage: 
    ///  - When the exact solution is set by setFields, this approach avoids
    ///    re-writting/hard-coding that;
    
    /// Initialize the targetField (aka exact solution) by `field` at the
    /// startTime

    instantList times = runTime.times();

    //while (runTime.loop())
    //forAll(times, timeI)

    /// Skip constant and startTime
    for (label timeI = 2; timeI < times.size(); ++timeI)
    {
        /// Update runTime
        runTime.setTime(times[timeI], timeI);

        /// Update mesh
        mesh.readUpdate();

        /// Update (read) fields and calculate the error

        // Faulty! Only `field` before the loop is registered with the mesh,
        // others are not, and cannot be found this way.

        //volScalarField field = 
        //  mesh.thisDb().lookupObject<volScalarField>(fieldName);

        calcErr<scalar>(mesh/*, startTime*/);
    }

    Info<< nl << "End"
        << endl;
}
