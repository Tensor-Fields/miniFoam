/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     | Website:  https://openfoam.org
    \\  /    A nd           | Copyright (C) 2011-2024 OpenFOAM Foundation
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

Class
    Foam::tractionVelocityFvPatchVectorField

Description
    Fixed traction boundary condition for the standard linear elastic, fixed
    coefficient displacement equation.

SourceFiles
    tractionVelocityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef tractionVelocityFvPatchVectorField_H
#define tractionVelocityFvPatchVectorField_H

#include "fixedGradientFvPatchFields.H"
//#include "Function1.H"

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

        autoPtr<Field<scalar> > pressure_;


protected:

        //- Update the coefficients associated with the patch field
        //  for the given pressure
        template<class Type>
        void updateCoeffs(const Type& pressure);

        //- Construct from patch and internal field
        tractionVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );


public:

    //- Runtime type information
    TypeName("tractionVelocity");


    // Constructors

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
            const fieldMapper&
        );

        //- Disallow copy without setting internal field reference
        tractionVelocityFvPatchVectorField
        (
            const tractionVelocityFvPatchVectorField&
        ) = delete;

        //- Copy constructor setting internal field reference
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


    // Member Functions

        // Access

            virtual const vectorField& traction() const
            {
                return traction_;
            }

            virtual vectorField& traction()
            {
                return traction_;
            }


        // Mapping functions

            //- Map the given fvPatchField onto this fvPatchField
            virtual void map(const fvPatchVectorField&, const fieldMapper&);

            //- Reset the fvPatchField to the given fvPatchField
            //  Used for mesh to mesh mapping
            virtual void reset(const fvPatchVectorField&);


        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "tractionVelocityFvPatchVectorFieldTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
