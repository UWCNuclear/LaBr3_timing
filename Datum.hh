// Class to define a set of values, which are represented by an array
// internally, so they can easily be used in root. We will need one instance
// of this class per thread (including the master thread). It handles a certain
// number of values for each detector, with a certain number of detectors.
// Both of these numbers can be set by the calling code.

#ifndef __DATUM_HH__
#define __DATUM_HH__

#include <cstring>

class Datum {

 private:
   double *values;
   unsigned int nperdet;
   unsigned int ndet;
   bool has_data;
   
 public:

   //--------------------------------------------------------------------------
   // Constructor
   Datum(unsigned int ndet_ = 0, unsigned int nperdet_ = 0) {
      values = NULL;
      nperdet = 0;
      ndet = 0;
      has_data = false;
      if (ndet_ || nperdet_) SetDimensions(ndet_, nperdet_);
   };

   //--------------------------------------------------------------------------
   // Destructor
   ~Datum() {
      if (values) delete [] values;
   };

   //--------------------------------------------------------------------------
   // Set the number of values
   void SetDimensions(unsigned int ndet_, unsigned int nperdet_) {

      // If we have already allocated an array, delete it
      if (values) delete [] values;
      values = NULL;

      // Store the parameters
      nperdet = nperdet_;
      ndet = ndet_;

      // If we have values, reserve memory and reset
      if (ndet_ * nperdet_ < 1) return;
      values = new double[ndet_ * nperdet_];
      Reset();
   };
   
   //--------------------------------------------------------------------------
   // Get pointer to the data
   double *GetPointer() {
      return(values);
   };

   //--------------------------------------------------------------------------
   // Get number of data values per detector
   unsigned int GetNPerDetector() {
      return(nperdet);
   };
   
   //--------------------------------------------------------------------------
   // Get number of detectors
   unsigned int GetNDetectors() {
      return(ndet);
   };
   
   //--------------------------------------------------------------------------
   // Reset
   void Reset() {
      memset(values, 0, sizeof(double) * (nperdet * ndet));
      has_data = false;
   };
   
   //--------------------------------------------------------------------------
   // Do we have data?
   bool HasData() {
      return(has_data);
   };
   
   //--------------------------------------------------------------------------
   // Copy data with overloaded = operator
   Datum &operator=(Datum &rhs) {
      unsigned int maxvalues = (ndet * nperdet < rhs.ndet * rhs.nperdet) ?
        ndet * nperdet : rhs.ndet * rhs.nperdet;

      memcpy(values, rhs.values, sizeof(double) * maxvalues);
      return(*this);
   };
   
   //--------------------------------------------------------------------------
   // Set a value
   void SetValue(unsigned int n, unsigned int v, double value_) {
      if (n >= ndet) return;
      if (v >= nperdet) return;
      values[n * nperdet + v] = value_;
      has_data = true;
   };
   
   //--------------------------------------------------------------------------
   // Get photocathode energy
   double GetValue(unsigned int n, unsigned int v) {
      if (n >= ndet) return(0);
      if (v >= nperdet) return(0);
      return(values[n * nperdet + v]);
   };
};

#endif
