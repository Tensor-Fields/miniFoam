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

Class
    Foam::fixedAxialStrainRateFvPatchVectorField

Description
    Adapted from solids4Foam to be compatible with bekaert deformation solver.

    Fixed displacement boundary condition with boundary non-orthogonal
    correction for solid solvers.
    Essentially fixedValue with non-orthogonal correction.
    Optionally time-varying.

SourceFiles
    fixedAxialStrainRateFvPatchVectorField.C

Author
    Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin, IE
        Contribution: This BC
    Philip Cardiff, UCD/UT. All rights reserved.
        Contribution: Base BC (fixedDisplacement)

Date
    25 Dec 2025, it was Christmas today

\*---------------------------------------------------------------------------*/

#ifndef fixedAxialStrainRateFvPatchVectorField_H
#define fixedAxialStrainRateFvPatchVectorField_H

#include "fvPatchFields.H"
#include "fixedValueFvPatchFields.H"
#include "interpolationTable.H"
#include "primitivePatchInterpolation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                 Class fixedAxialStrainRateFvPatch Declaration
\*---------------------------------------------------------------------------*/

class fixedAxialStrainRateFvPatchVectorField
:
    public fixedValueFvPatchVectorField
{
    // Private Data

        //- Constant total displacement field
        vectorField totalDisp_;

        //- Displacement time series
        interpolationTable<vector> dispSeries_;

        //- Patch interpolator to interpolate face values to the points
        mutable autoPtr<primitivePatchInterpolation> interpPtr_;

        vector axialDir_;
        scalar axialStrainRate_;

        /// End position in terms of the position component on the axial dir
        scalar endPosition_;
        vector endVelocity_;

    // Private Functions

        //- Make the patch interpolator
        void makeInterp() const;

        //- Return the patch interpolator
        primitivePatchInterpolation& interp();

protected:

        //- Set the point displacement field boundary condition
        void setPointDisplacement(const vectorField& faceDisp);

public:

    //- Runtime type information
    TypeName("fixedAxialStrainRate");


    // Constructors

        //- Construct from patch and internal field
        fixedAxialStrainRateFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        fixedAxialStrainRateFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given fixedAxialStrainRateFvPatchVectorField
        //  onto a new patch
        fixedAxialStrainRateFvPatchVectorField
        (
            const fixedAxialStrainRateFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        fixedAxialStrainRateFvPatchVectorField
        (
            const fixedAxialStrainRateFvPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new fixedAxialStrainRateFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        fixedAxialStrainRateFvPatchVectorField
        (
            const fixedAxialStrainRateFvPatchVectorField&,
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
                new fixedAxialStrainRateFvPatchVectorField(*this, iF)
            );
        }


    // Destructors

        //- Construct from patch and internal field
        virtual ~fixedAxialStrainRateFvPatchVectorField();


    // Member functions

        // Access functions

        //- Update the coefficients associated with the patch field
            virtual vectorField& totalDisp()
            {
                return totalDisp_;
            }

        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();

            //- Return patch-normal gradient
            //- with non-orthogonal correction regardless of whether snGrad
            //- is corrected or not
            virtual tmp<Field<vector> > snGrad() const;

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<vector> > gradientBoundaryCoeffs() const;

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const fvPatchFieldMapper&
            );

            //- Reverse map the given fvPatchField onto this fvPatchField
            virtual void rmap
            (
                const fvPatchField<vector>&,
                const labelList&
            );

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
