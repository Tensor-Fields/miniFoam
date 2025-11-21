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
    Foam::tractionVelocityFvPatchVectorField

Description
    Fixed traction boundary condition for the standard linear elastic, fixed
    coefficient displacement equation.

Roadmap
    - In updateCoeffs(), in the non-orthogonal variation, gradDold can be
      calculated from traction and pressure at old time.
    - Triple-check where traction and pressure take effect; is the arithmetic
      correct?
    - Add to constructor: Time-varying traction.

SourceFiles
    tractionVelocityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef tractionVelocityFvPatchVectorField_H
#define tractionVelocityFvPatchVectorField_H

#include "fvPatchFields.H"
#include "fixedGradientFvPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                   Class tractionVelocityFvPatch Declaration
\*---------------------------------------------------------------------------*/

class tractionVelocityFvPatchVectorField
:
    public fixedGradientFvPatchVectorField
{

    // Private Data

        vectorField traction_;
        scalarField pressure_;
        List<scalar> convergenceFactors_;
        word var_ = "orthogonal";
        /// Name of the displacement field
        word DName_ = "D";

    // Private Members
        void grad
        (
            const word& fieldName,
            /*const*/ autoPtr<fvPatchField<tensor> >& gradPtr
        );
public:

    //- Runtime type information
    TypeName("tractionVelocity");


    // Constructors

        //- Construct from patch and internal field
        tractionVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        tractionVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  tractionVelocityFvPatchVectorField onto a new patch
        tractionVelocityFvPatchVectorField
        (
            const tractionVelocityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        tractionVelocityFvPatchVectorField
        (
            const tractionVelocityFvPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new tractionVelocityFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        tractionVelocityFvPatchVectorField
        (
            const tractionVelocityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new tractionVelocityFvPatchVectorField(*this, iF)
            );
        }


    // Member functions

        // Access

            virtual const vectorField& traction() const
            {
                return traction_;
            }

            virtual vectorField& traction()
            {
                return traction_;
            }

            virtual const scalarField& pressure() const
            {
                return pressure_;
            }

            virtual  scalarField& pressure()
            {
                return pressure_;
            }


        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const fvPatchFieldMapper&
            );

            //- Reverse map the given fvPatchField onto this fvPatchField
            virtual void rmap
            (
                const fvPatchVectorField&,
                const labelList&
            );


        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
