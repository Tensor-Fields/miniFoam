argList::validOptions.insert("M", "Number of meshes");
argList::validOptions.insert("alpha", "Amplitude");
argList::validOptions.insert("beta", "0<..<1");
argList::validOptions.insert("nY", "N cells in Y");
argList::validOptions.insert("motionDirs", "1/0, dirs to be moved");
argList::validOptions.insert("fixedPatchNames", "Names");
argList::validOptions.insert("notFixedPatchNames", "Names");
argList::validOptions.insert("writeIntermediateMeshes", "");
argList::validOptions.insert("fixedTimeStep", "");
argList::validOptions.insert("cyclic", "");
argList::validOptions.insert("sinCycle", "");
argList::validOptions.insert("random", "");
argList::validOptions.insert("seed", "<number>");
argList::validOptions.insert("fixSeed", "");
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
const label M = args.optionFound("M")
    ? readScalar(IStringStream(args.options()["M"])())
    : 1;

const scalar alpha = args.optionFound("alpha")
    ? readScalar(IStringStream(args.options()["alpha"])())
    : 1;

const scalar beta = args.optionFound("beta")
    ? readScalar(IStringStream(args.options()["beta"])())
    : 1;

const label nY = args.optionFound("nY")
    ? readScalar(IStringStream(args.options()["nY"])())
    : 1;

/// True/false must be 1/0 for now
const List<string> dirIsMoving =
    args.optionFound("motionDirs")
    ? args.optionReadList<string>(word("motionDirs"))
    : List<string>(0);

/// List of patches whose points are fixed
const List<string> fixedPatchNames =
    args.optionFound("fixedPatchNames")
    ? args.optionReadList<string>(word("fixedPatchNames"))
    : List<string>(0);

/// List of patches whose points are allowed to move
const List<string> notFixedPatchNames =
    args.optionFound("notFixedPatchNames")
    ? args.optionReadList<string>(word("notFixedPatchNames"))
    : List<string>(0);

/// No direction is random in the initializer
List<bool> isMoving(3, false);

if (dirIsMoving.size() == 3)
{
    forAll(isMoving, dirI)
    {
        /// True/false must be 1/0 for now
        if ( dirIsMoving[dirI] == "1" ) isMoving[dirI] = true;
    }
}
else
{
    FatalErrorIn("parseArgs.H")
        << "Motion dirs (-motionDirs) is not specified."
        << endl;
}

List<string> illegalToAdvect(0);

if (args.optionFound("illegalToAdvect"))
{
    illegalToAdvect = args.optionReadList<string>("illegalToAdvect");
}

/// Seed the random generator for the first cycle
scalar seed0 = args.optionFound("seed")
    ? readScalar(IStringStream(args.options()["seed"])())
    : 0;

bool fixSeed = args.optionFound("fixSeed");
