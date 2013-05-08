
#include "canvas.hxx"

#include <gtkmm/drawingarea.h>

#include <vector>
#include <sstream>
#include <iostream>

#include <gdkmm.h>
#include <giomm.h>

using namespace std;


void Canvas::drawWaveform(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  int x = 33 + border;
  int y = 74 + 21 + border;
  
  int xSize = 255 - 2 * border;
  int ySize = 138 - 2 * border;
  
  
  // rectangle behind the waveform
  cr->rectangle( x-2, y, xSize+4, ySize );
  setColour( cr, COLOUR_GREY_4 );
  cr->fill();
  
  
  // check for instance access
  if ( true )
  {
    cr->move_to( x, y );
    cr->line_to( x + xSize, y + ySize );
    cr->move_to( x + xSize, y );
    cr->line_to( x, y + ySize );
    setColour(cr, COLOUR_GREY_1, 0.4);
    cr->stroke();
    
    cr->move_to( x + 7.5, y + 84 + 20 );
    setColour( cr, COLOUR_GREY_1 );
    cr->show_text( "Waveform: Work in progress..." );
    
    // outline
    cr->rectangle( x-2, y, xSize+4, ySize );
    setColour( cr, COLOUR_GREY_1 );
    cr->set_line_width(1.1);
    cr->stroke();
    return;
  }
  
  // draw "frequency guides"
  {
    std::valarray< double > dashes(2);
    dashes[0] = 2.0;
    dashes[1] = 2.0;
    cr->set_dash (dashes, 0.0);
    cr->set_line_width(1.0);
    cr->set_source_rgb (0.4,0.4,0.4);
    for ( int i = 0; i < 4; i++ )
    {
      cr->move_to( x + ((xSize / 4.f)*i), y );
      cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
    }
    for ( int i = 0; i < 4; i++ )
    {
      cr->move_to( x       , y + ((ySize / 4.f)*i) );
      cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
    }
    cr->stroke();
    cr->unset_dash();
  }
  
  // draw the waveform here, using dspInstance to get the sample data.
  // We use a mutex that's in the DSP part to serialize access to the
  // sample data, as it can be loaded / freed using the Worker thread in
  // DSP.
  // FIXME: MUTEX
  //g_mutex_lock( &dspInstance->sampleMutex );
  {
    // gather data on the sample were drawing:
    
    if ( dspInstance->sample[selectedSample] )
    {
      
      long   sampleFrames = dspInstance->sample[selectedSample]->info.frames;
      float* current = dspInstance->sample[selectedSample]->data;
      
      if ( sampleFrames == 0 || current == 0 )
      {
        cout << "Warning, GUI attempted drawing sample with 0 frames!" << endl;
        //g_mutex_unlock( &dspInstance->sampleMutex );
        return;
      }
      
      // draw the number of lines that is equal the number of pixels available
      // horizontally. 237 is the pixels horizontally
      int sampleIncrement = sampleFrames / 237;
      
      cr->move_to(x      , y+ySize*0.5);
      cr->line_to(x+xSize, y+ySize*0.5);
      cr->set_line_width(0.8);
      setColour( cr, COLOUR_GREY_1 );
      cr->stroke();
      
      cr->move_to(x      , y+ySize*0.5);
      for (long i = 0; i < sampleFrames; i += sampleIncrement )
      {
        cr->line_to( x + xSize * ( float(i) / sampleFrames), y + ySize * 0.5 + (*current) * ySize * 0.4 );
        current += sampleIncrement;
      }
      
      setColour( cr, COLOUR_ORANGE_1, 1 );
      cr->stroke();
    }
  }
  //g_mutex_unlock( &dspInstance->sampleMutex );
  
  
  if ( sampleNames[selectedSample].compare("") )
  {
    // filename text
    cr->move_to( x + 7.5, y + 84 + 25 );
    setColour( cr, COLOUR_GREY_1 );
    cr->show_text( sampleNames[selectedSample].substr( sampleNames[selectedSample].rfind("/")+1 ) );
  }
  // outline
  cr->rectangle( x-2, y, xSize+4, ySize );
  setColour( cr, COLOUR_GREY_1 );
  cr->set_line_width(1.1);
  cr->stroke();
  
  cr->restore();
}

void Canvas::drawPads(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  int x =  36;
  int y = 217;
  
  cr->rectangle( x-2, y+62, 254, 247 );
  setColour( cr, COLOUR_GREY_4 );
  cr->fill();
  
  // 256 wide, 251 high
  
  float drawY = y + 62*5;
  
  // loop over rest
  for ( int i = 0; i < 16; i++ )
  {
    int X = x + (62*(i%4));
    
    if ( i % 4 == 0)
    {
      drawY -= 62;
    }
    
    if ( padState[i] == PAD_PLAYING )
    {
      cr->set_source (padPlayImageSurface, X, drawY);
      cr->rectangle( X, drawY, padPlayPixbuf->get_width(), padPlayPixbuf->get_height() );
      cr->paint();
    }
    else if ( i == selectedSample ) // selected sample = orange
    {
      cr->set_source (padSelectImageSurface, X, drawY);
      cr->rectangle( X, drawY, padSelectPixbuf->get_width(), padSelectPixbuf->get_height() );
      cr->paint();
    }
    else if ( padState[i] == PAD_LOADED )
    {
      cr->set_source (padLoadImageSurface, X, drawY);
      cr->rectangle( X, drawY, padLoadPixbuf->get_width(), padLoadPixbuf->get_height() );
      cr->paint();
    }
    else
    {
      cr->set_source (padEmptyImageSurface, X, drawY);
      cr->rectangle( X, drawY, padEmptyPixbuf->get_width(), padEmptyPixbuf->get_height() );
      cr->paint();
    }
    
    //cout << "X " << X << "  Y " << drawY << "  " << int(padState[i]) << endl;
  }
  
  
  
  cr->restore();
}

void Canvas::drawMaster(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  int x = 766 + border - 270;
  int y = 330 + border;
  
  int xSize = 158 - 2 * border;
  int ySize = 195 - 2 * border;
  
  cr->set_line_width(1.1);
  
  // LIMITER ZONE
    //graph background
    cr->rectangle( x, y, 76, 76);
    setColour( cr, COLOUR_GREY_4 );
    cr->fill();
    
    {
      int xSize = 76;
      int ySize = 76;
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
    }
    
    // "normal" line
    cr->move_to( x     , y + 76 );
    cr->line_to( x + 76, y      );
    setColour( cr, COLOUR_GREY_1, 0.5 );
    cr->stroke();
    
    // "active" line
    cr->move_to( x , y + 76 - 76 * limiter );
    cr->line_to(x + 76 - 76 * limiter, y  );
    cr->line_to(x + 76, y       );
    cr->line_to(x + 76, y + 76  );
    cr->line_to(x     , y + 76  );
    cr->close_path();
    setColour( cr, COLOUR_BLUE_1, 0.2 );
    cr->fill_preserve();
    setColour( cr, COLOUR_BLUE_1 );
    cr->set_line_width(1.0);
    cr->stroke();
    
    //cr->line_to(x + xSize, y + ySize );
    //cr->line_to(x , y + ySize );
    cr->close_path();
    
    // master limiter graph outline
    cr->rectangle( x, y, 76, 76);
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
  
  // SAMPLE Zone
    // background
    cr->rectangle( x, y + 84, 76, 42);
    setColour( cr, COLOUR_GREY_4 );
    cr->fill();
    
    // text
    cr->move_to( 795-270, 438 );
    setColour( cr, COLOUR_GREY_1 );
    cr->show_text( "Sample" );
    
    // load button
    cr->rectangle( x + 4, y + 84 + 24, 32, 14);
    setColour( cr, COLOUR_GREY_1, 0.5 );
    cr->fill_preserve();
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
    
    // load text
    cr->move_to( x + 7.5, y + 84 + 35 );
    setColour( cr, COLOUR_GREY_4 );
    cr->show_text( "Load" );
    
    // save button
    cr->rectangle( x + 40, y + 84 + 24, 32, 14);
    setColour( cr, COLOUR_GREY_1, 0.5 );
    cr->fill_preserve();
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
    
    // save text
    cr->move_to( x + 43, y + 84 + 35 );
    setColour( cr, COLOUR_GREY_4 );
    cr->show_text( "Save" );
    
    // SAMPLE load save zone outline
    cr->rectangle( x, y + 84, 76, 42);
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
  
  
  // SESSION ZONE
    //load save zone background
    cr->rectangle( x, y + 134, 76, 42);
    setColour( cr, COLOUR_GREY_4 );
    cr->fill();
    
    // text
    cr->move_to( 795-270, 488 );
    setColour( cr, COLOUR_GREY_1 );
    cr->show_text( "Session" );
    
    // load button
    cr->rectangle( x + 4, y + 134 + 24, 32, 14);
    setColour( cr, COLOUR_GREY_1, 0.5 );
    cr->fill_preserve();
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
    
    // load text
    cr->move_to( x + 7.5, y + 134 + 35 );
    setColour( cr, COLOUR_GREY_4 );
    cr->show_text( "Load" );
    
    // save button
    cr->rectangle( x + 40, y + 134 + 24, 32, 14);
    setColour( cr, COLOUR_GREY_1, 0.5 );
    cr->fill_preserve();
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
    
    // save text
    cr->move_to( x + 43, y + 134 + 35 );
    setColour( cr, COLOUR_GREY_4 );
    cr->show_text( "Save" );
    
    // session load save zone outline
    cr->rectangle( x, y + 134, 76, 42);
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
  
  
  // FADER 
    //background
    cr->rectangle( x + xSize * 3 / 5.f, y, xSize * 2 / 5.f, ySize);
    setColour( cr, COLOUR_GREY_4 );
    cr->fill();
    
    // 0dB line
    cr->move_to( x + xSize * 3 / 5.f, y + 2 );
    cr->line_to( x + xSize          , y + 2 );
    setColour( cr, COLOUR_RECORD_RED );
    cr->stroke();
    
    // -10dB line
    cr->move_to( x + xSize * 3 / 5.f, y + 40 );
    cr->line_to( x + xSize          , y + 40 );
    setColour( cr, COLOUR_ORANGE_1 );
    cr->stroke();
    
    // -20dB line
    cr->move_to( x + xSize * 3 / 5.f, y + 76 );
    cr->line_to( x + xSize          , y + 76 );
    setColour( cr, COLOUR_GREEN_1 );
    cr->stroke();
    
    // master volume outline & center separator
    cr->rectangle( x + xSize * 3 / 5.f, y, xSize * 2 / 5.f, ySize + 1);
    cr->move_to( x + xSize * 4 / 5.f, y );
    cr->line_to( x + xSize * 4 / 5.f, y + ySize + 1 );
    setColour( cr, COLOUR_GREY_1 );
    cr->stroke();
    
    // Fader on the top
    cr->rectangle( 878-270, y + ySize*0.87*(1-volume), 16, 24);
    setColour( cr, COLOUR_GREY_4 );
    cr->fill_preserve();
    setColour( cr, COLOUR_BLUE_1);
    cr->set_line_width(1.5);
    cr->stroke();
    cr->set_line_width(1.1);
  
  
  cr->restore();
}





void Canvas::drawRemove(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  int x = 583 + border - 270;
  int y = 330 + border;
  
  int xSize = 158 - 2 * border;
  int ySize = 195 - 2 * border;
  
  // highpass, lowpass graph backgrounds
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_4 );
  cr->fill();
  
  // HIGHPASS
  {
      int x = 593 -270;
      int y = 340;
      
      int xSize = 138;
      int ySize = 175 / 2 - 5;
      
      bool active = highpassActive;
      float cutoff = 0.2 + ((1-highpass)*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      // move to bottom right, draw line to middle right
      cr->move_to( x + xSize, y + ySize );
      cr->line_to( x + xSize, y + (ySize*0.47));
      
      // Curve
      cr->curve_to( x + xSize - (xSize*cutoff)    , y+(ySize*0.5)    ,   // control point 1
                    x + xSize - (xSize*cutoff)    , y+(ySize * 0.0)     ,   // control point 2
                    x + xSize - (xSize*cutoff) -10, y+    ySize     );  // end of curve 1
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1, 0.2 );
      else
        setColour(cr, COLOUR_GREY_1, 0.2 );
      cr->close_path();
      cr->fill_preserve();
      
      // stroke cutoff line
      cr->set_line_width(1.5);
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
  }
  
  // LOWPASS
  {
      int x = 593 -270;
      int y = 340 + 175 / 2 + 5;
      
      int xSize = 138;
      int ySize = 175 / 2 - 5;
      
      
      bool active = lowpassActive;
      float cutoff = 0.2 + (lowpass*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      // move to bottom left, draw line to middle left
      cr->move_to( x , y + ySize );
      cr->line_to( x , y + (ySize*0.47));
      
      // Curve
      cr->curve_to( x + xSize * cutoff    , y+(ySize*0.5)  ,   // control point 1
                    x + xSize * cutoff    , y+(ySize * 0.0),   // control point 2
                    x + xSize * cutoff +10, y+ ySize       );  // end of curve 1
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1, 0.2 );
      else
        setColour(cr, COLOUR_GREY_1, 0.2 );
      cr->close_path();
      cr->fill_preserve();
      
      // stroke cutoff line
      cr->set_line_width(1.5);
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
  }
  
  // highpass, lowpass outline
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_1 );
  cr->set_line_width(1.1);
  cr->stroke();
  
  cr->restore();
}

void Canvas::drawEffect(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  
  int x = 583 + border - 270 + 184;
  int y = 330 + border- 235;
  
  int xSize = 158 - 2 * border;
  int ySize = 195 - 2 * border;
  
  // highpass, lowpass graph backgrounds
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_4 );
  cr->fill();
  
  // Echo
  {
      int ySize = 175 / 2 - 5;
      
      bool active = echoActive;
      float highpass = 0.0;
      float cutoff = 0.2 + ((1-highpass)*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1, 0.2 );
      else
        setColour(cr, COLOUR_GREY_1, 0.2 );
      cr->close_path();
      cr->fill_preserve();
      
      
      int drawX = x + 5;
      // draw echo "sticks"
      for ( int i = 0; i < 40; i++)
      {
        int drawY = y + (ySize-20)*(1-echoAmp)*((i+1.f)/4);
        cr->move_to( drawX, y + ySize);
        cr->line_to( drawX, drawY );
        drawX += 5 + ( (ySize-40)*echoTime );
        
        if ( drawY > y + ySize - 15 || drawX > x + xSize )
          break;
      }
      
      // stroke cutoff line
      cr->set_line_width(1.5);
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
      
            
      // center circle
      cr->arc( x+10+(xSize-20)*echoTime, y+10+(ySize-20)*(1-echoAmp), 6.5, 0, 6.29 );
      if ( active )
        setColour(cr, COLOUR_ORANGE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->set_line_width(2.1);
      cr->stroke();
      
  }
  
  // Reverb
  {
      int y = 330 + border - 235 + 93;
      int ySize = 175 / 2 - 5;
      
      bool active = reverbActive;
      float lowpass = 1.0;
      float cutoff = 0.2 + (lowpass*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      // move to bottom left, draw line to middle left
      cr->move_to( x , y + ySize*0.99 );
      cr->line_to( x + xSize*0.1, y + ySize*0.6 - (ySize*0.6*reverbAmp));
      cr->line_to( x + xSize*0.3+xSize*0.7*reverbTime, y + (ySize*0.99));
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1, 0.2 );
      else
        setColour(cr, COLOUR_GREY_1, 0.2 );
      cr->close_path();
      cr->fill_preserve();
      
      // stroke cutoff line
      cr->set_line_width(1.5);
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
      
      
      // center circle
      cr->arc( x+10+(xSize-20)*reverbTime, y+10+(ySize-20)*(1-reverbAmp), 6.5, 0, 6.29 );
      if ( active )
        setColour(cr, COLOUR_ORANGE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->set_line_width(2.1);
      cr->stroke();
  }
  
  // highpass, lowpass outline
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_1 );
  cr->set_line_width(1.1);
  cr->stroke();
  
  cr->restore();
}

void Canvas::drawSource(Cairo::RefPtr<Cairo::Context> cr)
{
  cr->save();
  
  int border = 10;
  
  int x = 583 + border - 270;
  int y = 330 + border - 235;
  
  int xSize = 158 - 2 * border;
  int ySize = 195 - 2 * border;
  
  // highpass, lowpass graph backgrounds
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_4 );
  cr->fill();
  
  // AD enveloped
  {
      int ySize = 175 / 2 - 5;
      
      bool active = true;
      float highpass = 0.0;
      float cutoff = 0.2 + ((1-highpass)*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      // move to bottom left
      cr->move_to( x+5  , y + ySize );
      
      // line to top, with x + attack
      cr->line_to( x + 5 + ((xSize*0.4)-5)*attack, y+ySize*0.25 + ySize*0.5*((1-gain)+0.5) );
      
      cr->line_to( x + xSize*0.75, y+ ySize*0.25 + ySize*0.5*((1-gain)+0.5) );
      
      cr->line_to( x + xSize - (xSize*0.25*(1-release)), y+ySize );
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1, 0.2 );
      else
        setColour(cr, COLOUR_GREY_1, 0.2 );
      cr->close_path();
      cr->fill_preserve();
      
      // stroke cutoff line
      cr->set_line_width(1.5);
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
      
  }
  
  // Speed / PAN
  {
      int y = 330 + border - 235 + 93;
      int ySize = 175 / 2 - 5;
      bool active = true;
      float lowpass = 1.0;
      float cutoff = 0.2 + (lowpass*0.7f);
      
      // draw "frequency guides"
      std::valarray< double > dashes(2);
      dashes[0] = 2.0;
      dashes[1] = 2.0;
      cr->set_dash (dashes, 0.0);
      cr->set_line_width(1.0);
      cr->set_source_rgb (0.4,0.4,0.4);
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x + ((xSize / 4.f)*i), y );
        cr->line_to( x + ((xSize / 4.f)*i), y + ySize );
      }
      for ( int i = 0; i < 4; i++ )
      {
        cr->move_to( x       , y + ((ySize / 4.f)*i) );
        cr->line_to( x +xSize, y + ((ySize / 4.f)*i) );
      }
      cr->stroke();
      cr->unset_dash();
      
      // speed line
      //cr->move_to( x        , y + ySize*((1-gain)-0.5) );
      //cr->line_to( x + xSize, y + ySize*((1-gain)-0.5) );
      
      // line to top, with x + attack
      cr->move_to( x + (xSize*0.5*speed), y+2 );
      cr->line_to( x + (xSize*0.5*speed), y+ySize );
      
      if ( active )
        setColour(cr, COLOUR_BLUE_1 );
      else
        setColour(cr, COLOUR_GREY_1 );
      cr->stroke();
  }
  
  // highpass, lowpass outline
  cr->rectangle( x, y, xSize, (ySize/2.f) - 5 );
  cr->rectangle( x, y + ySize/2.f + 5, xSize, (ySize/2.f) - 5 );
  setColour( cr, COLOUR_GREY_1 );
  cr->set_line_width(1.1);
  cr->stroke();
  
  cr->restore();
}
