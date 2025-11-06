# Cases

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
