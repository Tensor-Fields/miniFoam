/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2025 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/

#include "sinFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::sinFvPatchScalarField::
sinFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchScalarField(p, iF),
    scalarData_(dict.lookup<scalar>("scalarData", unitAny)),
    data_(dict.lookup<scalar>("data")),
    fieldData_("fieldData", iF.dimensions(), dict, p.size()),
    timeVsData_
    (
        Function1<scalar>::New
        (
            "timeVsData",
            db().time().userUnits(),
            unitAny,
            dict
        )
    ),
    wordData_(dict.lookupOrDefault<word>("wordName", "wordDefault")),
    labelData_(-1),
    boolData_(false)
{


    fixedValueFvPatchScalarField::evaluate();

    /*
    // Initialise with the value entry if evaluation is not possible
    fvPatchScalarField::operator=
    (
        scalarField("value", iF.dimensions(), dict, p.size())
    );
    */
}


Foam::sinFvPatchScalarField::
sinFvPatchScalarField
(
    const sinFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fieldMapper& mapper
)
:
    fixedValueFvPatchScalarField(ptf, p, iF, mapper),
    scalarData_(ptf.scalarData_),
    data_(ptf.data_),
    fieldData_(mapper(ptf.fieldData_)),
    timeVsData_(ptf.timeVsData_, false),
    wordData_(ptf.wordData_),
    labelData_(-1),
    boolData_(ptf.boolData_)
{}


Foam::sinFvPatchScalarField::
sinFvPatchScalarField
(
    const sinFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedValueFvPatchScalarField(ptf, iF),
    scalarData_(ptf.scalarData_),
    data_(ptf.data_),
    fieldData_(ptf.fieldData_),
    timeVsData_(ptf.timeVsData_, false),
    wordData_(ptf.wordData_),
    labelData_(-1),
    boolData_(ptf.boolData_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::sinFvPatchScalarField::map
(
    const fvPatchScalarField& ptf,
    const fieldMapper& mapper
)
{
    fixedValueFvPatchScalarField::map(ptf, mapper);

    const sinFvPatchScalarField& tiptf =
        refCast<const sinFvPatchScalarField>(ptf);

    mapper(fieldData_, tiptf.fieldData_);
}


void Foam::sinFvPatchScalarField::reset
(
    const fvPatchScalarField& ptf
)
{
    fixedValueFvPatchScalarField::reset(ptf);

    const sinFvPatchScalarField& tiptf =
        refCast<const sinFvPatchScalarField>(ptf);

    fieldData_.reset(tiptf.fieldData_);
}


void Foam::sinFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    forAll(patch().Cf(), faceI)
    {
        const vector& faceCenter = patch().Cf()[faceI];

        /*faceCenter[2]*/ fieldData_[faceI] 
            = sin(2 * constant::mathematical/*Constants*/::pi
                /// db() to access mesh
                * (faceCenter[0] + faceCenter[1] + db().time().value()));
    }

    fixedValueFvPatchScalarField::operator==
    (
      //  data_
      /*+*/ fieldData_
      //+ scalarData_*timeVsData_->value(db().time().value())
    );

    fixedValueFvPatchScalarField::updateCoeffs();
}


void Foam::sinFvPatchScalarField::write
(
    Ostream& os
) const
{
    fvPatchScalarField::write(os);
    writeEntry(os, "scalarData", scalarData_);
    writeEntry(os, "data", data_);
    writeEntry(os, "fieldData", fieldData_);
    writeEntry(os, db().time().userUnits(), unitAny, timeVsData_());
    writeEntry(os, "wordData", wordData_);
    writeEntry(os, "value", *this);
}


// * * * * * * * * * * * * * * Build Macro Function  * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        fvPatchScalarField,
        sinFvPatchScalarField
    );
}

// ************************************************************************* //
