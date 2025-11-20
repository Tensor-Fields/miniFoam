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

\*---------------------------------------------------------------------------*/

#include "noForceForVelocityFvPatchField.H"
#include "fvc.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
noForceForVelocityFvPatchField<Type>::noForceForVelocityFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    uniformValue_(pTraits<Type>::zero)
{}


template<class Type>
noForceForVelocityFvPatchField<Type>::noForceForVelocityFvPatchField
(
    const noForceForVelocityFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper&
)
:
    fixedValueFvPatchField<Type>(p, iF),
    uniformValue_(ptf.uniformValue_)
{
    fvPatchField<Type>::operator==(uniformValue_);
}


template<class Type>
noForceForVelocityFvPatchField<Type>::noForceForVelocityFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF),
    uniformValue_(pTraits<Type>(dict.lookup("")))
    //uniformValue_(pTraits<Type>(dict.lookup("uniformValue")))
{
    // Wrong
    //fvPatchField<Type>::operator==(patch().patchInternalField());

    fvPatchField<Type>::operator==(fvPatchField<Type>::patchInternalField());
}


template<class Type>
noForceForVelocityFvPatchField<Type>::noForceForVelocityFvPatchField
(
    const noForceForVelocityFvPatchField<Type>& ptf
)
:
    fixedValueFvPatchField<Type>(ptf),
    uniformValue_(ptf.uniformValue_)
{
    fvPatchField<Type>::operator==(uniformValue_);
}


template<class Type>
noForceForVelocityFvPatchField<Type>::noForceForVelocityFvPatchField
(
    const noForceForVelocityFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    uniformValue_(ptf.uniformValue_)
{
    fvPatchField<Type>::operator==(uniformValue_);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void noForceForVelocityFvPatchField<Type>::autoMap
(
    const fvPatchFieldMapper& m
)
{
    this->setSize(m.size());
    fvPatchField<Type>::operator==(uniformValue_);
}


template<class Type>
void noForceForVelocityFvPatchField<Type>::write(Ostream& os) const
{
    fvPatchField<Type>::write(os);
    os.writeKeyword("uniformValue")
        << uniformValue_ << token::END_STATEMENT << nl;
}

template<class Type>
void noForceForVelocityFvPatchField<Type>::evaluate
(
    const Pstream::commsTypes commsType
)
{
    /// Reference to the cell-center values adjacent to boundary faces
    auto& psi = this->patchIntenalField();

    autoPtr<GeometricField<Type, fvPatchField, volMesh> > gradPsiPtr;

    /// Find or create the gradient field
    if (this->patch().mesh().db().objectFound("grad("+this->psi().name()+")"))
    {
        gradPsiPtr.set
        (
            new
            GeometricField<Type, fvPatchField, volMesh>
            (
                this->patch().mesh().db()["grad("+this->psi().name()+")"]
            )
        );
    }
    else
    {
        gradPsiPtr.set
        (
            new
            GeometricField<Type, fvPatchField, volMesh>
            (
                //IOobject
                //(
                //    "grad("+this->psi().name()+")",
                //    this->patch().mesh().time().timeName(),
                //    this->patch().mesh()
                //),
                //this->patch().mesh()
                fvc::grad(this->internalField())
            )
        );
    }

    /// Reference to the gradient value at the cell-centers adjacent to the
    /// boudnary
    auto& gradPsi = gradPsiPtr().boundary().patchInternalField();

    /// Linear-extrapolate the internal value to the face center
    this->operator==
    (
        psi + this->patch().delta() & gradPsi
    );

    Info<< "    noForceForVelocityFvPatchField"
        << " boundary condition evaluated successfully"
        << endl;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
