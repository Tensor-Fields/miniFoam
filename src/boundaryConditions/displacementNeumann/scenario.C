#include "displacementNeumannFvPatchField.H"
#include "vector.H"
#include "scalar.H"
#include "Field.H"
#include "fvc.H"

/// From bc source to avoid compile errors
#include "surfaceMesh.H"
#include "fvsPatchField.H"
#include "fvMatrices.H"

namespace Foam
{
typedef Field<vector> vectorField;

/// Constructor
displacementNeumannFvPatchField::scenario::scenario()
:
bcPtr_(0),
pd_(0),
pt_(0),
ndtt_(0),
nttd_(0)
{}

displacementNeumannFvPatchField::scenario::scenario
(
    /// Boundary condition Pointer
    /*const*/ displacementNeumannFvPatchField* /*&*/ bcPtr,
    bool init
)
:
/// Does `this' refer to the updated value every iteration?
/// Boundary condition
bcPtr_(bcPtr),
pd_(bcPtr->pureDisplacement_),
pt_(bcPtr->pureTraction_),
ndtt_(bcPtr->normalDisplacementTangentialTraction_),
nttd_(bcPtr->normalTractionTangentialDisplacement_)
{
    if (init)
    {
        this->init();
    }
}

void displacementNeumannFvPatchField::scenario::ifScenariosConflict() const
{
    const bool& pd = pd_;
    const bool& pt = pt_;
    const bool& ndtt = ndtt_;
    const bool& nttd = nttd_;

    /// Only one scenario must be true
    if
    (
        (pd && pt    )
     || (pd && ndtt  )
     || (pd && nttd  )
     || (pt && ndtt  )
     || (pt && nttd  )
     || (ndtt && nttd)
    )
    {
        FatalErrorIn("ifScenariosConflict()")
            << "    "
            << "More than one scenario is true;\n"
            << "    "
            << "unable to choose which one.\n"
            << "    "
            << "Make sure only one of the following is true:\n\n"
            << "    "
            << "pureDisplacement = "
            << pd_ << "\n"
            << "    "
            << "pureTraction = "
            << pt_ << "\n"
            << "    "
            << "normalDisplacementTangentialTraction = "
            << ndtt_ << "\n"
            << "    "
            << "normalTractionTangentialDisplacement = "
            << nttd_ << "\n"
            << abort(FatalError);
    }
    /// Or if all are false
    if
    (
         (!pd)
      && (!pt)
      && (!ndtt)
      && (!nttd)
    )
    {
        FatalErrorIn("ifScenariosConflict()")
            << "    "
            << "None of the scenarios are true;\n"
            << "    "
            << "unable to choose which one.\n"
            << "    "
            << "Make sure only one of the following is true:\n\n"
            << "    "
            << "pureDisplacement = "
            << pd_ << "\n"
            << "    "
            << "pureTraction = "
            << pt_ << "\n"
            << "    "
            << "normalDisplacementTangentialTraction = "
            << ndtt_ << "\n"
            << "    "
            << "normalTractionTangentialDisplacement = "
            << nttd_ << "\n"
            << abort(FatalError);
    }
}

void displacementNeumannFvPatchField::scenario::print() const
{
    Info<< "    "
        << "pureDisplacement = "
        << pd_ << "\n"
        << "    "
        << "pureTraction = "
        << pt_ << "\n"
        << "    "
        << "normalDisplacementTangentialTraction = "
        << ndtt_ << "\n"
        << "    "
        << "normalTractionTangentialDisplacement = "
        << nttd_ << "\n";
}

void displacementNeumannFvPatchField::scenario::init
(
    /*const*/ displacementNeumannFvPatchField* constBcPtr
)
{
    if(constBcPtr)
    {
        /// Overwrite data
        /*const_cast<displacementNeumannFvPatchField*>(*/bcPtr_/*)*/ = constBcPtr;
        const_cast<bool&>(pd_) = constBcPtr->pureDisplacement_;
        const_cast<bool&>(pt_) = constBcPtr->pureTraction_;
        const_cast<bool&>(nttd_) = constBcPtr->normalTractionTangentialDisplacement_;
        const_cast<bool&>(ndtt_) = constBcPtr->normalDisplacementTangentialTraction_;
    }

    this->ifScenariosConflict();

    /// Boundary values
    const vectorField& bv = *bcPtr_;
    unitPsi_ = *bcPtr_;

    /// Init unitPsi_
    forAll(bv, faceI)
    {
        if (mag(bv[faceI]) > SMALL)
        {
            unitPsi_[faceI] = (*bcPtr_)[faceI] / ((mag(*bcPtr_)())[faceI]);
        }
    }

    /// Test: Set it to be vector::one
    //unitPsi_ = vectorField(unitPsi_.size(), vector::one);

    if (/*pureDisplacement*/pd_)
    {
        N_ = bcPtr_->patch().nf();
        //T_ = unitPsi_ - N_;

        //T_ = (*bcPtr_) - ((*bcPtr_) & N_) * N_;

        /// Normalize T_
        //forAll(T_, faceI)
        //{
        //    if(mag(T_[faceI]) > SMALL)
        //    {
        //        T_[faceI] /= mag(T_[faceI]);
        //    }
        //    else
        //    {
        //        T_[faceI] = vector::zero;
        //    }
        //}

        w0_ = 1;
        w2_ = 1;
        w3_ = 0;
        w4_ = 0;
    }
    else if (/*pureTraction*/pt_)
    {
        vectorField* NPtr = new vectorField
            (
                bcPtr_->patch().size(),
                vector::zero
            );

        N_ = *NPtr;

        T_ = unitPsi_;

        w0_ = 0; /* Ineffective; set it though */
        //w2_ = displacementNeumannFvPatchField::w2_;
        /// This equality is mandatory;
        /// Should we read w4_ or w2_ from the user input at all?
        //w4_ = displacementNeumannFvPatchField::w4_;
        //w3_ = displacementNeumannFvPatchField::w3_;

        /// w_'s are known in this scope

        if (bcPtr_->JWWeight_)
        {
            w2_ = bcPtr_->JWWeight_;
        }
        else
        {
            w2_ = 0;
        }

        if (bcPtr_->HWeight_)
        {
            w3_ = bcPtr_->HWeight_;
        }
        else
        {
            w3_ = 0;
        }

        /// Take one of w4_ or w2_,
        /// normalize to get w2_ + w3_ = 1
        Field<scalar> v(2);
        v[0] = w2_;
        v[1] = w3_;

        this->normalize(v);

        w2_ = v[0];
        w3_ = v[1];

        w4_ = w2_;
    }
    else if (/*normalDisplacementTangentialTraction*/ndtt_)
    {
        N_ = bcPtr_->patch().nf();

        /// Get the size correct
        T_ = N_;

        forAll(T_, faceI)
        {
            T_[faceI][0] = 0;
            T_[faceI][1] = 1;
            T_[faceI][2] = 0;
        }

        //T_ = unitPsi_ - N_;

        //T_ = (*bcPtr_) - ((*bcPtr_) & N_) * N_;

        /// Explicit boundary term
        //vectorField boundaryTerm = bcPtr_->snGrad();
        //T_ = boundaryTerm - (boundaryTerm & N_) * N_;

        /// Normalize T_
        //forAll(T_, faceI)
        //{
        //    if(mag(T_[faceI]) > SMALL)
        //    {
        //        T_[faceI] /= mag(T_[faceI]);
        //    }
        //    else
        //    {
        //        T_[faceI] = vector::zero;
        //    }
        //}

        /// w_'s are known in this scope

        w0_ = 1;

        if (bcPtr_->JWWeight_)
        {
            w2_ = bcPtr_->JWWeight_;
        }
        else
        {
            w2_ = 0;
        }

        if (bcPtr_->HWeight_)
        {
            w3_ = bcPtr_->HWeight_;
        }
        else
        {
            w3_ = 0;
        }

        /// Take one of w4_ or w2_,
        /// normalize to get w2_ + w3_ = 1
        Field<scalar> v(2);
        v[0] = w2_;
        v[1] = w3_;

        this->normalize(v);

        w2_ = v[0];
        w3_ = v[1];

        w4_ = w2_;
    }
    else if (/*bcPtr_->normalTractionTangentialDisplacement_*/nttd_)
    {
        notImplemented("scenario()")

        Info<< "normalTractionTangentialDisplacement"
            << "\nis not implemented.\n"
            << abort(FatalError);
    }
}

/// Normalize
/// Used to normalize weights.
void displacementNeumannFvPatchField::scenario::normalize(Field<scalar> weights)
{
    scalar s(0);

    forAll(weights, wI)
    {
        if (weights[wI] < 0)
        {
            FatalErrorIn("normalize()")
                << "    Negative weight spotted:\n"
                << "    "
                << weights[wI]
                << abort(FatalError);
        }

        s += weights[wI];
    }

    if (mag(s) > SMALL)
    {
        weights /= s;
    }
    /// else, weights are zero.
}

const vectorField& displacementNeumannFvPatchField::scenario::N() const
{
    /// Force re-creation to make sure boundary values are
    /// up-to-date
    
    /// Not possible, unless through casting
    //this->scenario();

    const vectorField& Nr = N_;
    return Nr;
}

const vectorField& displacementNeumannFvPatchField::scenario::T() const
{
    /// Force re-creation to make sure boundary values are
    /// up-to-date

    /// Not possible, unless through casting
    //this->scenario();

    const vectorField& Tr = T_;
    return Tr;
}

/// Select scenario
//void displacementNeumannFvPatchField::scenario::allocateScenario(const dictionary& dict)
//{
//    if (!dict.readIfPresent<bool>(word("pureDisplacement"), bcPtr_->pureDisplacement_))
//    {
//        bcPtr_->pureDisplacement_ = false;
//
//        Info<< "    "
//            << "pureDisplacement not found, set to `false'\n";
//    }
//
//    if (!dict.readIfPresent<bool>(word("pureTraction"), bcPtr_->pureTraction_))
//    {
//        bcPtr_->pureTraction_ = false;
//
//        Info<< "    "
//            << "pureTraction not found, set to `false'\n";
//    }
//
//    if
//    (
//        !dict.readIfPresent<bool>
//        (
//            word("normalDisplacementTangentialTraction"),
//            bcPtr_->normalDisplacementTangentialTraction_
//        )
//    )
//    {
//        bcPtr_->normalDisplacementTangentialTraction_ = false;
//
//        Info<< "    "
//            << "normalDisplacementTangentialTraction not found, set to"
//            << " `false'\n";
//    }
//
//    if
//    (
//        !dict.readIfPresent<bool>
//        (
//            word("normalTractionTangentialDisplacement"),
//            bcPtr_->normalTractionTangentialDisplacement_
//        )
//    )
//    {
//        normalTractionTangentialDisplacement_ = false;
//
//        Info<< "    "
//            << "normalTractionTangentialDisplacement not found, set to"
//            << " `false'\n";
//    }
//}
}
