
#ifndef OPENAV_DSP_DBMETER_H
#define OPENAV_DSP_DBMETER_H


#include <math.h>
#include <cmath>
#include <cstdlib>

template <class T> const T& max (const T& a, const T& b) {
  return (a<b)?b:a;
}

template <class T> const T& min (const T& a, const T& b) {
  return !(b<a)?a:b;
}

float fabsf(float dummy0);
float log10f(float dummy0);

class DBMeter
{
  public:
    DBMeter(int rate)
    {
      fSamplingFreq = rate;
      fConst0 = (96.f / float(min(192000, max(1, fSamplingFreq))));
      
      fRec0[0] = -96.f;
      fRec0[1] = -96.f;
    }
    
    int getNumInputs() { return 1;}
    int getNumOutputs() { return 1;}
    
    // call this to get the current dB value, range = -96 -> +10
    float getDB()
    {
      return fvbargraph0;
    }
    
    void process(int count, float** inputs, float** outputs)
    {
      float* input0 = inputs[0];
      float* output0 = outputs[0];
      for (int i = 0; (i < count); i = (i + 1))
      {
        float fTemp0 = float(input0[i]);
        fRec0[0] = max((fRec0[1] - fConst0), min(10.f, (20.f * log10f(max(1.58489e-05f, fabsf(fTemp0))))));
        fvbargraph0 = fRec0[0];
        output0[i] = float(fTemp0);
        fRec0[1] = fRec0[0];
      }
    }
  
  private:
    float fRec0[2];
    int fSamplingFreq;
    float fConst0;
    float fvbargraph0;
};

#endif // OPENAV_DSP_DBMETER_H

