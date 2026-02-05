/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "fixedAxialStrainRateFvPatchVectorField.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "fvcMeshPhi.H"
#include "pointMesh.H"
#include "pointFields.H"
#include "fixedValuePointPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * Private Member Functions  * * * * * * * * * * * * * * //

void fixedAxialStrainRateFvPatchVectorField::makeInterp() const
{
    if (interpPtr_.valid())
    {
        FatalErrorIn
        (
            "void fixedAxialStrainRateFvPatchVectorField::makeInterp() const"
        ) << "pointer already set" << abort(FatalError);
    }

    interpPtr_.set(new primitivePatchInterpolation(patch().patch()));
}


primitivePatchInterpolation& fixedAxialStrainRateFvPatchVectorField::interp()
{
    if (interpPtr_.empty())
    {
        makeInterp();
    }

    return interpPtr_();
}


// * * * * * * * * *  Protected Member Functions  * * * * * * * * * * * * * * //

void fixedAxialStrainRateFvPatchVectorField::setPointDisplacement
(
    const vectorField& faceDisp
)
{
    const fvMesh& mesh = patch().boundaryMesh().mesh();

    if
    (
        mesh.foundObject<pointVectorField>
        (
#ifdef OPENFOAMESIORFOUNDATION
            "point" + internalField().name()
#else
            "point" + dimensionedInternalField().name()
#endif
        )
    )
    {
        const pointVectorField& pointD =
            mesh.lookupObject<pointVectorField>
            (
#ifdef OPENFOAMESIORFOUNDATION
                "point" + internalField().name()
#else
                "point" + dimensionedInternalField().name()
#endif
            );

        // Check if the boundary is fixedValue
        if
        (
            pointD.boundaryField()[patch().index()].type()
         == fixedValuePointPatchVectorField::typeName
        )
        {
            // Use const_cast to set boundary condition
            fixedValuePointPatchVectorField& patchPointD =
                refCast<fixedValuePointPatchVectorField>
                (
                    const_cast<pointVectorField&>
                    (
                        pointD
#ifdef OPENFOAMESIORFOUNDATION
                    ).boundaryFieldRef()[patch().index()]
#else
                    ).boundaryField()[patch().index()]
#endif
                );

            // Interpolate face values to the points
            patchPointD == interp().faceToPointInterpolate(faceDisp);
        }
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    directionMixedFvPatchField<vector>(p, iF),
    totalDisp_(p.size(), vector::zero),
    //dispSeries_(),
    //interpPtr_(),
    axialDir_(vector::zero),
    axialDirIndex_(-1),
    axialStrainRate_(p.size(), 0),
    //endPosition_(),
    //endVelocity_(),
    axialStrainRatePtr_()
{
    Info<< "    Constructor 1 called\n";
}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    directionMixedFvPatchField<vector>(ptf, p, iF, mapper),
#ifdef OPENFOAMFOUNDATION
    totalDisp_(mapper(ptf.totalDisp_)),
#else
    totalDisp_(ptf.totalDisp_, mapper),
#endif
//    dispSeries_(ptf.dispSeries_),
//    interpPtr_(),
    axialDir_(),
    axialDirIndex_(-1),
    axialStrainRate_(ptf.axialStrainRate_, mapper),
    //endPosition_(),
    //endVelocity_(),
    axialStrainRatePtr_()
{
    Info<< "    Constructor 2 called\n";
}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    directionMixedFvPatchField<vector>(p, iF, dict),
    /// Keep the total displacement
    totalDisp_(p.size()),
    //dispSeries_(),
    //interpPtr_(),
    axialDir_(),
    axialDirIndex_(-1),
    axialStrainRate_(p.size(), 0),
    //Z0_(vectorField(patch().size()))
    Z0Ptr_(new vectorField(patch().Cf())),
    //endPosition_(),
    //endVelocity_(),
    axialStrainRatePtr_()
{
    Info<< "Creating " << type() << " boundary condition" << endl;

    readDict<vector>(dict, "axialDir", axialDir_);

    forAll(totalDisp_, elemI)
    {
        totalDisp_[elemI] = axialDir_;
    }

    /// Make sure axialDir_ is a unit vector
    axialDir_ /= (SMALL + mag(axialDir_));

    axialDirIndex_ = 0;

    if (!axialDir_[0]) 
    {
        if (axialDir_[1]) 
        {
            axialDirIndex_ = 1;
        }
        else if (axialDir_[2]) 
        {
            axialDirIndex_ = 2;
        }
        else
        {
            FatalErrorIn("updateCoeffs")
                << "All axialDir components are zero"
                << abort(FatalError);
        }
    }

    List<word> dirs(3);

    dirs[0]="x";
    dirs[1]="y";
    dirs[2]="z";

    Info<< "    Axial dir was detected to be "
        << dirs[axialDirIndex_]
        << endl;

    //Z0_ = patch().Cf();
    Info<< "    Z0_ size on creation = "
        << Z0Ptr_ -> size()
        << endl;

    //readDict<scalar>(dict, "axialStrainRate", axialStrainRate_);
    //readDict<scalar>(dict, "endPosition", endPosition_);
    //readDict<vector>(dict, "endVelocity", endVelocity_);

    // Check if displacement is time-varying
    //if (dict.found("displacementSeries"))
    //{
    //    Info<< "    displacement is time-varying" << endl;
    //    dispSeries_ =
    //        interpolationTable<vector>(dict.subDict("displacementSeries"));

    //    fvPatchField<vector>::operator==
    //    (
    //        dispSeries_(this->db().time().timeOutputValue())
    //    );
    //}

    Info<< "    Constructor 3 called\n";
}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& pivpvf
)
:
    directionMixedFvPatchField<vector>(pivpvf),
    totalDisp_(pivpvf.totalDisp_),
    //dispSeries_(pivpvf.dispSeries_),
    //interpPtr_(),
    axialDir_(pivpvf.axialDir_),
    axialDirIndex_(pivpvf.axialDirIndex_),
    axialStrainRate_(pivpvf.axialStrainRate_),
    //endPosition_(),
    //endVelocity_(),
    axialStrainRatePtr_()
{
    Info<< "    Constructor 4 called\n";
}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    directionMixedFvPatchField<vector>(pivpvf, iF),
    totalDisp_(pivpvf.totalDisp_),
    //dispSeries_(pivpvf.dispSeries_),
    //interpPtr_(),
    axialDir_(pivpvf.axialDir_),
    axialDirIndex_(pivpvf.axialDirIndex_),
    axialStrainRate_(pivpvf.axialStrainRate_),
    Z0Ptr_(),
    //endPosition_(),
    //endVelocity_(),
    axialStrainRatePtr_()
{
    Z0Ptr_ = autoPtr<vectorField>(new vectorField(pivpvf.Z0Ptr_()));

    Info<< "    Constructor 5 called\n";
    Info<< "    Z0Ptr_.size() = "
        << Z0Ptr_->size()
        << "\n";
}

// * * * * * * * * * * * * * * * *  Destructors  * * * * * * * * * * * * * * //

fixedAxialStrainRateFvPatchVectorField::~fixedAxialStrainRateFvPatchVectorField()
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

/// Why do lazy evaluation is because axial strain rate is fixed and needs to be
/// calculated only once
scalarField fixedAxialStrainRateFvPatchVectorField::axialStrainRate()
{
    vectorField I(patch().Cf().size(), vector::zero);

    forAll(I, facei)
    {
        I[facei][axialDirIndex_] = 1;
    }

    if (axialStrainRatePtr_.empty())
    {
        const scalar endTime = this->db().time().endTime().value();

        //vectorField I(patch().Cf().size(), vector::zero);

        forAll(I, facei)
        {
            //I[facei][axialDirIndex_] = 1;

            axialStrainRate_[facei] = - 1 / (SMALL + endTime)
                * log
                  (
                      /*I[facei] */1 + (totalDisp_[facei] & I[facei])
                          / (Z0Ptr_()[facei] & I[facei])
                  );
        }

        axialStrainRatePtr_.set(new scalarField(axialStrainRate_));
    }

    return axialStrainRatePtr_();
}

// Map from self
void fixedAxialStrainRateFvPatchVectorField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    directionMixedFvPatchField<vector>::autoMap(m);

#ifdef OPENFOAMFOUNDATION
    m(totalDisp_, totalDisp_);;
#else
    totalDisp_.autoMap(m);
#endif
}


// Reverse-map the given fvPatchField onto this fvPatchField
void fixedAxialStrainRateFvPatchVectorField::rmap
(
    const fvPatchField<vector>& ptf,
    const labelList& addr
)
{
    directionMixedFvPatchField<vector>::rmap(ptf, addr);

    const fixedAxialStrainRateFvPatchVectorField& dmptf =
        refCast<const fixedAxialStrainRateFvPatchVectorField>(ptf);

    totalDisp_.rmap(dmptf.totalDisp_, addr);
}


void fixedAxialStrainRateFvPatchVectorField::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    const volVectorField& Dold =
        db().lookupObject<volVectorField>("U").oldTime();

    const vectorField& D_old = Dold.boundaryField()[patch().index()];

    //vectorField velocity(patch().size(), vector::zero);
    vectorField D_new(patch().size(), vector::zero);

    //const scalar deltaT = this->db().time().deltaT().value();

    /// Current time (from TimeState)
    const scalar time = this->db().time().timeOutputValue();
 
    D_new = Z0Ptr_() * (exp(-axialStrainRate() * time) - 1);

    /// New position of each face = velocity * deltaT + old_position
    fvPatchField<vector>::operator==
    (
        //this->db().time().deltaT().value() * (1 - cutOff_) * velocity + D_old
        D_new - D_old
    );

    refGrad() = vectorField(this->size(), vector::zero);

    //fixedValueFvPatchVectorField::updateCoeffs();
    directionMixedFvPatchField<vector>::updateCoeffs();

    //If the corresponding point displacement field has a fixedValue type
    //boundary condition, then we wil update it
    setPointDisplacement
    (
        //this->db().time().deltaT().value() * (1 - cutOff_) * velocity + D_old
        D_new - D_old
    );

//#ifdef OPENFOAMESIORFOUNDATION
//    if (internalField().name() == "DU")
//#else
//    if (dimensionedInternalField().name() == "DU")
//#endif
//    {
//        // Incremental approach, so we wil set the increment of displacement
//        // Lookup the old displacement field and subtract it from the total
//        // displacement
//        //const volVectorField& Dold =
//        //    db().lookupObject<volVectorField>("U").oldTime();
//
//    }
//    else
//    {
//        FatalErrorIn("fixedAxialStrianRateFvPatchVectorField::updateCoeff()")
//            << "\nThe name of this field must be DU"
//            << abort(FatalError);
//    }

    //if (!cutOff_)
    //{
        ///*const*/ vectorField faceCenters = patch().Cf();

        //scalar lowerBoundPosition = SMALL;
        //scalar upperBoundPosition = GREAT;

        //forAll(faceCenters, faceI)
        //{
        //    upperBoundPosition = min(upperBoundPosition, faceCenters[faceI][axialDirIndex]);
        //    upperBoundPosition = max(upperBoundPosition, endPosition_);

        //    lowerBoundPosition = max(lowerBoundPosition, faceCenters[faceI][axialDirIndex]);
        //    lowerBoundPosition = min(lowerBoundPosition, endPosition_);
        //}

        //forAll(faceCenters, faceI)
        //{
        //    /// velocity = a z + b, where
        //    /// a: fixed strain rate in axial (say z) direction
        //    /// b: a constant, determined from end position and end velocity, i.e.,
        //    /// b = velocity_end - a z_end
        //    velocity[faceI]
        //  = axialStrainRate_ * (faceCenters[faceI] & axialDir_) * axialDir_
        //  + (endVelocity_ - axialStrainRate_ * endPosition_ * axialDir_);
        //}

        //vectorField trialNewPositions = this->db().time().deltaT().value()
        //    * velocity + D_old;

        //forAll(trialNewPositions, vectorI)
        // Check one face only
        //label vectorI = 0;
        //{
        //    if
        //    (
        //        (trialNewPositions[vectorI][axialDirIndex] > upperBoundPosition)
        //     || (trialNewPositions[vectorI][axialDirIndex] < lowerBoundPosition)
        //    )
        //    {
        //        /// Do not move the patch anymore
        //        cutOff_ = 1;
        //        //break;
        //    }
        //}
    //}
}


Foam::tmp<Foam::Field<vector> >
fixedAxialStrainRateFvPatchVectorField::snGrad() const
{
    //- fixedValue snGrad with no correction
    //  return (*this - patchInternalField())*this->patch().deltaCoeffs();

    const fvPatchField<tensor>& gradField =
        patch().lookupPatchField<volTensorField, tensor>
        (
#ifdef OPENFOAMESIORFOUNDATION
            "grad(" + internalField().name() + ")"
#else
            "grad(" + dimensionedInternalField().name() + ")"
#endif
        );

    // Unit normals
    const vectorField n(patch().nf());

    // Delta vectors
    const vectorField delta(patch().delta());

    //- Non-orthogonal correction vectors
    const vectorField k((I - sqr(n)) & delta);

    return
    (
        *this
        - (patchInternalField() + (k & gradField.patchInternalField()))
    )*patch().deltaCoeffs();
}

tmp<Field<vector> >
fixedAxialStrainRateFvPatchVectorField::gradientBoundaryCoeffs() const
{
    const fvPatchField<tensor>& gradField =
        patch().lookupPatchField<volTensorField, tensor>
        (
#ifdef OPENFOAMESIORFOUNDATION
            "grad(" + internalField().name() + ")"
#else
            "grad(" + dimensionedInternalField().name() + ")"
#endif
        );

    // Unit normals
    vectorField n(patch().nf());

    // Delta vectors
    vectorField delta(patch().delta());

    //- Non-orthogonal correction vectors
    vectorField k((I - sqr(n)) & delta);

    return
    (
        patch().deltaCoeffs()
       *(*this - (k & gradField.patchInternalField()))
    );
}

void fixedAxialStrainRateFvPatchVectorField::write(Ostream& os) const
{
    if (dispSeries_.size())
    {
        os.writeKeyword("displacementSeries") << nl;
        os << token::BEGIN_BLOCK << nl;
        dispSeries_.write(os);
        os << token::END_BLOCK << nl;
    }

    axialStrainRate_.writeEntry("negAxialStrainRate", os);

    directionMixedFvPatchField<vector>::write(os);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchVectorField,
    fixedAxialStrainRateFvPatchVectorField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
