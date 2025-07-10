// Read the mesh
IOobject meshIO
(
	fvMesh::defaultRegion,
    runTime.timeName(),
	runTime,
	IOobject::MUST_READ,
	IOobject::NO_WRITE
);

//fvMesh mesh(meshIO);

autoPtr<fvMesh> meshPtr
(
    new fvMesh(meshIO)
);

fvMesh& mesh =  meshPtr();
