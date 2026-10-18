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

Class
    Foam::displacementFvPatchField

Description
    Foam::displacementFvPatchField

    Originally developed for the problem of upsetting a cylindrical billet by
    fixed strain-rate, while the displacement imposed on this boundary behaves
    exactly like a flat, frictionless contact.

    Having said that, it is different from contact, in that, contact understands
    when two patches that were previously separate, now "touch" each other. Here
    though, there is no two patches; we calculate displacement for this very
    patch, and this displacement can behave like a "frictionless contact that
    alwasy remains in touch with this very patch, and does not touch anything
    else by the evolution of the simulation."

Roadmap
    - Tangentail displacement can easily be manipulated to be affected by
      controlled tangential friction, as friction = normalForce * frictionCoeff.

Usage
    fieldGradName    Optional    <name of the grad of the field, e.g.,
                                 grad(U) for U>
    strainingEndTime Optional    Optioanl end time for straining
    
Author
    Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin, IE
    Used fixedValueFvPatchField as the starting draft.

Date
    Started at 29 Dec 2025

SourceFiles
    displacementFvPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef displacementFvPatchField_H
#define displacementFvPatchField_H

#include "fixedValueFvPatchField.H"
#include "volMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                  Class displacementFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class displacementFvPatchField
:
    public fixedValueFvPatchField<vector>
{

private: 

    word fieldName_ = this->dimensionedInternalField().name();
    
    /// fieldGradName_ to have this default value, if not specified by the user:
    word fieldGradName_;// = word("grad(" + fieldName_ + ")");

    vector    axialDir_;

    autoPtr<vector>    axialDirPtr_;

    /// Displacement field at face-centers (face-center values, fcv_)
    /// Reads the actual values from the objectRegistry if needed.
    Field<vector>    fcv_;

    /// Initial face-center locations
    const Field<vector>    startLocation_;

    /// If the axial direction is, e.g., Z, axiallyFixedStrainRate means
    /// dE_z/dZ = constant, where E_z is the strain-rate in Z direction.
    bool    axiallyFixedStrainRate_;

    autoPtr<scalarField>    axialStrainRatePtr_;

    /// Optionally, the time when straining ends
    scalar    strainingEndTime_;

    /// Total displacement, in case of axially-fixed strain rate, specified by
    /// the user
    vector    totalDisp_;

    /// Object's memory to remember how much it is displaced by far
    /// Also necessary for correct restart.
    vectorField    accumulatedDisp_;

    /// Store accumulated displacement only when a displacement field does not
    /// exist in the objectRegistry.
    bool    storeAccumulatedDisp_ = false;

    word    displacementFieldName_;

    word    displacementIncrementFieldName_;

    bool    frictionless_;

public:

    //- Runtime type information
    TypeName("displacement");

    // Constructors

        //- Construct from patch and internal field

        //  Also required by runtime-selection mechanism
        displacementFvPatchField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        displacementFvPatchField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping the given displacementFvPatchField<Type>
        //  onto a new patch
        
        //  Also required by runtime-selection mechanism
        displacementFvPatchField
        (
            const displacementFvPatchField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        displacementFvPatchField
        (
            const displacementFvPatchField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<vector> > clone() const
        {
            Info<< "    Constructor clone()\n";
            return tmp<fvPatchField<vector> >
            (
                new displacementFvPatchField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        displacementFvPatchField
        (
            const displacementFvPatchField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<vector> > clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            Info<< "    Constructor clone(iF)\n";
            return tmp<fvPatchField<vector> >
            (
                new displacementFvPatchField(*this, iF)
            );
        }


    // Member functions

        //- Update coeffs
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;

        //- User-defined displacement calculation
        virtual void calc();

        //- User-defined displacement constraints
        virtual void constrain();

        //- Read displacement from objectRegistry or evaluate it
        virtual void read();
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//#ifdef NoRepository
//#   include "displacementFvPatchField.C"
//#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
