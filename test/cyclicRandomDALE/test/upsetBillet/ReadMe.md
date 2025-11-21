# First trials

<!---
--->
## bekaert_13  

- Die displacement increased to 12mm from 9mm to see more severe mesh
distortion;
- Billet mesh refined one level to see if the skewed corner of the billet
turns up smoother.

### Results  

Skewed corner is still a problem.

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
