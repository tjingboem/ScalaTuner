// ----------------------------------------------------------------------------
//
//  Copyright (C) 2010-2024 Fons Adriaensen <fons@linuxaudio.org>
//    
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// ----------------------------------------------------------------------------


#ifndef __RETUNER_H
#define __RETUNER_H


#include <fftw3.h>
#include "resampler.h"

using LV2AT::Resampler;


class Retuner
{
public:

    Retuner (int fsamp);
    ~Retuner (void);

    int process (int nfram, float *inp, float *out);

    void set_refpitch (float v)
    {
        _refpitch = v;
    }

    void set_notebias (float v)
    {
        _notebias = v / 13.0f;
    }

    void set_corrfilt (float v)
    {
        _corrfilt = (4 * _frsize) / (v * _fsamp);
    }

    void set_corrgain (float v)
    {
        _corrgain = v;
    }

    void set_corroffs (float v)
    {
        _corroffs = v;
    }

    void set_notemask (int k)
    {
        _notemask = k;
    }
   
    void set_lowlat (bool on)
    {
	_latency = _ipsize / (on ? 4 : 2);
    }
   
    // Table mode (Scala scales): tab[n] = log2(freq_n / refpitch) for MIDI
    // note n = 0..127; active = 128-bit mask (4 x uint32). The detected
    // pitch is pulled to the nearest active entry.
    void set_table (const float *tab, const unsigned int *active)
    {
        for (int i = 0; i < 128; i++) _tab [i] = tab [i];
        for (int i = 0; i < 4; i++) _tabmask [i] = active [i];
    }

    // Display only.
    int   get_note (void) const { return _lastnote; }              // table index or -1
    float get_cycle (void) const { return _cycle; }                // period in samples
    bool  voiced (void) const { return _count < 3; }
    int   latency (void) const { return _upsamp ? _latency / 2 : _latency; }
    float get_ratio (void) const { return _ratio; }

    int get_noteset (void)
    {
        int k;

        k = _notebits;
        _notebits = 0;
        return k;
    }

    float get_error (void)
    {
        return 12.0f * _error;
    }


private:

    float findcycle (void);
    void  finderror (void);
    float cubic (float *v, float a);

    int              _fsamp;
    int              _ifmin;
    int              _ifmax;
    bool             _upsamp;
    int              _fftlen;
    int              _ipsize;
    int              _frsize;
    int              _latency;
    int              _ipindex;
    int              _frindex;
    int              _frcount;
    float            _refpitch;
    float            _notebias;
    float            _corrfilt; 
    float            _corrgain;
    float            _corroffs;
    int              _notemask;
    float            _tab [128];
    unsigned int     _tabmask [4];
    int              _notebits;
    int              _lastnote;
    int              _count;
    float            _cycle;
    float            _error;
    float            _ratio;
    float            _phase;
    bool             _xfade;
    float            _rindex1;
    float            _rindex2;
    float           *_ipbuff;
    float           *_xffunc;
    float           *_Twind;
    float           *_Wcorr;
    float           *_Tdata;
    fftwf_complex   *_Fdata;
    fftwf_plan       _fwdplan;
    fftwf_plan       _invplan;
    Resampler        _resampler;
};


#endif
