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
    fixedValueFvPatchVectorField(p, iF),
    //totalDisp_(p.size(), vector::zero),
    //dispSeries_(),
    //interpPtr_(),
    axialDir_(),
    axialStrainRate_(),
    endPosition_(),
    endVelocity_()
{}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchVectorField(ptf, p, iF, mapper),
//#ifdef OPENFOAMFOUNDATION
//    totalDisp_(mapper(ptf.totalDisp_)),
//#else
//    totalDisp_(ptf.totalDisp_, mapper),
//#endif
//    dispSeries_(ptf.dispSeries_),
//    interpPtr_(),
    axialDir_(),
    axialStrainRate_(),
    endPosition_(),
    endVelocity_()
{}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchVectorField(p, iF, dict),
    //totalDisp_("value", dict, p.size()),
    //dispSeries_(),
    //interpPtr_(),
    axialDir_(),
    axialStrainRate_(),
    endPosition_(),
    endVelocity_()
{
    Info<< "Creating " << type() << " boundary condition" << endl;

    // Check if displacement is time-varying
    if (dict.found("displacementSeries"))
    {
        Info<< "    displacement is time-varying" << endl;
        dispSeries_ =
            interpolationTable<vector>(dict.subDict("displacementSeries"));

        fvPatchField<vector>::operator==
        (
            dispSeries_(this->db().time().timeOutputValue())
        );
    }
}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& pivpvf
)
:
    fixedValueFvPatchVectorField(pivpvf),
    //totalDisp_(pivpvf.totalDisp_),
    //dispSeries_(pivpvf.dispSeries_),
    //interpPtr_(),
    axialDir_(),
    axialStrainRate_(),
    endPosition_(),
    endVelocity_()
{}


fixedAxialStrainRateFvPatchVectorField::fixedAxialStrainRateFvPatchVectorField
(
    const fixedAxialStrainRateFvPatchVectorField& pivpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchVectorField(pivpvf, iF),
    //totalDisp_(pivpvf.totalDisp_),
    //dispSeries_(pivpvf.dispSeries_),
    //interpPtr_(),
    axialDir_(),
    axialStrainRate_(),
    endPosition_(),
    endVelocity_()
{}


// * * * * * * * * * * * * * * * *  Destructors  * * * * * * * * * * * * * * //


fixedAxialStrainRateFvPatchVectorField::~fixedAxialStrainRateFvPatchVectorField()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Map from self
void fixedAxialStrainRateFvPatchVectorField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedValueFvPatchVectorField::autoMap(m);

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
    fixedValueFvPatchVectorField::rmap(ptf, addr);

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

    const volVectorField& Dold =
        db().lookupObject<volVectorField>("U").oldTime();

    const vectorField& D_old = Dold.boundaryField()[patch().index()];

    vectorField velocity(patch().size(), vector::zero);

    /// Make sure axialDir_ is a unit vector
    axialDir_ /= mag(axialDir_);

    const vectorField& faceCenters = patch().Cf();

    forAll(faceCenters, faceI)
    {
        /// velocity = a z + b, where
        /// a: fixed strain rate in axial (say z) direction
        /// b: a constant, determined from end position and end velocity, i.e.,
        /// b = velocity_end - a z_end
        velocity[faceI]
      = axialStrainRate_ * (faceCenters[faceI] & axialDir_) * axialDir_
      + (endVelocity_ - axialStrainRate_ * endPosition_ * axialDir_);
    }

    /// New position of each face = velocity * deltaT + old_position
    fvPatchField<vector>::operator==
    (
        this->db().time().deltaT().value() * velocity + D_old
    );

    fixedValueFvPatchVectorField::updateCoeffs();

    // If the corresponding point displacement field has a fixedValue type
    // boundary condition, then we wil update it
    //setPointDisplacement(disp);
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

    fixedValueFvPatchVectorField::write(os);
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
