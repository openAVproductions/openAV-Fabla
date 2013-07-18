/*
 * Author: Harry van Haaren 2013
 *         harryhaaren@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

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
      fRec1[0] = -96.f;
      fRec1[1] = -96.f;
    }
    
    int getNumInputs() { return 2;}
    int getNumOutputs(){ return 2;}
    
    // call this to get the current dB value, range = -96 -> +10
    float getLeftDB()
    {
      return fvbargraph0;
    }
    float getRightDB()
    {
      return fvbargraph1;
    }
    
    void process(int count, float** inputs, float** outputs)
    {
      float* input0 = inputs[0];
      float* input1 = inputs[1];
      float* output0 = outputs[0];
      float* output1 = outputs[1];
      for (int i = 0; (i < count); i = (i + 1))
      {
        float fTemp0 = float(input0[i]);
        fRec0[0] = max((fRec0[1] - fConst0), min(10.f, (20.f * log10f(max(1.58489e-05f, fabsf(fTemp0))))));
        fvbargraph0 = fRec0[0];
        output0[i] = float(fTemp0);
        
        float fTemp1 = float(input1[i]);
        fRec1[0] = max((fRec1[1] - fConst0), min(10.f, (20.f * log10f(max(1.58489e-05f, fabsf(fTemp1))))));
        fvbargraph1 = fRec1[0];
        output1[i] = float(fTemp1);
        
        fRec0[1] = fRec0[0];
        fRec1[1] = fRec1[0];
      }
    }
  
  private:
    float fRec0[2];
    float fRec1[2];
    int fSamplingFreq;
    float fConst0;
    float fvbargraph0;
    float fvbargraph1;
};

#endif // OPENAV_DSP_DBMETER_H

