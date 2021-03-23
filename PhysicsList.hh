#ifndef __PHYSICS_LIST_HH__
#define __PHYSICS_LIST_HH__

#include <G4VModularPhysicsList.hh>
#include <G4EmStandardPhysics_option4.hh>
#include <G4SystemOfUnits.hh>

class PhysicsList : public G4VModularPhysicsList {
   
 public:

   // In the constructor, we register 
   PhysicsList() : G4VModularPhysicsList() {
      defaultCutValue = 1.0*mm;
      SetVerboseLevel(1);

      // Register the Em standard physics option4
      RegisterPhysics(new G4EmStandardPhysics_option4());
   };

   // The cuts are just set in the default way, using the parent class
   virtual void SetCuts() {
      G4VUserPhysicsList::SetCuts();
   };
};

#endif
