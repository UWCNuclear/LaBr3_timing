#ifndef __DETECTOR_CONSTRUCTION_HH__
#define __DETECTOR_CONSTRUCTION_HH__

#include <G4VUserDetectorConstruction.hh>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4SDManager.hh>
#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4UserLimits.hh>
#include <G4SubtractionSolid.hh>

#include <vector>

#include "SensitiveDetector.hh"

//-----------------------------------------------------------------------------
// This class generates a set of cylindrical detectors in a horizontal plane
// around the origin with a distance of 40 mm from the origin. The constructor
// is initialised with the number of detectors and they are spaced equally. The
// detectors are simple cylinders. For each detector a sensitive detector is
// created and for each event, the energy and time will be pu into an array,
// which is passed to the constructor (one element per detector) so that
// listmode can be constructed.
class DetectorConstruction : public G4VUserDetectorConstruction {

 private:
   G4NistManager *man; // NIST material manager
   G4LogicalVolume *log_world; // World logical volume
   std::vector <G4LogicalVolume *> log_sci, log_case; // Other logical volumes
   Datum *data; // Data storage (one Datum per thread, including master thread)
   int ndet;    // Number of detectors

   //--------------------------------------------------------------------------
   // Set up the materials we need using the NIST database
   void GetMaterials() {
      
      // Get an instance of the NIST materials manager
      man = G4NistManager::Instance();

      // Get the elements we need
      G4Element *N  = man->FindOrBuildElement("N");
      G4Element *O  = man->FindOrBuildElement("O");
      G4Element *La = man->FindOrBuildElement("La");
      G4Element *Br = man->FindOrBuildElement("Br");
      G4Element *Ce = man->FindOrBuildElement("Ce");
      
      // Air
      G4Material *Air = new G4Material("Air", 1.290*mg/cm3, 2);
      Air->AddElement(N, 0.765); // Real number = mass fraction
      Air->AddElement(O, 0.235);

      G4Material *LaBr3 = new G4Material("LaBr3", 5.07*g/cm3, 2);
      LaBr3->AddElement(La, 1);
      LaBr3->AddElement(Br, 3);

      G4Material *LaBr3_Ce = new G4Material("LaBr3_Ce", 5.08*g/cm3, 2);
      LaBr3_Ce->AddMaterial(LaBr3, 99.5*perCent);
      LaBr3_Ce->AddElement(Ce, 0.5*perCent);
   };
   
 public:

   //--------------------------------------------------------------------------
   // Constructor
   DetectorConstruction(Datum *data_, int ndet_) {
      // Get or construct materials
      GetMaterials();
      data = data_;
      ndet = ndet_;
   };

   //--------------------------------------------------------------------------
   // Construct detector
   G4VPhysicalVolume* Construct() {

      char name[1024];
      
      // Create world volume - 6x6x6 m^3 box of air
      G4Box *shape_world = new G4Box("world", 3*m, 3*m, 3*m);

      // Create logical volume for "world" by filling it with air
      log_world =
        new G4LogicalVolume(shape_world
                            , man->FindOrBuildMaterial("Air"),
                            "log_world", 0, 0, 0);

      // Place "world" volume in space
      G4VPhysicalVolume *phys_world =
        new G4PVPlacement(0, G4ThreeVector(), log_world, "world",
                          0, false, 0);

      // World volume is invisible (i.e. air is transparent)
      log_world->SetVisAttributes(G4VisAttributes::Invisible);
      
      // Create LaBr3(Ce) detector
      double r = 19.05*mm; // radius of detector = 3/4"
      double l = 38.10*mm; // length of detector = 1 1/2"
      double d = 40.*mm;   // distance of front of detector to source
      double gap = 1.*mm;  // gap between detector and case
      double t = 1.*mm;    // thickness of case

      // If we have more than 6 detectors, we have to pull them back a bit
      // so they don't touch - leave a gap of 1 mm
      double d2 = (r + t + gap * 2.) / tan(180.*deg / (double)ndet);
      if (d < d2) d = d2;
      
      // Shape is a tub of radius r and length l
      G4Tubs *shape_sci =
        new G4Tubs("scintillator", 0.*cm, r, l/2., 0.*deg, 360.*deg);

      // Create shape of case
      G4Tubs *shape_filled_case = new G4Tubs("filledcase", 0.*cm, r + gap + t,
                                             l/2 + gap + t, 0.*deg, 360.*deg);

      // Create hollow
      G4Tubs *shape_hollow = new G4Tubs("filledcase", 0.*cm, r + gap,
                                        l/2 + gap, 0.*deg, 360.*deg);

      // Create hollowed out case
      G4SubtractionSolid *shape_case =
        new G4SubtractionSolid("case", shape_filled_case, shape_hollow,
                               0, G4ThreeVector(0,0,0));
      
      // Loop over detectors
      for (int i = 0; i < ndet; i++) {
         
         // Create a logical volume for scintillator
         sprintf(name, "log_sci_%d", i);
         G4LogicalVolume *temp =
           new G4LogicalVolume(shape_sci, man->FindOrBuildMaterial("LaBr3_Ce"),
                               name, 0, 0, 0);
         log_sci.push_back(temp);

         // Create a logical volume for case
         sprintf(name, "log_case_%d", i);
         temp =
           new G4LogicalVolume(shape_case, man->FindOrBuildMaterial("G4_Al"),
                               name, 0, 0, 0);
         log_case.push_back(temp);

         // Set up rotation
         G4ThreeVector pos(0.*cm, 0.*cm, d + l / 2 + gap + t);
         G4RotationMatrix rot = G4RotationMatrix();
         pos.rotateY(360.*deg*(double)i/(double)ndet);
         rot.rotateY(360.*deg*(double)i/(double)ndet);
         
         // Create physical volume for scintillator
         sprintf(name, "sci_%d", i);
         new G4PVPlacement(G4Transform3D(rot, pos),
                           log_sci[i], name, log_world, false, 0, false);
         
         // Create physical volume for case
         sprintf(name, "case_%d", i);
         new G4PVPlacement(G4Transform3D(rot, pos),
                           log_case[i], name, log_world, false, 0, false);
      }
      return(phys_world);
   };

   //--------------------------------------------------------------------------
   // Construct sensitive detector
   void ConstructSDandField() {

      double offset[] = {200, 300, 50, 150, 330, 180, 250, 190};
      char name[1024];
      
      // Get thread ID
      int thread = (G4Threading::G4GetThreadId() + 1);

      // Create an sensitive detector manager
      G4SDManager *sd_manager = G4SDManager::GetSDMpointer();
      
      // Create sensitive detectors for each scinitillator logical volume
      for (int i = 0; i < ndet; i++) {
         sprintf(name, "LaBr3_%d", i);
         SensitiveDetector *sensitive = new SensitiveDetector(name);
         sensitive->SetSigmaCoefficients(5., 5e-3); // sigma = 5 + E * 0.005
         sensitive->SetTimeOffset(offset[i]);
         sensitive->SetDataPointer(data + thread);
         sensitive->SetID(i);
         sd_manager->AddNewDetector(sensitive);
         log_sci[i]->SetSensitiveDetector(sensitive);
      }
   };   
};

#endif
