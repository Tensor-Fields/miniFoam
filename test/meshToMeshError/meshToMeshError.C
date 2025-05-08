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
#include "leastSquaresGrad.H"
#include "IOobjectList.H"

using namespace Foam;

typedef GeometricField<vector, pointPatchField, pointMesh> pointVectorField;
typedef GeometricField<vector, fvPatchField, volMesh> volVectorField;

template<class Type>
void reportErrorStat
(
    const GeometricField
    <
        Type, fvPatchField, volMesh
    > Err,
    const GeometricField
    <
        Type, fvPatchField, volMesh
    > Err_wrt_exact,
    const fvMesh& meshTarget
)
{
    //Info<< "Min error: "
    //    << min(Err, VGREAT)
    //    << endl;

    //Info<< "Max error: "
    //    << max(Err, VGREAT)
    //    << endl;

    scalar avgErr = 0;
    scalar maxErr = VSMALL;
    scalar minErr = VGREAT;
    
    /// Calculate average error
    forAll(Err, cellI)
    {
        avgErr += Err[cellI];
        maxErr = max(mag(Err[cellI]), maxErr);
        minErr = min(mag(Err[cellI]), minErr);
    }

    Info<< "Average error: "
        << avgErr / Err.size()
        << endl;

    Info<< "Min absolute error: "
        << minErr
        << endl;

    Info<< "Max absolute error: "
        << maxErr
        << endl;

    Info<< "Median is not implemented."
        << endl;

    scalar max_err_wrt_exact = VSMALL;

    scalar L1Err = VSMALL;

    /// Call fvMesh::V()
    const DimensionedField<scalar, volMesh>& meshVol = meshTarget.V();

    forAll(Err_wrt_exact, cellI)
    {
        L1Err += mag(Err_wrt_exact[cellI]) * meshVol[cellI];

        max_err_wrt_exact = 
            max(max_err_wrt_exact, mag(Err_wrt_exact[cellI]));
    }

    Info<< "Max error norm is "
        << max_err_wrt_exact 
        << endl;

    Info<< "L1 error norm is "
        << L1Err
        << endl;
}

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
void calcErr
(
    const fvMesh& meshSource,
    const fvMesh& meshTarget,
    // args is enough, it contains the bool info
    //const bool randomPoints,
    const argList& args
)
{
    //IOobjectList fieldsSource(meshTarget, startTime);

    /// calcErr is being called in the time loop, and for times > startTime.
    const Time& runTime = meshTarget.time();

    Info<< "Read objects ..."
        << endl;

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
            "meshSource",
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

        //(
        //    IOobject
        //    (
        //        word(fieldSource.name()+"Ref"), 
        //        meshTarget,
        //        IOobject::NO_READ,
        //        IOobject::AUTO_WRITE
        //    ),
        //    meshTarget,
        //    dimensioned<Type>
        //    (
        //        "reference",
        //        fieldSource.dimensions(),
        //        pTraits<Type>::zero
        //    )
        //);

        /// Grad of fieldTarget and fieldSource
        /// at cell centers, needed for error calculation at
        /// random points
        fv::leastSquaresGrad<Type> lss(meshSource);
        fv::leastSquaresGrad<Type> lst(meshTarget);

        GeometricField
        <
            typename outerProduct<vector, Type>::type, fvPatchField, volMesh
        > gradFieldSource = 
            lss.calcGrad(fieldSource, word("grad" + fieldSource.name()));

        GeometricField
        <
            typename outerProduct<vector, Type>::type, fvPatchField, volMesh
        > gradFieldTarget = 
            lst.calcGrad(fieldTarget, word("grad" + fieldTarget.name()));

        const volVectorField& cellCenter = meshTarget.C();

        const volVectorField& sourceCellCenter = meshSource.C();

        /// Error at the cell centers of the target mesh
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
            dimensionedScalar("zero", fieldTarget.dimensions(), VGREAT)
        );

        /// Error with respect to the exact solution, aka, the prescribed
        /// field
        GeometricField
        <
            Type, fvPatchField, volMesh
        >
        Err_wrt_exact
        (
            IOobject
            (
                "Err_wrt_exact",
                runTime.timeName(),
                meshTarget,
                IOobject::NO_READ,
                IOobject::AUTO_WRITE
            ),
            meshTarget,
            dimensionedScalar("zero", fieldTarget.dimensions(), VGREAT)
        );

        /// Pseudo-code

        /// A location, (x,y,z), falls in cellI_orig in meshSource, and in cellI
        /// in meshTarget.
        
        /// The value of a field, e.g., T, at (x,y,z) is the same in both cases
        /// if the two cells enclosing this location in both cases have the same
        /// values.

        ///
        /// Volume-average error (not implemented)
        ///

        /// Calculate the overlapped volume between a given 
        /// meshTarget
        /// cell and meshSource cells, possibly using Monte-Carlo

        /// Calculate the volume-average error
        /// This seems to be a reasonable measure;
        /// - If two cells perfectly match, but their value is
        /// different, at each point of the cell in meshTarget, the 
        /// error
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
        /// `o`, the meshTarget cell center, and if field_o == field_*
        /// then the right half of the cell `o` is correctly evaluated
        /// while the left half is wrong by (field_o - field_+); the
        /// weighted volume average returns the error of
        /// 0.5 * (field_o - field_+)
        /// which is reasonable; because the smaller the overlap with
        /// `+` is, the less the error becomes.

        /// Calculate the error in random points
        if(args.optionFound("randomPoints"))
        {
            /// Average error at each cell, aka average error at the random
            /// points enclosed by a cell
            GeometricField
            <
                Type, fvPatchField, volMesh
            >*
            avgRandErrPtr = 
                new GeometricField
                <
                    Type, fvPatchField, volMesh
                >("avgRandErr", Err);

            Info<< "avgRandErr Created."
                << endl;

            /// Random points are being generated as a weighted average of the
            /// vertices, so we need N random weights for each random point, 
            /// where N is the number of
            /// the vertices. 

            /// Random points to be generated in meshTarget 
            /// - If cellI in meshTarget perfectly matches cellI_orig in
            /// meshSource, avgRandErr must be zero if cell values are identical

            /// - If cellI falls inside cellI_orig, avgRandError must be zero

            const cellList& cells = meshTarget.cells();
            //const cellList& cells = meshSource.cells();

            /// Each cell has a list of vertices
            List // of cells
            <
                pointField // List (field) of vertices
            > cellVertices(cells.size());

            /// We calculate random points based on meshSource, although,
            /// ideally, the results should be unchanged if meshTarget is used
            /// instesd.
            
            /// Seed the random generator only once
            Random rand(meshSource.time().value());

            Info<< "rand seeded"
                << endl;

            /// We need to create M random points inside each cell. M does not
            /// need
            /// to be the same for all cells. Initialize it to a fixed number
            /// for all cells for now.
            
            //const labelList& nRandPoints(cells.size(), 10); // Error
            const label& fixedNoOfRandPoints =
                //readScalar(IStringStream(args.additionalArgs()[0])());
                readScalar(IStringStream(args.options()["randomPoints"])());

            const labelList nRandPoints(cells.size(), fixedNoOfRandPoints);
            
            Info<< "nRandPoints initialized"
                << endl;

            // Error: Counts each point only once, so, does not return all
            // points connected to a specific cell
            //const labelListList& pointCell = meshSource.pointCells();

            List// of cells
            < 
                /// Give up scalarListListList due to memory access issues
                //List// of random points
                //<
                    //scalarList// of random weights
                    List<scalar>// of random weights
                //>
            > weights(cells.size());

            /// Calculate the random weights for each cell
            forAll(cells, cellI)
            {
                /// For each cell, we have a list of random points
                /// List // of cells
                /// <
                ///     List // of random points
                ///     <
                ///         vector // each random point
                ///     >
                /// >
                //List<List<vector> > randPoints(cells.size());

                /// Give up pointListList for the simpler pointList; randPoints
                /// of
                /// the previous cell will be overwritten and the list is
                /// resized for
                /// each new cell
                List<point> randPoints(nRandPoints[cellI]);

                /// Each cell corresponds to a list consisting errors at its 
                /// enclosed
                /// random points
                //labelListList& randErr(cells.size());
                scalarList randErr(nRandPoints[cellI]);

                avgRandErrPtr->operator[](cellI) = 0;

                for 
                (
                    label randPointI = 0; 
                    randPointI < nRandPoints[cellI]; 
                    ++randPointI
                )
                {
                    /// For each random point, create a list of weights
                    //weights[cellI][randPointI] =
                    //    scalarList(cellVertices[cellI].size(), -1);

                    // Give up scalarListListList, due to issues with memory
                    // allocation
                    //weights[cellI][randPointI].setSize(cellVertices[cellI].size());

                    /// Call cellShape::points()
                    cellVertices[cellI] = 
                        cells[cellI].points/*labels*/
                        (
                            //meshSource.faces(),
                            //meshSource.points()
                            meshTarget.faces(),
                            meshTarget.points()
                        );

                    scalar sumWeights = 0;

                    // Re-size weights for the new cell
                    weights[cellI].setSize(cellVertices[cellI].size());

                    forAll(cellVertices[cellI], vertexI)
                    {
                        /// Fill the list of the random weights
                        //weights[cellI][randPointI][vertexI] = rand.scalar01();
                        weights[cellI][vertexI] = rand.scalar01();

                        sumWeights += weights[cellI][vertexI];
                    }

                    /// Normalize weights; in order for the random point (the
                    /// weighted average point) to fall inside a cell, the sum 
                    /// of the
                    /// weights should be 1.
                    forAll(weights[cellI]/*[randPointI]*/, weightI)
                    {
                        /// \note we cannot do it outside the loop, because we
                        /// do not have /= operator for 
                        /// weights[cellI]
                        /// which is a labelList
                        weights[cellI]/*[randPointI]*/[weightI] /= sumWeights;
                    }

                    /// Calculate the random point
                    randPoints[randPointI][0] = 0;
                    randPoints[randPointI][1] = 0;
                    randPoints[randPointI][2] = 0;

                    forAll(cellVertices[cellI], vertexI)
                    {
                        randPoints/*[cellI]*/[randPointI] +=
                            weights[cellI]/*[randPointI]*/[vertexI] *
                                cellVertices[cellI][vertexI];
                    }

                    /// Get the reference of the random point
                    const point& pointI = randPoints/*[cellI]*/[randPointI];

                    ///    Read the field value
                    ///    Find cellI_orig in meshSource that

                    ///    - encloses meshTarget.C()[cellI]
                    ///      label findCell(const point& location) const
                    const label& sourceCellEnclosingThisPoint = 
                        meshSource.findCell(pointI);

                    ///    - OR is closest to meshTarget.C()[cellI]
                    ///      label findNearestCell(const point& location) const;

                    ///        Read T_source.value()

                    ///        Err = (T_source - T_target).value()
                    randErr/*[cellI]*/[randPointI] = 

                        ///
                        /// Criterion: 
                        /// Random point value must equal the cell center value
                        ///
                        -fieldSource[sourceCellEnclosingThisPoint] 
                            + fieldTarget[cellI];

                        ///
                        /// Criterion: 
                        /// Random point value must match the second-order
                        /// evaluation around the cell center enclosing the 
                        /// point
                        ///

                        /// Source and target field values at a random point 
                        /// obtained from
                        /// the Taylor expansion around the respecitve cell 
                        /// centers

                        /// Target value
                        //fieldTarget[cellI] + (gradFieldTarget[cellI] &
                        //    (pointI - cellCenter[cellI])) -
                        //        /// Source value
                        //        fieldSource[sourceCellEnclosingThisPoint] +
                        //            (gradFieldSource[cellI] & 
                        //                (pointI - 
                        //                    sourceCellCenter
                        //                    [
                        //                        sourceCellEnclosingThisPoint
                        //                    ]
                        //                ));

                    avgRandErrPtr->operator[](cellI) += 
                        randErr/*[cellI]*/[randPointI];
                }

                Info<< "Random points in cell "
                    << cellI
                    << ":"<< randPoints
                    << endl;

                avgRandErrPtr->operator[](cellI) /= 
                    (randPoints/*[cellI]*/.size() + VSMALL);
            }

            /// At cellI in meshTarget
            //forAll(meshTarget.C(), cellI)
            //{
            //    forAll(randPoints[cellI], randPointI)
            //    {
            //        /// Get the reference of the random point
            //        const point& pointI = randPoints[cellI][randPointI];

            //        ///    Read the field value
            //        ///    Find cellI_orig in meshSource that

            //        ///    - encloses meshTarget.C()[cellI]
            //        ///      label findCell(const point& location) const
            //        const label& sourceCellEnclosingThisPoint = 
            //            meshSource.findCell(pointI);

            //        ///    - OR is closest to meshTarget.C()[cellI]
            //        ///      label findNearestCell(const point& location) const;

            //        ///        Read T_source.value()

            //        ///        Err = (T_source - T_target).value()
            //        randErr[cellI][randPointI] = 
            //            -fieldSource[sourceCellEnclosingThisPoint] 
            //                + fieldTarget[cellI];

            //       avgRandErr[cellI] += randErr[cellI][randPointI];
            //    }

            //    avgRandErr[cellI] /= randPoints[cellI].size();
            //}

            //randErr.write();
            avgRandErrPtr->write();
        }

        /// fieldRef is a field corresponding to meshTarget, evaluated with
        /// second-order accuracy from fieldSource, to be used for error
        /// calculation of fieldTarget
        GeometricField
        <
            Type, fvPatchField, volMesh
        > fieldRef
        (
            word(fieldTarget.name()+"Ref"),
            fieldTarget
        );

        /// The default is error calculation at target mesh cell centers
        if (!args.optionFound("atSourceCellCenters"))
        {
            /// At cellI in meshTarget
            forAll(meshTarget.C(), cellI)
            {
                const point& pointI = meshTarget.C()[cellI];
                ///    Read T
                ///    Find cellI_orig in meshSource that

                ///    - encloses meshTarget.C()[cellI]
                ///      label findCell(const point& location) const
                //const label& sourceCellEnclosingThisPoint = 
                    //meshSource.findCell(pointI);

                ///    - OR is closest to meshTarget.C()[cellI]
                const label& sourceCellNearestToThisPoint = 
                    meshSource.findNearestCell(pointI);

                ///        Read T_source.value()
                ///        Err = (T_source - T_target).value()


                ///
                /// Criterion:
                /// 
                //Err[cellI] = 
                //    -fieldSource[sourceCellEnclosingThisPoint] + fieldTarget[cellI];

                ///
                /// Criterion:
                /// 

                /// Taylor expansion of the fieldSource at this point based on
                /// sourceCellNearestTo cellI center is considered the
                /// reference(aka correct) value
                fieldRef[cellI] = 
                    fieldSource[sourceCellNearestToThisPoint] +
                        (gradFieldSource[sourceCellNearestToThisPoint] & 
                            (
                                cellCenter[cellI] -
                                    sourceCellCenter
                                    [
                                        sourceCellNearestToThisPoint
                                    ]
                            )
                        );

                if(!args.optionFound("unbounded"))
                {
                    /// Cut-off the lower out of bound
                    fieldRef[cellI] = 
                        max(min(fieldSource).value(), fieldRef[cellI]);

                    /// Cut-off the upper out of bound
                    fieldRef[cellI] = 
                        min(max(fieldSource).value(), fieldRef[cellI]);
                }

                Err[cellI] = fieldTarget[cellI] - fieldRef[cellI];
                    //(
                    //    fieldSource[sourceCellNearestToThisPoint] 
                    //  + (gradFieldSource[cellI] & (cellCenter[cellI] -
                    //        sourceCellCenter[cellI]))
                    //);
            }
        }
        /// If the option is specified, error will be calculated at source mesh
        /// cell centers
        else
        {
            /// Reset fieldRef's registry

            /// fieldRef is a field corresponding to meshSource, evaluated with
            /// second-order accuracy from fieldTarget, to be used for error
            /// calculation of fieldTarget
            GeometricField
            <
                Type, fvPatchField, volMesh
            > fieldRef
            (
                word(fieldSource.name()+"Ref"),
                fieldSource
            );

            /// At cellI in meshTarget
            forAll(sourceCellCenter, cellI)
            {
                const point& pointI = sourceCellCenter[cellI];
                ///    Read T
                ///    Find cellI_orig in meshSource that

                ///    - encloses meshTarget.C()[cellI]
                ///      label findCell(const point& location) const
                //const label& sourceCellEnclosingThisPoint = 
                    //meshSource.findCell(pointI);

                ///    - OR is closest to meshTarget.C()[cellI]
                const label& targetCellNearestToThisPoint = 
                    meshTarget.findNearestCell(pointI);

                ///        Read T_source.value()
                ///        Err = (T_source - T_target).value()

                ///
                /// Criterion:
                /// 

                /// Taylor expansion of the fieldTarget at pointI around 
                /// targetCellNearestTo pointI is compared to the correct
                /// value, which is the sourceCell value.
                fieldRef[cellI] = 
                    fieldTarget[targetCellNearestToThisPoint] +
                        (gradFieldTarget[targetCellNearestToThisPoint] & 
                            (pointI - 
                                cellCenter
                                [targetCellNearestToThisPoint]
                            )
                        );

                if(!args.optionFound("unbounded"))
                {
                    /// Cut-off the lower out of bound
                    fieldRef[cellI] = 
                        max(min(fieldSource).value(), fieldRef[cellI]);

                    /// Cut-off the upper out of bound
                    fieldRef[cellI] = 
                        min(max(fieldSource).value(), fieldRef[cellI]);
                }

                Err[cellI] = fieldRef[cellI] - fieldSource[cellI];
                    //(
                    //    fieldSource[sourceCellNearestToThisPoint] 
                    //  + (gradFieldSource[cellI] & (cellCenter[cellI] -
                    //        sourceCellCenter[cellI]))
                    //);
            }
        }

        /// Apparently because we do not time-step as normal, so we have to
        /// explicitly write Err 
        fieldRef.write();
        Err.write();

        //targetField.write();
        //stepField.write();
        //field.rename("field_advected");
        //field.write();

        ///
        /// Calc error wrt a prescribed field
        ///
        if (!args.optionFound("noPrescribedFields"))
        {
            /// Initialize the 'exact' field
            GeometricField<Type, fvPatchField, volMesh> exactField = fieldTarget;

            /// Calculate the 'exact' field
            fieldExact/*<Type>*/(exactField, cellCenter);

            /// Calculate the error
            Err_wrt_exact = 
                fieldTarget - exactField;

            /// Write
            exactField.write();
            Err_wrt_exact.write();
        }

        /// Report error stat

        /// In conjunction with the if(atSourceCellCenters)
        /// this can be a separate
        /// function to be called here, for the sake of more abstractoin and
        /// efficiency; we loop unnecessarily over Err again to calculate
        /// stats, while we could calculate them in the loops of the
        /// if-statement.
        reportErrorStat(Err, Err_wrt_exact, meshTarget);
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

    //const word& startTime = runTime.startTime().name();

    fvMesh meshSource
    (
        IOobject
        (
            //polyMesh::defaultRegion,
            "meshSource",
            //startTime,
            //runTime.timeName(),
            runTime.constant(),
            runTime,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
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
            IOobject::NO_WRITE
        )
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
        meshTarget.readUpdate();

        /// Update (read) fields and calculate the error
        calcErr<scalar>
        (
            meshSource, 
            meshTarget, 
            //args.optionFound("randomPoints")
            args
        );
    }

    Info<< nl << "End."
        << endl;
}
