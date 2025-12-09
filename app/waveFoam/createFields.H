Info<< "Reading field u\n" << endl;
volScalarField u
(
    IOobject
    (
        "u",
        runTime.name(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);

IOdictionary transportProperties
(
	IOobject
	(
        "transportProperties",
		runTime.constant(),
        mesh,
		IOobject::MUST_READ
	)
);

dimensionedScalar c2
(
    "c2",
	pow(dimLength,2)/pow(dimTime,2),
	transportProperties.lookup<scalar>("c2")
);

