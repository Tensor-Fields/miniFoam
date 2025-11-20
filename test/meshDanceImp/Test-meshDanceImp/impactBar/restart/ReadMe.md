# Cases

## Possible causes

**Field Incompatibility**  
In the bekaert solver, some fields are set to be `READ_IF_PRESENT`;
forcing their re-calculation might solve the problem. To see them, see
`/root/foam/root-4.0/bekaert-v2.0/applications/solvers/plasticNonLinSolidFoam/log.readIfPresent`  
<!--- 18112025, BR, Maalik, start --->  
**Solution**  
  
*a)*
I implemented `cyclicRandomDALE -illegalToAdvect`, so, do not advect some fields, e.g.,
`F` or `grad(U)`; restart causes the billet to fly away. As if it thinks it is a compressed
billet with zero velocity, like a compressed spring; restart means releasing the spring,
and it flies away.  
  
*b)*
In addition to `illegalToAdvect`,
implemented `plasticNonLinSolidFoam -restartReady`
that forces old time fields to be read from the disc, so the solver remembers where it
was in terms of those fields. Finally, a file called `fixRestartIssue...` in
`plasticNon...` was commented, as it was de-registering the forced old time
fields. All of this together, gives inverted cells error at the very firts time step.
  
*c)*  
Do not use `illegalToAdvect`, i.e., advect everything; but use
`restartReady` to force old fields. This way, everything is remembered from
the past, both fields being `READ_IF_PRESENT`, and `.oldTime()` fields.
Run lasts longer, but we end up with an invalid mesh, see
[here](/home/maalik/foam/maalik-4.0/run/miniFoam/test/meshDanceImp/Test-meshDanceImp/impactBar/restart/bek_noExclude).
![deformed vs before-remap image](/home/maalik/foam/maalik-4.0/run/miniFoam/test/meshDanceImp/Test-meshDanceImp/impactBar/restart/bek_noExclude/figures/beforeRemapVsCrash.png)
  
*d)*  
`oldTime()`, when called for the first time, creates the old-time field (sets
a pointer to it) and names it `<fieldName>_0`, see
`GeometricField::oldTime()`. in the assignment
```c++
D.oldTime() = D_0;
```
where the RHS is previously read from the disk, first a field called
`D_0` is created by LHS, then we are not sure if the RHS `D_0` is
actually the one read from the disk, or the second one? So, before the
assignment, we read the old-time field into a different name, e.g.,
`D0`, then
```c++
D.oldTime() = D0;
```
This was tested and results are the same as *c)* above, see it
[here](/home/maalik/foam/maalik-4.0/run/miniFoam/test/meshDanceImp/Test-meshDanceImp/impactBar/restart/bek_fillOldTimes).
<!--- 18112025, BR, Maalik, end --->  

## Rezone+Remap

### Bekaert

*bek_rezone_remap*
  Billet goes crazy, results in inverted cells (see the checker-boarding of
  the density and relJ).

*true_s4f_40e-06_fe40*
  Billet bounces back

*true_s4f_40e-06_fe41*
  FPE
## Bekaert
Bekeart solver does not need a `restart` switch to be set. It restarts without
any problems.

*bek_40e-06/*
  The billet does not touch the die.

*bek_40e-06_penaltyChanged/*
  Seems okay. Deforms while impacting the die. Needs validation.

## `restart true;`
### fe41
*true_s4f_writeNow_fe41/*
Run one timestep and stop
Restart from latestTime for one timestep and stop.
**Results**
Successful

*true_s4f_20e-06_fe41/*
First writeTime is 20E-06
Run until 20E-6s and stop **Successful**
Restart from latestTime for one timestep and stop. **SIGFPE**

*true_s4f_40e-06_fe41/*
First writeTime is 20E-06
Run until 40E-6s and stop **Successful**
Restart from latestTime for one timestep and stop. **SIGFPE**

### fe40
*true_s4f_writeNow_fe40/*
Run one timestep and stop
Restart from latestTime for one timestep and stop.
**Results**
Successful

*true_s4f_20e-06_fe40/*
First writeTime is 20E-06
Run until 20E-6s and stop **Successful**
Restart from latestTime for one timestep and stop. **SIGFPE**

*true_s4f_40e-06_fe41/*
First writeTime is 20E-06
Run until 40E-6s and stop **Successful**
Restart from latestTime for one timestep and stop. **SIGFPE**

### bekaert

## `restart false;`, force `DD.write()`
*s4f_writeNow_fe41/*
Run one timestep and stop
Restart from latestTime for one timestep and stop.
**Results**
Successful

*s4f_40E-6_fe41/*
Run until 40E-6s and stop
Restart from latestTime for one timestep and stop.
**Results**
first writeTime is 20E-6s where it segfaults after writing
Restart segfaults
