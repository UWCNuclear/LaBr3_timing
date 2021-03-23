// Class to represent a single level. It has an energy, a lifetime, the percentage
// population from the parent, and the list of depopulating transitions.

#ifndef __LEVEL_HH__
#define __LEVEL_HH__

#include "Transition.hh"

//-----------------------------------------------------------------------------
// Class for a level
class Level {

 private:
   
   double energy; // Energy of level
   double tau; // Tau of level
   double population; // Population from parent
   std::vector <Transition *> transitions; // List of depopulating transitions
   double total_decay; // Total intensity of all depopulating transitions

   //--------------------------------------------------------------------------
   // Get the number of transitions which depopulate this level
   inline unsigned int GetNTransitions() {
      return(transitions.size());
   };

   //--------------------------------------------------------------------------
   // Get the index of the nth transition which depopulates this level
   Transition *GetTransition(unsigned int ind) {
      if (ind >= transitions.size()) return(NULL);
      return(transitions[ind]);
   };

 public:
   
   //--------------------------------------------------------------------------
   // Constructor
   Level(double energy_, double tau_, double population_) {
      energy = energy_;
      tau = tau_;
      population = population_;
      total_decay = 0;
   };
   
   //--------------------------------------------------------------------------
   // Get the energy of the level
   inline double GetEnergy() {
      return(energy);
   };

   //--------------------------------------------------------------------------
   // Get the tau of the level
   inline double GetTau() {
      return(tau);
   };
   
   //--------------------------------------------------------------------------
   // Get the total population of the level from the reaction
   inline double GetPopulation() {
      return(population);
   };

   //--------------------------------------------------------------------------
   // Add a depopulating transition
   void AddTransition(Transition *t) {
      transitions.push_back(t);
      total_decay += t->GetIntensity();
   };

   //--------------------------------------------------------------------------
   // Get the total gamma intensity decaying out of this level
   inline double GetDecayIntensity() {
      return(total_decay);
   };

   //--------------------------------------------------------------------------
   // Pick a transition decaying from the level at random, weighted by the
   // intensities. We return its index.
   Transition *PickDepopulatingTransition() {
      double r = G4UniformRand() * GetDecayIntensity(), sum = 0;
      for (unsigned int i = 0; i < GetNTransitions(); i++) {
         Transition *t = GetTransition(i);
         sum += t->GetIntensity();
         if (r < sum) return(t);
      }
      return(NULL);
   };

   //--------------------------------------------------------------------------
   // Show the depopulating transitions of the level
   void Show() {
      double decay = GetDecayIntensity();
      for (unsigned int i = 0; i < transitions.size(); i++) {
         printf("\tTransition: energy = %7.2f keV intensity = %.2f %%\n",
                transitions[i]->GetEnergy() / keV,
                transitions[i]->GetIntensity() * 100. / decay);
      }
   };
};
#endif
