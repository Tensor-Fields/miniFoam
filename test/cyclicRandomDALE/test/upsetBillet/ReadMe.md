# First trials
<!---
--->
## local: *form_48*

`displacementNeumann`, `value`-grad method; complex eigenvalues at
the first time step.

## local: *form_47*

`displacementNeumann`, `JW`-grad method; physical displacement field at
the first time step, did not converge though, hit `maxIter` for the first time
step.

## local: *form_46*

`displacementNeumann`, `H`-grad method; failed to produce correct results at
the first time step; stopped after just a few iterations.

## local: *form_45*

`displacementNeumann`, all three boudary treatments mixed: failed.

## Important for `axisymmetric` runs

Re-run to make sure their problem was not due to the
*explicit divergence of stress* being commented; the problem we were facing
in *gridStudy/frictionless/depot*.

## local: *form_44*

*form_44* is to examine `axisymmetric` boundary condition.  

In the initial implementation, we did not force face-outward components to
zero, so we see that unphysical component is actually the largest component.  

A new option is implemented: `-isAxisymmetric` to force `DU`-Z-component to be
zero. `Z` was and is hard-coded. However, `axisymmetric` is modified so that
it hacks the underlying field as the very first thing to do, that is, sets its
peripherial components to zero.  

Previously, `evaluate()` inside the constructor caused checker-boarding;
however, it turned out that something unknown is wrong in the `updateCoeffs()`
body, so after a modification, with or without `evaluate()` in the constructor,
we get `FPE`.


<img
    src="form_44/Figures/axisymmetric.png"
    style="width:75%; height:auto;"
/>

<img
    src="form_44/Figures/axisymmetric_2.png"
    style="width:75%; height:auto;"
/>

## local: *form_41 vs form_42*

*form_42* also shows the use of the newly implemenmted switch,
`-correctTractionFromDict` which tries to force traction on the boundaries as
a standalone field. As it is seen, the first results which is not only dependent
on this new approach, but on the combinatino of the BC's was not successful.  

We are trying to answer these questions:

- What is the mathematical model of a `wedge`? Is it the same as the normal
  gradient being zero?
- If yes, what is the differece between `wedge` and `symmetryPlane`? Can `wedge`
  patches be specified as `symmetryPlane`s?
- No need to mention that `zeroGradient` is not the appropriate equivalent to
  `wedge`; implementation-wise, it sets the face center
  value equal to the cell
  center value. Let us say *Y* is the direction of the wedge; if cell-center
  value is a vector like (1 1e-6 1), then both *front* and *back*
  `wedge` patches adapt this value; this is wrong, because, you expect,
  front and back be each other's mirror. Note that in our example, the
  *Y-component* should always remain close to zero throgh the governing equation
  and BC's, but, `zeroGradient` causes a small discrepency from zero to grow,
  see *form-40*.
  
  ## Results

  Left: `solidTraction` on side (*form_42*, left), vs, `extrapolate` on side
  (*form_41*, right)

  Left shows radial expansion, right does not show this.

  <img
      src="figures_and_states/anim_41_42/frames.gif"
      style="width:75%; height:auto;"
  />
  

## local: *form_36 vs form_37*

*form_36* is `zeroShear`, *form_37* is `displacement`.
The latter,

- a) Produces nearly correct $V_r--r$ graph; 
- b) (Almost) no checker-boarding for stress component contours;
- c) Correct displacement (See `U` field at the end of the simulation, which
  matches with the prescribed total displacement of `(-0.009 0 0)`).
- d) In terms of the final radius both do it correctly.

**a)**

<img
    src="form_37/Figures/Ur_vs_r.png"
/>

**b)**  
$\sigma_{YY}$, color-scale from *form_36*; obvious vertical checkerboarding;  

<img
    src="form_37/Figures/checkerboard_sigmaYY.png"
    style="width:75%; height:auto;"
/>

$\sigma_{ZZ}$, color-scale from *form_36*; obvious vertical checkerboarding;  

<img
    src="form_37/Figures/checkerboard_sigmaZZ_36.png"
    style="width:75%; height:auto;"
/>

$\sigma_{YY}$, color-scale from *form_37*; slight horizontal checkerboarding;  

<img
    src="form_37/Figures/checkerboard_sigmaYY_37.png"
    style="width:75%; height:auto;"
/>

$\sigma_{ZZ}$, color-scale from *form_37*; slight horizontal checkerboarding;  

<img
    src="form_37/Figures/checkerboard_sigmaZZ_37.png"
    style="width:75%; height:auto;"
/>

$\sigma_{XY}$, color-scale from *form_36*;

<img
    src="form_37/Figures/checkerboard_sigmaXY_36.png"
    style="width:75%; height:auto;"
/>

$\sigma_{XY}$, color-scale from *form_37*;

<img
    src="form_37/Figures/checkerboard_sigmaXY_37.png"
    style="width:75%; height:auto;"
/>


## local: *form_35*

Slower strain-rate makes the boundary condition `displacement` show much more
sensible behaviour (absolutely correct to some severe deformation).
Specifically, the rigid-plastic behaviour is interesting, in that, tha material
only deforms when everywhere is $\sigma_Y$; it might be an illusion also!

## Results

<img
    src="form_35/figures/anim_sigma_35/frames.gif"
    style="width:75%; height:auto;"
/>

## local: *form_33* vs *form_34*  

Newly developed boundary condition `displacement` which handles both
*fixed axial strain rate* and *frictionless* was tested.
Displacement `D` on a patch face is decomposed into normal and tangential
components;
$$
D_{normal} = (D.N)N = (N\otimes N)D
$$
where, for *form_33*,
$$
N_f=\text{face normal}
$$
and for *form_34*,
$$
N_x=\text{axial unit vector}
$$

## Results

<img
    src="form_34/figures/anim_mesh_34_vs_33/frames.gif"
    style="width:75%; height:auto;"
/>

### $\sigma$

<img
    src="form_34/figures/anim_sigmaCauchy_34_vs_33/frames.gif"
    style="width:75%; height:auto;"
/>

## smartsimet & local: *form_31*  

If
$$
Z(t) = a exp(-bt)
$$
then
<!--$$
\dot{Z}=\frac{\mathrm{d}Z}{\mathrm{d}t}=-abexp(-bt)=-bZ
$$
and
$$
\frac{\dot{Z}}{Z}=-b
$$-->
$$
u=Z-Z(t=0)=Z-a=a\big(exp(-bt)-1\big)  
$$
$$
\dot{u}=a(-bexp(-bt))=-b(aexp(-bt))=-bZ  
$$
$$
\frac{\mathrm{d}\dot{u}}{\mathrm{d}Z}=-b
$$
where $-b$ is the constant strain rate (Hoffman & Sachs, Introduction to the
Theory of Plasticity, p.247).

**Note**: Mesh has its own $Z$, and it reads $u$ only from the table, so
arbitrary *a* does not produce wrong points,
$$
Z_{mesh} = u + Z_{mesh, t=0}
$$

$-b$ is the constant strain rate, and it is automatically calculated from
the provided dictionary,
```cpp
billetTop
{
    axialDir    (D 0 0);
}
```
where
$$
-b=\frac{1}{t_{endTime}}ln(1+\frac{D}{Z_{t=0}})
$$
and $D$ is the total displacement of the patch.

## smartsimet & local: *form_30*

Tried to implement `fixedAxialStrainRate` boundary condition.
It did not work properly, and we could produce that behaviour using
a table as detailed in *form_31*

## smartsimet & local: *form_29*

Force perfectly plastic by providing one line in *plasticStrainVsStress* file.
See *elastoPlasticKirchoff.C* where it sets `perfectPlasticity_` flag to `true`
under the above condition.

## Results

*form_29* and *form_27* outlines match (exept for times that only one of the
is written to the disk).

**Conclusion**: The `elastoPlasticKirchoff` model predicts perfect plastic
behaviour when there is no shear (at least for a cylinder under compression).

*grey*: *form_27*, *red*: *form_29*

<img
   src="figures_and_states/anim_perfect_plastic_27_29/frames.gif"
   style="width:50%; height:auto"
/>

## smartsimet & local: *form_28*

- Very large `E` does not cause `form` to produce rigid-perfectly plastic
behaviour, instead, it gives checker-boarding.

<img
   src="form_28/figures/checkerboarding.png"
   style="width:50%; height:auto"
/>

## smartsimet & local: *form_27*

- `fixedDisplacementZeroShear` for `billetTop`
- `solids4FoamFixedDisplacement` for `die`
- Fixed strain-rate imposed through *constant/timeVsDisp*

## Results

Displacement magnitude, `U`:  

<img
    src="form_27_smartsimet/figures/anim_displacementMag/frames.gif"
    style="width:50%; height:auto;"
/>  

I see **rigid-plastic** behaviour, `sigmaCauchy`; note that `7e8` is the yield
stress:

<img
    src="form_27_smartsimet/figures/anim_sigmaCauchy/frames.gif"
    style="width:50%; height:auto"
/>

<img
    src="form_27_smartsimet/figures/anim_mesh/frames.gif"
    style="width:50%; height:auto"
/>

## smartsimet & local: *form_21* to *form_25*

Former: *form_20* but parallel. Problem with this case is that
`billetContact` where we specify `solidTraction` also includes the sides of the
billet.  
*form_22* is a new mesh with `billetSide`, `billetCorner`, and `billetTop` being
separate patches. Ran on 4 CPUs.  

### Results

*form_22* with `pressure 7e8`, gives max `sigmaCauchyEq 2.5e8`, while, in conventional
frictional cases the latter is ~`7e8`.  

*form_23*: `pressure 70e8`; stalls at ~0.05s. Maybe larger force is needed.  

*form_24*: `pressure 700e8`;  

*form_25*: `pressure 7000e8`; `cells have become inverted` at the very beginning.  
Decreasing time step by a factor of 10 did not solve the issue. The problematic
cell written does not show any apparent problem.

## bahar: *form_19* & *form_20*  

The former has negative pressure at the billet top as the BC, so it is sucked from
there; corrected in the latter.

## bahar: *form_18* & *bek_18*  

Coarse mesh with `displacement` changing by time.

### Results

Results are physically wrong, because we posed the problem physically wrong.
The displacement does not apply any forces.

## smartsimet: *gridStudy*  

three-level Lagrangian run using `form`.

### Results

#### level_0, 1, 2

Levels 0 and 2 are fine, *level_1* stops for an unknown reason, the error is
cell inversion, but it is not clear why it happens, and no inversion is
visible in the problematic `writeTime`.

### level_0, 1, 2, try3

We thought the problem with *level_1* might be the corner cell intruding the
die, so we ran all cases with a bit more round corner; all fine (actually
*level_0_try3* mesh was mistakenly the same as before), but
*level_1_try3* stops, although the workaround delays the problem.

### level_-1

One systematic level below *level_1*;

### level_1.1

Same as *level_1*, except the die has one cell only, and radial and
longitudinal cell count is now `21` instead of `20`.

## smartsimet: *form-17*  

*16* with some changes:

- Cut-corner is one cell only;  
- Contact is *not* `frictionless`;
- `meshDance` reads `forceWriteAtRemapTime` from *ALEDict*, and successfully
writes
-`d2dt2Scheme Euler` not `integrated`, as the second is still in *miniFoam/*`libminiFoam`
only.

## smartsimet: *form-15 && 16*  

*16* is similar to *local/form-15*.

### The error

*smartsimet/form-15* is
an unseccessful test that is kept for documentation.
It was because
`meshDance` was included twice in *form/Make*; once by
*form/Make/`libminiFoam`*, and once through *form/Make/`libsolidModels`*.
Also, we had *controlDict/`libminiFoam`*. It was a not-neat mix!

## local: *form-15*  

Cut-corner billet; goes well to the `endTime`.  
Remap times are not written.

## local: *form-14*  

Copies *usmartsimet/bekaert_17*; to be modified. 

## local: *form-13*  

A try to run `form` with round-corner billet.

### Results  

Unknown problem right at the beginning (FPE).
Modifications tested include
- `divScheme`
- `stabilisePressure false`

## local: *form-12*  

The deformed-die problem in *form_11* was thought to be because of
the malfunction of `meshDance::rezone()`. A new functionality was added,
whereby points in a `boundBox` can be forced to be fixed anyway, overriding
`freePatches` and `slidingPatches` settings, see *constant/ALEDict*.

### Results  

After checking that the functionality works fine, we still see the exact
mal-forming of the die and the billet as shown in *form-11*. Vetting
*log* showed that after remap, `DUEq` needs more iterations, because it does
not meet the residual in `500`. And the residual at `maxIter` increases every
time step (obviously, as the previous time's residual is not tight enough,
and so, next time has its own error accumulated on top of the previous time).  

### Update    

`nCorr` is forced to be `10e4` after remap, and automatically reset to
default after the iteration loop (see `form.C`). Aborts earlier!
*0.0015s* compared to previously *0.0020s*. However, billet and die are
cleaner (less skewed), and we see clear-cut boundary in the contact, see
[figure](form_12/figures/deformedDie.png).
![abort](form_12/figures/deformedDie.png)

### Results  

### Update    

Sharp boundary and the specific configuration at the error time suggested that
the die being one cell is the problem. New test was done using conforming mesh
lines of the billet and the die. Turns out that the last cell in the die is
the problem.  
![last cell in the die](form_12/figures/dieLastCell.png)

### Update    

After changing die outside from `contact` to `fixedDisplacement`, it now runs
ad writes four time steps (previously three time step). Diverging mesh is very
strange  
![diverging](form_12/figures/invertedCells.png)

### Update  

A variation with 
- high `remapSteps`
- `nRemapIter 5` (non-linear correction, if it exists at all)
- `nMomentumIterAfterRemap 2000`
was run  

### Results  

Crashed after the second remap (*0.25*s). It turns out that the core problem
is `DUEq` itself; complex eigenvalues appear only in the equation after remap.

### Update  

Quiet unpredictably, wrong time indexing was causing all the troubles, both
write-time problem, and the divergence, which I could not even imagine they
are related.  
The rescue elexir was: `Time::timeIndex()`  
P.S.: I did not sleep last night, stayed in the office over night, and this
is the next day before noon.

## local: *form-11*  

Ran with `-forcePosRelJ`, re-compiled `meshDance` (see
*form-10*), `writeInterval 13`

### Results  

Write-time problem in *form-10* is solved, but that might be due to the
change in *controlDict/*`writeTime`. It is unlikely though, because of
the change explained in *form-10* about `meshDance`. Next test will approve
or disapprove it.  

However, we get SIGFPE during solving `DUEq`, right after the second remap
(0.2505s, while remap time is 0.2500). Complex eigen values error from
*tensor.C*.

### Update  

Changed
- `writeInterval 10`
- `nRemapStep 150` (previously `50`)
- `rezoneFrequency 100` (previously `250`)

### Results  

Again aborted right after the second remap; same error: complex eigen values,
and problematic cells in the `relJ` check.

### Update  

Tried 
- `writeInterval 1`
- `rezoneFrequency 1`
to find the root of the problem. After four time steps (and therefore
four remap steps), it crashes, with a deformed die mesh. Therefore, our thought
as to the problem being exactly on the second remap is wrong.  
![deformed die](form_11/figures/deformedDie.png)

### Results  

## local: *form-10*  

Ran with `-forcePosRelJ`.  

### Results  

Stopped faster! At *0.25*s.  

### Update  

Remap and rezone block moved to right after the iteration loop; advect
everything except *materials* and another two fields. `form.C` is responsible
to update fields it needs to update.  
Note that all fields are registered with the mesh at the time of their
creation, if you do not advect fields, you do not get rid of the wrong
values, they remain there.

### Results  

Stopped at *0.25*s, ran without passing options to `form`. Also,
  
- The first rezone time is *0.125*
- The first written time time after remap is *0.1275*
- While `deltaT`, which controls the `writeTime` dictates *0.13*  

### Update  

In *meshDance.C*, when resetting time, we also reset `timeIndex=0` to test.
Also, in `form.C`, re-evaluate everything from remapped fields.  

### Results  

Same problem in writting after rezone (**Note**: `meshDance` was not
re-compiled, so, the change for the write times is
not tested); same crash at the same time.
`DU`, `relJ`, `sigmaCauchy`, all has jumped up unreasonably on the
problematic write time.  

**Note**  
We do not update surface fields through the remapping process; they are not
valid after the remap, if they are used anywhere, the lead to wrong results.
Same goes for `pointField`s.

## local: `form_8, 9`  

### Results  

`form` (*form-8*) does not see the contact, and it is very fast and unphysical compared to
`plasticNonLinSolid` (*form-9*)  

### Update  

Some *important* corrections made *form-8* to work as expected.
What was the problem actually? I do not remember XD  
Stopped due to out-of-bound `relJ` at *0.5*s, while `endTime`=1s.  

## local: `form_6, 7`  

Just remove them  

## local: `form_5`  

Some updates to `meshDance`, particularly concerned with both
physical time and remap time being the same

### Results  

*ALEDict* now controls if you want to write remapped field and mesh,
and if yes, how frequently  

## local: `form_4`  

Copy of *bekaert_19* to test `rezoneAndRemap` with round-corner mesh

### Results  

Perfect rezone and remap.

## local: `form_3`  

Test `meshDance` remap + rezone functionality by a test app, `rezoneAndRemap`

## local: `form_2`  

- Rezone every time step to test it.

## local: `form_1`  

- Uses `form`; just experimenting.  

### Results  

Ran to *0.5* without rezone; ran from there
with rezone; crashed with `cell inversion` error.  

## local: s4f_2  

- fe41: Unable to map; sigfpe at the first time step after creating the first
`tmp` field.

### Results  

- Debug: Make sure *system/fvSchemes*`.ddtScheme.default`
is not `steadyState`  
- After debug: Sigfpe at the first time step after remap.

## local: s4f_1  

- fe40: Unable to map; sigfpe at the first time step after creating the first
`tmp` field.

### Results  

- Debug: Make sure *system/fvSchemes*`.ddtScheme.default`
is not `steadyState`  
- After debug: Sigfpe at the first time step after remap.

## smartsim: bekaert_19  

- Like *18*, using the switch `-restartReady` both before and after
re-start.

### Results  

Same as *18*.

## bekaert_18  

- Like 17, rezoned and remapped.  
- Struggles to restart; we need to `pull` latest updates for
the solver that we implemented during the *impactBar* test.

### Results  

Sigfpe in contact bc at the first time step after restart.

## bekaert_17  

- Billet with round corner

### Results  

- Successfully goes to `endTime` (1s).

## bekaert_15  

- Frictional contact between the die and the billet instead of frictionless.

### Results  

Better; only the exact corner cell is problematic

## bekaert_14  

- Die mesh lines match billet mesh lines to see if it solves the skewed
corner problem

### Results  

Same as *bekaert_13*

## bekaert_13  

- Die displacement increased to 12mm from 9mm to see more severe mesh
distortion;
- Billet mesh refined one level to see if the skewed corner of the billet
turns up smoother.

### Results  

<<<<<<< HEAD
The skewed corner is still a problem.
=======
Skewed corner is still a problem.
>>>>>>> 72b1cbb7d709eeaf4c8018334edf800880513c6f

## bekaert_12  

- The problem in *bekaert_11* is solved in the implementation.
- `maxPenaltyScale` reduced to `1` from `20`

### Results  

![problematic contact](/home/maalik/foam/maalik-4.0/run/miniFoam/test/cyclicRandomDALE/test/upsetBillet/bekaert_12/figures/squished.png)

## bekaert_11  

`solids4FoamFixedDisplacement` was looking for `D`, instead of `U`;
the latter is what `plasticNonLinSolidFoam` wants.

## bekaert_10  

Obviously, reducing the displacement in *constant/timeVsDisp* delays the
simulation crash, but the displacement is not consistent with the value
specified there.  

### solids4FoamFixedDisplacement

I thoght it might have something to do with the implementation of
`fixedDisplacement` in bekaert compared to that in s4f.  
I created a copy of that in s4f, made it into `libminiFoamBCs.so`, and
loaded it. Still the same problem.  
However, we have loaded *bekaert libraries* too, so, if the implementation
is the problem, the current run might still be using that of bekaert, not
miniFoam.

### Results  

![problematic contact](/home/maalik/foam/maalik-4.0/run/miniFoam/test/cyclicRandomDALE/test/upsetBillet/bekaert_10/figures/squiched.png)

## bekaert_9  

*constant/timeVsDisp* to be -0.09mm at the `endTime`; die has one cell only

### Results  

Same results as *bekaert_6*

## bekaert_8  

*constant/timeVsDisp* to be -0.09mm at the `endTime`

### Results  

Same results as *bekaert_6*

## bekaert_7  

Load *bekaert* libraries instead of those of *solids4Foam*; same
results as *bekaert_6*
  
## bekaert_6  

Billet height is 15mm. We set *constant/timeVsDisp* to be -0.9mm at the
`endTime`; however, the billet gets squished way before that.
  
![problematic contact](/home/maalik/foam/maalik-4.0/run/miniFoam/test/cyclicRandomDALE/test/upsetBillet/bekaert_6/figures/squished.png)
  
## bekaert_5  

Like *bekaert_4*, but `frictionless`
  
### Results  

Wrong displacement, dies moves away  

## bekaert_4  

Like *bekaert_3*, but correct displacement
  
### Results  

Wrong displacement, dies moves away  

## bekaert_3  

`fricionalModel` (not `normalFriction`) changed to `standardPenalty`
instead of `frictionless` to match solids4Foam settings.
  
### Results  

Contact problem + die displacement was still wrong
![problematic contact](/home/maalik/foam/maalik-4.0/run/miniFoam/test/cyclicRandomDALE/test/upsetBillet/bekaert_3/figures/problematic_contact.png)

## bekaert_2  

Die displacement was wrong (goes to negative coordinates)
  
## bekaert_1  

Wrongly ran with small time steps (those of impactBar); terminitated manually.

## Old cases
  
- *upsetBillet_orig*: Base case;
Result: Contact is problematic, in that, a node of the
cyliner sticks to the die

- *upsetBillet_frictionless*: Unphyscical behaviour
- *upsetBillet_1*: Less cells on the die, inspired by cylinderCrush;
Result: A node sticks to the die

- *upsetBillet_2*: Only one cell on the die, inspired by impactBar
Result: A node sticks to the die
