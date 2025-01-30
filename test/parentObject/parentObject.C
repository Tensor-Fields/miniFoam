/**
 * \brief What does .parent() return?
 *
 * \author Maalik(ali@tensorfields.com) 
 * \date 19-11-2024
 */

#include "argList.H"
#include "foamTime.H"
#include "fvMesh.H"
#include "IOobject.H"

using namespace Foam;

int main(int argc, char* argv[])
{
   argList args(argc, argv);

   Time runTime
   (
       "controlDict",
	   args.rootPath(),
	   args.caseName()
   );

   fvMesh mesh
   (
       IOobject
	   (
		   "region0",
		   runTime.timeName(),
		   runTime,
	       IOobject::MUST_READ   
	   )
   );

   Info << "mesh.parent = " << mesh.parent() << endl;
   //Info << "mesh.time() = " << mesh.time() << endl; // Error: OStream << Time, no such an operator
   Info << "mesh.time().timeName() = " << mesh.time().timeName() << endl;
}
