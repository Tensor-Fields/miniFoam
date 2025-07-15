#include "./err.H"

/// Definitions

/// Constructors
template<class T>
Foam::err<T>::err
(
    const argList& args,
    const fvMesh& meshSource,
    const fvMesh& meshTarget
)
:
args_(args),
//fieldTarget_(),
//fieldSource_(),
//fieldExact_(),
fieldExactPtr_(/*NULL*/),
//(
//    new GeometricField<T, fvPatchField, volMesh>
//    (
//        IOobject
//        (
//            "fieldExact",
//            meshSource.time().timeName(),
//            meshTarget,
//            IOobject::NO_READ,
//            IOobject::NO_WRITE
//        ),
//        meshTarget,
//        dimensioned<T>("0", dimless, pTraits<T>::zero)
//    )
//),
errPtr_(/*NULL*/),
//(
//    new GeometricField<T, fvPatchField, volMesh>
//    (
//        IOobject
//        (
//            "err",
//            meshSource.time().timeName(),
//            meshTarget,
//            IOobject::NO_READ,
//            IOobject::NO_WRITE
//        ),
//        meshTarget,
//        dimensioned<T>("0", dimless, pTraits<T>::zero)
//    )
//),
meshSource_(meshSource),
meshTarget_(meshTarget),
minErr_(VGREAT),
maxErr_(VSMALL),
avgErr_(VSMALL),
L1ErrPtr_(NULL)
{
    /// Set fieldExactPtr_ and initialize 
    //fieldExact();
}

template<class Type>
void Foam::err<Type>::errStats()
{
    /// Calculate  error statistics
    forAll(errPtr_(), cellI)
    {
        avgErr_ += errPtr_()[cellI];
        minErr_ = min(mag(errPtr_()[cellI]), minErr_);
        maxErr_ = max(mag(errPtr_()[cellI]), maxErr_);
    }
  
    avgErr_ /= errPtr_().size();
}

template<class Type>
void Foam::err<Type>::report()
{
    errStats();

    Info<< "Average error: "
        << avgErr_
        << endl;

    Info<< "Min absolute error: "
        << minErr_
        << endl;

    Info<< "Max absolute error: "
        << maxErr_
        << endl;

    //Info<< "Median is not implemented."
    //    << endl;

    Info<< "L1 error norm is "
        << L1Err()
        << endl;
}

template <class type>
Foam::scalar Foam::err<type>::int_over_whole_mesh
(
    GeometricField<type, fvPatchField, volMesh> integrandField
)
{
    scalar intValue = VSMALL;

    /// Call fvMesh::V()
    const DimensionedField<scalar, volMesh>& meshVol = meshTarget_.V();
        //const_cast<fvMesh&>(meshTarget_).V();

    forAll(integrandField, cellI)
    {
        // Operator precedence: [] > *
        // see: https://en.cppreference.com/w/c/language/operator_precedence.html
        intValue += ( integrandField[cellI] * meshVol[cellI] );
    }

    return intValue;
}

///
/// EDIT:
/// Getting arguments for a class seems bad design; applications get arguments,
/// and we do not know
/// beforehand what applications will use this class.
/// It is the driver's responsibility to read or create the exact field.
/// But, 
/// for now it is also the driver's responsibility to be aware of the arguments
/// recognised by this class.
///

/// Calculate the exact (prescribed) field at the cell centers of the
/// target mesh
template<class Type>
Foam::GeometricField<Type, Foam::fvPatchField, Foam::volMesh>&
Foam::err<Type>::fieldExact
(const GeometricField<Type, fvPatchField, volMesh>& ft)
{
    //if (!fieldExactPtr_.valid())
    //{
        /// Create the exact field
        //GeometricField<Type, fvPatchField, volMesh> f//; //= fieldExact_;
        //(
        //    IOobject
        //    (
        //        "f",
        //        meshTarget_.time().timeName(),
        //        meshTarget_,
        //        IOobject::NO_READ,
        //        IOobject::NO_WRITE
        //    ),
        //    meshTarget_,
        //    dimensioned<Type>("f", dim, pTraits<Type>::zero)
        //);

        // Copy fieldTarget into f, then re-evaluate f as the exact field
        GeometricField<Type, fvPatchField, volMesh> f = ft;

        /// Unnecessary references to member data, to be removed in the next review
        const volVectorField& meshTargetCellCenters = meshTarget_.C();
        const argList& args = args_;

        ///
        /// Function body actually begins
        ///
        const volVectorField& p = meshTargetCellCenters;
 
        const scalar& pi = mathematicalConstant::pi;

        // Foam::sin, as calling sin out of the namespace is ambigious
        // pointI is equivalent to the cell index.
        const scalar a = readScalar(IStringStream(args.options()["a"])());
        const scalar b = readScalar(IStringStream(args.options()["b"])());
        const scalar c = readScalar(IStringStream(args.options()["c"])());
        const scalar d = readScalar(IStringStream(args.options()["d"])());

        // Now re-evaluate f
        if (args.optionFound("sinsin"))
        {
            forAll(p, pointI)
            {
                const scalar& x = p[pointI][0];
                const scalar& y = p[pointI][1];
                const scalar& z = p[pointI][2];

                f[pointI] = a + b * Foam::sin(c * pi * x) * Foam::sin(d * pi * y);
            }
        }
        else
        {
            //forAll(p, pointI)
            //{
            //    const scalar& x = p[pointI][0];
            //    const scalar& y = p[pointI][1];
            //    const scalar& z = p[pointI][2];

            //    f[pointI] = a + b * Foam::sin(c * pi * x) * Foam::cos(d * pi * y);
            //}
 
            notImplemented("Only a + b sin(c pi x)sin(d pi x) is implemented.");
        }

        fieldExactPtr_.reset
        (
            new 
            GeometricField<Type, fvPatchField, volMesh>(f)
        );
    //}

    return fieldExactPtr_();
}

template<class Type>
void Foam::err<Type>::calcErr()
{
    /// Unnecessary references to member data, replace them with actual member
    /// data in the function body below
    const fvMesh& meshSource = meshSource_;
    const fvMesh& meshTarget = meshTarget_;
    const argList& args = args_;
    //GeometricField
    //<
    //    Type, fvPatchField, volMesh
    //>& fieldTarget = fieldTarget_;

    //GeometricField
    //<
    //    Type, fvPatchField, volMesh
    //>& fieldSource = fieldSource_;

    ///
    /// Function body actually begins
    ///

    /// calcErr is meant to be called in the time loop, and for times > startTime.
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
            << fieldTarget.objectPath()
            << endl;

        const IOobject fieldSourceIO
        (
            fieldTarget.name(),
            meshSource.instance(),
            //runTime.timeName(),
            "0",
            meshSource,
            IOobject::MUST_READ,
            IOobject::NO_WRITE
        );

        GeometricField
        <
            Type, fvPatchField, volMesh
        > fieldSource(fieldSourceIO, meshSource);

        Info<< "meshSource.instance(): "
            << meshSource.instance()
            << endl;

        Info<< "Source field "
            << fieldSource.name()
            << " found at "
            << fieldSource.objectPath()
            << endl;

        ///
        /// Calc err wrt a given source mesh and field
        ///
        if (args.optionFound("sourceCase"))
        {
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

            /// Error at the cell centers of the target mesh, wrt the source mesh
            GeometricField
            <
                Type, fvPatchField, volMesh
            >
            Err
            (
                IOobject
                (
                    fieldTarget.name()+"_err",
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
                    /// Particularly when doing cyclic study, when meshSource
                    /// and meshTarget coincide
                    /// 
                    Err[cellI] = 
                        //-fieldSource[sourceCellEnclosingThisPoint]
                        -fieldSource[sourceCellNearestToThisPoint] 
                            + fieldTarget[cellI];

                    ///
                    /// Criterion:
                    /// 

                    /// Taylor expansion of the fieldSource at this point based on
                    /// sourceCellNearestTo cellI center is considered the
                    /// reference(aka correct) value
                    //fieldRef[cellI] = 
                    //    fieldSource[sourceCellNearestToThisPoint] +
                    //        (gradFieldSource[sourceCellNearestToThisPoint] & 
                    //            (
                    //                cellCenter[cellI] -
                    //                    sourceCellCenter
                    //                    [
                    //                        sourceCellNearestToThisPoint
                    //                    ]
                    //            )
                    //        );

                    //if(!args.optionFound("unbounded"))
                    //{
                    //    /// Cut-off the lower out of bound
                    //    fieldRef[cellI] = 
                    //        max(min(fieldSource).value(), fieldRef[cellI]);

                    //    /// Cut-off the upper out of bound
                    //    fieldRef[cellI] = 
                    //        min(max(fieldSource).value(), fieldRef[cellI]);
                    //}

                    /// When using fieldRef
                    //Err[cellI] = fieldTarget[cellI] - fieldRef[cellI];

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
            //fieldRef.write();
            //Err.write();
            errPtr_.reset
            (
                new GeometricField<Type, fvPatchField, volMesh>
                (Err)
            );

            //targetField.write();
            //stepField.write();
            //field.rename("field_advected");
            //field.write();

            /// Report error stat

            /// In conjunction with the if(atSourceCellCenters)
            /// this can be a separate
            /// function to be called here, for the sake of more abstraction and
            /// efficiency; we loop unnecessarily over Err again to calculate
            /// stats, while we could calculate them in the loops of the
            /// if-statement.
            //report();
        }
        ///
        /// Default behaviour: Calc error wrt a prescribed field, for a given
        /// source mesh
        ///
        else
        {
            /// Calculate the prescribed exact field
            //fieldExact();
            Info<< "Calc error wrt the given exact field ..."
                << endl;

            //GeometricField<Type, fvPatchField, volMesh> test = 
            //    (fieldTarget - fieldExact(fieldTarget));

            //Info<< "test field created, test.name() = "
            //    << test.name()
            //    << endl;

            errPtr_.reset
            (
                new GeometricField<Type, fvPatchField, volMesh>
                (
                    //IOobject
                    //(
                    //    "err",
                    //    meshSource.time().timeName(),
                    //    meshTarget,
                    //    IOobject::NO_READ,
                    //    IOobject::NO_WRITE
                    //),
                    //meshTarget,

                    //fieldTarget - fieldExact(fieldTarget.dimensions())

                    // To preserve BC, fieldTarget.name(), and possibly other
                    // info, we input the fieldTarget to fieldExact
                    (fieldTarget - fieldExact(fieldTarget))
                )
            );

            errPtr_().rename(fieldTarget.name()+"_err");

            Info<< "Error calculated."
                << endl;

            /// Report errors
            //report();
        }
    }
}

template<class T>
Foam::scalar
Foam::err<T>::L1Err()
{
    if (!L1ErrPtr_.valid())
    {
        L1ErrPtr_.set(new scalar(int_over_whole_mesh(mag(errPtr_()))));
    }

    return L1ErrPtr_();
}

template<class T>
void Foam::err<T>::updateMeshTarget()
{
    const_cast<fvMesh&>(meshTarget_).readUpdate();
}
