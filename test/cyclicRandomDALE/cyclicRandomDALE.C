/**
  * A driver for random, cyclic mesh motion + advective remap.
  *
  * Usage example:
  * cyclicRandomDALE \
  *     -M 2 \
  *     -alpha .125 \
  *     -nY 2 \
  *     -randomDirs \( "1" "1" "0" \) \
  *     -writeIntermediateMeshes \
  *     -fixedTimeStep
  *
  * where two meshes will be created, one random and one final mesh which
  * matches the initial mesh. 'nY' means that there are only 2 cells in Y 
  * direction.
  * More on alpha and nY in meshDance lib.
  *
  * \note: If `fixedTimeStep` is not specified, mesh motion will be
  * Courant-number-aware, and one needs to provide the relevant info in
  * controlDict, i.e., 
  * - maxCo (default to 1)
  * - adjustTimeStep (default to false)
  * - maxDeltaT (default to GREAT)
  *
  * \note: stepTowards() is overshoot-proof, but not undershoot-proof.
  * We've not checked if logically it is possible for undershoot to occur, but,
  * if at the last stepTowards(), at which mesh points are supposed to be at the
  * prescribed final points, some/all points fall behind those target points,
  * the class meshDance have no control over them as of now.
  * 
  * \author: Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin,
  * \date: Started at May 15 2025
  */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "IOobject.H"
#include "GeometricField.H"
#include "vector.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "pointField.H"
#include "scalar.H"
#include "faceList.H"
#include "labelList.H"
#include "Xfer.H"
#include "pointPatchField.H"
#include "pointMesh.H"
#include "dimensionedVector.H"
#include "dimensionSets.H"
#include "fvc.H"
#include "fvm.H"
#include "IOobjectList.H"
#include "meshDance.H"
//#include "pointGenerator.H"
#include "meshWire.H"

using namespace Foam;

int main(int argc, char* argv[])
{
    argList::validOptions.insert("M", "Number of meshes");
    argList::validOptions.insert("alpha", "Amplitude");
    argList::validOptions.insert("nY", "N cells in Y");
    argList::validOptions.insert("randomDirs", "1/0, dirs to be randomized");
    argList::validOptions.insert("notFixedPatchNames", "Names");
    argList::validOptions.insert("writeIntermediateMeshes", "");
    argList::validOptions.insert("fixedTimeStep", "");
    argList::validOptions.insert("seed", "<number>");

    argList args(argc, argv);

    /// Number of intermediate random meshes is M-1
    /// The Mth deformed mesh is the first mesh, so cyclic
    const label M = readScalar(IStringStream(args.options()["M"])());
    const scalar alpha = readScalar(IStringStream(args.options()["alpha"])());
    const label nY = readScalar(IStringStream(args.options()["nY"])());

    /// True/false must be 1/0 for now
    const List<string> dirIsRandom = args.optionReadList<string>
    (word("randomDirs"));

    /// List of patches whose points are allowed to move
    const List<string> notFixedPatchNames = args.optionReadList<string>
    (word("notFixedPatchNames"));

    /// Write intermediate meshes during advection of every random step, 1/0
    const bool writeIntermediateMeshes = args.optionFound
    ("writeIntermediateMeshes");

    /// No direction is random in the initializer
    List<bool> isRandom(3, false);

    forAll(isRandom, dirI)
    {
        /// True/false must be 1/0 for now
        if ( dirIsRandom[dirI] == "1" ) isRandom[dirI] = true;
    }

    /// Create time
    Time runTime("controlDict", args.rootPath(), args.caseName());

    #include "createTimeControls.H"
    
    /// Create the dancing mesh
    meshDance    myMesh(runTime);

    /// Access the underlying fvMesh
    const fvMesh& mesh = myMesh.mesh();

    /// Copy a frozen version of points; because myMesh.points() keeps
    /// changing
    const pointField frozenInitialPoints = myMesh.points();

    ///
    /// Decide whether points on patches move or not
    ///
    const pointMesh& pMesh = pointMesh(mesh);

    labelList fixedPatches
    (pMesh.boundary().size() - notFixedPatchNames.size(), -1);

    label fixedPatchI = 0;

    forAll(pMesh.boundary(), pPatchI)
    {
        forAll(notFixedPatchNames, patchI)
        {
            if (pMesh.boundary()[pPatchI].name() == notFixedPatchNames[patchI])
            {
                break;
            }

            fixedPatches[fixedPatchI] = pPatchI;

            ++fixedPatchI;
        }
    }

    /// Initialize isPatch point list
    boolList isPatch(mesh.points().size(), false);

    /// Mark patch points (those who are meant to be fixed only)
    forAll(fixedPatches, patchI)
    {
        const labelList& pp = pMesh.boundary()[fixedPatches[patchI]]
        .meshPoints();

        forAll(pp, ppI)
        {
            isPatch[pp[ppI]] = true;
        }
    }

    /// Loop over random meshes
    for (label meshi = 0; meshi < M; ++meshi)
    {
        /// "mesh 0" is the init mesh
        Info<< "\nCycle "
            << meshi+1
            << " out of "
            << M
            << endl;

        /// Create the random point generator, based on the frozen points
        //pointGenerator pGen(frozenInitialPoints, alpha/nY, isRandom, isPatch);
        //meshWire pGen(frozenInitialPoints, alpha/nY, isRandom, isPatch, mesh);
        meshWire pGen
	(
	    frozenInitialPoints,
	    alpha/nY,
	    isRandom,
	    isPatch,
	    readScalar(IStringStream(args.options()["seed"])()),
	    mesh
	);

        //Info<< "meshWire point generator created"
        //    << endl;

        /// If !last_mesh
        /// Create next random mesh
        if (meshi < M-1)
        {
            ///
            /// Final random points of the mesh at the step meshi
            /// Avoid re-calculation by taking it outside the fictitious-time
            /// loop
            ///

            /// 1000 is some arbitrary number, used for seed generation
            //const pointField& rp = pGen.randomize(1000 * meshi);
            const pointField& rp = pGen.randomize();

            Info<< "Random mesh points generated"
                << endl;

            /// Set overall point motion
            myMesh.setOverallPointMotion(myMesh.updatedStartingPoints(), rp);

            ///
            /// Fictitious time loop for advective remap
            ///

            while (runTime.run())
            {
                /// If time step is NOT fixed
                if (!args.optionFound("fixedTimeStep"))
                {
                    #include "readTimeControls.H"

                    /// Set time step
                    /// Uses delta, not cell edges, to calculate meshCo.
                    #include "meshCourantNo.H"

                    /// setDeltaT.H needs CoNum
                    scalar& CoNum = meshCoNum;

                    /// Re-set time step, if requested
                    /// Place it inside time loop for online interaction with Co settings
                    if (adjustTimeStep)
                    {
                        #include "setDeltaT.H"
                    }
                }

                /// March time
                runTime++;

                Info<< "    Time: "
                    << runTime.value()
                    << endl;

                /// Move the mesh incrementally
                myMesh.stepTowards();

                /// Advect fields
                myMesh.advect<scalar>();

                /// Write intermediate mesh
                if (writeIntermediateMeshes)
                {
                    if (runTime.write())
                    {
                        myMesh.write();
                    }
                }
            }

            ///
            /// Write final mesh and fields of each random step
            ///

            /// Mesh and fields will be written to caseDir/meshi/
            /// Duplicate for the last loop iter
            myMesh.write();

            Info<< "Cycle "
                << meshi+1
                << " written to "
                << runTime.timeName()
                << endl;

            ///
            /// Reset settings for the next random mesh motion step
            /// or the last step
            ///

            /// Re-set startTime
            //myMesh.setStartTime(runTime.endTime());

            /// Re-set starting points
            //myMesh.resetStartingPoints(myMesh.points());

            /// Re-set mesh motion settings at the end of each cycle
	    /// - startTime
	    /// - starting points
	    /// - sumMotionFraction
	    /// - endTime
	    ///
	    myMesh.resetMotion();
 
            //runTime.setEndTime(runTime.endTime() + timeSpan);
        }
        /// Else
        /// Force last mesh to be the first mesh
        else if (meshi == M-1)
        {
            /// Move mesh back to its undeformed shape
            myMesh.setOverallPointMotion(myMesh.points(), frozenInitialPoints);

            /// Fictitious time loop for advective remap
            while (runTime.run())
            {
                runTime++;

                //Info<< "Time = "
                //    << runTime.timeName()
                //    << endl;

                /// Move the mesh incrementally, new timestep size takes effect.
                myMesh.cyclicStep();

                /// Advect Field
                myMesh.advect<scalar>();

                /// Write intermediate mesh
                if (writeIntermediateMeshes)
                {
                    if (runTime.write())
                    {
                        myMesh.write();
                    }
                }
            }

            ///
            /// Write final mesh and fields of each random step
            ///

            /// Mesh and fields will be written to caseDir/M/
            myMesh.write();

            Info<< "Cycle "
                << meshi + 1
                << " written to "
                << runTime.timeName()
                << endl;
        }
        else
        {
            FatalErrorIn("meshCounter")
                << "meshi is !(< or == or >) M"
                << abort(FatalError);
        }
    }

    Info<< "\n\nEnd."
        << endl;
}
