/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  Tensor         | miniFOAM: The Computational Continuum Playground
   ~~~~~~~   Fields         | Website:  https://tensorfields.com
    O   O                   | Copyright (C) 2023-2026 TensorFields
      O                     |
-------------------------------------------------------------------------------

Application
    mag

Description
    Calculate magnitude of a vector field.

    Writes the results, controlled by controlDict

Author
    Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin

Date
    11 Nov 2025
\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "foamTime.H"

#include "fvMesh.H"
#include "volMesh.H"
#include "surfaceMesh.H"

#include "GeometricField.H"
#include "vector.H"
#include "fvPatchField.H"
#include "fvsPatchField.H"

// Important
#include "surfaceFields.H"
// end Important

#include "fvc.H"
#include "fvm.H"
#include "fvMatrices.H"

#include "snGradScheme.H"

// Set precision
#include "IOmanip.H"

using namespace Foam;

int main(int argc, char* argv[])
{
    argList::validOptions.insert("field","<name>");

    argList args(argc, argv);

	if(!args.optionFound("field"))
	{
	    FatalErrorIn("main()")
		    << "-field option is mandatory."
			<< abort(FatalError);
	}

    Time runTime("controlDict", args.rootPath(), args.caseName());

    fvMesh mesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime,
            IOobject::MUST_READ
        )
    );

	Info<< "Mesh created.\n";

    GeometricField<vector, fvPatchField, volMesh>
    vf
    (
        IOobject
        (
            args.options()["field"],
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

	Info<< "Field created.\n";

    GeometricField<scalar, fvPatchField, volMesh> mgvf = mag(vf);
    
    //while (runTime.loop())
    //{
		Info<< "Time: "
		    << runTime.timeName()
			<< endl;

        /// Re-read field --- When in loop
        //GeometricField<vector, fvPatchField, volMesh>
        //vf
        //(
        //    IOobject
        //    (
        //        args.options()["field"],
        //        runTime.timeName(),
        //        mesh,
        //        IOobject::MUST_READ,
        //        IOobject::AUTO_WRITE
        //    ),
        //    mesh
        //);

        mgvf = mag(vf);

        //if (runTime.write())
        //{
            mgvf.write();

			Info<< mgvf.name()
			    << " written in "
				<< runTime.timeName()
				<< "\n";
        //}
    //}

	Info<< "End.\n";

    return 0;
}
