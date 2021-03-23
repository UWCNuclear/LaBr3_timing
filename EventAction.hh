#ifndef __EVENT_ACTION_HH__
#define __EVENT_ACTION_HH__

#include <G4UserEventAction.hh>
#include <G4Threading.hh>
#include <G4AutoLock.hh>

#include "Datum.hh"

#include <TTree.h>

//-----------------------------------------------------------------------------
// Class to simulate listmode. We need an array of energies of type double,
// which we pass to the constructor. A root file should already be open for
// writing with a tree created and a branch for the energies
class EventAction : public G4UserEventAction {
 private:
   TTree *tree;
   static G4Mutex mutex;
   Datum *data;
   int ndata;
   
 public:

   //--------------------------------------------------------------------------
   // Constructor
   EventAction(Datum *data_, int ndata_, TTree *tree_) {
      tree = tree_;
      ndata = ndata_;
      data = data_;
   };

   // Destructor
   ~EventAction() {
   };

   //--------------------------------------------------------------------------
   // For each event, we fill the tree
   virtual void EndOfEventAction(const G4Event *) {

      // Get the thread ID + 1 (-1 = master, others 0...N)
      int thread = (G4Threading::G4GetThreadId() + 1);

      // We need to lock here, or root crashes
      G4AutoLock l(&mutex);

      // Copy the data from the thread-specific store to the root-
      // specific store
      data[0] = data[thread];

      // Fill the tree
      tree->Fill();

      // Reset thread-specific data
      data[thread].Reset();
   };
};
G4Mutex EventAction::mutex = G4MUTEX_INITIALIZER;
#endif
