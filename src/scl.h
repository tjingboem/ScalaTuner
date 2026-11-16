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

// tab[n] = log2 (freq of MIDI note n / reference A (note 69)).
// `root` carries 1/1 at its 12-TET pitch; notes map one per degree.
static inline void scl_table (const Scl *sc, int root, float *tab)
{
    const double rootlog = (root - 69) / 12.0;
    const double period  = sc->oct [sc->n - 1];
    for (int n = 0; n < 128; n++)
    {
        int k = n - root;
        int q = k / sc->n;
        int r = k % sc->n;
        if (r < 0) { r += sc->n; q--; }
        tab [n] = (float)(rootlog + q * period + (r ? sc->oct [r - 1] : 0.0));
    }
}
