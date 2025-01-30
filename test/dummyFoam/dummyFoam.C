/**
 * Pass command-line arguments to the code
 *
 * \author: Maalik (ali@tensorfields.com), Blue Room, UCD, Dublin, Ireland 
 * \date: Jan 07 2025
 */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
//#include 

using namespace Foam;

int main(int argc, char* argv[])
{
    // OF12
    //argList::addOptions("patchName", "banana")
    // foam-extend-4.0
    //argList::validOptions.insert("patchName", "banana");
    argList::validArgs.append("patchName");

    // Important: should be after args and options setting
    argList args(argc, argv);

    const word patchName = args.additionalArgs()[0];

    Info<< "Option patchName is read:"
        << nl << patchName
        << endl;

    /// Calculate displacement increment
    //scalar nTimeSteps = 

    Time runTime
    (
        "controlDict",
        args.rootPath(),
        args.caseName()
    );

    fvMesh startMesh
    (
        IOobject
        (
            polyMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            IOobject::MUST_READ
        )
    );

    Info<< "endTime = "
        << startMesh.time().endTime()
        << endl;

    Info<< "startTime = "
        << startMesh.time().startTime()
        << endl;

    Info<< "deltaT = "
        << startMesh.time().controlDict().lookup("deltaT")
        << endl;

    Info<< "nSteps = "
        <<
        (
            startMesh.time().endTime().value()
          - startMesh.time().startTime().value()
        ) / startMesh.time().deltaT().value()
        << endl;

    Info<< "End"
        << endl;
}
