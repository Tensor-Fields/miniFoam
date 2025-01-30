/**
 * Pass command-line arguments to the code
 *
 * \author: Maalik (ali@tensorfields.com), Blue Room, UCD, Dublin, Ireland 
 * \date: Jan 07 2025
 */

#include "argList.H"
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

    Info<< "End"
        << endl;
}
