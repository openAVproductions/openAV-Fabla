
#ifndef OPENAV_DSP_COMPRESSOR_H
#define OPENAV_DSP_COMPRESSOR_H


#include <math.h>
#include <cmath>
#include <cstdlib>

/*
template <class T> const T& max (const T& a, const T& b) {
  return (a<b)?b:a;
}

template <class T> const T& min (const T& a, const T& b) {
  return !(b<a)?a:b;
}
*/

class Compressor
{
  public:
    int getNumInputs() { return 2; }
    int getNumOutputs(){ return 2; }
    
    void setAttack(float a)
    {
      // 5 -> 50ms
      attack = 0.005 + a * 0.045;
    }
    
    void setRelease(float r)
    {
      // 5 -> 150ms
      release = 0.005 + r * 0.145;
    }
    
    void setThreshold(float t)
    {
      // -20db -> 0 dB
      threshold = (t * 30) - 30;
    }
    
    void setRatio(float r)
    {
      // 1 -> 20
      ratio = 1 + r * 19;
    }
  
    Compressor(int rate)
    {
      fSamplingFreq = rate;
      
      iConst0 = min(192000, max(1, fSamplingFreq));
      fConst1 = (2.0f / iConst0);
      fConst2 = (1.0f / iConst0);
      
      
      ratio = 1.0f;
      threshold = 0.0f;
      attack = 0.1f;
      release = 0.1f;
      
      fRec0[0] = 0;
      fRec0[1] = 0;
      
      fRec1[0] = 0;
      fRec1[1] = 0;
      
      fRec2[0] = 0;
      fRec2[1] = 0;
    }
    
    void process (int count, float** input, float** output)
    {
      float 	fSlow0 = attack;
      float 	fSlow1 = expf((0 - (fConst1 / fSlow0)));
      float 	fSlow2 = threshold;
      float 	fSlow3 = expf((0 - (fConst2 / fSlow0)));
      float 	fSlow4 = expf((0 - (fConst2 / release)));
      float 	fSlow5 = (1.0f - fSlow4);
      float 	fSlow6 = (1.0f - fSlow3);
      float 	fSlow7 = ((1.0f - fSlow1) * ((1.0f / float(ratio)) - 1));
      float* input0 = input[0];
      float* input1 = input[1];
      float* output0 = output[0];
      float* output1 = output[1];
      
      for (int i=0; i<count; i++) {
        float fTemp0 = (float)input0[i];
        float fTemp1 = (float)input1[i];
        float fTemp2 = fabsf((fabsf(fTemp1) + fabsf(fTemp0)));
        fRec2[0] = ((fSlow5 * fTemp2) + (fSlow4 * max(fTemp2, fRec2[1])));
        fRec1[0] = ((fSlow6 * fRec2[0]) + (fSlow3 * fRec1[1]));
        fRec0[0] = ((fSlow7 * max(((20 * log10f(fRec1[0])) - fSlow2), 0.f)) + (fSlow1 * fRec0[1]));
        float fTemp3 = powf(10,(0.05f * fRec0[0]));
        output0[i] = (float)(fTemp0 * fTemp3);
        output1[i] = (float)(fTemp1 * fTemp3);
        // post processing
        fRec0[1] = fRec0[0];
        fRec1[1] = fRec1[0];
        fRec2[1] = fRec2[0];
      }
    }

  private:
    int fSamplingFreq;
    float attack;
    int   iConst0;
    float fConst1;
    float threshold;
    float fConst2;
    float release;
    float fRec2[2];
    float fRec1[2];
    float ratio;
    float fRec0[2];
};

#endif // OPENAV_DSP_COMPRESSOR_H
