#ifdef G4MULTITHREADED
#include <G4MTRunManager.hh>
#else
#include <G4RunManager.hh>
#endif
#include <G4UIterminal.hh>
#include <G4UItcsh.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>
#include <Randomize.hh>
  
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TString.h>

#include <cstdio>
#include <ctime>

#include "Datum.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "UserActionInitialization.hh"

//-----------------------------------------------------------------------------
int main(int argc, char **argv) {

   int c, nthreads = 3, ndet = 6;
   const char *filename = "LaBr_timing.root";
   const char *levelscheme = "levelscheme.dat";
   extern char *optarg;
   bool visualise = false;
   
   // Set random number generator to Ranlux
   G4Random::setTheEngine(new CLHEP::RanluxEngine);
   G4Random::setTheSeed((long)time(NULL), 3);
   G4Random::showEngineStatus();

   // Handle arguments
   while(1) {
      c = getopt(argc, argv, "l:n:o:t:v");
      if (c == -1) break;

      switch(c) {
       case 'l': // Level scheme
         levelscheme = optarg;
         break;
       case 'n': // Number of detectors
         ndet = atoi(optarg);
         break;
       case 'o': // Output root file
         filename = optarg;
         break;
       case 't': // Number of threads
         nthreads = atoi(optarg);
         break;
       case 'v': // Turn on visualisation
         visualise = true;
         break;
       default:
         fprintf(stderr, "Usage: %s [-l levelscheme] [-n number_of_detectors] [-o output_rootfile] [-t nthreads] [-v]\n", argv[0]);
         exit(-1);
         break;
      }
   }

   // Open a root file
   TFile *f = TFile::Open(filename, "recreate");
   
   // Create and set up a run manager
#ifdef G4MULTITHREADED
   G4MTRunManager *run_manager = new G4MTRunManager();

   // Set number of threads
   run_manager->SetNumberOfThreads(nthreads);
#else
   G4RunManager *run_manager = new G4RunManager();
#endif

   // Reserve memory (one Datum per thread, including the master thread)
   // Each datum should have space for energy and time for each detector
   Datum *data = new Datum[run_manager->GetNumberOfThreads() + 1];
   for (int i = 0; i < run_manager->GetNumberOfThreads() + 1; i++)
     data[i].SetDimensions(ndet, 5);
   
   // Create a tree   
   TTree *tree = new TTree("g4", "geant4 tree");

   // Create branch
   tree->Branch("values", data[0].GetPointer(),
    Form("values[%d]/D", data[0].GetNDetectors() * data[0].GetNPerDetector()));

   // Set initialisation of run manager
   run_manager->SetUserInitialization(new DetectorConstruction(data, ndet));
   run_manager->SetUserInitialization(new PhysicsList());
   run_manager->SetUserInitialization(new UserActionInitialization(data, ndet,
                                                                   tree,
                                                                   levelscheme));
   run_manager->Initialize();

   // Get the user interface manager
   G4UImanager *UImanager = G4UImanager::GetUIpointer();

   // If the user specified visualisation, create a visualisation manager
   // and start it. Otherwise, if the user specified the number of events,
   // run them and quit, and if not, start an interactive text-based sesssion.
   if (visualise) {

      // Create a new visualisation manager
      G4VisManager *vis_manager = new G4VisExecutive();
      vis_manager->Initialize();

      // Create interactive GUI session
      G4UIExecutive *ui = new G4UIExecutive(argc, argv);
      UImanager->ExecuteMacroFile("init_gui.mac");

      // Switch to interactive
      ui->SessionStart();

      // Delete interactive session
      delete ui;

      // Delete the visualisation manager
      delete vis_manager;
   } else {

      // Create an interactive session
      G4UIterminal *ui = new G4UIterminal(new G4UItcsh);
      ui->SetPrompt("LaBr_timing> ");

      // Initialise terminal
      UImanager->ExecuteMacroFile("init_terminal.mac");

      // Switch to interactive
      ui->SessionStart();

      // Delete interactive session
      delete ui;
   }

   // Write tree and all histograms
   f->Write();

   // Clean up - this implicitly deletes the detector construction, physics
   // list, primary generator and sensitive detector, so do not do this
   // explictly or we will get a "double free" error.
   delete run_manager;

   delete [] data;

   // Close root file
   f->Close();
}
