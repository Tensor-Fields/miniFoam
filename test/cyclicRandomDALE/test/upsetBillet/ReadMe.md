# First trials

<!---
--->
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
