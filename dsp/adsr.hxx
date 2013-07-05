
#ifndef OPENAV_DSP_ADSR_H
#define OPENAV_DSP_ADSR_H

#include <stdio.h>

/// define max / min values for the ADSR in milliseconds

// Adsr class, where 0-1 is the output
class ADSR
{
  public:
    /// setup ADSR with samplerate (Hz), attack decay and release in seconds (float),
    /// and sustain as a value between [0-1]
    ADSR(int srate, float attackMS, float decayMS, float sustain01, float releaseMS )
    {
      sr = srate;
      
      att = attackMS * sr;
      dec = decayMS * sr;
      sus = sustain01;
      rel = releaseMS * sr;
      
      //printf("%f, %f, %f", attack, decay, sustain);
      
      finish = true;
      
      w = 10.0f / (srate * 1); // t = 1? // FS = sample rate
      a = 0.07f;               // Controls damping
      b = 1.0f / (1.0f - a);
      g1 = g2 = 0.f;
      
      progress = att + dec + rel;
      released = true;
      
      // by default there is no lowpass on the output to avoid sudden value jumps
      enableSmoothing = false;
    }
    
    /// set attack in milliseconds
    void attack(float a)
    {
      att = (a + 0.01) * sr;
    }
    
    /// set decay in milliseconds
    void decay(float d)
    {
      dec = (d + 0.01) * sr;
    }
    
    /// set sustain value, range 0 to 1
    void sustain(float s)
    {
      sus = s;
    }
    
    /// set release in milliseconds
    void release(float r)
    {
      rel = r * sr;
    }
    
    void trigger()
    {
      progress = 0;
      released = false;
      finish = false;
    }
    
    void release()
    {
      progress = att + dec;
      released = true;
    }
    
    bool finished()
    {
      return finish;
    }
    
    void setSmoothing(bool s)
    {
      enableSmoothing = s;
    }
    
    float process(int nframes)
    {
      progress += nframes;
      
      float output = 0.f;
      
      if ( progress < att ) // ATTACK
      {
        output = (progress / att);
      }
      else if ( progress < att + dec ) // DECAY
      {
        output = 1 - (1 - sus) * ((progress - att) / dec);
      }
      else if ( released && progress > att + dec && progress < att + dec + rel )
      {
        output = sus - ((sus) * (progress - (att+dec)) / rel);
      }
      else if ( !released )
      {
        output = sus;
      }
      else
      {
        finish = true; // note end of sound: so can stop voice
      }
      
      
      if ( enableSmoothing )
      {
        g1 += w * (output - g1 - a * g2 - 1e-20f);
        g2 += w * (b * g1 - g2 + 1e-20f);
      }
      else
      {
        return output;
      }
      
      return g2;
    }
  
  private:
    int sr;
    
    enum State {
      STATE_ATTACK,
      STATE_DECAY,
      STATE_SUSTAIN,
      STATE_RELEASE,
    };
    
    // smoothing filter state
    bool enableSmoothing;
    float w, a, b, g1, g2;
    
    float att, dec, rel;
    bool released;
    float sus;
    bool finish;
    
    float progress;
};

#endif // OPENAV_DSP_ADSR_H
