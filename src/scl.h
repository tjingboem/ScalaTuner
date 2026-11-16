// ScalaTuner - stereo autotuner that snaps pitch to Scala (.scl) scales.
// Copyright (C) 2026 tjingboem productions
//
// This program is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version. It is
// distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License (the file LICENSE) for more details.
//
// Pitch tracking core: zita-at1 0.8.2, Copyright (C) 2010-2024 Fons Adriaensen (GPL v3).

// Scala (.scl) file parsing and scale -> pitch table.
#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#define SCL_MAX 1024

struct Scl
{
    int  n;              // notes per period (>= 1), degree n is the period
    double oct [SCL_MAX]; // degrees 1..n in octaves (log2 ratio); oct[n-1] = period
    char desc [96];      // description line

    void twelve_tet (void)
    {
        n = 12;
        for (int i = 0; i < 12; i++) oct [i] = (i + 1) / 12.0;
        strcpy (desc, "12-tone equal temperament");
    }
};

// One pitch line: cents if it contains '.', else ratio "a/b" or integer "a".
static inline bool scl_pitch (const char *line, double *oct)
{
    while (*line == ' ' || *line == '\t') line++;
    char tok [128];
    size_t l = strcspn (line, " \t\r\n");
    if (l == 0 || l >= sizeof (tok)) return false;
    memcpy (tok, line, l);
    tok [l] = 0;
    char *end;
    if (strchr (tok, '.'))
    {
        double c = strtod (tok, &end);
        if (end == tok) return false;
        *oct = c / 1200.0;
        return true;
    }
    double num = strtod (tok, &end);
    if (end == tok) return false;
    double den = 1;
    if (*end == '/') den = strtod (end + 1, 0);
    if (num <= 0 || den <= 0) return false;
    *oct = log2 (num / den);
    return true;
}

static inline bool scl_load (const char *path, Scl *sc)
{
    FILE *f = fopen (path, "r");
    if (!f) return false;
    char line [512];
    int  state = 0, count = 0, got = 0;
    bool ok = false;
    sc->desc [0] = 0;
    while (fgets (line, sizeof (line), f))
    {
        if (line [0] == '!') continue;
        if (state == 0)
        {
            size_t l = strcspn (line, "\r\n");
            if (l >= sizeof (sc->desc)) l = sizeof (sc->desc) - 1;
            memcpy (sc->desc, line, l);
            sc->desc [l] = 0;
            state = 1;
        }
        else if (state == 1)
        {
            count = atoi (line);
            if (count < 1 || count > SCL_MAX) break;
            state = 2;
        }
        else if (got < count)
        {
            double o;
            if (!scl_pitch (line, &o)) break;
            sc->oct [got++] = o;
        }
        if (state == 2 && got == count) { ok = true; break; }
    }
    fclose (f);
    if (!ok || sc->oct [count - 1] <= 0) return false; // period must exceed 1/1
    sc->n = count;
    return true;
}

struct TabInfo
{
    int n;      // number of entries
    int kmin;   // step number (from 1/1 at the Scale Root) of entry 0
};

// The notes of the scale over the audible range (about 27 Hz to 7 kHz at A = 440),
// in ascending order: tab[e] = log2 (freq / reference A) of step kmin + e.
// Step 0 is 1/1 and sounds at the 12-TET pitch of MIDI note `root`; step k is
// the note k scale degrees above it (negative below). `max` is the room in tab.
static inline TabInfo scl_build (const Scl *sc, int root, float *tab, int max)
{
    const double rootlog = (root - 69) / 12.0;
    const double period  = sc->oct [sc->n - 1];
    double lo = -4.5, hi = 4.5;
    TabInfo ti = { 0, 0 };
    for (int pass = 0; pass < 8; pass++)
    {
        int qlo = (int)floor ((lo - rootlog) / period) - 1;
        int qhi = (int)ceil ((hi - rootlog) / period) + 1;
        ti.n = 0;
        bool full = false;
        for (int q = qlo; q <= qhi && !full; q++)
            for (int r = 0; r < sc->n; r++)
            {
                double v = rootlog + q * period + (r ? sc->oct [r - 1] : 0.0);
                if (v < lo || v > hi) continue;
                if (ti.n == 0) ti.kmin = q * sc->n + r;
                if (ti.n == max) { full = true; break; }
                tab [ti.n++] = (float)v;
            }
        if (!full) break;
        lo += 0.5; hi -= 0.5;   // very fine scales: narrow the range until it fits
    }
    return ti;
}
