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

#include "tractionVelocityFvPatchVectorField.H"
#include "dictionary.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * //

//template<class Type>
tractionVelocityFvPatchVectorField::tractionVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchField<vector>(p, iF),
    //gradient_(p.size(), pTraits<Type>::zero),
    tractionRate_(p.size(), pTraits<vector>::zero),
    pressureRate_(p.size(), pTraits<scalar>::zero)
{}


//template<class Type>
tractionVelocityFvPatchVectorField::tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchField<vector>(ptf, p, iF, mapper),
    //gradient_(ptf.gradient_, mapper),
    tractionRate_(ptf.tractionRate_, mapper),
    pressureRate_(ptf.pressureRate_, mapper)
{}


//template<class Type>
tractionVelocityFvPatchVectorField::tractionVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchField<vector>(p, iF, dict),
    //gradient_("gradient", dict, p.size()),

    // Calls the proper constructor of Field<Type>
    tractionRate_("tractionRate", dict, p.size())
    pressureRate_("pressureRate", dict, p.size())
{
    evaluate();
}


//template<class Type>
tractionVelocityFvPatchVectorField::tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& ptf
)
:
    fixedGradientFvPatchField<vector>(ptf),
    //gradient_(ptf.gradient_),
    tractionRate_(ptf.tractionRate_),
    pressureRate_(ptf.pressureRate_)
{}

//template<class Type>
tractionVelocityFvPatchVectorField::tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchField<vector>(ptf, iF),
    //gradient_(ptf.gradient_),
    tractionRate_(ptf.tractionRate_),
    pressureRate_(ptf.pressureRate_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//template<class Type>
//void tractionVelocityFvPatchVectorField<Type>::autoMap
//(
//    const fvPatchFieldMapper& m
//)
//{
//    fixedGradientFvPatchField<Type>::autoMap(m);
//    gradient_.autoMap(m);
//}
//
//
//template<class Type>
//void tractionVelocityFvPatchVectorField<Type>::rmap
//(
//    const fvPatchField<Type>& ptf,
//    const labelList& addr
//)
//{
//    fixedGradientFvPatchField<Type>::rmap(ptf, addr);
//
//    const tractionVelocityFvPatchVectorField<Type>& fgptf =
//        refCast<const tractionVelocityFvPatchVectorField<Type> >(ptf);
//
//    gradient_.rmap(fgptf.gradient_, addr);
//}

/// Does fixedGrad::evaluate() not suffice? Do we need to have it here?
/// Apparently yes, cause you do not have access to gradient_ unless
/// through a member function
//template<class Type>
void tractionVelocityFvPatchVectorField::evaluate(const Pstream::commsTypes)
{
    /// Take traction and pressure into effect by calculating grad(U) at the new
    /// time, where U is the displacement.
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    Field<vector>::operator=
    (
        // You do not have access to gradient_ unless through a member function
        //this->patchInternalField() + gradient_/this->patch().deltaCoeffs()

        /// Considers normal-distance only
        //this->patchInternalField() + gradient()/this->patch().deltaCoeffs()

        this->patchInternalField() + gradient() & this->patch().delta()
    );

    fixedGradientFvPatchField<vector>::evaluate();
}

//template<class Type>
void Foam::tractionVelocityFvPatchVectorField::updateCoeffs()
{
    updateCoeffs(pressureRate_);
}

//template<class Type>
void Foam::tractionVelocityFvPatchVectorField::updateCoeffs
(
    const Type& pressureRate
)
{
    const label patchi = patch().index();

    //const solidDisplacementThermo& thermo =
    //    db().lookupObject<solidDisplacementThermo>
    //    (
    //        physicalProperties::typeName
    //    );

    const scalarField& E = db().lookupObject<scalarField>("E");
    //thermo.E(patchi);

    const scalarField& nu = db().lookupObject<scalarField>("nu");
    //thermo.nu(patchi);

    const scalarField mu(E/(2.0*(1.0 + nu)));
    const scalarField lambda
    (
        //thermo.planeStress()
        db().objectFound<Field<bool>* >("planeStress")
      ? nu*E/((1 + nu)*(1 - nu))
      : nu*E/((1 + nu)*(1 - 2*nu))
    );
    const scalarField threeK
    (
        //thermo.planeStress()
        db().objectFound<Field<bool>* >("planeStress")
      ? E/(1 - nu)
      : E/(1 - 2*nu)
    );

    const scalarField twoMuLambda(2*mu + lambda);

    const vectorField n(patch().nf());

    /// In OF13/solidDisplacement module, 
    /// sigmaD = mu * twoSymm(gradD) + lambda * I * tr(gradD)
    /// i.e., the stress from the elastic constitutive law.
    /// Note: twoSymm(tensor&) defined in primitives/.../TensorI.C
    /// Returns twice the symmetric part of the arg.
    const fvPatchField<symmTensor>& sigmaDot =
        patch().lookupPatchField<volSymmTensorField, symmTensor>("sigmaDot"/*D*/);

    /// Overwrite fixedGrad::gradient_
    /// Returns Field<Type>

    /// Rate formulation; does not give material lateral contraction in a
    /// tensile test
    //gradient() =
    //(
    //    (tractionRate_ - pressureRate*n)
    //  + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaDot)
    //)/twoMuLambda;

    const volTensorField gradU
    //gradient() =
  = (
        (traction_ - pressure*n)
      + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
    )/twoMuLambda;

    //const volTensorField gradUold fvc::grad(U);

    /// If you have a time varying traction, and you want take the most effect
    /// of that into your boundary condition calculations, you might need to
    /// access traction at the old time.
    //(
    //    (traction_ - pressure*n)
    //  + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
    //)/twoMuLambda;

    /// grad(v) --- new time
    gradient()
  = 2 / this->mesh().time().deltaT()
    /// grad(U) --- new time (traction and pressure take effect)
  * (
        (traction_ - pressure*n)
      + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
    )/twoMuLambda;
  - 2 / this->mesh().time().deltaT()
    /// grad(U) --- old time
  * fvc::grad
  (
      //U.oldTime()
      this->mesh().lookUp<volVectorField>("U").oldTime()
  )
    /// grad(v) --- old time
  - fvc::grad(this->psi().oldTime())

    //if (thermo.thermalStress())
    //{
    //    const scalarField& alphav = thermo.alphav(patchi);

    //    gradient() +=
    //        n*threeK*alphav*thermo.T().boundaryField()[patchi]/twoMuLambda;
    //}

    fixedGradientFvPatchVectorField::updateCoeffs();
}


//template<class Type>
//tmp<Field<Type> > tractionVelocityFvPatchVectorField<Type>::valueInternalCoeffs
//(
//    const tmp<scalarField>&
//) const
//{
//    return tmp<Field<Type> >
//    (
//        new Field<Type>(this->size(), pTraits<Type>::one)
//    );
//}
//
//
//template<class Type>
//tmp<Field<Type> > tractionVelocityFvPatchVectorField<Type>::valueBoundaryCoeffs
//(
//    const tmp<scalarField>&
//) const
//{
//    return gradient()/this->patch().deltaCoeffs();
//}
//
//
//template<class Type>
//tmp<Field<Type> > tractionVelocityFvPatchVectorField<Type>::
//gradientInternalCoeffs() const
//{
//    return tmp<Field<Type> >
//    (
//        new Field<Type>(this->size(), pTraits<Type>::zero)
//    );
//}
//
//
//template<class Type>
//tmp<Field<Type> > tractionVelocityFvPatchVectorField<Type>::
//gradientBoundaryCoeffs() const
//{
//    return gradient();
//}
//
//
//template<class Type>
//void tractionVelocityFvPatchVectorField<Type>::write(Ostream& os) const
//{
//    fixedGradientFvPatchField<Type>::write(os);
//    gradient_.writeEntry("gradient", os);
//}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
