pointField readNewPoints
(
    const Time& runTime,
    const word& targetMeshDir
)
{
    fvMesh& mesh
    (
        IOobject
        (
            targetMeshDir,
            runTime.constant(),
            runTime,
            IOobject::MUST_READ
        )
    );

    return pointField(mesh.points());
}
