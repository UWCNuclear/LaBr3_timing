// We cannot use the general particle source, because of the way it sets the
// global times. If we have an isotope with a half life of several years, the
// global times are of that order of magnitude. However, we want to look at
// time differences of the order of a few picoseconds. Since the global time
// is represented as a double, we don't have enough precision to do this.
// So instead, I have my own simplified version.

#ifndef __PRIMARY_GENERATOR_HH__
#define __PRIMARY_GENERATOR_HH__

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4ParticleGun.hh>
#include <G4Gamma.hh>
#include <G4MTRandExponential.hh>
#include <G4SystemOfUnits.hh>

#include "LevelScheme.hh"

//-----------------------------------------------------------------------------
// This is a simple class to use the G4ParticleGun to generate the gammas.
class PrimaryGenerator : public G4VUserPrimaryGeneratorAction {

 private:
   G4ParticleGun gun; // An instance of the particle gun
   LevelScheme ls;    // The level scheme to generate

   //--------------------------------------------------------------------------
   // Generate a single gamma ray of a given energy in a random direction from
   // the origin
   void GenerateGamma(G4Event *event, G4double E) {
      
      // Set it to fire gammas
      gun.SetParticleDefinition(G4Gamma::GammaDefinition());

      // Set the energy
      gun.SetParticleEnergy(E);
      
      // Set the initial position (the origin)
      gun.SetParticlePosition(G4ThreeVector(0,0,0));
      
      // Pick a random isotropically distributed direction i.e. linear in
      // cos(theta) and linear in phi
      G4double cosTheta = 2 * G4UniformRand() - 1;
      G4double phi = CLHEP::twopi * G4UniformRand();
      G4ThreeVector direction(0,0,1);
      direction.setTheta(std::acos(cosTheta));
      direction.setPhi(phi);
      gun.SetParticleMomentumDirection(direction);
                                       
      // Fire the gun
      gun.GeneratePrimaryVertex(event);
   };

   //--------------------------------------------------------------------------
   // Increase the global time to allow for lifetime in between states
   void AddTime(G4double tau) {

      // Get previous time
      G4double t = gun.GetParticleTime();

      // Add a random decay time weighted exponentially
      t += G4RandExponential::shoot(tau);

      // Set new time
      gun.SetParticleTime(t);
   };
   
 public:

   //--------------------------------------------------------------------------
   // Constructor
   PrimaryGenerator(const char *filename) {

      // Read the level scheme
      ls.Read(filename);

      // Show it
      ls.Show();
   };
   
   //--------------------------------------------------------------------------
   // Generate primaries - three gammas, all isotropic and directionally
   // uncorrelated with each other, with levels of specified lifetimes in
   // between
   void GeneratePrimaries(G4Event *event) {

      // Initialise the absolute time to zero
      gun.SetParticleTime(0);

      // Pick an initial level at random weighted by the population
      Level *level = ls.PickPrimaryLevel();
      while (level) {

         // Get the tau of the level
         double tau = level->GetTau();
         if (tau < 0) break; // negative means stable

         // Pick a depopulating transition
         Transition *transition = level->PickDepopulatingTransition();
         if (!transition) break; // NULL means no depopulating transition

         // Get the gamma energy for that transition
         double Egamma = transition->GetEnergy();

         // Generate the gamma
         GenerateGamma(event, Egamma);
         
         // Get level which it populates
         level = transition->GetFinal();
         tau = level->GetTau();
         if (tau < 0) break; // negative means stable

         // Allow for lifetime in between
         AddTime(tau);
      }
   };
};

#endif
