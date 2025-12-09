Info<< "Reading field p\n" << endl;
volScalarField z
(
    IOobject
    (
        "z",
        runTime.name(),
        mesh,
        IOobject::MUST_READ,
        IOobject::AUTO_WRITE
    ),
    mesh
);
