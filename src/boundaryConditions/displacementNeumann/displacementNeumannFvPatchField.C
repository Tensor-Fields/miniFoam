/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | foam-extend: Open Source CFD
   \\    /   O peration     | Version:     4.0
    \\  /    A nd           | Web:         http://www.foam-extend.org
     \\/     M anipulation  | For copyright notice see file Copyright
-------------------------------------------------------------------------------
License
    This file is part of foam-extend.

    foam-extend is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    foam-extend is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with foam-extend.  If not, see <http://www.gnu.org/licenses/>.

Assumptions

    -- Written for a vector field

    -- Displacement for this patch is a vectorField saved in fcv_ (abbreviated
    for 'face-center value').
    This displacement is calculated with reference to the location of the patch
    at the start of the simulation (startLocation_).

    -- As a result, the displacement field, should be defined on the same basis
    as fcv_, i.e., with respect to the face locations at the start of the
    simulation.

    -- Displacement field is searched for, by the keyword displacementName in
    the dictionary.

    -- fcv_ is evaluated to zero (by default) if a displacement field is not
    found in the objectRegistry.

\*---------------------------------------------------------------------------*/

#include "displacementNeumannFvPatchField.H"
#include "fvc.H"
#include "surfaceMesh.H"
#include "fvsPatchField.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

displacementNeumannFvPatchField::displacementNeumannFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchField<vector>(p, iF),
    axialDir_(),
    axialDirPtr_(),
    axiallyFixedStrainRate_(false),
    axialStrainRatePtr_(NULL),
    strainingEndTime_(),
    totalDisp_(),
    accumulatedDisp_(p.size(), vector::zero),
    startLocation_(p.Cf()),
    displacementFieldName_(),
    frictionless_(false),
    fcv_(), // ?
    displacementIncrementFieldName_(),
    //dirichletDir_(axialDir_),
    scenario_(/*this, false*/)
{
    Info<< "    Constructor 1\n";

    FatalErrorIn("Constructor")
        << "Currently, this constructor must not be used, unless it is clear\n"
        << "what its purpose is\n"
        << "It cannot be commented, because runTimeSelection mechanism asks\n"
        << "for it"
        << abort(FatalError);

    fullyConstruct();
}

displacementNeumannFvPatchField::displacementNeumannFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    /// Do not call this; it requests "gradient" in the dict, while it is
    /// irrelevant in this implementation
    fixedGradientFvPatchField<vector>(p, iF, dict),
    /// OK; call it and then overwrite gradient_
    /// Impossible; gradient_ is private to fixedGrad
    /// Also impossible to not call the constructor at all; there does not exist
    /// an empty constructor for fiexedGrad; you have to have "gradient" in your
    /// dict :(

    /// Call this instead
    //fvPatchField<vector>(p, iF, dict),
    /// Impossible! It is not the mother, it is the granny!

    /// And keep gradient_ happy!
    /// No! It is private to fixedGrad
    //gradient_(p.size(), vector::zero),

    axialDir_(),
    axialDirPtr_(),
    fcv_(),
    startLocation_(p.Cf()),
    axiallyFixedStrainRate_(false),
    axialStrainRatePtr_(NULL),
    strainingEndTime_(),
    totalDisp_(),
    accumulatedDisp_(p.size(), vector::zero),
    storeAccumulatedDisp_(false),
    displacementFieldName_(),
    displacementIncrementFieldName_(),
    frictionless_(false),
    correctNonOrthogonal_(false),
    //dirichletDir_(),
    userBoundaryTraction_(p.size(), vector::zero),
    muName_(),
    //muPtr_(),
    lambdaName_(),
    //lambdaPtr_(),
    //weights_(3, vector::zero),
    JWWeight_(),
    HWeight_(),
    //boundaryValueWeight_(),
    fieldName_(this->dimensionedInternalField().name()),
    pureDisplacement_
    (
        dict.lookupOrDefault<bool>
        (
            word("pureDisplacement"), false
        )
    ),
    pureTraction_
    (
        dict.lookupOrDefault<bool>
        (
            word("pureTraction"), false
        )
    ),
    normalDisplacementTangentialTraction_
    (
        dict.lookupOrDefault<bool>
        (
            word("normalDisplacementTangentialTraction"), false
        )
    ),
    normalTractionTangentialDisplacement_
    (
        dict.lookupOrDefault<bool>
        (
            word("normalTractionTangentialDisplacement"), false
        )
    ),
    w1_
    (
        dict.lookupOrDefault<vector>
        (
            word("implicitWeight"), vector::one
        )
    ),
    /*------------------------------------------------------------------------*\
                                    DANGER ZONE
    \*------------------------------------------------------------------------*/
    /// Things get initialized In the Order of Declaration,
    /// Not the Order Written in the .C File!

    /// Although scenario_ goes after, e.g., pureDisplacement_, it does not get
    /// the correct pureDisplacement_ from `this', if the former is declared
    /// after scenario_.
    /*------------------------------------------------------------------------*/
    //scenario_(this, false)

    /// Safer alternative --- init in the body.
    scenario_()
{
    Info<< "    Constructor 2\n";

    if
    (
        !dict.readIfPresent<word>
        (word("displacementFieldName"), displacementFieldName_)
    )
    {
        FatalErrorIn("Constructor")
            << "'displacementFieldName' keyword is not specified\n"
            << abort(FatalError);
    }

    if
    (
        !dict.readIfPresent<word>
        (word("displacementIncrementFieldName"), displacementIncrementFieldName_)
    )
    {
        FatalErrorIn("Constructor")
            << "'displacementIncrementFieldName' keyword is not specified\n"
            << abort(FatalError);
    }

    /// Before any hassle, see if this is the right field.
    /// Operations in this class assume the field to be either
    /// displacement or displacement-increment
    if
    (
        (fieldName_ != displacementFieldName_)
     && (fieldName_ != displacementIncrementFieldName_)
    )
    {
        FatalErrorIn("Constructor")
            << "This boundary condition assumes the field to be either\n"
            << "displacement or displacement-increment.\n"
            << "However, this field's name is\n"
            << fieldName_
            << "\nwhich is neither "
            << displacementFieldName_
            << " nor "
            << displacementIncrementFieldName_
            << abort(FatalError);
    }

    if
    (
        dict.readIfPresent<bool>
        ("axiallyFixedStrainRate", axiallyFixedStrainRate_)
    )
    {
        if
        (
            !dict.readIfPresent<vector>
            (word("totalDisp"), totalDisp_)
        )
        {
            FatalErrorIn("Constructor")
                << "When 'axiallyFixedStrainRate' is 'true'\n"
                << "totalDisp should be specified\n"
                << abort(FatalError);
        }
        else
        {
            /// Let us say there are these posibilities for totalDisp_:
            /// (-10 0 0)
            /// (10 -10 0)
            axialDir_ = totalDisp_ / mag(totalDisp_);

            /// Now, axialDir_ is
            /// (-1 0 0)      <-- Make it positive
            /// (0.7 -0.7 0)  <-- Filter it out

            /// Keep axialDir_ pointing to positive directions always
            /// This is to keep the signs of the components of the vectors
            /// multiplied by this unaffected.
            forAll(axialDir_, cmpt)
            {
                axialDir_[cmpt] = mag(axialDir_[cmpt]);
            }

            /// For now, only accept axialDir_ along with one of the positive
            /// directions of the frame
            if
            (
                (axialDir_[0] == 0 || mag(1 - axialDir_[0]) < SMALL)
             && (axialDir_[1] == 0 || mag(1 - axialDir_[1]) < SMALL)
             && (axialDir_[2] == 0 || mag(1 - axialDir_[2]) < SMALL)
            )
            {
                /// We are OK!
                Info<< "    "
                    << "Axial direction: "
                    << axialDir_
                    << endl;
            }
            else
            {
                FatalErrorIn("Constructor")
                    << "For now, totalDisp must align with a coordinate axis\n"
                    << abort(FatalError);
            }

            axialDirPtr_.set(new vector(axialDir_));

        }
    }

    if
    (
        !dict.readIfPresent<scalar>
            (word("strainingEndTime"), strainingEndTime_)
    )
    {
        strainingEndTime_ = this->db().time().endTime().value();

        Info<< "    Strain rate will be calculated based on the endTime in"
            << " \ncontrolDict\n";
    }

    dict.readIfPresent<bool>(word("frictionless"), frictionless_);

    dict.readIfPresent<vectorField>(word("traction"), userBoundaryTraction_);

    dict.readIfPresent<bool>
        (word("correctNonOrthogonal"), correctNonOrthogonal_);

    if
    (
        !dict.readIfPresent<word>(word("shearModulusName"), muName_)
    )
    {
        muName_ = word("mu");
    }

    if
    (
        !dict.readIfPresent<word>(word("LameName"), lambdaName_)
    )
    {
        lambdaName_ = word("lambda");
    }

    if
    (
        !dict.readIfPresent<scalar>
            (word("JWWeight"), JWWeight_)
    )
    {
        WarningIn("Constructor")
            << "    JWWeight"
            << " not found in dict; set to 0\n";

        JWWeight_ = 0;
    }

    if
    (
        !dict.readIfPresent<scalar>
            (word("HWeight"), HWeight_)
    )
    {
        WarningIn("Constructor")
            << "    HWeight"
            << " not found in dict; set to 0\n";

        HWeight_ = 0;
    }

    /// Define a vector called "displacementNeumann (fcv_, face-center value)",
    /// which is
    /// faceLocationsNow - startLocation_
    this->read();

    /// Init weights, etc.
    //dict.readIfPresent<vectorField>(word("weights"), weights_);

    //pureDisplacement_ = false;
    //pureTraction_ = false;
    //normalDisplacementTangentialTraction_ = false;
    //normalTractionTangentialDisplacement_ = false;

    //dict.readIfPresent<bool>(word("pureDisplacement"), pureDisplacement_);
    //dict.readIfPresent<bool>(word("pureTraction"), pureTraction_);
    //dict.readIfPresent<bool>
    //(
    //    word("normalDisplacementTangentialTraction"),
    //    normalDisplacementTangentialTraction_
    //);
    //dict.readIfPresent<bool>
    //(
    //    word("normalTractionTangentialDisplacement"),
    //    normalTractionTangentialDisplacement_
    //);

    //Info<< "\n    We are before scenario.init();"
    //    << "\n"
    //    << "    pureDisplacement_ = "
    //    << pureDisplacement_
    //    << "\n"
    //    << "    pureTraction_ = "
    //    << pureTraction_
    //    << "\n"
    //    << "    normalDisplacementTangentialTraction_ = "
    //    << normalDisplacementTangentialTraction_
    //    << "\n"
    //    << "    normalTractionTangentialDisplacement_ = "
    //    << normalTractionTangentialDisplacement_
    //    << "\n";

    scenario_.init(this);

    fullyConstruct();

    /// Let the first iteration only be with wrong BC's; so do not evaluate().
    /// This is because if "U"
    /// (displacement field) is not created yet, some functions does not work,
    /// particularly, explicitTangentialDisplacement().

    displacementNeumannFvPatchField::evaluate();
}

displacementNeumannFvPatchField::displacementNeumannFvPatchField
(
    const displacementNeumannFvPatchField& pf, // patch field
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchField<vector>(pf, p, iF, mapper),
    startLocation_(pf.startLocation_, mapper),
    fcv_(pf.fcv_, mapper),
    userBoundaryTraction_(pf.userBoundaryTraction_, mapper),
    accumulatedDisp_(pf.accumulatedDisp_, mapper),
    pureTraction_(pf.pureTraction_),
    pureDisplacement_(pf.pureDisplacement_),
    normalDisplacementTangentialTraction_
    (pf.normalDisplacementTangentialTraction_),
    normalTractionTangentialDisplacement_
    (pf.normalTractionTangentialDisplacement_),
    //scenario_(this)
    scenario_()
{
    Info<< "    Constructor 3\n";

    axialDir_ = pf.axialDir_;
    //dirichletDir_ = pf.dirichletDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    //accumulatedDisp_ = pf.accumulatedDisp_;
    storeAccumulatedDisp_ = pf.storeAccumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;

    correctNonOrthogonal_ = pf.correctNonOrthogonal_;
    muName_ = pf.muName_;
    //muPtr_ = pf.muPtr_;
    lambdaName_ = pf.lambdaName_;
    //lambdaPtr_ = pf.lambdaPtr_;
    //weights_ = pf.weights_;
    //boundaryValueWeight_ = pf.boundaryValueWeight_;
    fieldName_ = pf.fieldName_;
    //scenario_ = pf.scenario_;
    w0_ = pf.w0_;
    w1_ = pf.w1_;
    w2_ = pf.w2_;
    w3_ = pf.w3_;
    w4_ = pf.w4_;

    JWWeight_ = pf.JWWeight_;
    HWeight_ = pf.HWeight_;

    scenario_.init(this);
    /// Init weights, etc.
    fullyConstruct();
}

/// Called by clone()
displacementNeumannFvPatchField::displacementNeumannFvPatchField
(
    const displacementNeumannFvPatchField& pf
)
:
    fixedGradientFvPatchField<vector>(pf),
    startLocation_(pf.startLocation_),
    fcv_(pf.fcv_),
    userBoundaryTraction_(pf.userBoundaryTraction_),
    accumulatedDisp_(pf.accumulatedDisp_),
    pureTraction_(pf.pureTraction_),
    pureDisplacement_(pf.pureDisplacement_),
    normalDisplacementTangentialTraction_
    (pf.normalDisplacementTangentialTraction_),
    normalTractionTangentialDisplacement_
    (pf.normalTractionTangentialDisplacement_),
    //scenario_(this)
    scenario_()
{
    Info<< "    Constructor 4\n";

    axialDir_ = pf.axialDir_;
    //dirichletDir_ = pf.dirichletDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    //accumulatedDisp_ = pf.accumulatedDisp_;
    storeAccumulatedDisp_ = pf.storeAccumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;

    correctNonOrthogonal_ = pf.correctNonOrthogonal_;
    muName_ = pf.muName_;
    //muPtr_ = pf.muPtr_;
    lambdaName_ = pf.lambdaName_;
    //lambdaPtr_ = pf.lambdaPtr_;
    //weights_ = pf.weights_;
    //boundaryValueWeight_ = pf.boundaryValueWeight_;
    fieldName_ = pf.fieldName_;
    //scenario_ = pf.scenario_;
    w0_ = pf.w0_;
    w1_ = pf.w1_;
    w2_ = pf.w2_;
    w3_ = pf.w3_;
    w4_ = pf.w4_;

    JWWeight_ = pf.JWWeight_;
    HWeight_ = pf.HWeight_;

    scenario_.init(this);
    /// Init weights, etc.
    fullyConstruct();
}

/// Called by clone()

/// Note: Not having the following constructor, as well as clone(iF)
/// construcgtor (see .H file),
/// causes a parent object to be created every
/// time-step, instead of the current derived patch type.
/// That is, from the second time-step onwards, you will have a fixedValue
/// boundary condition on this patch.

displacementNeumannFvPatchField::displacementNeumannFvPatchField
(
    const displacementNeumannFvPatchField& pf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchField<vector>(pf, iF),
    startLocation_(pf.startLocation_),
    fcv_(pf.fcv_),
    userBoundaryTraction_(pf.userBoundaryTraction_),
    accumulatedDisp_(pf.accumulatedDisp_),
    pureTraction_(pf.pureTraction_),
    pureDisplacement_(pf.pureDisplacement_),
    normalDisplacementTangentialTraction_
    (pf.normalDisplacementTangentialTraction_),
    normalTractionTangentialDisplacement_
    (pf.normalTractionTangentialDisplacement_),
    //scenario_(this)
    scenario_()
{
    Info<< "    Constructor 5\n";

    axialDir_ = pf.axialDir_;
    //dirichletDir_ = pf.dirichletDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    //accumulatedDisp_ = pf.accumulatedDisp_;
    storeAccumulatedDisp_ = pf.storeAccumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;

    correctNonOrthogonal_ = pf.correctNonOrthogonal_;
    muName_ = pf.muName_;
    //muPtr_ = pf.muPtr_;
    lambdaName_ = pf.lambdaName_;
    //lambdaPtr_ = pf.lambdaPtr_;
    //weights_ = pf.weights_;
    //boundaryValueWeight_ = pf.boundaryValueWeight_;
    fieldName_ = pf.fieldName_;
    //scenario_ = pf.scenario_;
    w0_ = pf.w0_;
    w1_ = pf.w1_;
    w2_ = pf.w2_;
    w3_ = pf.w3_;
    w4_ = pf.w4_;

    JWWeight_ = pf.JWWeight_;
    HWeight_ = pf.HWeight_;

    scenario_.init(this);
    /// Init weights, etc.
    fullyConstruct();
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void displacementNeumannFvPatchField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedGradientFvPatchField<vector>::autoMap(m);

    const_cast<vectorField&>(startLocation_).autoMap(m);
    fcv_.autoMap(m);
    //axialDir_.autoMap(m);
    //dirichletDir_autoMap(m);
    //axialDirPtr_.autoMap(m);
    //axiallyFixedStrainRate_.autoMap(m);
    //axialStrainRatePtr_.autoMap(m);
    //strainingEndTime_.autoMap(m);
    //totalDisp_.autoMap(m);
    accumulatedDisp_.autoMap(m);
    //displacementFieldName_.autoMap(m);
    //frictionless_.autoMap(m);
    //displacementIncrementFieldName_.autoMap(m);

    Info<< "    @@@@@@@@@@ autoMap called @@@@@@@@@@\n";
}

void displacementNeumannFvPatchField::rmap
(
    const fvPatchField<vector>& ptf,
    const labelList& addr
)
{
    fixedGradientFvPatchField<vector>::rmap(ptf, addr);

    const displacementNeumannFvPatchField& dnfpf =
        refCast<const displacementNeumannFvPatchField>(ptf);

    const_cast<vectorField&>(startLocation_).rmap(dnfpf.startLocation_, addr);
    fcv_.rmap(dnfpf.fcv_, addr);
    //axialDir_.rmap(dnfpf.axialDir_, addr);
    //dirichletDir_.rmap(dnfpf.dirichletDir_, addr);
    //axialDirPtr_.rmap(dnfpf.axialDirPtr_, addr);
    //axiallyFixedStrainRate_.rmap(dnfpf.axiallyFixedStrainRate_, addr);
    //axialStrainRatePtr_.rmap(dnfpf.axialStrainRatePtr_, addr);
    //strainingEndTime_.rmap(dnfpf.strainingEndTime_, addr);
    //totalDisp_.rmap(dnfpf.totalDisp_, addr);
    //accumulatedDisp_.rmap(dnfpf.accumulatedDisp_, addr);
    //displacementFieldName_.rmap
    //    (dnfpf.displacementFieldName_, addr);
    //frictionless_.rmap(dnfpf.frictionless_, addr);
    //displacementIncrementFieldName_.rmap
    //    (dnfpf.displacementIncrementFieldName_, addr);

    Info<< "    @@@@@@@@@@ rmap called @@@@@@@@@@\n";
}

void displacementNeumannFvPatchField::write(Ostream& os) const
{
    fixedGradientFvPatchField<vector>::write(os);

    //writeEntry("type", os);
    //writeEntry("value", os);

    os.writeKeyword("displacementFieldName")
        << displacementFieldName_ << token::END_STATEMENT << nl;

    os.writeKeyword("displacementIncrementFieldName")
        << displacementIncrementFieldName_
        << token::END_STATEMENT
        << nl;

    os.writeKeyword("frictionless")
        << frictionless_ << token::END_STATEMENT << nl;
    os.writeKeyword("axiallyFixedStrainRate")
        << axiallyFixedStrainRate_ << token::END_STATEMENT << nl;
    //os.writeKeyword("fieldGradName")
    //    << fieldGradName_ << token::END_STATEMENT << nl;
    os.writeKeyword("startLocation")
        << startLocation_ << token::END_STATEMENT << nl;

    if (storeAccumulatedDisp_)
    {
        os.writeKeyword("accumulatedDisp")
            << accumulatedDisp_ << token::END_STATEMENT << nl;
    }

    if (axiallyFixedStrainRate_)
    {
        os.writeKeyword("axialStrainRate")
            << axialStrainRatePtr_() << token::END_STATEMENT << nl;
        os.writeKeyword("totalDisp")
            << totalDisp_ << token::END_STATEMENT << nl;
        os.writeKeyword("axialDir")
            << axialDir_ << token::END_STATEMENT << nl;
        os.writeKeyword("strainingEndTime")
            << strainingEndTime_ << token::END_STATEMENT << nl;
    }
}
 
/// Face values are directly obtained, regardless of gradient at faces (obtained
/// by gradient()) which is used only for assembling the matrix, that is, the
/// system of equations 
/// for this very varible.

/// Face values have no contribution to the system, as far as the only implicit
/// contributions come from fvm::ddt and fvm::laplacian.

/// BTW, this function gives face values.
void displacementNeumannFvPatchField::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        /// Set updated_ to true
        this->updateCoeffs();
    }

    /// Force reading the displacement field if exists
    this->read();

    /// Now, calculate the new displacement components based on the prescribed
    /// rules by the
    /// user, e.g., fixed-strain-rate, etc.
    this->userDisplacement();
    //Info<< "    userDisplacement took effect.\n";

    /// Update other displacement components
    /// by applying constraints, e.g., friction-less
    this->constrain();
    //Info<< "    constraints took effect.\n";

    /// At this point, displacement (fcv_) is up-to-date

    /// Update face values, if 'this' points to displacement
    if (fieldName_ == displacementFieldName_)
    {
        Field<vector>::operator=(fcv_);
    }
    /// Update face values, if 'this' is
    /// displacement-increment
    else if (fieldName_ == word(displacementIncrementFieldName_))
    {
        //auto D0 = this->db()
        //    .lookupObject<volVectorField>
        //        (word(displacementFieldName_ + "_0"))
        //            .boundaryField()[patch().index()];

        Field<vector>::operator=(fcv_ - accumulatedDisp_);
    }

    /// Set updated_ to true (actually calls
    /// fvPatchField<vector>::updateCoeffs()
    //fixedGradientFvPatchField<vector>::updateCoeffs();

    /// Update accumulated displacement
    if (storeAccumulatedDisp_)
    {
        accumulatedDisp_ += fcv_;
    }

    /// Set updated_ to false; why?
    fvPatchField<vector>::evaluate();
}

/// Return face-center values
Field<vector> displacementNeumannFvPatchField::fcv() const
{
    if (fieldName_ == displacementFieldName_)
    {
        return fcv_;
    }
    else if (fieldName_ == word(displacementIncrementFieldName_))
    {
        return fcv_ - accumulatedDisp_;
    }

    FatalErrorIn("fcv()")
        << "Field "
        << fieldName_
        << " is not available.\n"
        << abort(FatalError);
}

tmp<Field<vector> > displacementNeumannFvPatchField::valueInternalCoeffs
(
    const tmp<scalarField>&
) const
{
    notImplemented("displacementNeumannFvPatchField does not handle\n");

    Info<< "fvm::div; you can easily implement it though!\n"
        << abort(FatalError);
}

tmp<Field<vector> > displacementNeumannFvPatchField::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    notImplemented("displacementNeumannFvPatchField does not handle\n");

    Info<< "fvm::div; you can easily implement it though!\n"
        << abort(FatalError);
}

/// OpenFOAM's name for this function is completely misleading. So, re-direct it
/// to a function with a proper name which does what OpenFOAM intends for this
/// function to do.
tmp<Field<vector> > displacementNeumannFvPatchField::
gradientInternalCoeffs() const
{
    tmp<Field<vector> > tfv = this->internalCoeffInGradEval();

    return tfv;
}

/// snGrad at the boundary is calculated as

/// snGrad = w1 * snGradFromValue + w2 * JWnGrad + w3 * HnGrad,

/// where w's are weights, and particularly, the first term also contributes to
/// the coeff matrix, while the next two terms only contribute to the source
/// vector

/// Rule in weight calculation:
/// For each (normal and tangential) of snGrad, either displacement should be
/// specified, or traction, not both. 
//void displacementNeumannFvPatchField::calcWeights()
//{
//    for(label dirI = 0; dirI < 3; ++dirI)
//    {
//        /// if for dirI, displacement is NOT specified
//        /// Note: there are only two directions, t and n.
//        if (!dirichletDir_[dirI])
//        {
//            /// We do NOT have psi @ face, easiest manifestation of that in the
//            /// weighted average is:
//            w1_[dirI] = 0;
//            w2_[dirI] = JWWeight_; // * neumannDir()[dirI];
//            w3_[dirI] = HWeight_ ; // * neumannDir()[dirI];
//        }
//        /// If Dirichlet, i.e., traction @ boundary is unknown
//        else
//        {
//            /// boundaryValueWeight(), returns 1 if not input by the user, for a
//            /// Dirichlet direction
//            w1_[dirI] = boundaryValueWeight_; // * dirichletDir_[dirI];
//
//            /// JW and H grads need traction @ boundary; unknown!
//            w2_[dirI] = 0;
//
//            w3_[dirI] = 0;
//        }
//    }
//
//    this->normalizeWeights();
//}

//void displacementNeumannFvPatchField::normalizeWeights()
//{
//    /// Normalize weights
//    for(label dirI = 0; dirI < 3; ++dirI)
//    {
//        scalar s = w1_[dirI] + w2_[dirI] + w3_[dirI];
//
//        if (mag(s) < SMALL)
//        {
//            FatalErrorIn("calcWeights()")
//                << "Seems like the sum of the three weights for direction "
//                << dirI
//                << " is zero"
//                << abort(FatalError);
//        }
//
//        w1_[dirI] /= s;
//        w2_[dirI] /= s;
//        w3_[dirI] /= s;
//    }
//}

//vector displacementNeumannFvPatchField::neumannDir() const
//{
//    /// BAD
//    /// Does not need to do the calculation on every call; go for lazy eval
//    autoPtr<vector> neumannDirPtr(new vector(vector::one));
//
//    forAll(dirichletDir_, dirI)
//    {
//        if (dirichletDir_[dirI])
//        {
//            neumannDirPtr()[dirI] = 0;
//        }
//    }
//
//    return neumannDirPtr();
//}


tmp<Field<vector> > displacementNeumannFvPatchField::
internalCoeffInGradEval() const
{
    /// OpenFOAM-style---inspired by fixedValueFvPatchFields.C
    Field<vector> coeffs(this->patch().size(), vector::zero);

    //const Field<vector>& N = scenario_.N();
    //const Field<vector>& T = scenario_.T();

    const Field<vector>& N = scenario_.N();
    const Field<vector>& T = this->conjugateN(this->patchInternalField());
    //    - ((N & this->patchInternalField()) * N);

    ///// Normalize T
    //forAll(T, faceI)
    //{
    //    if(mag(T[faceI]) > SMALL)
    //    {
    //        T[faceI] /= mag(T[faceI]);
    //    }
    //    else
    //    {
    //        T[faceI] = vector::zero;
    //    }
    //}

    const scalar& w0 = scenario_.w0();
    const scalar& w2 = scenario_.w2();
    const scalar& w3 = scenario_.w3();
    const scalar& w4 = scenario_.w4();

    if (w0)
    {
        /// Normal component of the boundary term --- implicit part
        coeffs
     += w0
      * this->scale
        (
            -w1_/* Must be (1 1 1) for full implicit treatment */,
            (
                //-vector::one*this->patch().deltaCoeffs()

                /// Control implicitness
                this->patch().deltaCoeffs() * cmptMultiply(N, N)
            )
        );
    }

    if (w2)
    {
        coeffs
     += w2
      * this->scale
        (
            -w1_/* Must be (1 1 1) for full implicit treatment */,
            (
                //-vector::one*this->patch().deltaCoeffs()

                /// Control implicitness
                this->patch().deltaCoeffs() * cmptMultiply(T, T)
            )
        );
    }

    if (w3)
    {
        coeffs
     += w3
      * this->scale
        (
            -w1_/* Must be (1 1 1) for full implicit treatment */,
            (
                //-vector::one*this->patch().deltaCoeffs()

                /// Control implicitness
                this->patch().deltaCoeffs() * cmptMultiply(T, T)
            )
        );
    }

    return coeffs;
}

/// Implicit-control vectors
//vectorField displacementNeumannFvPatchField::Lambda() const
//{
//    return this->patch().nf();
//}

/// Scale a vector v component-wise by another vector s
vectorField displacementNeumannFvPatchField::scale
(
    const vector/*Field*/& s,
    const vectorField& v
) const
{
    autoPtr<vectorField> tv(new vectorField(v.size(), vector::zero));

    forAll(v, vI)
    {
        tv()[vI][0] = s/*[sI]*/[0] * v[vI][0];
        tv()[vI][1] = s/*[sI]*/[1] * v[vI][1];
        tv()[vI][2] = s/*[sI]*/[2] * v[vI][2];
    }

    return tv();
}

vectorField displacementNeumannFvPatchField::scale
(
    const vector/*Field*/& s,
    const tmp<vectorField>& v
) const
{
    return this->scale(s, v());
}

/// OpenFOAM's name for this function is completely misleading. So, re-direct it
/// to a function with a proper name which does what OpenFOAM intends for this
/// function to do.
tmp<Field<vector> > displacementNeumannFvPatchField::
gradientBoundaryCoeffs() const
{
    tmp<Field<vector> > tfv = this->explicitTermsInGradEval();

    return tfv;
}

tmp<Field<vector> > displacementNeumannFvPatchField::
explicitTermsInGradEval() const
{
    const Field<scalar>& mu = *muPtr_/*()*/;
    const Field<scalar>& la = *lambdaPtr_/*()*/;

    const Field<vector>& N = scenario_.N();
    //const Field<vector>& T = scenario_.T();

    const scalar& w0 = scenario_.w0();
    const scalar& w2 = scenario_.w2();
    const scalar& w3 = scenario_.w3();
    const scalar& w4 = scenario_.w4();

    const Field<scalar>& delta = this->patch().deltaCoeffs();
    const Field<vector>& psi = this->patchInternalField();

    /// OpenFOAM-style---inspired by fixedValueFvPatchFields.C
    Field<vector> grad(this->patch().size(), vector::zero);

    if (w0)
    {
        grad
        /// Note: Operator precedence : * > +=
     += w0
      * (
            (N & this->snGrad()) * N
          + this->scale
            (
                w1_/* Must be (1 1 1) for full implicit treatment */,
                /// Control implicitness
                delta * cmptMultiply(cmptMultiply(N, psi), N)
            )
        );
    }

    Field<vector>& Tp = this->conjugateN(psi);

    if (w2)
    {
        Field<vector>& TsnGrad = this->conjugateN(this->snGrad());

        grad
     += w2
      * (
            (TsnGrad & this->snGrad()) * TsnGrad
          + this->scale
            (
                w1_/* Must be (1 1 1) for full implicit treatment */,
                /// Control implicitness
                delta * cmptMultiply(cmptMultiply(Tp, psi), Tp)
            )
        );

        if (w4)
        {
            const vectorField& sigmaExp = this->explicitBoundaryTraction();
            const vectorField& sigmaUser = this->userBoundaryTraction_;

            const vectorField& Texp = this->conjugateN(sigmaExp);
            const vectorField& Tuser = this->conjugateN(sigmaUser);

            grad
         += w4
          * (
                (
                    (Texp & sigmaExp) * Texp
                  - (Tuser & sigmaUser) * Tuser
                ) / (2 * mu + la)
            );
        }
    }

    if (w3)
    {
        const vectorField& Hsigma = transform
            (
                this->H(),
                userBoundaryTraction_
            );

        const vectorField& TH = this->conjugateN(Hsigma);

        grad
     += w3
      * (Hsigma & TH) * TH
      + w3
      * this->scale
        (
            w1_/* Must be (1 1 1) for full implicit treatment */,
            /// Control implicitness
            delta * cmptMultiply(cmptMultiply(Tp, psi), Tp)
        );
    }

    return grad;
}

displacementNeumannFvPatchField::~displacementNeumannFvPatchField()
{
    /// Every time step, a boundary condition object is created, where a clone
    /// constructor is used.

    /// The new object needs to force reading mechanical properties, because they
    /// are not constant with time in general.

    /// Therefore, remove these data to make sure they are not transferred to
    /// the next time.
    //muPtr_.clear();
    //lambdaPtr_.clear();

    delete muPtr_;
    delete lambdaPtr_;
}

Field<vector> displacementNeumannFvPatchField::
explicitBoundaryTraction() const
{
    /// Approach 1: Evaluate cell-center traction obtained based on
    ///             boundary-normal, and extrapolate it to the boundary.

    //tensorField sigma = this->db().lookupObject<volTensorField>("sigma")
    //    .boundaryField()[patch().index()]/*.patchInternalField()?*/;

    //vectorField traction = sigma & patch().nf();

    //return traction 
    //    + (fvc::grad(traction) & patch.deltaCoeffs());

    /// Approach 2: Evaluate boundary-traction directly from sigma

    symmTensorField sigma = this->db().lookupObject<volSymmTensorField>("sigmaCauchy")
        .boundaryField()[patch().index()];

    return sigma & patch().nf();
}

Field<vector> displacementNeumannFvPatchField::
correctNonOrthogonal() const
{
    if (correctNonOrthogonal_)
    {
        return this->internalGrad() & k();
    }

    return Field<vector>(patch().size(), vector::zero);
}

/// Tangental vectors (non-orthogonality vectors)
Field<vector> displacementNeumannFvPatchField::k() const
{
    const vectorField& n = patch().nf();
    const vectorField& d = patch().delta();

    return d - transform(n * d, n);
}


/// Find the tensor that
/// converts the traction at the current cell center into the vector produced
/// by the
/// displacement grad (at cell center), operating on the face normal.
/// Then apply this tensor on the given traction at the face center, to obtain
/// snGrad(displacement).

/// We have a few flavours of this implementation
Field<vector> displacementNeumannFvPatchField::H1() const
{
    const vectorField& n = patch().nf();

    /// internal traction
    const vectorField& it = explicitInternalTraction();
    /// Unit vectors of internal traction
    vectorField nit(it.size(), vector::zero); // / (SMALL + mag(it));

    forAll(nit, nitI)
    {
        if (mag(nit[nitI]) < SMALL)
        {
            nit[nitI][0] = 0;
            nit[nitI][1] = 0;
            nit[nitI][2] = 0;
        }
        else
        {
            nit[nitI] /= mag(nit[nitI]);
        }
    }

    /// internal-grad normal
    //const vectorField& ign = transform(internalGrad(), n);
    const vectorField& ign = internalGrad() & n;
    /// Unit vectors of ign
    vectorField nign(ign.size(), vector::zero);// / (SMALL + mag(ign));

    forAll(nign, nignI)
    {
        if ((mag(nign[nignI]) > SMALL) || (mag(nit[nignI]) > SMALL))
        {
            nign[nignI] /= mag(nign[nignI]);
        }
    }

    /// R rotates `internal traction' unit vector to
    /// `internal-grad \otimes normal' unit vector
    const tensorField& R = rotationTensor(nit, nign);

    Field<vector> snGrad(R.size(), vector::zero);

    forAll(snGrad, faceI)
    {
        if (mag(it[faceI]) > SMALL)
        {
            snGrad[faceI] = 
                transform
                (
                    /// Scaled R; this tensor, operates on `it', produces `ign'
                    mag(ign[faceI]) / (SMALL + mag(it[faceI])) * R[faceI],
                    /// So, apply it to `bt' (boundary traction), to produce
                    /// `snGrad'
                    userBoundaryTraction_[faceI]
                );
        }
    }

    return snGrad;
}

/// Another flavour
Field<tensor> displacementNeumannFvPatchField::H() const
{
    /// explicit boundary traction
    const Field<vector>& bt = explicitBoundaryTraction();

    /// Unit vectors of bt
    Field<vector> unitTraction(this->patch().size(), vector::zero);
    Field<vector>& ut = unitTraction;

    /// Calculate unitTraction, ruling out mag(bt)=0 in the denominator
    forAll(ut, utI)
    {
        if (mag(bt[utI]) < SMALL)
        {
            ut[utI][0] = 0;
            ut[utI][1] = 0;
            ut[utI][2] = 0;
        }
        else
        {
            ut[utI] /= mag(ut[utI]);
        }
    }

    /// explicit snGrad
    const Field<vector>& sng = this->snGrad();

    /// Unit vectors of sng
    Field<vector> unitSnGrad(this->patch().size(), vector::zero);
    Field<vector>& usng = unitSnGrad;

    /// Calculate usng, ruling out mag(sng)=0 in the denominator
    forAll(usng, usngI)
    {
        if (mag(sng[usngI]) < SMALL)
        {
            usng[usngI][0] = 0;
            usng[usngI][1] = 0;
            usng[usngI][2] = 0;
        }
        else
        {
            usng[usngI] /= mag(sng[usngI]);
        }
    }

    /// Scale factor
    Field<scalar> s(this->patch().size(), 0);

    forAll(s, tI)
    {
        /// Rule out mag(bt) = 0 in the denominator; already defaulted to zero
        if (mag(bt[tI]) > SMALL)
        {
            s[tI] = mag(sng[tI]) / mag(bt[tI]);
        }
    }

    return s * rotationTensor(unitTraction, unitSnGrad);
}

/// Explicit snGrad (transform(explicitGrad, n))
tmp<Field<vector> > displacementNeumannFvPatchField::snGrad() const
{
    return this->patch().deltaCoeffs()
        * ((*this) - this->patchInternalField() - correctNonOrthogonal());
}

Field<vector> displacementNeumannFvPatchField::
explicitInternalTraction() const
{
    /// Approach 1: Evaluate cell-center traction obtained based on
    ///             boundary-normal and cell-center stress

    symmTensorField sigma = this->db()
        .lookupObject<volSymmTensorField>("sigmaCauchy")
            .boundaryField()[patch().index()].patchInternalField();

    return sigma & patch().nf();
}

/// Force user-specified displacement "components" at the boundary
/// Obviously, not all components are user-specified.
void    displacementNeumannFvPatchField::userDisplacement()
{
    if (axiallyFixedStrainRate_)
    {
        //setAxialDirIndex();

        /// Current time (from TimeState)
        const scalar time = this->db().time().timeOutputValue();
        
        if (axialStrainRatePtr_.empty())
        {
            axialStrainRatePtr_.set
            (
                new scalarField
                (
                    1 / (SMALL + strainingEndTime_)
                  * log
                    (
                        1
                      + Field<scalar>
                        (
                            this->size(), totalDisp_ & axialDir_
                        )
                        / mag(startLocation_ & axialDir_)
                    )
                )
            );
        }

        /// Displacement in every time step is with reference to the start of
        /// that time step.
        /// For axially-fixed strain-rate, displacement (D) is
        fcv_ =
            startLocation_
                * (
                      exp
                      (
                          axialStrainRatePtr_()
                              * min(time, strainingEndTime_)
                      ) - 1
                  );

        /// Update displacement
        //fcv_ = D;
    }
    else
    {
        WarningIn("    userDisplacement()")
            << "No rule was found to update displacement values\n"
            << "Displacement field did not change."
            << endl;
    }
}

/// Apply constraints
/// See descriptions inside the function body
void    displacementNeumannFvPatchField::constrain()
{
    /// Frictionless means 
    /// Normal displacement is the user input
    /// Tangential traction is zero
    if (frictionless_)
    {
        /// (n*n)a = (a.n)n
        /// That is, "n*n" operating on vector "a",
        /// produces a vector which is
        /// the projection of "a" on "n".
        Field<tensor> Q = patch().nf() * patch().nf();

        if (/*axiallyFixedStrainRate_*//*Leave it to n; makes more sense*/false)
        {
            Q = Field<tensor>(this->size(), axialDir_ * axialDir_);
        }

        /// See 'black notes', p.2, 31 Dec 2025 for the proof.
          fcv_
        = 
          /// Only accept the normal-projected vector from the user
          transform(Q, fcv_)
          /// Evaluate the tangential-projected vector by extrapolation
          /// Extrapolation can or cannot be informed by traction, see the
          /// function below
        + explicitTangentialDisplacement(Q);
    }
    else
    {
        Info<< "    No constraint found to be applied to displacement\n";
    }
}

/// Displacement at cell-centers
Field<vector>
displacementNeumannFvPatchField::internalDisplacement() const
{
    /// Displacement name
    const word dName = displacementFieldName_;

    /// If displacement does NOT exist, then
    ///- We are solving for the displacement-increment
    ///- accumulatedDisp_ stored displacement history
    if 
    (
        !this->db()
            .foundObject<volVectorField>(dName)
    )
    {
        WarningIn("internalDisplacement()")
            << "Displacement field "
            << dName
            << " was not found.\n"
            << "It is read from the displacement history (accumulatedDisp_).\n";

        return accumulatedDisp_;
    }
    else
    {
        return this->db()
                   .lookupObject<volVectorField>
                   (
                       dName
                   ).boundaryField()[patch().index()].patchInternalField();
    }
}

/// Displacement Grad at cell-centers
Field<tensor>
displacementNeumannFvPatchField::internalGrad() const
{
    /// Displacement name
    const word dName = displacementFieldName_;

    /// Displacement gradient name
    const word& displacementFieldGradName/*_*/ =
        word("grad(" + dName + ")");

    const word& dgName = displacementFieldGradName/*_*/;

    /// Displacement gradient at cell center
    Field<tensor> ccg;

    /// If displacement does NOT exist
    if 
    (
        !this->db()
            .foundObject<volVectorField>(dName)
    )
    {
        //ccg = Field<tensor>(patch().size(), tensor::zero);

        FatalErrorIn("internalGrad()")
            << "Displacement field "
            << dName
            << " was not found.\n"
            << abort(FatalError);
            //<< "    Its gradient is approximated to zero tensor.\n";

        //return ccg;
    }
 
    /// If displacement-gradient exists
    if 
    (
        this->db()
            .foundObject<volTensorField>(dgName)
    )
    {
        ccg = this->db()
                  .lookupObject<volTensorField>
                   (
                       dgName
                   ).boundaryField()[patch().index()].patchInternalField();
    }
    else
    {
        ccg = fvc::grad
              (
                  this->db().lookupObject<volVectorField>
                  (dName)
              )().boundaryField()[patch().index()].patchInternalField();
        /// This function is called multiple times per time step;
        /// Does gradient calculation add overhead?
        /// No, because otherwise grad update is done outside this scope,
        /// wherein
        /// we read that in "if".
    }

    return ccg;
}

/// Designed to obtain tangential displacement vector
Field<vector>
displacementNeumannFvPatchField::explicitTangentialDisplacement
(const Field<tensor>& Q) const
{
    /// Face-center value is based on which Taylor expansion?
    /// Around cell-center
    bool TaylorCC = true;
    /// Around face-center
    /// In this case, grad at face is needed which correlates with traction
    bool TaylorFC = false;

    if (!TaylorCC)
    {
        notImplemented("explicitTangentialDisplacement()");

        Info<< "Extrapolation around cell-center is implemented only.\n"
            << abort(FatalError);
    }

    /// Displacement name
    const word dName = displacementFieldName_;

    // Displacement at cellCenters (cell-center value, ccv)
    Field<vector> ccv(internalDisplacement());

    /// Displacement gradient at cell center
    Field<tensor> ccg(internalGrad());

    /// Tangential component extrapolation:
    /// fcv.t = ccv.t + [ (grad @ ccv)(delta) ].t

    /// Vector representation, where tt = t*t tensor:
    /// tt fcv = tt ccv + tt [ (grad @ ccv)(delta) ]

    const Field<tensor>& tt = 
        //Field<diagTensor>(patch().size(), diagTensor(1,1,1)) - Q;
        Field<tensor>(patch().size(), I) - Q;

    return
        /// Tangential-projected ccv
        transform(tt, ccv)
            /// Tangential-projected difference
            + transform(tt, ccg & patch().delta());
}

void    displacementNeumannFvPatchField::read()
{
    if (this->db().foundObject<volVectorField>(displacementFieldName_))
    {
        /*const vectorField& D*/
        accumulatedDisp_ =
            this->db()
                .lookupObject
                     <volVectorField>(word(displacementFieldName_))
                         .boundaryField()[patch().index()];
    }
    else
    {
        storeAccumulatedDisp_ = true;

        WarningIn("read()")
            << "Field "
            << displacementFieldName_
            << " not found in the objectRegistry.\n"
            << "Initialized to zero."
            << endl;
    }

    fcv_ = accumulatedDisp_;
}

/// Must be called in all constructors
void displacementNeumannFvPatchField::fullyConstruct()
{
    /// Init weights and projections from the user input
    //initWeights();
    //calcWeights();
    //normalizeWeights();
    //scenario(this);

    /// Make sure material properties at this timestep are set.
    /// By calling material() in the constructor ONLY, we avoid property
    /// creation every iteration.
    /// At the same time, we FORCE its reallocation every timestep this way.
    material<scalar>(muName_, muPtr_);
    material<scalar>(lambdaName_, lambdaPtr_);
}

Field<vector>&
displacementNeumannFvPatchField::conjugateN(const Field<vector>& psi) const
{
    //Field<vector>& psi = this->patchInternalField();
    const Field<vector>& N = scenario_.N();

    tmp<Field<vector> > tT = psi - ((N & psi) * N);

    Field<vector>& T = tT();

    const scalarField& magT = mag(T);

    /// Normalize T
    forAll(T, faceI)
    {
        if(magT[faceI] > SMALL)
        {
            T[faceI] /= magT[faceI];
        }
        else
        {
            T[faceI] = vector::zero;
        }
    }

    return tT();
}

/// Must be called in all constructors
//void displacementNeumannFvPatchField::initWeights()
//{
//    /// From user
//    for (label row = 0; row<3; ++row)
//    {
//        w1_[row] = weights_[0][row];
//        w2_[row] = weights_[1][row];
//        w3_[row] = weights_[2][row];
//    }
//
//    /// Make positive
//    this->makePositive(w1_);
//    this->makePositive(w2_);
//    this->makePositive(w3_);
// 
//    /// Re-set
//    if (!pureTraction_)
//    {
//        w1_[0] = 1;
//        w1_[1] = 1;
//        w1_[2] = 1;
//    }
//}

/// Take a vector and make its components positive
//void displacementNeumannFvPatchField::makePositive(vector& weight)
//{
//    forAll(weight, wI)
//    {
//        if (weight[wI] < 0)
//        {
//            weight[wI] = mag(weight[wI]);
//        }
//    }
//}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchVectorField, displacementNeumannFvPatchField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
