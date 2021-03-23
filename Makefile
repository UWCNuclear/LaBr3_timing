# EXE to create
EXE = LaBr_timing

# Objects needed
OBJS += LaBr_timing.o

# Dependencies
DEPS += Datum.hh
DEPS += DetectorConstruction.hh
DEPS += EventAction.hh
DEPS += Level.hh
DEPS += LevelScheme.hh
DEPS += PhysicsList.hh
DEPS += PrimaryGenerator.hh
DEPS += SensitiveDetector.hh
DEPS += Transition.hh
DEPS += UserActionInitialization.hh

# Must use g++ compiler
CXX = g++

# Flags
CXXFLAGS += -Wall
CXXFLAGS += -g
CXXFLAGS += -DG4MULTITHREADED

# For Geant4
CXXFLAGS += $(shell geant4-config --cflags)
LDFLAGS  += $(shell geant4-config --libs)

# For root
CXXFLAGS += $(shell root-config --cflags)
LDFLAGS  += $(shell root-config --libs)

all: $(EXE)

LaBr_timing.o: LaBr_timing.cc $(DEPS)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

clean:
	rm -f *~ $(OBJS) $(EXE) LaBr_timing.root analyse_C.d analyse_C.so \
	analyse.pdf

%.root: %.ls $(EXE)
	printf '/run/beamOn 50000000\nexit\n' | ./$(EXE) -o $@ -l $^
