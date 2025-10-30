*referenceCase*: frictionCoeff = 0.25

*0/*: Half wire length, deltaT = 1e-3, frictionCoeff = 2.5

*4/*: Half wire length, deltaT = 1e-3, frictionCoeff = 250

*1/*: Half wire length, deltaT = 2e-3, frictionCoeff = 2.5
    FPE occured. Before the first write-time is reached.
	Shortage of RAM/ other Hardware issue is possible.
	Re-started without any change.

	FPE again.
	Is nCPUs an issue?
	Re-started in serial.

*1_8/*: Re-started *1/* on 8 CPUs, xeno; aborted due to cell inversion. See logs and settings in the directory.

*2/*: Half wire length, deltaT = 2e-3, frictionCoeff = 25
    FPE occured. Before the first write-time is reached.
	Shortage of RAM/ other Hardware issue is possible.
	Re-started without any change.

*3/*: Half wire length, deltaT = 2e-3, frictionCoeff = 250

## Usage
```shell
## Link global scripts and templates specific to flat12x4 in the current dir
./linkFiels . flat12x4

## Create a study, e.g., nCPUsStudy to study three values for nCPUs
## It will create three exactly same processline and you modify each
## individually

./createStudy nCPUsStudy 3

