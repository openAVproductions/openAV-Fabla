
#ifndef FABLA_VOICE_H
#define FABLA_VOICE_H

#include "adsr.hxx"
#include "denormals.hxx"

class Voice
{
  public:
    Voice(int rate)
    {
      sr = rate;
      playingBool = 0;
      
      adsr[ADSR_FREQ]   = new ADSR( sr, 200, 200, 0.7, 300 );
      adsr[ADSR_VOL]    = new ADSR( sr, 200, 200, 0.7, 300 );
      adsr[ADSR_FILTER] = new ADSR( sr, 100, 100, 0.7, 500 );
    }
    
    ADSR* adsr[3];
    
    void play(int inNote, int vel)
    {
      playingBool = true;
      note = inNote;
      
      adsr[ADSR_FREQ]->trigger();
      adsr[ADSR_VOL]->trigger();
      adsr[ADSR_FILTER]->trigger();
    }
    
    bool playing(){return playingBool;}
    
    void stopIfNoteEquals(int n)
    {
      if ( note == n )
      {
        adsr[ADSR_VOL]->release();
        adsr[ADSR_FILTER]->release();
      }
    }
    
    
    
    float process(int nframes )
    {
      if( playingBool )
      {
        // split here based on osc[0]->enabled() for CPU saving
        float accum = 0.f; 
        
        if ( adsr[ADSR_VOL]->finished() )
        {
          // turn off voice if ADSR has finished
          playingBool = false;
        }
        
        return accum * adsr[ADSR_VOL]->process(1);
      }
      return 0.f;
    }
  
  private:
    enum ADSR_ENUM {
      ADSR_FILTER = 0,
      ADSR_FREQ,
      ADSR_VOL,
    };
    
    int sr;
    int note;
    bool playingBool;
};

#endif // FABLA_VOICE_H
