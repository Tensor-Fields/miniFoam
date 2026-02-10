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
    Foam::displacementNeumannFvPatchField

Description
    Foam::displacementNeumannFvPatchField

    Originally developed for the problem of upsetting a cylindrical billet by
    fixed strain-rate, while the displacementNeumann imposed on this boundary behaves
    exactly like a flat, frictionless contact.

    Having said that, it is different from contact, in that, contact understands
    when two patches that were previously separate, now "touch" each other. Here
    though, there is no two patches; we calculate displacementNeumann for this very
    patch, and this displacementNeumann can behave like a "frictionless contact that
    alwasy remains in touch with this very patch, and does not touch anything
    else by the evolution of the simulation."

Roadmap
    - Tangentail displacementNeumann can easily be manipulated to be affected by
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
    displacementNeumannFvPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef displacementNeumannFvPatchField_H
#define displacementNeumannFvPatchField_H

#include "fixedGradientFvPatchField.H"
#include "volMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                  Class displacementNeumannFvPatchField Declaration
\*---------------------------------------------------------------------------*/

class displacementNeumannFvPatchField
:
    public fixedGradientFvPatchField<vector>
{

private: 

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

    /// Total displacementNeumann, in case of axially-fixed strain rate, specified by
    /// the user
    vector    totalDisp_;

    /// Object's memory to remember how much it is displaced by far
    /// Also necessary for correct restart.
    vectorField    accumulatedDisp_;

    /// Store accumulated displacementNeumann only when a displacementNeumann field does not
    /// exist in the objectRegistry.
    bool    storeAccumulatedDisp_;// = false;

    word    displacementFieldName_;

    word    displacementIncrementFieldName_;

    bool    frictionless_;

    bool    correctNonOrthogonal_;

    //vector  dirichletDir_;

    Field<vector> userBoundaryTraction_;

    word muName_;

    //autoPtr<scalarField> muPtr_;
    scalarField* muPtr_;

    word lambdaName_;

    //autoPtr<scalarField> lambdaPtr_;
    scalarField* lambdaPtr_;

    word fieldName_;// = this->dimensionedInternalField().name();

    /// Scenarios
    bool pureDisplacement_;
    bool pureTraction_;
    bool normalDisplacementTangentialTraction_;
    bool normalTractionTangentialDisplacement_;

    /// Scenario parameters
    //Field<vector> weights_;

    /// Vector weight
    vector w1_;

    /// Scalar weights
    scalar w0_;
    scalar w2_;
    scalar w3_;
    scalar w4_;

    /// Seems like it is always 1 for Dirichlet
    //scalar boundaryValueWeight_;

    /// Explicit snGrad formula weights
    scalar JWWeight_;

    scalar HWeight_;

    /// Helper class
    /// Creates scenario objects to produce correct weights and projection
    /// vectors
    class scenario
    {
        private:
            /// Pointer to the boundary condition
            /*const*/ displacementNeumannFvPatchField* bcPtr_;
            /// Unit vector of Psi_boundary
            vectorField unitPsi_;
            /// First projection of Psi_boundary
            vectorField N_;
            /// Second projection of Psi_boundary
            vectorField T_;

            /// Weights for different terms
            scalar w0_, w2_, w3_, w4_ = 0;

            /// Scenarios
            const bool pd_, pt_, ndtt_, nttd_;
    
            /// Private members
            void normalize(Field<scalar>);
            void print() const;
    
        public:
            /// Constructors --- Able to edit the boundary condition

            /// Note: `this' cannot be passed by reference
            scenario
            (
                /*const*/ displacementNeumannFvPatchField* /*&*/,
                bool init = true
            );
            scenario();
    
            /// Member functions
            //void allocateScenario(const dictionary&);
            void ifScenariosConflict() const;
            void init(/*const*/ displacementNeumannFvPatchField* bcPtr = 0);

            /// Access members
            const vectorField& N() const;
            const vectorField& T() const;

            inline scalar w0() const
            {
                return w0_;
            }

            inline scalar w2() const
            {
                return w2_;
            }

            inline scalar w3() const
            {
                return w3_;
            }

            inline scalar w4() const
            {
                return w4_;
            }
    };

    /*------------------------------------------------------------------------*\
                                    DANGER ZONE
    \*------------------------------------------------------------------------*/
    /// Things get initialized In the Order of Declaration,
    /// Not the Order Written in the .C File!

    /// If, e.g., pureDisplacement_ comes after scenario_ here, then your code
    /// is Wrong!
    /*------------------------------------------------------------------------*/
    scenario scenario_;
    /*------------------------------------------------------------------------*/

public:

    //- Runtime type information
    TypeName("displacementNeumann");

    // Constructors

        //- Construct from patch and internal field

        //  Also required by runtime-selection mechanism
        displacementNeumannFvPatchField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        displacementNeumannFvPatchField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping the given displacementNeumannFvPatchField<Type>
        //  onto a new patch
        
        //  Also required by runtime-selection mechanism
        displacementNeumannFvPatchField
        (
            const displacementNeumannFvPatchField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        displacementNeumannFvPatchField
        (
            const displacementNeumannFvPatchField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<vector> > clone() const
        {
            //Info<< "    Constructor clone()\n";
            return tmp<fvPatchField<vector> >
            (
                new displacementNeumannFvPatchField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        displacementNeumannFvPatchField
        (
            const displacementNeumannFvPatchField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<vector> > clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            //Info<< "    Constructor clone(iF)\n";
            return tmp<fvPatchField<vector> >
            (
                new displacementNeumannFvPatchField(*this, iF)
            );
        }


    // Destructor
    ~displacementNeumannFvPatchField();

    // Member functions

        //- Maps
        virtual void autoMap
        (
            const fvPatchFieldMapper& m
        );

        virtual void rmap
        (
            const fvPatchField<vector>& ptf,
            const labelList& addr
        );

        //- System assembly data
        virtual tmp<Field<vector> > valueInternalCoeffs
        (
            const tmp<scalarField>&
        ) const;

        virtual tmp<Field<vector> > valueBoundaryCoeffs
        (
            const tmp<scalarField>&
        ) const;

        /// Implicit contribution to the system

        /// This needs weights
        virtual tmp<Field<vector> > gradientInternalCoeffs()  const;

        virtual tmp<Field<vector> > internalCoeffInGradEval() const;

        /// Explicit contribution to the system

        /// This needs weights
        virtual tmp<Field<vector> > gradientBoundaryCoeffs()  const;

        virtual tmp<Field<vector> > explicitTermsInGradEval() const;

        virtual tmp<Field<vector> > snGrad() const;

        //- Evaluate
        virtual void evaluate
        (
            const Pstream::commsTypes commsType = Pstream::blocking
        );

        //- Calc weights to choose pure Dirichlet, pure Neumann, and in the
        //  latter case, also to average snGrad obtained by JW and H methods.
        //virtual void calcWeights();

        /// Scale a vector v component-wise by another vector s
        virtual vectorField scale
        (
            const vector/*Field*/& s,
            const vectorField& v
        ) const;

        virtual vectorField scale
        (
            const vector/*Field*/& s,
            const tmp<vectorField>& v
        ) const;

        //- A vector, where 1 means the BC in the direction corresponding to
        //  that component is pure Neumann
        //virtual vector neumannDir() const;

        //- Update coeffs
        //virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;

        //- User-defined displacementNeumann calculation
        //virtual void calc();

        //- User-defined displacementNeumann constraints
        virtual void constrain();

        //- Read displacementNeumann from objectRegistry or evaluate it
        virtual void read();

        //- This boundary Field
        virtual Field<vector> fcv() const;
        
        //- Traction at cell center, based on the face normal
        virtual Field<vector> explicitInternalTraction() const;

        //- Traction at face center, based on the available stress at face
        virtual Field<vector> explicitBoundaryTraction() const;

        virtual Field<vector> correctNonOrthogonal() const;

        /// Flavours of transformation, applied to boundary traction, producing
        /// snGrad
        virtual Field<tensor> H() const;

        virtual Field<vector> H1() const;

        /// Control implicitness (so-called Mask)
        //virtual vectorField Lambda() const;

        virtual void userDisplacement();

        virtual Field<vector>
        explicitTangentialDisplacement(const Field<tensor>&) const;

        //- Grad of the field at cell centers
        virtual Field<vector> internalDisplacement() const;

        //- Grad of the field at cell centers
        virtual Field<tensor> internalGrad() const;

        //- Same as above; temporary, to avoid editting a number of instances
        //virtual Field<tensor> explicitGrad() const;

        //- Non-orthogonality vectors
        virtual Field<vector> k() const;

        //- Must be called in all constructors.
        //  Includes all necessary initializations
        void fullyConstruct();

        /// Conjugate normal field; if normal field is face normals, it gives
        /// the field of tangential vectors.
        Field<vector>& conjugateN(const vectorField&) const;
        
        //- Init weights from a single tensorial input
        //void initWeights();

        //- Normalize weights
        //void normalizeWeights();

        ///
        /// Template functions
        ///

        /// Read 'key' from the database
        /// Reads GeometricField<Type>, pours its boundary-value to Field<Type>
        template<class Type>
        Field<Type> readBoundaryValueFromDb
        (const word& key/*, Field<Type>& var, bool error*/) const
        {
            if
            (
                this->db().foundObject
                    <GeometricField<Type, fvPatchField, volMesh> >(key)
            )
            {
                return this->db().lookupObject
                    <GeometricField<Type, fvPatchField, volMesh> >(key)
                        .boundaryField()[patch().index()];
            }

            FatalErrorIn("readBoundaryValueFromDb()")
                << "Object "
                << key
                << " not found in the objectRegistry.\n"
                << abort(FatalError);
        }

        /// Template cannot be virtual
        template <class Type>
        void material
        (
            const word& propertyName,
            //autoPtr<Field<Type> >& propertyPtr
            Field<Type>*& propertyPtr
        ) const
        {
            /// Every time step, a boundary condition object is created, where a clone
            /// constructor is used.
            /// The new object need to force reading mechanical properties, because they
            /// are not constant with time in general.
        
            /// BAD
            /// A bad approach keeps re-reading every iteration, instead of
            /// every time step
        
            /// GOOD
            /// Set autoPtr on object construction, clear on destruction, read (not
            /// from the database, but from the autoPtr) in between.
        
            /// This `if' was designed to avoid property creation every
            /// iteration. However, this goal is now accomplished by calling
            /// material() in the constructor ONLY.
            //if (propertyPtr.empty())
            //if (!propertyPtr)
            //{
                /// Read GeometricField<scalar>, evaluate Field<scalar> at this patch
                //propertyPtr.reset
                //(
                //    new Field<Type>
                //    (
                //        this->readBoundaryValueFromDb<Type>
                //            (propertyName/*, true*/)
                //    )
                //);

                propertyPtr = 
                    new Field<Type>
                    (
                        this->readBoundaryValueFromDb<Type>
                            (propertyName/*, true*/)
                    );

                //if (propertyPtr.empty()) 
                //{
                //    FatalErrorIn("material()")
                //        << "autoPtr to "
                //        << propertyName
                //        << " was not allocated."
                //        << abort(FatalError);
                //}
            //}
            //else
            //{
            //    WarningIn("material()")
            //        << propertyName
            //        << " is already set and its size is "
            //        << propertyPtr->size()
            //        << endl;
            //}
        }
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//#ifdef NoRepository
//# include "displacementNeumannFvPatchFieldTemplates.C"
//#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
