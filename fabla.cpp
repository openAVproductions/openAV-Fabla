//-----------------------------------------------------
//
// Code generated with Faust 0.9.46 (http://faust.grame.fr)
//-----------------------------------------------------
/* link with  */
#include <math.h>
#include <cmath>
template <int N> inline float faustpower(float x) 		{ return powf(x,N); } 
template <int N> inline double faustpower(double x) 	{ return pow(x,N); }
template <int N> inline int faustpower(int x) 			{ return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <> 	 inline int faustpower<0>(int x) 		{ return 1; }
template <> 	 inline int faustpower<1>(int x) 		{ return x; }
/************************************************************************

	IMPORTANT NOTE : this file contains two clearly delimited sections : 
	the ARCHITECTURE section (in two parts) and the USER section. Each section 
	is governed by its own copyright and license. Please check individually 
	each section for license and copyright information.
*************************************************************************/

/*******************BEGIN ARCHITECTURE SECTION (part 1/2)****************/

/************************************************************************
    FAUST Architecture File
	Copyright (C) 2003-2011 GRAME, Centre National de Creation Musicale
    ---------------------------------------------------------------------
    This Architecture section is free software; you can redistribute it 
    and/or modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either version 3 of 
	the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License 
	along with this program; If not, see <http://www.gnu.org/licenses/>.

	EXCEPTION : As a special exception, you may create a larger work 
	that contains this FAUST architecture section and distribute  
	that work under terms of your choice, so long as this FAUST 
	architecture section is not modified. 


 ************************************************************************
 ************************************************************************/

#include <libgen.h>
#include <math.h>
#include <iostream>

#include "faust/cpp_ui.h"
#include "faust/misc.h"
#include "faust/dsp.h"

using namespace std;


/******************************************************************************
*******************************************************************************

							       VECTOR INTRINSICS

*******************************************************************************
*******************************************************************************/


/******************************************************************************
*******************************************************************************

								USER INTERFACE

*******************************************************************************
/**************************BEGIN USER SECTION **************************/
		
#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif  

typedef long double quad;

#define FAUSTCLASS FablaDSP

class FablaDSP : public dsp {
  private:
	FAUSTFLOAT 	fslider0;
	int 	iConst0;
	float 	fConst1;
	float 	fConst2;
	FAUSTFLOAT 	fslider1;
	float 	fConst3;
	FAUSTFLOAT 	fslider2;
	float 	fConst4;
	float 	fRec13[2];
	FAUSTFLOAT 	fslider3;
	float 	fRec12[2];
	int 	IOTA;
	float 	fVec0[8192];
	float 	fConst5;
	int 	iConst6;
	FAUSTFLOAT 	fslider4;
	float 	fConst7;
	FAUSTFLOAT 	fslider5;
	float 	fConst8;
	FAUSTFLOAT 	fslider6;
	float 	fConst9;
	float 	fRec15[2];
	FAUSTFLOAT 	fslider7;
	float 	fRec18[2];
	float 	fRec20[3];
	float 	fRec19[3];
	float 	fRec17[3];
	float 	fRec16[3];
	float 	fRec14[131072];
	FAUSTFLOAT 	fslider8;
	float 	fVec1[2048];
	int 	iConst10;
	float 	fRec10[2];
	float 	fConst11;
	float 	fConst12;
	float 	fRec24[2];
	float 	fRec23[2];
	float 	fVec2[8192];
	float 	fConst13;
	int 	iConst14;
	float 	fVec3[1024];
	int 	iConst15;
	float 	fRec21[2];
	float 	fConst16;
	float 	fConst17;
	float 	fRec28[2];
	float 	fRec27[2];
	float 	fVec4[8192];
	float 	fConst18;
	int 	iConst19;
	float 	fVec5[2048];
	int 	iConst20;
	float 	fRec25[2];
	float 	fConst21;
	float 	fConst22;
	float 	fRec32[2];
	float 	fRec31[2];
	float 	fVec6[8192];
	float 	fConst23;
	int 	iConst24;
	float 	fVec7[1024];
	int 	iConst25;
	float 	fRec29[2];
	float 	fConst26;
	float 	fConst27;
	float 	fRec36[2];
	float 	fRec35[2];
	float 	fVec8[16384];
	float 	fConst28;
	int 	iConst29;
	float 	fVec9[2048];
	int 	iConst30;
	float 	fRec33[2];
	float 	fConst31;
	float 	fConst32;
	float 	fRec40[2];
	float 	fRec39[2];
	float 	fVec10[8192];
	float 	fConst33;
	int 	iConst34;
	float 	fVec11[2048];
	int 	iConst35;
	float 	fRec37[2];
	float 	fConst36;
	float 	fConst37;
	float 	fRec44[2];
	float 	fRec43[2];
	float 	fVec12[16384];
	float 	fConst38;
	int 	iConst39;
	float 	fVec13[2048];
	int 	iConst40;
	float 	fRec41[2];
	float 	fConst41;
	float 	fConst42;
	float 	fRec48[2];
	float 	fRec47[2];
	float 	fVec14[16384];
	float 	fConst43;
	int 	iConst44;
	float 	fVec15[1024];
	int 	iConst45;
	float 	fRec45[2];
	float 	fRec2[3];
	float 	fRec3[3];
	float 	fRec4[3];
	float 	fRec5[3];
	float 	fRec6[3];
	float 	fRec7[3];
	float 	fRec8[3];
	float 	fRec9[3];
	FAUSTFLOAT 	fslider9;
	float 	fConst46;
	FAUSTFLOAT 	fslider10;
	float 	fRec1[3];
	FAUSTFLOAT 	fslider11;
	FAUSTFLOAT 	fslider12;
	float 	fRec0[3];
	FAUSTFLOAT 	fslider13;
	float 	fRec49[2];
	FAUSTFLOAT 	fslider14;
	float 	fRec50[2];
	float 	fRec52[3];
	float 	fRec51[3];
  public:
	static void metadata(Meta* m) 	{ 
		m->declare("effect.lib/name", "Faust Audio Effect Library");
		m->declare("effect.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("effect.lib/copyright", "Julius O. Smith III");
		m->declare("effect.lib/version", "1.33");
		m->declare("effect.lib/license", "STK-4.3");
		m->declare("effect.lib/reference", "https://ccrma.stanford.edu/realsimple/faust_strings/");
		m->declare("filter.lib/name", "Faust Filter Library");
		m->declare("filter.lib/author", "Julius O. Smith (jos at ccrma.stanford.edu)");
		m->declare("filter.lib/copyright", "Julius O. Smith III");
		m->declare("filter.lib/version", "1.29");
		m->declare("filter.lib/license", "STK-4.3");
		m->declare("filter.lib/reference", "https://ccrma.stanford.edu/~jos/filters/");
		m->declare("music.lib/name", "Music Library");
		m->declare("music.lib/author", "GRAME");
		m->declare("music.lib/copyright", "GRAME");
		m->declare("music.lib/version", "1.0");
		m->declare("music.lib/license", "LGPL");
		m->declare("math.lib/name", "Math Library");
		m->declare("math.lib/author", "GRAME");
		m->declare("math.lib/copyright", "GRAME");
		m->declare("math.lib/version", "1.0");
		m->declare("math.lib/license", "LGPL");
	}

	virtual int getNumInputs() 	{ return 1; }
	virtual int getNumOutputs() 	{ return 2; }
	static void classInit(int samplingFreq) {
	}
	virtual void instanceInit(int samplingFreq) {
		fSamplingFreq = samplingFreq;
		fslider0 = 2.0f;
		iConst0 = min(192000, max(1, fSamplingFreq));
		fConst1 = floorf((0.5f + (0.174713f * iConst0)));
		fConst2 = ((0 - (6.907755278982138f * fConst1)) / iConst0);
		fslider1 = 6e+03f;
		fConst3 = (6.283185307179586f / float(iConst0));
		fslider2 = 2e+02f;
		fConst4 = (3.141592653589793f / iConst0);
		for (int i=0; i<2; i++) fRec13[i] = 0;
		fslider3 = 3.0f;
		for (int i=0; i<2; i++) fRec12[i] = 0;
		IOTA = 0;
		for (int i=0; i<8192; i++) fVec0[i] = 0;
		fConst5 = floorf((0.5f + (0.022904f * iConst0)));
		iConst6 = int((int((fConst1 - fConst5)) & 8191));
		fslider4 = 0.0f;
		fConst7 = (0.001f * iConst0);
		fslider5 = 0.0f;
		fConst8 = expf((0 - (2e+01f / iConst0)));
		fslider6 = 2e+04f;
		fConst9 = (1.0f - fConst8);
		for (int i=0; i<2; i++) fRec15[i] = 0;
		fslider7 = 1e+01f;
		for (int i=0; i<2; i++) fRec18[i] = 0;
		for (int i=0; i<3; i++) fRec20[i] = 0;
		for (int i=0; i<3; i++) fRec19[i] = 0;
		for (int i=0; i<3; i++) fRec17[i] = 0;
		for (int i=0; i<3; i++) fRec16[i] = 0;
		for (int i=0; i<131072; i++) fRec14[i] = 0;
		fslider8 = 6e+01f;
		for (int i=0; i<2048; i++) fVec1[i] = 0;
		iConst10 = int((int((fConst5 - 1)) & 2047));
		for (int i=0; i<2; i++) fRec10[i] = 0;
		fConst11 = floorf((0.5f + (0.153129f * iConst0)));
		fConst12 = ((0 - (6.907755278982138f * fConst11)) / iConst0);
		for (int i=0; i<2; i++) fRec24[i] = 0;
		for (int i=0; i<2; i++) fRec23[i] = 0;
		for (int i=0; i<8192; i++) fVec2[i] = 0;
		fConst13 = floorf((0.5f + (0.020346f * iConst0)));
		iConst14 = int((int((fConst11 - fConst13)) & 8191));
		for (int i=0; i<1024; i++) fVec3[i] = 0;
		iConst15 = int((int((fConst13 - 1)) & 1023));
		for (int i=0; i<2; i++) fRec21[i] = 0;
		fConst16 = floorf((0.5f + (0.127837f * iConst0)));
		fConst17 = ((0 - (6.907755278982138f * fConst16)) / iConst0);
		for (int i=0; i<2; i++) fRec28[i] = 0;
		for (int i=0; i<2; i++) fRec27[i] = 0;
		for (int i=0; i<8192; i++) fVec4[i] = 0;
		fConst18 = floorf((0.5f + (0.031604f * iConst0)));
		iConst19 = int((int((fConst16 - fConst18)) & 8191));
		for (int i=0; i<2048; i++) fVec5[i] = 0;
		iConst20 = int((int((fConst18 - 1)) & 2047));
		for (int i=0; i<2; i++) fRec25[i] = 0;
		fConst21 = floorf((0.5f + (0.125f * iConst0)));
		fConst22 = ((0 - (6.907755278982138f * fConst21)) / iConst0);
		for (int i=0; i<2; i++) fRec32[i] = 0;
		for (int i=0; i<2; i++) fRec31[i] = 0;
		for (int i=0; i<8192; i++) fVec6[i] = 0;
		fConst23 = floorf((0.5f + (0.013458f * iConst0)));
		iConst24 = int((int((fConst21 - fConst23)) & 8191));
		for (int i=0; i<1024; i++) fVec7[i] = 0;
		iConst25 = int((int((fConst23 - 1)) & 1023));
		for (int i=0; i<2; i++) fRec29[i] = 0;
		fConst26 = floorf((0.5f + (0.210389f * iConst0)));
		fConst27 = ((0 - (6.907755278982138f * fConst26)) / iConst0);
		for (int i=0; i<2; i++) fRec36[i] = 0;
		for (int i=0; i<2; i++) fRec35[i] = 0;
		for (int i=0; i<16384; i++) fVec8[i] = 0;
		fConst28 = floorf((0.5f + (0.024421f * iConst0)));
		iConst29 = int((int((fConst26 - fConst28)) & 16383));
		for (int i=0; i<2048; i++) fVec9[i] = 0;
		iConst30 = int((int((fConst28 - 1)) & 2047));
		for (int i=0; i<2; i++) fRec33[i] = 0;
		fConst31 = floorf((0.5f + (0.192303f * iConst0)));
		fConst32 = ((0 - (6.907755278982138f * fConst31)) / iConst0);
		for (int i=0; i<2; i++) fRec40[i] = 0;
		for (int i=0; i<2; i++) fRec39[i] = 0;
		for (int i=0; i<8192; i++) fVec10[i] = 0;
		fConst33 = floorf((0.5f + (0.029291f * iConst0)));
		iConst34 = int((int((fConst31 - fConst33)) & 8191));
		for (int i=0; i<2048; i++) fVec11[i] = 0;
		iConst35 = int((int((fConst33 - 1)) & 2047));
		for (int i=0; i<2; i++) fRec37[i] = 0;
		fConst36 = floorf((0.5f + (0.256891f * iConst0)));
		fConst37 = ((0 - (6.907755278982138f * fConst36)) / iConst0);
		for (int i=0; i<2; i++) fRec44[i] = 0;
		for (int i=0; i<2; i++) fRec43[i] = 0;
		for (int i=0; i<16384; i++) fVec12[i] = 0;
		fConst38 = floorf((0.5f + (0.027333f * iConst0)));
		iConst39 = int((int((fConst36 - fConst38)) & 16383));
		for (int i=0; i<2048; i++) fVec13[i] = 0;
		iConst40 = int((int((fConst38 - 1)) & 2047));
		for (int i=0; i<2; i++) fRec41[i] = 0;
		fConst41 = floorf((0.5f + (0.219991f * iConst0)));
		fConst42 = ((0 - (6.907755278982138f * fConst41)) / iConst0);
		for (int i=0; i<2; i++) fRec48[i] = 0;
		for (int i=0; i<2; i++) fRec47[i] = 0;
		for (int i=0; i<16384; i++) fVec14[i] = 0;
		fConst43 = floorf((0.5f + (0.019123f * iConst0)));
		iConst44 = int((int((fConst41 - fConst43)) & 16383));
		for (int i=0; i<1024; i++) fVec15[i] = 0;
		iConst45 = int((int((fConst43 - 1)) & 1023));
		for (int i=0; i<2; i++) fRec45[i] = 0;
		for (int i=0; i<3; i++) fRec2[i] = 0;
		for (int i=0; i<3; i++) fRec3[i] = 0;
		for (int i=0; i<3; i++) fRec4[i] = 0;
		for (int i=0; i<3; i++) fRec5[i] = 0;
		for (int i=0; i<3; i++) fRec6[i] = 0;
		for (int i=0; i<3; i++) fRec7[i] = 0;
		for (int i=0; i<3; i++) fRec8[i] = 0;
		for (int i=0; i<3; i++) fRec9[i] = 0;
		fslider9 = 315.0f;
		fConst46 = (6.283185307179586f / iConst0);
		fslider10 = 0.0f;
		for (int i=0; i<3; i++) fRec1[i] = 0;
		fslider11 = 315.0f;
		fslider12 = 0.0f;
		for (int i=0; i<3; i++) fRec0[i] = 0;
		fslider13 = 0.0f;
		for (int i=0; i<2; i++) fRec49[i] = 0;
		fslider14 = -2e+01f;
		for (int i=0; i<2; i++) fRec50[i] = 0;
		for (int i=0; i<3; i++) fRec52[i] = 0;
		for (int i=0; i<3; i++) fRec51[i] = 0;
	}
	virtual void init(int samplingFreq) {
		classInit(samplingFreq);
		instanceInit(samplingFreq);
	}
	virtual void buildUserInterface(UI* interface) {
		interface->openVerticalBox("fabla");
		interface->declare(0, "0", "");
		interface->declare(0, "tooltip", "~ ZITA REV1 FEEDBACK DELAY NETWORK (FDN) & SCHROEDER ALLPASS-COMB REVERBERATOR (8x8). See Faust's effect.lib for documentation and references");
		interface->openHorizontalBox("Zita_Rev1");
		interface->declare(0, "1", "");
		interface->openHorizontalBox("Input");
		interface->declare(&fslider8, "1", "");
		interface->declare(&fslider8, "style", "knob");
		interface->declare(&fslider8, "tooltip", "Delay in ms before reverberation begins");
		interface->declare(&fslider8, "unit", "ms");
		interface->addVerticalSlider("In Delay", &fslider8, 6e+01f, 2e+01f, 1e+02f, 1.0f);
		interface->closeBox();
		interface->declare(0, "2", "");
		interface->openHorizontalBox("Decay Times in Bands (see tooltips)");
		interface->declare(&fslider2, "1", "");
		interface->declare(&fslider2, "style", "knob");
		interface->declare(&fslider2, "tooltip", "Crossover frequency (Hz) separating low and middle frequencies");
		interface->declare(&fslider2, "unit", "Hz");
		interface->addVerticalSlider("LF X", &fslider2, 2e+02f, 5e+01f, 1e+03f, 1.0f);
		interface->declare(&fslider3, "2", "");
		interface->declare(&fslider3, "style", "knob");
		interface->declare(&fslider3, "tooltip", "T60 = time (in seconds) to decay 60dB in low-frequency band");
		interface->declare(&fslider3, "unit", "s");
		interface->addVerticalSlider("Low RT60", &fslider3, 3.0f, 1.0f, 8.0f, 0.1f);
		interface->declare(&fslider0, "3", "");
		interface->declare(&fslider0, "style", "knob");
		interface->declare(&fslider0, "tooltip", "T60 = time (in seconds) to decay 60dB in middle band");
		interface->declare(&fslider0, "unit", "s");
		interface->addVerticalSlider("Mid RT60", &fslider0, 2.0f, 1.0f, 8.0f, 0.1f);
		interface->declare(&fslider1, "4", "");
		interface->declare(&fslider1, "style", "knob");
		interface->declare(&fslider1, "tooltip", "Frequency (Hz) at which the high-frequency T60 is half the middle-band's T60");
		interface->declare(&fslider1, "unit", "Hz");
		interface->addVerticalSlider("HF Damping", &fslider1, 6e+03f, 1.5e+03f, 2.352e+04f, 1.0f);
		interface->closeBox();
		interface->declare(0, "3", "");
		interface->openHorizontalBox("RM Peaking Equalizer 1");
		interface->declare(&fslider9, "1", "");
		interface->declare(&fslider9, "style", "knob");
		interface->declare(&fslider9, "tooltip", "Center-frequency of second-order Regalia-Mitra peaking equalizer section 1");
		interface->declare(&fslider9, "unit", "Hz");
		interface->addVerticalSlider("Eq1 Freq", &fslider9, 315.0f, 4e+01f, 2.5e+03f, 1.0f);
		interface->declare(&fslider10, "2", "");
		interface->declare(&fslider10, "style", "knob");
		interface->declare(&fslider10, "tooltip", "Peak level in dB of second-order Regalia-Mitra peaking equalizer section 1");
		interface->declare(&fslider10, "unit", "dB");
		interface->addVerticalSlider("Eq1 Level", &fslider10, 0.0f, -15.0f, 15.0f, 0.1f);
		interface->closeBox();
		interface->declare(0, "4", "");
		interface->openHorizontalBox("RM Peaking Equalizer 2");
		interface->declare(&fslider11, "1", "");
		interface->declare(&fslider11, "style", "knob");
		interface->declare(&fslider11, "tooltip", "Center-frequency of second-order Regalia-Mitra peaking equalizer section 2");
		interface->declare(&fslider11, "unit", "Hz");
		interface->addVerticalSlider("Eq2 Freq", &fslider11, 315.0f, 4e+01f, 2.5e+03f, 1.0f);
		interface->declare(&fslider12, "2", "");
		interface->declare(&fslider12, "style", "knob");
		interface->declare(&fslider12, "tooltip", "Peak level in dB of second-order Regalia-Mitra peaking equalizer section 2");
		interface->declare(&fslider12, "unit", "dB");
		interface->addVerticalSlider("Eq2 Level", &fslider12, 0.0f, -15.0f, 15.0f, 0.1f);
		interface->closeBox();
		interface->declare(0, "5", "");
		interface->openHorizontalBox("Output");
		interface->declare(&fslider13, "1", "");
		interface->declare(&fslider13, "style", "knob");
		interface->declare(&fslider13, "tooltip", "-1 = dry, 1 = wet");
		interface->addVerticalSlider("Dry/Wet Mix", &fslider13, 0.0f, -1.0f, 1.0f, 0.01f);
		interface->declare(&fslider14, "2", "");
		interface->declare(&fslider14, "style", "knob");
		interface->declare(&fslider14, "tooltip", "Output scale factor");
		interface->declare(&fslider14, "unit", "dB");
		interface->addVerticalSlider("Level", &fslider14, -2e+01f, -7e+01f, 4e+01f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->addHorizontalSlider("hipcutoff", &fslider7, 1e+01f, 1e+01f, 4e+03f, 0.1f);
		interface->addHorizontalSlider("lopcutoff", &fslider6, 2e+04f, 1e+02f, 2e+04f, 0.1f);
		interface->openVerticalBox("stereoecho");
		interface->openVerticalBox("echo  1000");
		interface->addHorizontalSlider("feedback", &fslider5, 0.0f, 0.0f, 1e+02f, 0.1f);
		interface->addHorizontalSlider("millisecond", &fslider4, 0.0f, 0.0f, 1e+03f, 0.1f);
		interface->closeBox();
		interface->closeBox();
		interface->closeBox();
	}
	virtual void compute (int count, FAUSTFLOAT** input, FAUSTFLOAT** output) {
		float 	fSlow0 = fslider0;
		float 	fSlow1 = expf((fConst2 / fSlow0));
		float 	fSlow2 = faustpower<2>(fSlow1);
		float 	fSlow3 = (1.0f - fSlow2);
		float 	fSlow4 = cosf((fConst3 * fslider1));
		float 	fSlow5 = (1.0f - (fSlow4 * fSlow2));
		float 	fSlow6 = sqrtf(max(0, ((faustpower<2>(fSlow5) / faustpower<2>(fSlow3)) - 1.0f)));
		float 	fSlow7 = (fSlow5 / fSlow3);
		float 	fSlow8 = (fSlow7 - fSlow6);
		float 	fSlow9 = (1.0f / tanf((fConst4 * fslider2)));
		float 	fSlow10 = (1 + fSlow9);
		float 	fSlow11 = (0 - ((1 - fSlow9) / fSlow10));
		float 	fSlow12 = (1.0f / fSlow10);
		float 	fSlow13 = fslider3;
		float 	fSlow14 = ((expf((fConst2 / fSlow13)) / fSlow1) - 1);
		float 	fSlow15 = (fSlow1 * ((1.0f + fSlow6) - fSlow7));
		int 	iSlow16 = int((1 + int((int((int((fConst7 * fslider4)) - 1)) & 65535))));
		float 	fSlow17 = (0.01f * fslider5);
		float 	fSlow18 = (fConst9 * fslider6);
		float 	fSlow19 = (fConst9 * fslider7);
		int 	iSlow20 = int((int((fConst7 * fslider8)) & 8191));
		float 	fSlow21 = expf((fConst12 / fSlow0));
		float 	fSlow22 = faustpower<2>(fSlow21);
		float 	fSlow23 = (1.0f - fSlow22);
		float 	fSlow24 = (1.0f - (fSlow4 * fSlow22));
		float 	fSlow25 = sqrtf(max(0, ((faustpower<2>(fSlow24) / faustpower<2>(fSlow23)) - 1.0f)));
		float 	fSlow26 = (fSlow24 / fSlow23);
		float 	fSlow27 = (fSlow26 - fSlow25);
		float 	fSlow28 = ((expf((fConst12 / fSlow13)) / fSlow21) - 1);
		float 	fSlow29 = (fSlow21 * ((1.0f + fSlow25) - fSlow26));
		float 	fSlow30 = expf((fConst17 / fSlow0));
		float 	fSlow31 = faustpower<2>(fSlow30);
		float 	fSlow32 = (1.0f - fSlow31);
		float 	fSlow33 = (1.0f - (fSlow4 * fSlow31));
		float 	fSlow34 = sqrtf(max(0, ((faustpower<2>(fSlow33) / faustpower<2>(fSlow32)) - 1.0f)));
		float 	fSlow35 = (fSlow33 / fSlow32);
		float 	fSlow36 = (fSlow35 - fSlow34);
		float 	fSlow37 = ((expf((fConst17 / fSlow13)) / fSlow30) - 1);
		float 	fSlow38 = (fSlow30 * ((1.0f + fSlow34) - fSlow35));
		float 	fSlow39 = expf((fConst22 / fSlow0));
		float 	fSlow40 = faustpower<2>(fSlow39);
		float 	fSlow41 = (1.0f - fSlow40);
		float 	fSlow42 = (1.0f - (fSlow4 * fSlow40));
		float 	fSlow43 = sqrtf(max(0, ((faustpower<2>(fSlow42) / faustpower<2>(fSlow41)) - 1.0f)));
		float 	fSlow44 = (fSlow42 / fSlow41);
		float 	fSlow45 = (fSlow44 - fSlow43);
		float 	fSlow46 = ((expf((fConst22 / fSlow13)) / fSlow39) - 1);
		float 	fSlow47 = (fSlow39 * ((1.0f + fSlow43) - fSlow44));
		float 	fSlow48 = expf((fConst27 / fSlow0));
		float 	fSlow49 = faustpower<2>(fSlow48);
		float 	fSlow50 = (1.0f - fSlow49);
		float 	fSlow51 = (1.0f - (fSlow4 * fSlow49));
		float 	fSlow52 = sqrtf(max(0, ((faustpower<2>(fSlow51) / faustpower<2>(fSlow50)) - 1.0f)));
		float 	fSlow53 = (fSlow51 / fSlow50);
		float 	fSlow54 = (fSlow53 - fSlow52);
		float 	fSlow55 = ((expf((fConst27 / fSlow13)) / fSlow48) - 1);
		float 	fSlow56 = (fSlow48 * ((1.0f + fSlow52) - fSlow53));
		float 	fSlow57 = expf((fConst32 / fSlow0));
		float 	fSlow58 = faustpower<2>(fSlow57);
		float 	fSlow59 = (1.0f - fSlow58);
		float 	fSlow60 = (1.0f - (fSlow4 * fSlow58));
		float 	fSlow61 = sqrtf(max(0, ((faustpower<2>(fSlow60) / faustpower<2>(fSlow59)) - 1.0f)));
		float 	fSlow62 = (fSlow60 / fSlow59);
		float 	fSlow63 = (fSlow62 - fSlow61);
		float 	fSlow64 = ((expf((fConst32 / fSlow13)) / fSlow57) - 1);
		float 	fSlow65 = (fSlow57 * ((1.0f + fSlow61) - fSlow62));
		float 	fSlow66 = expf((fConst37 / fSlow0));
		float 	fSlow67 = faustpower<2>(fSlow66);
		float 	fSlow68 = (1.0f - fSlow67);
		float 	fSlow69 = (1.0f - (fSlow4 * fSlow67));
		float 	fSlow70 = sqrtf(max(0, ((faustpower<2>(fSlow69) / faustpower<2>(fSlow68)) - 1.0f)));
		float 	fSlow71 = (fSlow69 / fSlow68);
		float 	fSlow72 = (fSlow71 - fSlow70);
		float 	fSlow73 = ((expf((fConst37 / fSlow13)) / fSlow66) - 1);
		float 	fSlow74 = (fSlow66 * ((1.0f + fSlow70) - fSlow71));
		float 	fSlow75 = expf((fConst42 / fSlow0));
		float 	fSlow76 = faustpower<2>(fSlow75);
		float 	fSlow77 = (1.0f - fSlow76);
		float 	fSlow78 = (1.0f - (fSlow76 * fSlow4));
		float 	fSlow79 = sqrtf(max(0, ((faustpower<2>(fSlow78) / faustpower<2>(fSlow77)) - 1.0f)));
		float 	fSlow80 = (fSlow78 / fSlow77);
		float 	fSlow81 = (fSlow80 - fSlow79);
		float 	fSlow82 = ((expf((fConst42 / fSlow13)) / fSlow75) - 1);
		float 	fSlow83 = (fSlow75 * ((1.0f + fSlow79) - fSlow80));
		float 	fSlow84 = fslider9;
		float 	fSlow85 = powf(10,(0.05f * fslider10));
		float 	fSlow86 = (fConst46 * (fSlow84 / sqrtf(max(0, fSlow85))));
		float 	fSlow87 = ((1.0f - fSlow86) / (1.0f + fSlow86));
		float 	fSlow88 = ((1 + fSlow87) * (0 - cosf((fConst46 * fSlow84))));
		float 	fSlow89 = fslider11;
		float 	fSlow90 = powf(10,(0.05f * fslider12));
		float 	fSlow91 = (fConst46 * (fSlow89 / sqrtf(max(0, fSlow90))));
		float 	fSlow92 = ((1.0f - fSlow91) / (1.0f + fSlow91));
		float 	fSlow93 = ((1 + fSlow92) * (0 - cosf((fConst46 * fSlow89))));
		float 	fSlow94 = (0.0010000000000000009f * fslider13);
		float 	fSlow95 = (0.0010000000000000009f * fslider14);
		FAUSTFLOAT* input0 = input[0];
		FAUSTFLOAT* output0 = output[0];
		FAUSTFLOAT* output1 = output[1];
		for (int i=0; i<count; i++) {
			fRec13[0] = ((fSlow12 * (fRec6[1] + fRec6[2])) + (fSlow11 * fRec13[1]));
			fRec12[0] = ((fSlow15 * (fRec6[1] + (fSlow14 * fRec13[0]))) + (fSlow8 * fRec12[1]));
			fVec0[IOTA&8191] = (1e-20f + (0.35355339059327373f * fRec12[0]));
			fRec15[0] = (fSlow18 + (fConst8 * fRec15[1]));
			float fTemp0 = tanf((fConst4 * fRec15[0]));
			float fTemp1 = (1.0f / fTemp0);
			float fTemp2 = (1 + ((0.7653668647301795f + fTemp1) / fTemp0));
			float fTemp3 = (1 - (1.0f / faustpower<2>(fTemp0)));
			float fTemp4 = (1 + ((1.8477590650225735f + fTemp1) / fTemp0));
			fRec18[0] = (fSlow19 + (fConst8 * fRec18[1]));
			float fTemp5 = tanf((fConst4 * fRec18[0]));
			float fTemp6 = (1.0f / fTemp5);
			float fTemp7 = (1 + ((0.7653668647301795f + fTemp6) / fTemp5));
			float fTemp8 = faustpower<2>(fTemp5);
			float fTemp9 = (1.0f / fTemp8);
			float fTemp10 = (1 - fTemp9);
			float fTemp11 = (1 + ((1.8477590650225735f + fTemp6) / fTemp5));
			fRec20[0] = ((float)input0[i] - (((fRec20[2] * (1 + ((fTemp6 - 1.8477590650225735f) / fTemp5))) + (2 * (fTemp10 * fRec20[1]))) / fTemp11));
			float fTemp12 = (0 - fTemp9);
			fRec19[0] = (((((fRec20[0] / fTemp8) + (2 * (fRec20[1] * fTemp12))) + (fRec20[2] / fTemp8)) / fTemp11) - (((fRec19[2] * (1 + ((fTemp6 - 0.7653668647301795f) / fTemp5))) + (2 * (fRec19[1] * fTemp10))) / fTemp7));
			fRec17[0] = (((((fRec19[0] / fTemp8) + (2 * (fRec19[1] * fTemp12))) + (fRec19[2] / fTemp8)) / fTemp7) - (((fRec17[2] * (1 + ((fTemp1 - 1.8477590650225735f) / fTemp0))) + (2 * (fTemp3 * fRec17[1]))) / fTemp4));
			fRec16[0] = (((fRec17[2] + (fRec17[0] + (2 * fRec17[1]))) / fTemp4) - (((fRec16[2] * (1 + ((fTemp1 - 0.7653668647301795f) / fTemp0))) + (2 * (fRec16[1] * fTemp3))) / fTemp2));
			fRec14[IOTA&131071] = (((fRec16[2] + (fRec16[0] + (2 * fRec16[1]))) / fTemp2) + (fSlow17 * fRec14[(IOTA-iSlow16)&131071]));
			float fTemp13 = (0.3f * fRec14[(IOTA-iSlow20)&131071]);
			float fTemp14 = ((fTemp13 + fVec0[(IOTA-iConst6)&8191]) - (0.6f * fRec10[1]));
			fVec1[IOTA&2047] = fTemp14;
			fRec10[0] = fVec1[(IOTA-iConst10)&2047];
			float 	fRec11 = (0.6f * fVec1[IOTA&2047]);
			fRec24[0] = ((fSlow12 * (fRec2[1] + fRec2[2])) + (fSlow11 * fRec24[1]));
			fRec23[0] = ((fSlow29 * (fRec2[1] + (fSlow28 * fRec24[0]))) + (fSlow27 * fRec23[1]));
			fVec2[IOTA&8191] = (1e-20f + (0.35355339059327373f * fRec23[0]));
			float fTemp15 = ((fTemp13 + fVec2[(IOTA-iConst14)&8191]) - (0.6f * fRec21[1]));
			fVec3[IOTA&1023] = fTemp15;
			fRec21[0] = fVec3[(IOTA-iConst15)&1023];
			float 	fRec22 = (0.6f * fVec3[IOTA&1023]);
			float fTemp16 = (fRec22 + fRec11);
			fRec28[0] = ((fSlow12 * (fRec4[1] + fRec4[2])) + (fSlow11 * fRec28[1]));
			fRec27[0] = ((fSlow38 * (fRec4[1] + (fSlow37 * fRec28[0]))) + (fSlow36 * fRec27[1]));
			fVec4[IOTA&8191] = (1e-20f + (0.35355339059327373f * fRec27[0]));
			float fTemp17 = (fVec4[(IOTA-iConst19)&8191] - (fTemp13 + (0.6f * fRec25[1])));
			fVec5[IOTA&2047] = fTemp17;
			fRec25[0] = fVec5[(IOTA-iConst20)&2047];
			float 	fRec26 = (0.6f * fVec5[IOTA&2047]);
			fRec32[0] = ((fSlow12 * (fRec8[1] + fRec8[2])) + (fSlow11 * fRec32[1]));
			fRec31[0] = ((fSlow47 * (fRec8[1] + (fSlow46 * fRec32[0]))) + (fSlow45 * fRec31[1]));
			fVec6[IOTA&8191] = (1e-20f + (0.35355339059327373f * fRec31[0]));
			float fTemp18 = (fVec6[(IOTA-iConst24)&8191] - (fTemp13 + (0.6f * fRec29[1])));
			fVec7[IOTA&1023] = fTemp18;
			fRec29[0] = fVec7[(IOTA-iConst25)&1023];
			float 	fRec30 = (0.6f * fVec7[IOTA&1023]);
			float fTemp19 = (fRec30 + (fRec26 + fTemp16));
			fRec36[0] = ((fSlow12 * (fRec3[1] + fRec3[2])) + (fSlow11 * fRec36[1]));
			fRec35[0] = ((fSlow56 * (fRec3[1] + (fSlow55 * fRec36[0]))) + (fSlow54 * fRec35[1]));
			fVec8[IOTA&16383] = (1e-20f + (0.35355339059327373f * fRec35[0]));
			float fTemp20 = ((fTemp13 + fVec8[(IOTA-iConst29)&16383]) + (0.6f * fRec33[1]));
			fVec9[IOTA&2047] = fTemp20;
			fRec33[0] = fVec9[(IOTA-iConst30)&2047];
			float 	fRec34 = (0 - (0.6f * fVec9[IOTA&2047]));
			fRec40[0] = ((fSlow12 * (fRec7[1] + fRec7[2])) + (fSlow11 * fRec40[1]));
			fRec39[0] = ((fSlow65 * (fRec7[1] + (fSlow64 * fRec40[0]))) + (fSlow63 * fRec39[1]));
			fVec10[IOTA&8191] = (1e-20f + (0.35355339059327373f * fRec39[0]));
			float fTemp21 = ((fTemp13 + fVec10[(IOTA-iConst34)&8191]) + (0.6f * fRec37[1]));
			fVec11[IOTA&2047] = fTemp21;
			fRec37[0] = fVec11[(IOTA-iConst35)&2047];
			float 	fRec38 = (0 - (0.6f * fVec11[IOTA&2047]));
			fRec44[0] = ((fSlow12 * (fRec5[1] + fRec5[2])) + (fSlow11 * fRec44[1]));
			fRec43[0] = ((fSlow74 * (fRec5[1] + (fSlow73 * fRec44[0]))) + (fSlow72 * fRec43[1]));
			fVec12[IOTA&16383] = (1e-20f + (0.35355339059327373f * fRec43[0]));
			float fTemp22 = ((fVec12[(IOTA-iConst39)&16383] + (0.6f * fRec41[1])) - fTemp13);
			fVec13[IOTA&2047] = fTemp22;
			fRec41[0] = fVec13[(IOTA-iConst40)&2047];
			float 	fRec42 = (0 - (0.6f * fVec13[IOTA&2047]));
			fRec48[0] = ((fSlow12 * (fRec9[1] + fRec9[2])) + (fSlow11 * fRec48[1]));
			fRec47[0] = ((fSlow83 * (fRec9[1] + (fSlow82 * fRec48[0]))) + (fSlow81 * fRec47[1]));
			fVec14[IOTA&16383] = (1e-20f + (0.35355339059327373f * fRec47[0]));
			float fTemp23 = ((fVec14[(IOTA-iConst44)&16383] + (0.6f * fRec45[1])) - fTemp13);
			fVec15[IOTA&1023] = fTemp23;
			fRec45[0] = fVec15[(IOTA-iConst45)&1023];
			float 	fRec46 = (0 - (0.6f * fVec15[IOTA&1023]));
			fRec2[0] = (fRec21[1] + (fRec10[1] + (fRec25[1] + (fRec29[1] + (fRec33[1] + (fRec37[1] + (fRec41[1] + (fRec45[1] + (fRec46 + (fRec42 + (fRec38 + (fRec34 + fTemp19))))))))))));
			fRec3[0] = (0 - ((fRec33[1] + (fRec37[1] + (fRec41[1] + (fRec45[1] + (fRec46 + (fRec42 + (fRec34 + fRec38))))))) - (fRec21[1] + (fRec10[1] + (fRec25[1] + (fRec29[1] + fTemp19))))));
			float fTemp24 = (fRec26 + fRec30);
			fRec4[0] = (0 - ((fRec25[1] + (fRec29[1] + (fRec41[1] + (fRec45[1] + (fRec46 + (fRec42 + fTemp24)))))) - (fRec21[1] + (fRec10[1] + (fRec33[1] + (fRec37[1] + (fRec38 + (fRec34 + fTemp16))))))));
			fRec5[0] = (0 - ((fRec25[1] + (fRec29[1] + (fRec33[1] + (fRec37[1] + (fRec38 + (fRec34 + fTemp24)))))) - (fRec21[1] + (fRec10[1] + (fRec41[1] + (fRec45[1] + (fRec46 + (fRec42 + fTemp16))))))));
			float fTemp25 = (fRec22 + fRec26);
			float fTemp26 = (fRec11 + fRec30);
			fRec6[0] = (0 - ((fRec10[1] + (fRec29[1] + (fRec37[1] + (fRec45[1] + (fRec46 + (fRec38 + fTemp26)))))) - (fRec21[1] + (fRec25[1] + (fRec33[1] + (fRec41[1] + (fRec42 + (fRec34 + fTemp25))))))));
			fRec7[0] = (0 - ((fRec10[1] + (fRec29[1] + (fRec33[1] + (fRec41[1] + (fRec42 + (fRec34 + fTemp26)))))) - (fRec21[1] + (fRec25[1] + (fRec37[1] + (fRec45[1] + (fRec46 + (fRec38 + fTemp25))))))));
			float fTemp27 = (fRec22 + fRec30);
			float fTemp28 = (fRec11 + fRec26);
			fRec8[0] = (0 - ((fRec10[1] + (fRec25[1] + (fRec37[1] + (fRec41[1] + (fRec42 + (fRec38 + fTemp28)))))) - (fRec21[1] + (fRec29[1] + (fRec33[1] + (fRec45[1] + (fRec46 + (fRec34 + fTemp27))))))));
			fRec9[0] = (0 - ((fRec10[1] + (fRec25[1] + (fRec33[1] + (fRec45[1] + (fRec46 + (fRec34 + fTemp28)))))) - (fRec21[1] + (fRec29[1] + (fRec37[1] + (fRec41[1] + (fRec42 + (fRec38 + fTemp27))))))));
			float fTemp29 = (0.37f * (fRec3[0] + fRec4[0]));
			float fTemp30 = (fSlow88 * fRec1[1]);
			fRec1[0] = (0 - (((fSlow87 * fRec1[2]) + fTemp30) - fTemp29));
			float fTemp31 = (fSlow87 * fRec1[0]);
			float fTemp32 = (0.5f * ((fSlow85 * ((fTemp31 + (fRec1[2] + fTemp30)) - fTemp29)) + (fTemp31 + (fTemp30 + (fTemp29 + fRec1[2])))));
			float fTemp33 = (fSlow93 * fRec0[1]);
			fRec0[0] = (0 - (((fSlow92 * fRec0[2]) + fTemp33) - fTemp32));
			float fTemp34 = (fSlow92 * fRec0[0]);
			fRec49[0] = (fSlow94 + (0.999f * fRec49[1]));
			float fTemp35 = (1.0f + fRec49[0]);
			float fTemp36 = (fRec14[(IOTA-0)&131071] * (1.0f - (0.5f * fTemp35)));
			fRec50[0] = (fSlow95 + (0.999f * fRec50[1]));
			float fTemp37 = powf(10,(0.05f * fRec50[0]));
			output0[i] = (FAUSTFLOAT)(fTemp37 * (fTemp36 + (0.25f * (fTemp35 * ((fSlow90 * ((fTemp34 + (fRec0[2] + fTemp33)) - fTemp32)) + (fTemp34 + (fTemp33 + (fTemp32 + fRec0[2]))))))));
			float fTemp38 = (0.37f * (fRec3[0] - fRec4[0]));
			float fTemp39 = (fSlow88 * fRec52[1]);
			fRec52[0] = (0 - (((fSlow87 * fRec52[2]) + fTemp39) - fTemp38));
			float fTemp40 = (fSlow87 * fRec52[0]);
			float fTemp41 = (0.5f * ((fSlow85 * ((fTemp40 + (fRec52[2] + fTemp39)) - fTemp38)) + (fTemp40 + (fTemp39 + (fTemp38 + fRec52[2])))));
			float fTemp42 = (fSlow93 * fRec51[1]);
			fRec51[0] = (0 - (((fSlow92 * fRec51[2]) + fTemp42) - fTemp41));
			float fTemp43 = (fSlow92 * fRec51[0]);
			output1[i] = (FAUSTFLOAT)(fTemp37 * (fTemp36 + (0.25f * (fTemp35 * ((fSlow90 * ((fTemp43 + (fRec51[2] + fTemp42)) - fTemp41)) + (fTemp43 + (fTemp42 + (fTemp41 + fRec51[2]))))))));
			// post processing
			fRec51[2] = fRec51[1]; fRec51[1] = fRec51[0];
			fRec52[2] = fRec52[1]; fRec52[1] = fRec52[0];
			fRec50[1] = fRec50[0];
			fRec49[1] = fRec49[0];
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
			fRec9[2] = fRec9[1]; fRec9[1] = fRec9[0];
			fRec8[2] = fRec8[1]; fRec8[1] = fRec8[0];
			fRec7[2] = fRec7[1]; fRec7[1] = fRec7[0];
			fRec6[2] = fRec6[1]; fRec6[1] = fRec6[0];
			fRec5[2] = fRec5[1]; fRec5[1] = fRec5[0];
			fRec4[2] = fRec4[1]; fRec4[1] = fRec4[0];
			fRec3[2] = fRec3[1]; fRec3[1] = fRec3[0];
			fRec2[2] = fRec2[1]; fRec2[1] = fRec2[0];
			fRec45[1] = fRec45[0];
			fRec47[1] = fRec47[0];
			fRec48[1] = fRec48[0];
			fRec41[1] = fRec41[0];
			fRec43[1] = fRec43[0];
			fRec44[1] = fRec44[0];
			fRec37[1] = fRec37[0];
			fRec39[1] = fRec39[0];
			fRec40[1] = fRec40[0];
			fRec33[1] = fRec33[0];
			fRec35[1] = fRec35[0];
			fRec36[1] = fRec36[0];
			fRec29[1] = fRec29[0];
			fRec31[1] = fRec31[0];
			fRec32[1] = fRec32[0];
			fRec25[1] = fRec25[0];
			fRec27[1] = fRec27[0];
			fRec28[1] = fRec28[0];
			fRec21[1] = fRec21[0];
			fRec23[1] = fRec23[0];
			fRec24[1] = fRec24[0];
			fRec10[1] = fRec10[0];
			fRec16[2] = fRec16[1]; fRec16[1] = fRec16[0];
			fRec17[2] = fRec17[1]; fRec17[1] = fRec17[0];
			fRec19[2] = fRec19[1]; fRec19[1] = fRec19[0];
			fRec20[2] = fRec20[1]; fRec20[1] = fRec20[0];
			fRec18[1] = fRec18[0];
			fRec15[1] = fRec15[0];
			IOTA = IOTA+1;
			fRec12[1] = fRec12[0];
			fRec13[1] = fRec13[0];
		}
	}
};



/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/
					
FablaDSP		DSP;
list<GUI*>	GUI::fGuiList;

//-------------------------------------------------------------------------
// 									MAIN -> NOT PRESENT, main is in the rest of the code
//------------------------------------------------------------------------- 


		
/********************END ARCHITECTURE SECTION (part 2/2)****************/

