// Class to represent a single transition. We have its energy, intensity and a
// pointer to the level it populates

#ifndef __TRANSITION_HH__
#define __TRANSITION_HH__

// Forward declaration
class Level;

//-----------------------------------------------------------------------------
// Class for a transition
class Transition {

 private:
   
   double energy;    // Energy of the transition
   double intensity; // Intensity of the transition
   Level *final;     // Level populated by the transition

 public:
   
   //--------------------------------------------------------------------------
   // Constructor
   Transition(Level *final_, double intensity_, double energy_) {
      final = final_;
      intensity = intensity_;
      energy = energy_;
   };

   //--------------------------------------------------------------------------
   // Get the intensity of the transition
   inline double GetIntensity() {
      return(intensity);
   };

   //--------------------------------------------------------------------------
   // Get the energy of the transition
   inline double GetEnergy() {
      return(energy);
   };

   //--------------------------------------------------------------------------
   // Get the final level (i.e. the one populated by this transition)
   inline Level *GetFinal() {
      return(final);
   };
};

#endif
