# Restart from 4e-5

## No rezone and remap; plain restart with s4f
Segfault:
```bash
setCellDisplacements: reading cellDisplacements

Program received signal SIGFPE, Arithmetic exception.
0x00007ffff11c7f0c in __ieee754_pow_fma (x=-0.16241053752786372, y=-0.33333333333333331)
    at ../sysdeps/ieee754/dbl-64/e_pow.c:167
167	../sysdeps/ieee754/dbl-64/e_pow.c: No such file or directory.
(gdb) bt
#0  0x00007ffff11c7f0c in __ieee754_pow_fma (x=-0.16241053752786372, y=-0.33333333333333331)
    at ../sysdeps/ieee754/dbl-64/e_pow.c:167
#1  0x00007ffff114db14 in __pow (x=-0.16241053752786372, y=-0.33333333333333331) at w_pow_compat.c:30
#2  0x00007ffff23c4f28 in Foam::pow(Foam::Field<double>&, Foam::UList<double> const&, double const&) ()
   from /root/foam/foam-extend-4.0/lib/linux64GccDPOpt/libfoam.so
#3  0x00007ffff780519a in Foam::tmp<Foam::GeometricField<double, Foam::fvPatchField, Foam::volMesh> > Foam::pow<Foam::fvPatchField, Foam::volMesh>(Foam::GeometricField<double, Foam::fvPatchField, Foam::volMesh> const&, Foam::dimensioned<double> const&) ()
   from /root/foam/foam-extend-4.0/lib/linux64GccDPOpt/libincompressibleRASModels.so
#4  0x00007ffff78242b0 in Foam::tmp<Foam::GeometricField<double, Foam::fvPatchField, Foam::volMesh> > Foam::pow<Foam::fvPatchField, Foam::volMesh>(Foam::GeometricField<double, Foam::fvPatchField, Foam::volMesh> const&, double const&) ()
   from /root/foam/foam-extend-4.0/lib/linux64GccDPOpt/libincompressibleRASModels.so
#5  0x00007ffff5e17358 in Foam::neoHookeanElasticMisesPlastic::correct(Foam::GeometricField<Foam::SymmTensor<double>, Foam::fvPatchField, Foam::volMesh>&) () from /root/foam/root-4.0/lib/linux64GccDPOpt/libsolids4FoamModels.so
#6  0x00007ffff5c239fa in Foam::solidModels::nonLinGeomUpdatedLagSolid::evolve() ()
   from /root/foam/root-4.0/lib/linux64GccDPOpt/libsolids4FoamModels.so
#7  0x000055555540229f in main ()
```
## rezone+remap and restart with s4f
Note: If you see no fields are being convected, it might be a matter of I/O
settings; a workaround is to copy your latestTime to a newDir with a proper
number name, e.g., 1, 2, ..., and make sure writing options in controlDict
writes the results where you want, then ./rezoneAndRemap.

Runs and writes the next time step successfully, although that is physically
wrong.

## rezone+remap and restart: bekaert

### Changes to an s4f case:
- *dynamicMeshDict*
```c++
staticFvMesh -> solidDynamicFvMesh
```
- *fvSolution*
Add
```c++
solidMechanics
{...}
```
- Copy *DD* as *DU*
- Create a file called *createSets* and
```bash
setSet -batch createSets -latestTime
setsToZones
setMatFromCellZones
```
or simply run
```bash
./createMaterials.sh
```
- Edit *mechanicalProperties*
- Remove *latestTime/U*; bekaert solver interprets it as displacement.
