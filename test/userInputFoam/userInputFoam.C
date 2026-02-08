/**
 * Pass command-line arguments to the code
 *
 * Usage:
 * userInputFoam patchNameeee -patchNames \( "a" "b" "c" \)
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
    //argList::validArgs.append("list of patches"); // Does not give a proper
                                                    // stringList

    argList::validOptions.insert("patchNames","list of patchNames");
    argList::validOptions.insert("label","<label>");

    // Important: should be after args and options setting
    argList args(argc, argv);

    const word patchName = args.additionalArgs()[0];
    const label dummyLabel = readLabel(IStringStream(args.options()["label"])());

    Info<< "label = "
        << dummyLabel
        << '\n';

    //const stringList patchNames = args.additionalArgs();

    const List<string> patchNames = args.optionReadList<string>(word("patchNames"));

    //Info<< patchNames.size() - 1
    //    << " patch names found"
    //    << endl;

    //Info<< "patchNames are"
    //    << endl;

    //for (label i = 1; i < patchNames.size(); ++i) 
    forAll(patchNames, i)
    {
        Info<< "patchNames[" << i  << "]"<< "="
            << patchNames[i]
            << endl;
    }

    Info<< "Option patchName is read:"
        << nl << patchName
        << endl;

    Info<< "End"
        << endl;
}
