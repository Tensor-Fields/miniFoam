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

Documentation
    In the following, previous approaches for frictionless implementation is
    summarized. Some of them are tested, some of them are not.

    Approach 1:
        - Evaluate the cell-center value, projected to the tangential direction
        - Extrapolate the tangential cell-center value to the face-center
    
    // cellCenterValuesProjectedToNormal
    Field<vector> ccvn = transform(Q, ccv);
    
    // cellCenterValuesProjectedToTangential (ccvt)
    Field<vector> ccvt = ccv - transform(Q, ccv);
    
    // face-center value, tangentail (fcvt); extrapolated ccvt to the face center
    fcvt = ccvt + transformedGrad;
    
    /// Approach 2:
    ///     - Extrapolate the cell-center value to the face-center
    ///     - Keep the tangentail projection of the extrapolated value only

    /// Approach 3: (does not produce zero shear stress)

        /// Cell-center value extrapolated to the face center (fcv)
        Field<vector> fcv = ccv + (ccg & patch().delta());

        /// fcv projected to the tangential direction (fcvt)
        Field<vector> fcvt = fcv - transform(Q, fcv);
    
    /// Approach 4:

    /// Tangential Displacement increment, at face center,
    /// equals
    /// A4-1. Tangential Displacement increment, at cell center (tdic),
    Field<vector> tdic = ccv - transform(Q, ccv);

    /// Approach 4-2. In a sunny, normal day,

    /// fcv_ = ccv_ + grad(D) delta

    /// That is
    /// fcv_n = transform(Q, fcv_)
    /// fcv_t = fcv_ - fcv_n

    /// and
    /// fcv_n = ccv_n + transform(Q, grad(D) delta)
    /// fcv_t = (ccv - ccv_n) + transform(") - transform(Q, transform("))

    /// However, fcv_n is prescribed!

    /// In a frictionless day though,
    /// grad(D)_n = 0; why? It just seems right!
    /// so,
    fcv_ =
        ccv
            * (grad - transform(Q, grad))
                & (delta - transform(Q, delta));
    
    /// Normal displacement increment (di), at face center, (ndif)
    /// equals
    /// the projected di to the normal direction
    Field<vector> ndif =
        transform(Q, fcv_ - accumulatedDisp_);
    
    /// final displacement increment (di), at face center,
    /// (face-center value, fcv_)
    fcv_ = tdic + ndif;

\*---------------------------------------------------------------------------*/

#include "displacementFvPatchField.H"
#include "fvc.H"
#include "surfaceMesh.H"
#include "fvsPatchField.H"
#include "fvMatrices.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

displacementFvPatchField::displacementFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(p, iF),
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
    displacementIncrementFieldName_()
{
    Info<< "    Constructor 1\n";

    FatalErrorIn("Constructor")
        << "Currently, this constructor must not be used, unless it is clear\n"
        << "what its purpose is\n"
        << "It cannot be commented, because runTimeSelection mechanism asks\n"
        << "for it"
        << abort(FatalError);
}

displacementFvPatchField::displacementFvPatchField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<vector>(p, iF, dict),
    axialDir_(),
    axialDirPtr_(),
    axiallyFixedStrainRate_(false),
    axialStrainRatePtr_(NULL),
    strainingEndTime_(),
    totalDisp_(),
    accumulatedDisp_(p.size(), vector::zero),
    startLocation_(p.Cf()),
    displacementFieldName_(),
    frictionless_(false)
{
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
    /// Operations in updateCoeffs() assume the field to be either
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
            << word("D" + displacementFieldName_)
            << abort(FatalError);
    }

    if
    (
        !dict.readIfPresent<word>("displacementGradName", fieldGradName_)
    )
    {
        fieldGradName_ = word("grad(" + fieldName_ + ")");
    }

    if
    (
        dict.readIfPresent<bool>
        ("axiallyFixedStrainRate", axiallyFixedStrainRate_)
    )
    {
        if (axiallyFixedStrainRate_)
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
    dict.readIfPresent<vectorField>(word("accumulatedDisp"), accumulatedDisp_);

    /// Define a vector called "displacement (fcv_, face-center value)",
    /// which is
    /// faceLocationsNow - startLocation_
    this->read();
}

displacementFvPatchField::displacementFvPatchField
(
    const displacementFvPatchField& pf, // patch field
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<vector>(pf, p, iF, mapper),
    startLocation_(pf.startLocation_, mapper),
    fcv_(pf.fcv_, mapper)
{
    axialDir_ = pf.axialDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    accumulatedDisp_ = pf.accumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;
}

displacementFvPatchField::displacementFvPatchField
(
    const displacementFvPatchField& pf
)
:
    fixedValueFvPatchField<vector>(pf),
    startLocation_(pf.startLocation_),
    fcv_(pf.fcv_)
{
    axialDir_ = pf.axialDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    accumulatedDisp_ = pf.accumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;
}

/// Note: Not having the following constructor, as well as clone(iF)
/// construcgtor (see .H file),
/// causes a parent object to be created every
/// time-step, instead of the current derived patch type.
/// That is, from the second time-step onwards, you will have a fixedValue
/// boundary condition on this patch.

displacementFvPatchField::displacementFvPatchField
(
    const displacementFvPatchField& pf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(pf, iF),
    startLocation_(pf.startLocation_),
    fcv_(pf.fcv_)
{
    axialDir_ = pf.axialDir_;
    axialDirPtr_ = pf.axialDirPtr_;
    axiallyFixedStrainRate_ = pf.axiallyFixedStrainRate_;
    axialStrainRatePtr_ = pf.axialStrainRatePtr_;
    strainingEndTime_ = pf.strainingEndTime_;
    totalDisp_ = pf.totalDisp_;
    accumulatedDisp_ = pf.accumulatedDisp_;
    displacementFieldName_ = pf.displacementFieldName_;
    frictionless_ = pf.frictionless_;
    displacementIncrementFieldName_ = pf.displacementIncrementFieldName_;
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void displacementFvPatchField::write(Ostream& os) const
{
    //fvPatchField<vector>::write(os);
    //writeEntry("type", os);
    os.writeKeyword("type")
        << "displacement" << token::END_STATEMENT << nl;
    os.writeKeyword("displacementFieldName")
        << displacementFieldName_ << token::END_STATEMENT << nl;
    os.writeKeyword("frictionless")
        << frictionless_ << token::END_STATEMENT << nl;
    os.writeKeyword("axiallyFixedStrainRate")
        << axiallyFixedStrainRate_ << token::END_STATEMENT << nl;
    os.writeKeyword("fieldGradName")
        << fieldGradName_ << token::END_STATEMENT << nl;
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
    writeEntry("value", os);
}
 
void displacementFvPatchField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    /// Force reading the displacement field if exists
    this->read();

    /// Now, calculate the new displacement based on the prescribed rules by the
    /// user, e.g., fixed-strain-rate, etc.
    this->calc();

    /// Update displacement by applying constraints, e.g., friction-less
    this->constrain();
    //this->constrainSigma();

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

    /// Follow the convention
    fixedValueFvPatchField<vector>::updateCoeffs();

    /// Update accumulated displacement
    if (storeAccumulatedDisp_)
    {
        accumulatedDisp_ += fcv_;
    }
}

void    displacementFvPatchField::calc()
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
        //Field<vector> D =
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
        //WarningIn("    calc()")
        //    << "No rule was found to update displacement values\n"
        //    << "Displacement field did not change."
        //    << endl;
    }
}

void    displacementFvPatchField::constrain()
{
    if (frictionless_)
    {
        /// (n*n)a = (a.n)n
        /// That is, "n*n" operating on vector "a",
        /// produces a vector which is
        /// the projection of "a" on "n".
        Field<tensor> Q = patch().nf() * patch().nf();

        if (axiallyFixedStrainRate_)
        {
            Q = Field<tensor>(this->size(), axialDir_ * axialDir_);
        }

        /// Displacement name
        word& dName = displacementFieldName_;
        /// Displacement increment name
        //word& diName = displacementIncrementFieldName_;

        // Displacement-increment at cellCenters (cell-center value, ccv)
        // Update: Displacement at cellCenters (cell-center value, ccv)
        //Field<vector> ccv = this->patchInternalField();
        Field<vector> ccv = 
            this->db()
                .lookupObject
                     <volVectorField>(dName)
                         .boundaryField()[patch().index()].patchInternalField();
                     //<volVectorField>(diName).boundaryField()[patch().index()];

        /// Displacement gradient name
        word displacementFieldGradName/*_*/ =
            word("grad(" + dName + ")");

        /// Displacement increment gradient name
        //word displacementIncrementFieldGradName/*_*/ =
        //    word("grad(" + diName + ")");

        word& dgName = displacementFieldGradName/*_*/;
        //word& digName = displacementIncrementFieldGradName/*_*/;

        /// Displacement-increment gradient at cell centers
        /// Update: Changed to displacement gradient
        Field<tensor> ccg;

        if 
        (
            this->db()
                .foundObject<volTensorField>(dgName)
                //.foundObject<volTensorField>(digName)
        )
        {
            ccg = this->db()
                      .lookupObject<volTensorField>
                       (
                           dgName
                           //digName
                       ).boundaryField()[patch().index()].patchInternalField();
        }
        else
        {
            ccg = fvc::grad
                  (
                      this->db().lookupObject<volVectorField>
                      (dName)
                      //(diName)
                  )().boundaryField()[patch().index()].patchInternalField();
            /// This function is called multiple times per time step;
            /// Does gradient calculation add overhead?
            /// No, because otherwise grad update is done outside this scope, wherein
            /// we read that in "if".
        }

        /// See 'black notes', p.2, 31 Dec 2025 for the proof.
          fcv_
        = transform(Q, fcv_)
        + ccv - transform(Q, ccv)
        + (ccg & patch().delta()) - transform(Q, ccg & patch().delta());
    }
    else
    {
        Info<< "    No constraint found to be applied to displacement\n";
    }
}

void    displacementFvPatchField::read()
{
    if (this->db().foundObject<volVectorField>(displacementFieldName_))
    {
        /*const vectorField& D*/
        accumulatedDisp_ =
            this->db()
                .lookupObject
                     <volVectorField>(word(displacementFieldName_))
                         .boundaryField()[patch().index()];

        fcv_ = accumulatedDisp_;

        //Info<< "    Displacement on patch "
        //    << patch().name()
        //    << " read from "
        //    << displacementFieldName_
        //    << endl;
    }
    else
    {
        storeAccumulatedDisp_ = true;

        fcv_ = accumulatedDisp_;
        //Field<vector>(this->size(), vector::zero);

        WarningIn("read()")
            << "Field "
            << displacementFieldName_
            << " not found in the objectRegistry.\n"
            << "Initialized to zero."
            << endl;
    }
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchVectorField, displacementFvPatchField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
