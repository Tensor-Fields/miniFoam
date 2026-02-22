## Test

- Edit mesh generator script, mainly these two lines for the mesh size,
```sh
-nRT      40 \
-nZT      40 \
```
- Run

```sh
./run.sh -h
```

For example,

```sh
./run.sh form endTime 1 template/ "false" dale_60_1/ \
    place_holder genMesh_upsetBillet.sh
```

- Then, inside a *DALE* case,

```sh
./calcError.sh U ../<Lagrangian_counterpart>
```

calculates both the magnitude of `U` which is a `vectorField`, and then, the  
error for *all* `scalarField`s (not `mag(U)` only). 

## Runs

### dale_11x11, s4f_11x11

This case, like all $N\times N$ cases, where roughly $10<N<20$, was run twice;
once with $nIterPerRezone=100$ and $0$, where in the latter, DALE is
essentially off. Also in the former, the run aborts way before any rezone
happens.

### Results

`form` and `wedge`  
It is obvious that *contact* is the source of the problem. Contact does not see
the billet, or billetContact does not see the die.
<img
    src="doc/dale_11x11/figures/contact/frames.gif"
    style="width=75%; height=auto;"
/>

[`plasticNonLinGeomSolid` and `solidWedge`] or [`form` and `solidWedge`]  
No abort anymore; seems okay, manually stopped.
<img
    src="doc/plastic_solidWedge_11x11/figures/frames.gif"
    style="width=75%; height=auto;"
/>

[`s4f` and `wedge`]  
No problem.


### doc/dale_10_3

- Corrected behaviour now. When `-atSourceCellCenters` is passed, error and  
  reference fields are written at the *source* directory; when not passed,  
  results are written at the target. A complete comparison is shown below

<img
    src="figures/dale_vs_lag_10_errors.png"
    style="width:75%; height:auto;"
/>

### dale_10_2

- Without the last two options, results are still problematic.

```sh
meshToMeshError -sourceCase $Ref -latestTargetTime -latestSourceTime
```

<img
    src="figures/dale_10_2_vs_lag_error_cam2.png"
    style="width:75%; height:auto;"
/>

### dale_10_1

- Compared with *Lag_10_1*, the error is small, but when the command below  
  used, the written error field is large. Figure shows everything when  
  *warpped by scalar*.

```sh
meshToMeshError -sourceCase $Ref -latestTargetTime -latestSourceTime \
    -atSourceCellCenters -unbounded
```

<img
    src="figures/dale_vs_lag_error.png"
    style="width:75%; height:auto;"
/>

*dale_30_1*: Copy of *dale_30*, using original `wedge` and `symmetry`.  
*Lag_30_1*: Counterpart of *dale_30_1*, without ALE.
