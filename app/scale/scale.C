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
    argList::validOptions.insert("scale","<number>");

    argList args(argc, argv);

	if(!args.optionFound("field"))
	{
	    FatalErrorIn("main()")
		    << "-field option is mandatory."
			<< abort(FatalError);
	}

	if(!args.optionFound("scale"))
	{
	    FatalErrorIn("main()")
		    << "-scale option is mandatory."
			<< abort(FatalError);
	}

	const scalar scale = readScalar(IStringStream(args.options()["scale"])());

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

	vf = vf * scale;

	Info<< "Field scaled.\n";

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

        //if (runTime.write())
        //{
            vf.rename
			(
			    vf.name()
			  + "*" //--- Error while reading from the disk
			  + args.options()["scale"]
			);

            vf.write();

			Info<< vf.name()
			    << " written in "
				<< runTime.timeName()
				<< "\n";
        //}
    //}

	Info<< "End.\n";

    return 0;
}
