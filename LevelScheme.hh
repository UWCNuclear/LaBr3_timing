// Class to define a level scheme for use by the primary generator. We can't
// use the general particle source, because its absolute times are of the order
// of the half life of the isotope (e.g. years) but we are interested in time
// diffferences of a few picoseconds. Since the time is represented as a double
// we don't have enough precision to do this.

#ifndef __LEVEL_SCHEME_H__
#define __LEVEL_SCHEME_H__

#include <vector>
#include <TString.h>

#include "Transition.hh"
#include "Level.hh"

//-----------------------------------------------------------------------------
// Class for a level scheme
class LevelScheme {

 private:
   std::vector <Level *> levels;            // List of levels
   std::vector <Transition *> transitions;  // List of transitions
   double total_population;                 // Total population from parent

   //--------------------------------------------------------------------------
   // Add a transition using the indices
   void AddTransition(Level *initial, Level *final, double intensity,
                      double energy) {
      if (!initial || !final) return;

      // Create the transition
      Transition *t = new Transition(final, intensity, energy);

      // Add it to the level scheme
      transitions.push_back(t);

      // Add it also to the parent level
      initial->AddTransition(t);
   };

 public:

   //--------------------------------------------------------------------------
   // Constructor
   LevelScheme() {
      total_population = 0;
   };

   //--------------------------------------------------------------------------
   // Add a level to the level scheme
   void AddLevel(double energy, double tau, double population) {
      Level *l = new Level(energy, tau, population);
      levels.push_back(l);
      total_population += population;
   };

   //--------------------------------------------------------------------------
   // Get the closest level to a given energy
   Level *GetLevel(double energy) {
      Level *result = NULL;
      double diff = 1e30;
      for (unsigned int i = 0; i < levels.size(); i++) {
         if (fabs(levels[i]->GetEnergy() - energy) < diff) {
            diff = fabs(levels[i]->GetEnergy() - energy);
            if (diff < 2) result = levels[i];
         }
      }
      return(result);
   };
   
   //--------------------------------------------------------------------------
   // Add a transition given the energies of the levels
   void AddTransition(double E1, double E2, double intensity) {
      Level *initial = GetLevel(E1);
      Level *final = GetLevel(E2);
      if (!initial || !final) return;
      AddTransition(initial, final, intensity, E1 - E2);
   };

   //--------------------------------------------------------------------------
   // Show the level scheme
   void Show() {

      // Loop over levels
      for (unsigned int i = 0; i < levels.size(); i++) {

         // If tau is negative it is stable
         if (levels[i]->GetTau() < 0)
         printf("Level: energy = %8.3f keV stable            population from parent = %.2f %%\n", levels[i]->GetEnergy() / keV,
                levels[i]->GetPopulation() * 100. / total_population);
         else
         printf("Level: energy = %8.3f keV tau = %8.2f ps population from parent = %.2f %%\n", levels[i]->GetEnergy() / keV,
                levels[i]->GetTau() / ns * 1000.,
                levels[i]->GetPopulation() * 100. / total_population);

         // Now show the transitions depopulating that level
         levels[i]->Show();
      }
   };

   //--------------------------------------------------------------------------
   // Pick a level for the primary population at random, weighted by the value
   // of the population given by the user. We return its index.
   Level *PickPrimaryLevel() {
      double r = G4UniformRand() * total_population, sum = 0;
      for (unsigned int i = 0; i < levels.size(); i++) {
         sum += levels[i]->GetPopulation();
         if (r < sum) return(levels[i]);
      }
      return(NULL);
   };


   //--------------------------------------------------------------------------
   // Read the level scheme from a file
   void Read(const char *filename) {

      // Open the file
      FILE *fp = fopen(filename, "r");
      if (!fp) {
         fprintf(stderr, "Unable to read file %s\n", filename);
         return;
      }
      
      // Parse it
      TString st;
      while(st.Gets(fp)) {
         int status;
         double Elev, tau, pop, E1, E2, inten;

         // Try to read a level
         status = sscanf(st.Data(), "level %lf%lf%lf", &Elev, &tau, &pop);
         if (status == 3) AddLevel(Elev * keV, tau * 1e-3 * ns, pop);

         // Try to read a transition
         status = sscanf(st.Data(), "transition %lf%lf%lf", &E1, &E2, &inten);
         if (status == 3) AddTransition(E1 * keV, E2 * keV, inten);
      }

      // Close the file
      fclose(fp);
   }
};

#endif
