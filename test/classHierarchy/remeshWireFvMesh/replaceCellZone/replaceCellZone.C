using namespace Foam;

class replaceCellZone
{
    public:
        replaceCellZone(const fvMesh&);
};

replaceCellZone::replaceCellZone(const fvMesh& mesh)
{}

replaceCellZone::manipPatches()
{
    patchManipFunc::splitPatches(x);
}
