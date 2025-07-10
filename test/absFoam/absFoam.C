#include "scalar.H"

//#include "InfoProxy.H"
//#include "messageStream.H"

#include "Ostream.H" // endl
#include "Istream.H" // Info

#include "foamTime.H"
#include "fvMesh.H"
#include "GeometricField.H"

using namespace Foam;

int main(int argc, char* argv[])
{
    scalar a = -1;

    Info<<"a="<< a  << endl;

    Info<<"abs(a) = "
        << abs(a)
        << endl;

    Info<<"max(-2, 1) = "
        << max(-2, 1) 
        << endl;

    Info<<"max(abs(-2), 1) = "
        << max(abs(-2), 1) 
        << endl;

    scalar b = VSMALL;

    Info<<"b="<< b  << endl;

    b = max(abs(-2), b);

    Info<< "max(abs(-2), b) = "
        << b  << endl;

    //fvMesh
    //(
    //    polyMesh::defaultRegion,
    //    runTime.timeName(),
    //    runTime,
    //    IOobject::MUST_READ,
    //    IOobject::NO_WRITE
    //);
}
