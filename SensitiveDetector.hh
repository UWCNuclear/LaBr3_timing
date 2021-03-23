#ifndef __SENSITIVE_DETECTOR_HH__
#define __SENSITIVE_DETECTOR_HH__

#include <G4VSensitiveDetector.hh>
#include <G4String.hh>
#include <G4HCofThisEvent.hh>
#include <G4Step.hh>
#include <G4TouchableHistory.hh>
#include <G4Threading.hh>
#include <Randomize.hh>

#include <TH1I.h>
#include <TROOT.h>

#include "Datum.hh"

//-----------------------------------------------------------------------------
// This class handles sensitive detectors. For each one, we make a root
// histogram. A root file should be opened beforehand and not closed until
// the instance of this class is destroyed, as the destructor writes it to
// the root file. Optionally, you can set an energy pointer, which tells
// the instance of the class where to store (as a double) the energy in keV
// accumulated during the event. This can be used for listmode, where you
// set the pointer for each detector to point to a different element of an
// array and write that as an n'tuple. The user can set sigma coefficients
// associated with the resolution. A linear interpolation is assumed.
class SensitiveDetector : public G4VSensitiveDetector {

 private:
   Datum *data;           // Data for current event
   double sigma0;         // Offset of sigma
   double sigma1;         // Slope of sigma
   TH1I *h;               // Histogram
   double sumE;           // Energy sum
   double sumT;           // Time
   double sumN;           // Number of hits
   double sumX;           // Sum over X-coordinate
   double sumY;           // Sum over Y-coordinate
   double sumZ;           // Sum over Z-coordinate
   double offT;           // Time offset
   int id;                // Detector ID

 public:
   
   //--------------------------------------------------------------------------
   // Constructor
   SensitiveDetector(G4String name) : G4VSensitiveDetector(name) {
      
      // Create the root histogram and initialise the sigma coefficients
      if (G4Threading::G4GetThreadId() == -1) // For master thread
        h = new TH1I(name, name, 3000, 0, 3000);
      else    // For others, find the one we created already
        h = (TH1I *)gROOT->FindObject(name);

      // Initialise coefficients
      sigma0 = 0;
      sigma1 = 1;
      offT = 0;
   };
   
   //--------------------------------------------------------------------------
   // Destructor
   ~SensitiveDetector() {

      // Do nothing unless master thread
      if (G4Threading::G4GetThreadId() != -1) return;

      // Delete histogram
      delete h;
   };

   //--------------------------------------------------------------------------
   // Set the sigma coefficients for the resolution of the detector
   void SetSigmaCoefficients(double sigma0_, double sigma1_) {
      sigma0 = sigma0_;
      sigma1 = sigma1_;
   };
   
   //--------------------------------------------------------------------------
   // Set time offset
   void SetTimeOffset(double offT_) {
      offT = offT_;
   };
   
   //--------------------------------------------------------------------------
   // Set the pointer to the place where the energy is stored (appropriate
   // to given thread)
   void SetDataPointer(Datum *data_) {
      data = data_;
   };

   //--------------------------------------------------------------------------
   // Set ID
   void SetID(int id_) {
      id = id_;
   };
   
   //--------------------------------------------------------------------------
   // Initialise an event - zero the sums
   void Initialize(G4HCofThisEvent *) {
      sumE = 0;
      sumT = 0;
      sumX = 0;
      sumY = 0;
      sumZ = 0;
      sumN = 0;
   };
   
   //--------------------------------------------------------------------------
   // Process the hits of a step of the event - increment the energy
   G4bool ProcessHits(G4Step *step, G4TouchableHistory *) {

      // Get the pre step point
      G4StepPoint *preStepPoint = step->GetPreStepPoint();
      G4TouchableHandle theTouchable = preStepPoint->GetTouchableHandle();
      G4ThreeVector worldPosition = preStepPoint->GetPosition();
      G4ThreeVector localPosition = theTouchable->GetHistory()->
        GetTopTransform().TransformPoint(worldPosition);

      // Increase sums
      sumE += step->GetTotalEnergyDeposit()/keV; // in keV
      sumT += preStepPoint->GetGlobalTime() / ns * 1000.; // in ps
      sumX += localPosition.x() / mm; // Position in mm
      sumY += localPosition.y() / mm; // Position in mm
      sumZ += localPosition.z() / mm; // Position in mm
      sumN += 1.;
      return(true);
   };
   
   //--------------------------------------------------------------------------
   // End the event - store the energy and histogram it
   void EndOfEvent(G4HCofThisEvent *) {

      // Do nothing if below threshold of 0.01 keV
      if (sumE < 0.01) return;

      // Spread energy over sigma
      sumE = G4RandGauss::shoot(sumE, sigma0 + sigma1 * sumE);

      // Store values in listmode data - item 0 is the energy deposited in the
      // crystal, item 1 is the average time of the interactions, item 2 is
      // the average x-coordinate of the interactions, item 3 for y and item
      // 4 for z.
      data->SetValue(id, 0, sumE); // Sum of energy deposited
      data->SetValue(id, 1, sumT / sumN + offT); // Average time + offset
      data->SetValue(id, 2, sumX / sumN);
      data->SetValue(id, 3, sumY / sumN);
      data->SetValue(id, 4, sumZ / sumN);
      
      // Fill histogram
      h->Fill(sumE);
   };
};

#endif
