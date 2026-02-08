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
  *     -seed
  *     -fixedTimeStep
  *
  * where two meshes will be created, one random and one final mesh which
  * matches the initial mesh. 'nY' means that there are only 2 cells in Y 
  * direction.
  * More on alpha and nY in meshDance lib.
  *
  * \note: `seed` is an initial seed for a random generator. In each cycle, a
  * random number will be generated for random point motion.
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
#include "scalar.H"
#include "vector.H"
//#include "tensor.H"
//#include "symmTensor.H"
//#include "sphericalTensor.H"
#include "fvPatchField.H"
#include "volMesh.H"
#include "pointField.H"
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
//#include "Rand.H"

// Apparently needed when advecting fields, possibly for creating solids4foam BC's
//#include "physicsModel.H"

using namespace Foam;

int main(int argc, char* argv[])
{
    argList::validOptions.insert("M", "Number of meshes");
    argList::validOptions.insert("alpha", "Amplitude");
    argList::validOptions.insert("nY", "N cells in Y");
    argList::validOptions.insert("randomDirs", "1/0, dirs to be randomized");
    argList::validOptions.insert("fixedPatchNames", "Names");
    argList::validOptions.insert("notFixedPatchNames", "Names");
    argList::validOptions.insert("writeIntermediateMeshes", "");
    argList::validOptions.insert("fixedTimeStep", "");
    argList::validOptions.insert("seed", "<number>");
    argList::validOptions.insert("cyclic", "");
    argList::validOptions.insert("sinCycle", "");
    argList::validOptions.insert("targetMesh", "<target case>");
    argList::validOptions.insert("maxIter", "<N>");
    argList::validOptions.insert("flipFluxSign", "");
    argList::validOptions.insert("N_iter", "");
    argList::validOptions.insert("phi", "<phase difference for sinCycle>");
    argList::validOptions.insert("illegalToAdvect", "<names>");

    // If you want a physicsModel to be created
    //argList::validOptions.insert("solids4foam", "");

    argList args(argc, argv);

    bool cyclic = args.optionFound("cyclic");

    /// Number of intermediate random meshes is M-1
    /// The Mth deformed mesh is the first mesh, so cyclic
    const label M = readScalar(IStringStream(args.options()["M"])());
    const scalar alpha = readScalar(IStringStream(args.options()["alpha"])());
    const label nY = readScalar(IStringStream(args.options()["nY"])());

    /// True/false must be 1/0 for now
    const List<string> dirIsRandom = args.optionReadList<string>
    (word("randomDirs"));

    /// List of patches whose points are fixed
    const List<string> fixedPatchNames = args.optionReadList<string>
    (word("fixedPatchNames"));

    /// List of patches whose points are allowed to move
    const List<string> notFixedPatchNames = args.optionReadList<string>
    (word("notFixedPatchNames"));

    /// Write intermediate meshes during advection of every random step, 1/0
    //const bool writeIntermediateMeshes = args.optionFound
    //("writeIntermediateMeshes");

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
    
    /// Create a list of fixed patches, needed by the constructor
    // Impossible, we do not know the mesh at this point; pass it as an arg

    /// Create the dancing mesh

    //label N_iter = 0;

    //if (args.optionFound("N_iter"))
    //{
    //    N_iter = (readScalar(IStringStream(args.options()["N_iter"])()));
    //} 

    List<string> illegalToAdvect(0);

    if (args.optionFound("illegalToAdvect"))
    {
        illegalToAdvect = args.optionReadList<string>("illegalToAdvect");
    }

    meshDance    myMesh
    (
        runTime, 
    label(readScalar(IStringStream(args.options()["maxIter"])())),
        fixedPatchNames,
    args.optionFound("flipFluxSign"),
    0,//N_iter // 0 means dynamic calculation
        illegalToAdvect
    );

    /// Access the underlying fvMesh
    const fvMesh& mesh = myMesh.mesh();

    const_cast<fvMesh&>(mesh).setTopoWriteOpt(IOobject::AUTO_WRITE);

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

    /// Seed the random generator for the first cycle
    scalar seed0 = readScalar(IStringStream(args.options()["seed"])());
    Random rand(seed0);

    /// Create the actual random number to feed the first-cycle point generator.
    scalar seed = rand.scalar01();

    //if (args.optionFound("solids4foam"))
    //{
    //    autoPtr<physicsModel> physics = physicsModel::New(runTime);

    //    Info<< "physics created"
    //        << endl;
    //}

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
            mesh
        );

        /// Workaround to create a new random number every time: Re-seed
        /// the Random object
            //Random rand(seed);

        // \note: It was observed that when seed ~ 0.75, scalar01() gets stuck
        // at this number.

        Info<< "seed was "
            <<seed<< endl;

        /// \note: By just '=' the random number stucks
        /// \note: Turned out that small seeds are problematic.
        /// When seed0 is generated by RAND in bash, it is normally > 1000
        seed = seed0 * rand.scalar01();
        //seed += rand.scalar01();

        /// \note: When reaches some number a, produces b, and b, in the next cycle,
        /// produces a, back-and-forth ...
            //seed = rand.GaussNormal();

        Info<< "seed becomes "
            <<seed<< endl;

        //Info<< "meshWire point generator created"
        //    << endl;

        /// If !last_mesh
        /// Create next random mesh
        //if (meshi < M)
        {
            /// Set overall point motion, depending on whether it is the last
        /// cycle or not

            autoPtr<pointField> rp(new pointField(mesh.points()));

				if (meshi < M-1 || (meshi == M-1 && !cyclic))
					{
						///
						/// Final random points of the mesh at the step meshi
						/// Avoid re-calculation by taking it outside the fictitious-time
						/// loop
						///

						/// 1000 is some arbitrary number, used for seed generation
						//const pointField& rp = pGen.randomize(1000 * meshi);

					/// Seed with a random number
					/// \note: scalar01() returned a fixed number given a fixed seed
					/// \note: rand.GaussNormal() returns a fixed number
						//const pointField& rp = pGen.randomize(rand.scalar01());
						//const pointField& rp = pGen.randomize(rand.GaussNormal());

						//const pointField& rp = pGen.randomize(seed);

						bool rpNotSet = true;

				if (args.optionFound("random") && rpNotSet)
						{
							rp.reset(new pointField(pGen.randomize(seed)));

							rpNotSet = false;
						}

				if (args.optionFound("sinCycle") && rpNotSet)
						{
							Info<< "sinCycle mesh motion is selected."
								<< endl;

							if (args.optionFound("phi"))
							{
								if ( M < SMALL)
								{
									FatalErrorIn("")
										<< "M=0 is not acceptable for sinusoidal"
										<< " rezoning"
										<< endl;
								}

											//Info<< "Comment me!\n"
											//    << "scalar((meshi + 1 + SMALL) / (M)) = "
											//    << (meshi + SMALL + 1) / (M + SMALL)
											//    << nl
											//    << scalar(M)
											//    << endl;

								rp.reset
								(
									new pointField
									(
										pGen.sinCycle
										(
											(meshi + 1.0 + SMALL) / (M + SMALL),
											//M + SMALL,
											1,
											readScalar(IStringStream(args.options()["phi"])())
										)
									)
								);
							}
							else
							{
								WarningIn("")
									<< "option '-phi' not specified, defaults to 0"
									<< endl;

								rp.reset
								(
									new pointField
									(
										pGen.sinCycle
										(
											(meshi + 1.0 + SMALL) / (M + SMALL),
											1,
											0
										)
									)
								);
							}

							//myMesh
					//.setOverallPointMotion(myMesh.updatedStartingPoints(), rp());
							
							rpNotSet = false;
						}

				if (args.optionFound("targetMesh") && rpNotSet)
				{
					//const word targetMeshDir = args.options()["targetMesh"];

					const polyMesh targetMesh
					(
						IOobject
					(
						//polyMesh::defaultRegion,
							//args.options()["targetMesh"],
						//targetMeshDir,
							//fileName(args.options()["targetMesh"]+"/constant/polyMesh"),
						"", "",
							args.options()["targetMesh"]+"/constant",
						runTime,
						IOobject::MUST_READ,
						IOobject::NO_WRITE
					)
					);

					rp.reset(new pointField(targetMesh.points()));
				}

						myMesh
				.setOverallPointMotion(myMesh.updatedStartingPoints(), rp());
				}
				else if (meshi == M-1 && cyclic)
					{
						myMesh
				.setOverallPointMotion(myMesh.points(), frozenInitialPoints);
				}
				else
				{
					FatalErrorIn("")
					<< "The combination of cyclic state and mesh configuration number is not"
					<< "\ndefined."
					<< abort(FatalError);
				}

					///
					/// Fictitious time loop for advective remap
					///

					while (runTime.run())
					{
						/// If time step is NOT fixed
						//if (!args.optionFound("fixedTimeStep"))
						{
							#include "readTimeControls.H"

							/// Set time step
							/// Uses delta, not cell edges, to calculate meshCo.
							#include "meshCourantNo.H"

							/// setDeltaT.H needs CoNum
							scalar& CoNum = meshCoNum;

							/// Re-set time step, if requested
							/// Place it inside time loop for online interaction with Co settings
							#include "setDeltaT.H"
						}

						/// March time
						runTime++;

						Info<< "    Time: "
							<< runTime.value()
							<< endl;

						/// Move the mesh incrementally
				/// Break if the end of the cycle is hit.
						if(myMesh.stepTowards())
				{
					Info<< "\n    Final mesh in this cycle is hit; ignore next"
					<< "\n    time steps, advect fields and move to the" 
								<< "\n    next cycle."
					<< endl;

							/// VolFields
							myMesh.advect<scalar>(runTime.write());
							myMesh.advect<vector>(runTime.write());
							myMesh.advect<tensor>(runTime.write());
							myMesh.advect<symmTensor>(runTime.write());

							/// We do not have sphericalTensor in Taylor impact problem
							//myMesh.advect<sphericalTensor>(runTime.write());

							/// SurfFields
							/// advects() does not work for now
							//myMesh.advects<scalar>(runTime.write());
							//myMesh.advects<vector>(runTime.write());
							//myMesh.advects<tensor>(runTime.write());
							//myMesh.advects<symmTensor>(runTime.write());

							if (runTime.write())
							{
								myMesh.writeMesh();

								Info<< "Write polyMesh"
									<< endl;
							}

					break;
				}

						/// Volume fields
						myMesh.advect<scalar>(runTime.write());
						myMesh.advect<vector>(runTime.write());
						myMesh.advect<tensor>(runTime.write());
						myMesh.advect<symmTensor>(runTime.write());

						/// We do not have sphericalTensor in Taylor impact problem
						//myMesh.advect<sphericalTensor>(runTime.write());

						/// Surface fields
						/// advects() does not work for now
						//myMesh.advects<scalar>(runTime.write());
						//myMesh.advects<vector>(runTime.write());
						//myMesh.advects<tensor>(runTime.write());
						//myMesh.advects<symmTensor>(runTime.write());

						/// Write intermediate mesh
						//if (writeIntermediateMeshes)
						//{
						if (runTime.write())
						{
							myMesh.writeMesh();

							Info<< "Write polyMesh"
								<< endl;
						}
						//}
					}

					///
					/// Write final mesh and fields of each random step
					///

					/// Mesh and fields will be written to caseDir/meshi/
					/// Duplicate for the last loop iter
					//myMesh.write();

					Info<< "Cycle "
						<< meshi+1
						<< " written to "
						<< runTime.timeName()
						<< endl;

					/// Re-set mesh motion settings at the end of each cycle
				/// - startTime
				/// - starting points
				/// - sumMotionFraction
				/// - endTime
				///
				myMesh.resetMotion();
        }
    }

    Info<< "\n\nEnd."
        << endl;
}
