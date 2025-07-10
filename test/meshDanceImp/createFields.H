//
// Read material velocity field and mesh velocity field
// and the scalar field
//

// Read material velocity
volVectorField U
(
    IOobject
	(
	    "U",
		runTime.timeName(),
		mesh,
		IOobject::READ_IF_PRESENT
	),
	mesh,
	dimensionedVector("0", dimLength/dimTime, vector::zero)
);

//#include "createPhi.H";

Info<< "Reading/calculating face flux field phi\n" << endl;

surfaceScalarField phi
(
    IOobject
    (
        "phi",
        runTime.timeName(),
        mesh,
        IOobject::READ_IF_PRESENT,
        IOobject::AUTO_WRITE
    ),
    linearInterpolate(U) & mesh.Sf()
);

// Read the existing scalar field
Info << "Read the existing fields" << endl;

// Create boundaries to be added to the newMesh
Info << "Init list of patch pointers" << endl;

//scalar maxIter(10);
