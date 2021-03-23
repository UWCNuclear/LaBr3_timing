#ifndef __USER_ACTION_INITIALIZATION_HH__
#define __USER_ACTION_INITIALIZATION_HH__

#include <G4VUserActionInitialization.hh>

#include "PrimaryGenerator.hh"
#include "EventAction.hh"
#include "Datum.hh"

#include <TTree.h>

class UserActionInitialization : public G4VUserActionInitialization {

 private:
   TTree *tree;
   Datum *data;
   int ndata;
   const char *levelscheme;
   
 public:
   //--------------------------------------------------------------------------
   // Constructor
   UserActionInitialization(Datum *data_, int ndata_, TTree *tree_,
                            const char *levelscheme_) : G4VUserActionInitialization() {
      data = data_;
      ndata = ndata_;
      tree = tree_;
      levelscheme = levelscheme_;
   }

   //--------------------------------------------------------------------------
   // Build method - set up primary generator and event action
   void Build() const {
      SetUserAction(new PrimaryGenerator(levelscheme));
      SetUserAction(new EventAction(data, ndata, tree));
   }
};

#endif
