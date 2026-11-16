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

// ScalaTuner: stereo autotuner snapping to Scala (.scl) scales. CLAP plugin.
// DSP core: zita-at1 0.8.2 retuner by Fons Adriaensen (GPL v3).
#pragma once
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <clap/clap.h>
#include "retuner.h"
#include "scl.h"

enum {
    P_SCALE, P_ROOT, P_TUNE, P_BIAS, P_SMOOTH, P_CORR, P_OFFS, P_MODE,
    P_MINHZ, P_MAXHZ, P_THRESH, P_LOOK, P_COUNT
};

struct ParamInfo
{
    const char *name;
    double min, max, def;
    bool   stepped;
};

extern const ParamInfo param_info [P_COUNT];

struct GuiMsg
{
    int      kind; // 0 gesture begin, 1 value, 2 gesture end
    uint32_t id;
    double   value;
};

struct Gui;

struct Plugin
{
    clap_plugin_t        plugin;
    const clap_host_t   *host;
    const clap_host_params_t *host_params;
    const clap_host_latency_t *host_latency;

    // Parameters (real values). Written by host events, GUI; read by audio.
    std::atomic<double> val [P_COUNT];
    int nscales;                         // number of scale files; param max = nscales

    // Scale list: names [i] is scale index i + 1; index 0 is 12-TET.
    std::vector<std::string> env_dirs;   // from SCALATUNER_SCALES (not editable here)
    std::vector<std::string> cfg_dirs;   // from ~/.config/scalatuner/scales_dir (editable in the GUI)
    std::vector<std::string> dirs;       // all folders in use
    std::vector<int>         dir_counts; // number of .scl files in each of dirs
    std::vector<std::string> names;      // scale names
    std::vector<std::string> paths;      // full file path of each name
    std::mutex               names_mtx;  // guards names/paths against the loader thread

    // Currently loaded scale (loader thread writes, audio try_locks).
    std::mutex   mtx;
    Scl          cur;
    uint32_t     cur_gen;
    int          cur_index;
    bool         cur_ok;
    std::thread  loader;
    std::atomic<bool> quit;

    // Audio side
    Retuner *rt [2];
    double   sr;
    uint32_t maxframes;
    std::vector<float> scratch [2];
    int      held [128];
    Scl      local;
    uint32_t seen_gen;
    int      tab_root;
    bool     tab_valid;
    float    tab [TAB_MAX];        // scale table (see scl_build)
    int      tab_n, tab_kmin;
    float    maxdist;              // largest pull, in octaves
    unsigned int act [TAB_MAX / 32]; // allowed entries
    unsigned char cls [SCL_MAX];   // held scale steps (pitch classes)
    std::atomic<int> latency;      // reported delay in frames
    std::atomic<bool> latency_dirty; // tell the host from the main thread
    int      look_applied;

    // GUI -> audio parameter edits (single producer / single consumer).
    GuiMsg   q [256];
    std::atomic<unsigned> q_w, q_r;

    // Audio -> GUI live display.
    std::atomic<int>   d_note [2];
    std::atomic<float> d_err [2];     // semitones
    std::atomic<float> d_cycle [2];   // samples, 0 if none
    std::atomic<int>   d_voiced [2];

    Gui *gui;
};

// GUI -> host parameter changes (called from the GUI thread).
void plugin_gui_param (Plugin *p, int kind, uint32_t id, double value);
std::string plugin_param_text (Plugin *p, uint32_t id, double value);
void note_name (int midi, char *buf, size_t n);
#define NOTE_NONE (-100000)   // d_note: no note

// Scale folders (GUI thread). Return false and set *err on failure.
bool plugin_add_dir (Plugin *p, const std::string &path, std::string *err);
void plugin_remove_dir (Plugin *p, int cfg_index);

// Implemented in gui.cc
const clap_plugin_gui_t           *gui_extension (void);
const clap_plugin_timer_support_t *gui_timer_extension (void);
void gui_destroy_if_any (Plugin *p);
