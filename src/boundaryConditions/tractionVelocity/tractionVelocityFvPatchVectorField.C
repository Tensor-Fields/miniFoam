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
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "fvc.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

tractionVelocityFvPatchVectorField::
tractionVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchVectorField(p, iF),
    traction_(p.size(), vector::zero),
    pressure_(p.size(), 0.0),
    /// Size = 2; initialize with 0.
    /// Always problematic; init empty, fill it in the constructor body
    // Wrong //convergenceFactors_(List<scalar>(2, 0))
    // Wrong //convergenceFactors_(2, 0)
    convergenceFactors_(2)
    // convergenceFactors_()
    /// Variation of implementation; default to var1
{
    fvPatchVectorField::operator=(patchInternalField());
    gradient() = vector::zero;

    // Wrong //convergenceFactors_ = List<scalar>(2, 0);
    // Wrong //convergenceFactors_(List<scalar>(2, 0));

    /// Default to 2 and 1
    convergenceFactors_[0] = 2;
    convergenceFactors_[1] = 1;
}


tractionVelocityFvPatchVectorField::
tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& tdpvf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchVectorField(tdpvf, p, iF, mapper),
    traction_(tdpvf.traction_, mapper),
    pressure_(tdpvf.pressure_, mapper)//,
    //convergenceFactors_(tdpvf.convergenceFactors_, mapper)
{}


tractionVelocityFvPatchVectorField::
tractionVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchVectorField(p, iF),
    traction_("traction", dict, p.size()),
    pressure_("pressure", dict, p.size()),
    /// Init
    convergenceFactors_
    (
        dict.lookupOrDefault<scalarList>
        ("convergenceFactors", scalarList(2))
    ),
    var_
    (
        dict.lookupOrDefault<word>
        ("formulation", "orthogonal")
    ),
    DName_
    (
        dict.lookupOrDefault<word>
        ("DName", "D")
    )
{
    fvPatchVectorField::operator=(patchInternalField());
    gradient() = vector::zero;

    /// Default to 2 and 1
    convergenceFactors_[0] = 2;
    convergenceFactors_[1] = 1;

    /// Print info
    if (dict.found("convergenceFactors"))
    {
        Info<< "    "
            << "convergenceFactors"
            << " read from the dictionary."
            << endl;
    }
    else
    {
        Info<< "    "
            << "convergenceFactors"
            << " NOT found in the dictionary, set to default."
            << endl;
    }

    if (dict.found("formulation"))
    {
        Info<< "    "
            << "formulation"
            << " read from the dictionary."
            << endl;
    }
    else
    {
        Info<< "    "
            << "formulation"
            << " NOT found in the dictionary, set to 'orthogonal'."
            << endl;
    }

    if (dict.found("DName"))
    {
        Info<< "    "
            << "DName"
            << " read from the dictionary."
            << endl;
    }
    else
    {
        Info<< "    "
            << "DName"
            << " NOT found in the dictionary, set to 'D'."
            << endl;
    }
}


tractionVelocityFvPatchVectorField::
tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& tdpvf
)
:
    fixedGradientFvPatchVectorField(tdpvf),
    traction_(tdpvf.traction_),
    pressure_(tdpvf.pressure_),
    convergenceFactors_(tdpvf.convergenceFactors_)
{}


tractionVelocityFvPatchVectorField::
tractionVelocityFvPatchVectorField
(
    const tractionVelocityFvPatchVectorField& tdpvf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedGradientFvPatchVectorField(tdpvf, iF),
    traction_(tdpvf.traction_),
    pressure_(tdpvf.pressure_),
    convergenceFactors_(tdpvf.convergenceFactors_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void tractionVelocityFvPatchVectorField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedGradientFvPatchVectorField::autoMap(m);
    traction_.autoMap(m);
    pressure_.autoMap(m);
    //convergenceFactors_.autoMap(m);
}


void tractionVelocityFvPatchVectorField::rmap
(
    const fvPatchVectorField& ptf,
    const labelList& addr
)
{
    fixedGradientFvPatchVectorField::rmap(ptf, addr);

    const tractionVelocityFvPatchVectorField& dmptf =
        refCast<const tractionVelocityFvPatchVectorField>(ptf);

    traction_.rmap(dmptf.traction_, addr);
    pressure_.rmap(dmptf.pressure_, addr);
    //convergenceFactors_.rmap(dmptf.convergenceFactors_, addr);
}


void tractionVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const dictionary& mechanicalProperties =
        db().lookupObject<IOdictionary>("mechanicalProperties");

    //const dictionary& thermalProperties =
    //    db().lookupObject<IOdictionary>("thermalProperties");

    dimensionedScalar rho(mechanicalProperties.lookup("rho"));
    dimensionedScalar rhoE(mechanicalProperties.lookup("E"));
    dimensionedScalar nu(mechanicalProperties.lookup("nu"));

    dimensionedScalar E = rhoE/rho;
    dimensionedScalar mu = E/(2.0*(1.0 + nu));
    dimensionedScalar lambda = nu*E/((1.0 + nu)*(1.0 - 2.0*nu));
    dimensionedScalar threeK = E/(1.0 - 2.0*nu);

    Switch planeStress(mechanicalProperties.lookup("planeStress"));

    if (planeStress)
    {
        lambda = nu*E/((1.0 + nu)*(1.0 - nu));
        threeK = E/(1.0 - nu);
    }

    vectorField n = patch().nf();

    const fvPatchField<symmTensor>& sigmaD =
        patch()
        /// Keep the object registry happy
        .lookupPatchField<volSymmTensorField,symmTensor>("sigma")/*("sigmaD")*/;

    /// grad(v_new)
        ///- Get v_new, v_old, and D_old names
        /// DimensionedField<> calls IOobject::name()
        const word& vNew = dimensionedInternalField().name();

        ///- Get v_old, and D_old names
        const word& vOld = db().lookupObject<volVectorField>(vNew)
            .oldTime().name();
        const word& DOld = db().lookupObject<volVectorField>(DName_)
            .oldTime().name();

    /// The following could be member data, but we need them only here and in grad().
    /// So, init them empty and pass them to grad.

    /// "F" in names stands for "face"; we evaluate grad values on patch faces

        ///- Set pointer to patchField of grad(v_new)
        autoPtr<fvPatchField<tensor> > gradVFPtr;
        grad(vNew, gradVFPtr);

        ///- Set pointer to patchField of grad(v_old)
        autoPtr<fvPatchField<tensor> > gradVOldFPtr;
        grad(vOld, gradVOldFPtr);

        ///- Set pointer to patchField of grad(D_old)
        autoPtr<fvPatchField<tensor> > gradDOldFPtr;
        grad(DOld, gradDOldFPtr);

    /// Choose a variation of the BC implementation according to the dict.

    ///- var1) Coeff used: 2mu + lambda
    if (var_ == "orthogonal")
    {
        scalar twoMuLambda = (2*mu + lambda).value();

        //gradient() =
        //(
        //    (traction_ + pressure_*n)/rho.value()
        //  + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
        //)/twoMuLambda;

        /// Overwrite fixedGrad::gradient_
        /// Returns Field<Type>, so no dimensions.

          /// grad(v) --- new time
          gradient()
        = 2 / db().time().deltaT().value()
          /// grad(U) --- new time (traction and pressure take effect)
        * (
              (traction_ + pressure_*n)/rho.value()
            + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
          )/twoMuLambda;
        - 2 / db().time().deltaT().value()
        /// grad(U) --- old time
        /// fvcGrad accepts GeometricField only
        * fvc::grad
        (
            //U.oldTime()
            db().lookupObject<volVectorField>("D").oldTime()
        )()
        /// Make dimless
        .internalField()
        /// grad(v) --- old time
        - fvc::grad
        (
            db().lookupObject<volVectorField>("v").oldTime()
        )()
        /// Make dimless
        .internalField();

        //Switch thermalStress(thermalProperties.lookup("thermalStress"));

        //if (thermalStress)
        //{
        //    dimensionedScalar alpha(thermalProperties.lookup("alpha"));
        //    dimensionedScalar threeKalpha = threeK*alpha;

        //    const fvPatchField<scalar>& T =
        //        patch().lookupPatchField<volScalarField, scalar>("T");

        //    gradient() += n*threeKalpha.value()*T/twoMuLambda;
        //}

        /// Orthogonal mesh only
        fixedGradientFvPatchVectorField::updateCoeffs();
    }
    else if (var_ == "nonOrthogonal")
    {
        Info<< "    "
            << "nonOrthogonal tractionVelocity applies ..."
            << endl;

        scalar convergeCoeff = (convergenceFactors_[0]*mu + convergenceFactors_[1]*lambda).value();

        /// n.grad(v_new)
        Field<vector> snGrad

      = 2 / db().time().deltaT().value()

        /// grad(D) --- new time (traction and pressure take effect)
      * (
            (traction_ - pressure_*n)/rho.value()
          + convergeCoeff*fvPatchField<vector>::snGrad() - (n & sigmaD)
        )/convergeCoeff;

      - 2 / db().time().deltaT().value()

        /// grad(D) --- old time
      * gradDOldFPtr()

        /// grad(v) --- old time
      - gradVOldFPtr();

        /// Non-orthogonal vector
        ///- fvPatch::delta() returns tmp<vectorField>
        Field<vector> k = patch().delta()() - (patch().delta()() & n)*n
        /*patch().nf()*/;

        /// General (non-orthogonal) mesh
        Field<vector>::operator=
        (
            /*this->*/patchInternalField() 
          - snGrad / /*this->*/patch().deltaCoeffs()
          - (k & gradVFPtr())
        );
    }
    //// Back-up of non-orthogonal variation
    ////{
    ////    //List<scalar>& convergeFactors = convergenceFactors_;
    ////    scalar convergeCoeff = (convergenceFactors_[0]*mu + convergenceFactors_[1]*lambda).value();

    ////    //gradient() =
    ////    //(
    ////    //    (traction_ + pressure_*n)/rho.value()
    ////    //  + twoMuLambda*fvPatchField<vector>::snGrad() - (n & sigmaD)
    ////    //)/twoMuLambda;

    ////    /// Overwrite fixedGrad::gradient_
    ////    /// Returns Field<Type>, so no dimensions.

    ////      /// grad(v) --- new time
    ////      gradient()
    ////    = 2 / db().time().deltaT().value()
    ////      /// grad(U) --- new time (traction and pressure take effect)
    ////    * (
    ////          (traction_ - pressure_*n)/rho.value()
    ////        + convergeCoeff*fvPatchField<vector>::snGrad() - (n & sigmaD)
    ////      )/convergeCoeff;
    ////    - 2 / db().time().deltaT().value()
    ////    /// grad(U) --- old time
    ////    /// fvcGrad accepts GeometricField only
    ////    * fvc::grad
    ////    (
    ////        //U.oldTime()
    ////        db().lookupObject<volVectorField>("D").oldTime()
    ////    )()
    ////    /// Make dimless
    ////    .internalField()
    ////    /// grad(v) --- old time
    ////    - fvc::grad
    ////    (
    ////        db().lookupObject<volVectorField>("v").oldTime()
    ////    )()
    ////    /// Make dimless
    ////    .internalField();

    ////    /// Rename gradient(), it is not a correct name
    ////    /*typename outerProduct<vector, vector>::type*/ Field<vector> snGrad = gradient();

    ////    /// Non-orthogonal vector
    ////        ///- fvPatch::delta() returns tmp<vectorField>
    ////    Field<vector> k = patch().delta()() - (patch().delta()() & n)*n
    ////    /*patch().nf()*/;

    ////    /// General (non-orthogonal) mesh
    ////    Field<vector>::operator=
    ////    (
    ////        /*this->*/patchInternalField() 
    ////      - snGrad / /*this->*/patch().deltaCoeffs()
    ////      - k
    ////      & Field<vector>
    ////      (
    ////          fvc::interpolate
    ////          (
    ////              fvc::grad
    ////              (
    ////                  db().lookupObject<volVectorField>("v").oldTime()
    ////              )
    ////          )
    ////      )
    ////    );
    ////}
    else
    {
        FatalErrorIn("tractionVelocity.C")
            << "formulation can be "
            << "non-/orthogonal"
            << endl;
    }
}

void tractionVelocityFvPatchVectorField::grad
(
    const word& fieldName,
    /*const*/ autoPtr<fvPatchField<tensor> >& gradPtr
)
{
    ///- Create grad(field) name
    const word& gradFieldName = "grad(" + fieldName + ")";

    ///- If grad(v_new) exists, use it
    if (db().foundObject<volTensorField>(gradFieldName))
    {
        gradPtr.reset
        (
            new fvPatchField<tensor>
            (
                db()
                .lookupObject<volTensorField>(gradFieldName)
                .boundaryField()[patch().index()]
            )
        );
    }
    ///- If grad(v_new) not found, create it
    else
    {
        gradPtr.reset
        (
            new fvPatchField<tensor>
            (
                fvc::grad
                (
                    db()
                    .lookupObject<volVectorField>(fieldName)
                )()
                .boundaryField()[patch().index()]
            )
        );
    }
}
       
void tractionVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchVectorField::write(os);
    traction_.writeEntry("traction", os);
    pressure_.writeEntry("pressure", os);
    convergenceFactors_.writeEntry("convergenceFactors", os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField(fvPatchVectorField, tractionVelocityFvPatchVectorField);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
