
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
      
      adsr = new ADSR( sr, 200, 200, 0.7, 300 );
    }
    
    ADSR* adsr;
    
    void play(int inNote, int vel)
    {
      playingBool = true;
      note = inNote;
      adsr->trigger();
    }
    
    bool playing(){return playingBool;}
    
    void stopIfNoteEquals(int n)
    {
      if ( note == n )
      {
        adsr->release();
      }
    }
    
    
    
    void process( int nframes, float* buffer )
    {
      if( playingBool )
      {
        float accum = 0.f; 
        
        if ( adsr->finished() )
        {
          // turn off voice if ADSR has finished
          playingBool = false;
        }
        
        //return accum * adsr->process(1);
      }
    }
  
  private:
    int sr;
    int note;
    bool playingBool;
};

#endif // FABLA_VOICE_H
