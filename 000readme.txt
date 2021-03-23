This example creates a root tree for 6 LaBr3 detectors around a
source. The level scheme for the source is read from a file, specified
with the -l option. We can't use the general particle source, because
of precision issues in the absolute time (we need
picosecond-precision, but the events have times of several years and
this is beyond the precision of a double).

We should probably try adding some passive shielding between the
detectors as well...

BGO suppression would also be interesting...

Each event has 2*NDET Double_t values, which are energy:time pairs for
each detector (with NDET detectors). The times are absolute.

The subdirectories have levelscheme files with a .ls extension and
root files with the results of that simulation.

Main program:

LaBr_timing.cc

Classes:

Datum.hh                    - per thread data for all detectors (E & T)
DetectorConstruction.hh     - construction of N detectors
EventAction.hh              - fill root tree after each event
Level.hh                    - single level of level scheme
LevelScheme.hh              - whole level scheme
PhysicsList.hh              - physics list (just standard EM option4)
PrimaryGenerator.hh         - generate primaries from level scheme
SensitiveDetector.hh        - sensitive detector (sum E & average T)
Transition.hh               - single transition of level scheme
UserActionInitialization.hh - register primary generator and event action

