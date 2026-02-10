## Usage

```sh
./run.sh -h    # Prints help

## - Edit mesh generator script

## - Run

./run.sh form endTime 1 template/ dale_60_1/ place_holder genMesh_upsetBillet.sh

## Test

```sh
./run.sh -h
```

## Runs

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
