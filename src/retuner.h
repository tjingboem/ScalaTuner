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


#include <math.h>
#include <fftw3.h>
#include "resampler.h"

using LV2AT::Resampler;

#define TAB_MAX 16384   // largest number of notes in the scale table


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

    // Pitch detection range in Hz, and the level (dB, full-scale sine = 0)
    // below which the input is taken as silence.
    void set_range (float fmin, float fmax)
    {
        int lo = (int)(_fsamp / fmax);
        int hi = (int)(_fsamp / fmin);
        if (lo < 4) lo = 4;                      // find-peak window needs >= 1 sample
        if (hi > _fftlen / 2 - 4) hi = _fftlen / 2 - 4;  // lags beyond half the window are not valid
        if (hi < lo * 2) hi = lo * 2;
        _ifmin = lo;
        _ifmax = hi;
    }

    // Looseness 0..1: how little evidence of a pitch is enough. 0 is zita-at1: a peak
    // of at least 0.6 in the autocorrelation, with some power in its spectrum bin
    // and not 20 dB below a stronger peak. Higher values follow weaker, noisier and
    // gliding pitches, with more flicker between notes. At 1 every periodicity
    // that can be found is followed.
    void set_looseness (float l)
    {
        if (l < 0.0f) l = 0.0f;
        if (l > 1.0f) l = 1.0f;
        float k = (1.0f - l) * (1.0f - l);
        _vthr_v = 0.6f * (1.0f - l);     // required autocorrelation peak
        _a1min  = 1e-4f * k;             // required power in the spectrum bin
        _rmin   = 1e-2f * k;             // weakest peak, relative to a stronger one
    }

    void set_threshold (float db)
    {
        _thresh = powf (10.0f, db / 10.0f);
    }

    // Hold: how long the correction stays after the pitch is lost, instead of
    // returning to the original pitch. seconds: 0 = off, < 0 = without limit.
    void set_hold (float seconds)
    {
        if (seconds < 0) _hold = -1;
        else if (seconds == 0) _hold = 0;
        else
        {
            int per = 4 * _frsize / (_upsamp ? 2 : 1);   // input frames between pitch estimates
            _hold = (int)(seconds * _fsamp / per + 0.5f);
            if (_hold < 1) _hold = 1;
        }
    }

    void set_notemask (int k)
    {
        _notemask = k;
    }
   
    void set_lowlat (bool on)
    {
	_latency = _ipsize / (on ? 4 : 2);
    }
   
    // Scala scales. The table holds log2 (freq / refpitch) for every note of the
    // scale over the whole audible range, in ascending order (n <= TAB_MAX).
    // The detected pitch is pulled to the nearest ACTIVE entry; set_active gives
    // one bit per entry. maxdist is the largest distance, in octaves, that a pitch
    // may be pulled.
    void set_table (const float *tab, int n)
    {
        _tabn = n > TAB_MAX ? TAB_MAX : n;
        for (int i = 0; i < _tabn; i++) _tab [i] = tab [i];
        _lastnote = -1;
    }

    void set_active (const unsigned int *mask)
    {
        int w = (_tabn + 31) / 32;
        unsigned int any = 0;
        for (int i = 0; i < w; i++) { _tabmask [i] = mask [i]; any |= mask [i]; }
        _tabany = any != 0;
    }

    void set_maxdist (float oct)
    {
        _maxdist = oct;
    }

    // Display only.
    int   get_note (void) const { return _lastnote; }              // table index or -1
    float get_cycle (void) const { return _cycle; }                // period in samples
    bool  voiced (void) const { return _count < 3; }
    // Lookahead 0..4 = delay of 1, 2, 4, 8, 16 times the zita-at1 delay
    // (21, 43, 85, 171, 341 ms at 48 kHz). A change moves the read position at once.
    void set_lookahead (int step)
    {
        if (step < 0) step = 0;
        if (step > 4) step = 4;
        int lat = _lat0 << step;
        if (lat == _latency) return;
        _latency = lat;
        float r = (float)(_ipindex - _latency);
        while (r < 0) r += _ipsize;
        _rindex1 = r;
        _rindex2 = r;
        _xfade = false;
    }
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
    float            _thresh;
    float            _vthr_v;
    float            _a1min;
    float            _rmin;
    bool             _upsamp;
    int              _fftlen;
    int              _ipsize;
    int              _frsize;
    int              _latency;
    int              _lat0;
    int              _ipindex;
    int              _frindex;
    int              _frcount;
    float            _refpitch;
    float            _notebias;
    float            _corrfilt; 
    float            _corrgain;
    float            _corroffs;
    int              _notemask;
    int              _tabn;
    bool             _tabany;
    float            _maxdist;
    float            _tab [TAB_MAX];
    unsigned int     _tabmask [TAB_MAX / 32];
    int              _notebits;
    int              _lastnote;
    int              _count;
    int              _hold;     // estimates to hold the correction, -1 = unlimited, 0 = off
    int              _unv;      // unvoiced estimates in a row
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
