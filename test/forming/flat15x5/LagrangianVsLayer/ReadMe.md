### rolling1-10-3-3

- The naming convention:
<table>
    <tr>
        <th>Roll-mesh-level</th>
        <th>wire-mesh-level</th>
        <th>Roll-contact-level</th>
        <th>wire-contact-level</th>
    </tr>
        <th>-1</th>
        <th>0</th>
        <th>-3</th>
        <th>-3</th>
    <tr>
    </tr>
</table>

#### Results

- Parallel, `nCorrector=500` `->` <span style="color:red;">Error</span>:  
  Incompatible sizes (where in the code is yet to be discovered; `gdb`  
  produced `no stack`.)
- Parallel and Serial, `nCorrector=2` `->` Passes the *time*, at which it  
  previously crashed.

### l0_layer

- level-0, using layer add/remove.

#### Results

- Incompatible fields in summation (not clear where exactly in the code;  
  `libsolidModelsBekaert.so` not compiled in the debug mode, so not clear  
  whether it is that or in openfoam libraries.)

### l0_long

- Increased wire length by a factor of 3.

### rolling1_level-1

- Same as *rolling1_level0*, `contactCellSize` increased to produce a rougher
  mesh.

#### Results

- The wire is very short.

<img
    src="processLine1/rolling1_level-1/figures/shortWire/frames.gif"
    style="width:75%; height=auto;"
/>

### rolling1_level0

#### Results

- `form`: `Incompatible fields` in size, see *log.form*.
<<<<<<< Updated upstream
- `plasticNonLinSolidFoam`: Same error.
=======
- `plasticNonLinSolidFoam`: same.
>>>>>>> Stashed changes
