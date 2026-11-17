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

#include "plugin.h"
#include <algorithm>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static std::mutex fftw_lock;   // the FFTW planner is not thread-safe

const ParamInfo param_info [P_COUNT] = {
    { "Scale",      0,    0,    0,   true  },  // max filled in per instance
    { "Scale Root",  0,    127,  60,  true  },
    { "A Reference", 400,  480,  440, false },
    { "Bias",       0,    1,    0.5, false },
    { "Smoothing",  0.02, 0.5,  0.1, false },
    { "Correction", 0,    1,    1,   false },
    { "Offset",     -2,   2,    0,   false },
    { "Mode",       0,    2,    0,   true  },
    { "Lowest Pitch",  50,   500,  75,    false },
    { "Highest Pitch", 200,  4000, 1200,  false },
    { "Threshold",     -90,  -20,  -50,   false },
    { "Lookahead",     0,    4,    0,     true  },
    { "Hold",          0,    3,    0,     true  },
    { "Detector",      0,    1,    0,     false },
    { "Lock",          0,    1,    0,     true  },
};

static const char *mode_names [3] = { "Auto", "MIDI", "Manual" };

void note_name (int m, char *buf, size_t n)
{
    static const char *nn [12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    if (m <= NOTE_NONE / 2) { snprintf (buf, n, "-"); return; }
    int pc = ((m % 12) + 12) % 12, oct = (m - pc) / 12 - 1;
    snprintf (buf, n, "%s%d", nn [pc], oct);
}

// ----------------------------------------------------------------------------
// Scale list and loader

static std::string home_dir (void)
{
    const char *home = getenv ("HOME");
    return home ? home : "";
}

static std::string config_file (void) { return home_dir () + "/.config/scalatuner/scales_dir"; }

static bool is_dir (const std::string &d)
{
    DIR *dd = opendir (d.c_str ());
    if (!dd) return false;
    closedir (dd);
    return true;
}

static std::string clean_path (std::string s)
{
    size_t a = s.find_first_not_of (" \t\r\n"), b = s.find_last_not_of (" \t\r\n");
    s = a == std::string::npos ? "" : s.substr (a, b - a + 1);
    if (s.size () >= 2 && s [0] == '~' && s [1] == '/') s = home_dir () + s.substr (1);
    else if (s == "~") s = home_dir ();
    while (s.size () > 1 && s.back () == '/') s.pop_back ();
    return s;
}

static void save_dir_settings (Plugin *p);

static void load_dir_settings (Plugin *p)
{
    p->env_dirs.clear ();
    p->cfg_dirs.clear ();
    const char *env = getenv ("SCALATUNER_SCALES");
    if (env && *env)
    {
        std::string e = env;
        size_t pos = 0;
        while (pos <= e.size ())
        {
            size_t c = e.find (':', pos);
            if (c == std::string::npos) c = e.size ();
            std::string d = clean_path (e.substr (pos, c - pos));
            if (!d.empty ()) p->env_dirs.push_back (d);
            pos = c + 1;
        }
    }
    bool seeded = false;
    if (FILE *f = fopen (config_file ().c_str (), "r"))
    {
        char line [2048];
        while (fgets (line, sizeof (line), f))
        {
            std::string d = clean_path (line);
            if (d == "# seeded") seeded = true;
            if (!d.empty () && d [0] != '#') p->cfg_dirs.push_back (d);
        }
        fclose (f);
    }
    // First run with this version: the standard folder becomes an ordinary entry
    // in the list, so it can be removed like any other.
    if (!seeded)
    {
        const char *cand [] = { "/scl", "/scales", "/.scala/scl" };
        for (const char *c : cand)
        {
            std::string d = home_dir () + c;
            if (is_dir (d))
            {
                if (std::find (p->cfg_dirs.begin (), p->cfg_dirs.end (), d) == p->cfg_dirs.end ()) p->cfg_dirs.push_back (d);
                break;
            }
        }
        save_dir_settings (p);
    }
}

static void save_dir_settings (Plugin *p)
{
    std::string dir = home_dir () + "/.config/scalatuner";
    mkdir ((home_dir () + "/.config").c_str (), 0755);
    mkdir (dir.c_str (), 0755);
    if (FILE *f = fopen (config_file ().c_str (), "w"))
    {
        fprintf (f, "# seeded\n");
        for (auto &d : p->cfg_dirs) fprintf (f, "%s\n", d.c_str ());
        fclose (f);
    }
}

static std::string lower (std::string s)
{
    for (auto &c : s) c = tolower ((unsigned char)c);
    return s;
}

static void scan_scales (Plugin *p)
{
    // Folders in use: environment, then the config file.
    std::vector<std::string> dirs;
    auto add = [&] (const std::string &d) {
        if (!d.empty () && std::find (dirs.begin (), dirs.end (), d) == dirs.end ()) dirs.push_back (d);
    };
    for (auto &d : p->env_dirs) add (d);
    for (auto &d : p->cfg_dirs) add (d);

    struct E { std::string name, path; };
    std::vector<E> all;
    std::vector<int> counts;
    for (auto &dir : dirs)
    {
        int cnt = 0;
        DIR *d = opendir (dir.c_str ());
        if (!d) { counts.push_back (-1); continue; }
        while (struct dirent *e = readdir (d))
        {
            size_t l = strlen (e->d_name);
            if (l > 4 && !strcasecmp (e->d_name + l - 4, ".scl"))
                { all.push_back ({ std::string (e->d_name, l - 4), dir + "/" + e->d_name }); cnt++; }
        }
        closedir (d);
        counts.push_back (cnt);
    }
    std::sort (all.begin (), all.end (), [] (const E &a, const E &b) {
        std::string la = lower (a.name), lb = lower (b.name);
        if (la != lb) return la < lb;
        if (a.name != b.name) return a.name < b.name;
        return a.path < b.path;
    });
    std::lock_guard<std::mutex> lk (p->names_mtx);
    p->dirs = dirs;
    p->dir_counts = counts;
    p->names.clear ();
    p->paths.clear ();
    for (auto &e : all) { p->names.push_back (e.name); p->paths.push_back (e.path); }
    p->nscales = (int)p->names.size ();
}

// Rescan after the folder list changed; keep the current scale selected by name.
static void rescan_keep_scale (Plugin *p)
{
    int cur = (int)lrint (p->val [P_SCALE]);
    std::string curpath;
    {
        std::lock_guard<std::mutex> lk (p->names_mtx);
        if (cur > 0 && cur <= (int)p->paths.size ()) curpath = p->paths [cur - 1];
    }
    scan_scales (p);
    int idx = 0;
    {
        std::lock_guard<std::mutex> lk (p->names_mtx);
        for (int i = 0; i < (int)p->paths.size (); i++)
            if (p->paths [i] == curpath) { idx = i + 1; break; }
    }
    p->val [P_SCALE] = idx;
    if (p->host_params)
        p->host_params->rescan (p->host, CLAP_PARAM_RESCAN_INFO | CLAP_PARAM_RESCAN_VALUES | CLAP_PARAM_RESCAN_TEXT);
}

bool plugin_add_dir (Plugin *p, const std::string &raw, std::string *err)
{
    std::string d = clean_path (raw);
    if (d.empty ()) { *err = "empty path"; return false; }
    if (!is_dir (d)) { *err = "not a folder"; return false; }
    for (auto &x : p->cfg_dirs) if (x == d) { *err = "already added"; return false; }
    p->cfg_dirs.push_back (d);
    save_dir_settings (p);
    rescan_keep_scale (p);
    return true;
}

void plugin_remove_dir (Plugin *p, int i)
{
    if (i < 0 || i >= (int)p->cfg_dirs.size ()) return;
    p->cfg_dirs.erase (p->cfg_dirs.begin () + i);
    save_dir_settings (p);
    rescan_keep_scale (p);
}

static void loader_main (Plugin *p)
{
    int loaded = -1;
    std::string loaded_path;
    while (!p->quit)
    {
        int want = (int)lrint (p->val [P_SCALE]);
        std::string path;
        {
            std::lock_guard<std::mutex> lk (p->names_mtx);
            if (want < 0 || want > (int)p->paths.size ()) want = 0;
            if (want > 0) path = p->paths [want - 1];
        }
        if (want != loaded || path != loaded_path)
        {
            Scl *tmp = new Scl;
            bool ok = true;
            if (want == 0) tmp->twelve_tet ();
            else if (!scl_load (path.c_str (), tmp)) { tmp->twelve_tet (); ok = false; }
            {
                std::lock_guard<std::mutex> lk (p->mtx);
                p->cur = *tmp;
                p->cur_index = want;
                p->cur_ok = ok;
                p->cur_gen++;
            }
            delete tmp;
            loaded = want;
            loaded_path = path;
        }
        usleep (20000);
    }
}

// ----------------------------------------------------------------------------
// Parameters

std::string plugin_param_text (Plugin *p, uint32_t id, double v)
{
    char b [128];
    switch (id)
    {
    case P_SCALE:
    {
        int i = (int)lrint (v);
        if (i <= 0 || i > p->nscales) return "12-TET";
        return p->names [i - 1];
    }
    case P_ROOT:
    {
        // Name and sounding frequency of the note that carries the scale's 1/1.
        char nm [16];
        int n = (int)lrint (v);
        note_name (n, nm, sizeof (nm));
        snprintf (b, sizeof (b), "%s  %.1f Hz", nm, p->val [P_TUNE] * pow (2.0, (n - 69) / 12.0));
        return b;
    }
    case P_TUNE:   snprintf (b, sizeof (b), "%.1f Hz", v); return b;
    case P_BIAS:   snprintf (b, sizeof (b), "%.2f", v); return b;
    case P_SMOOTH: snprintf (b, sizeof (b), "%.3f", v); return b;
    case P_CORR:   snprintf (b, sizeof (b), "%d %%", (int)lrint (v * 100)); return b;
    case P_OFFS:   snprintf (b, sizeof (b), "%+.2f st", v); return b;
    case P_MINHZ:
    case P_MAXHZ:  snprintf (b, sizeof (b), "%.0f Hz", v); return b;
    case P_THRESH: snprintf (b, sizeof (b), "%.0f dB", v); return b;
    case P_LOOK:
    {
        // delay in ms for this step at the current sample rate
        int base = p->sr < 64000 ? 1024 : p->sr < 128000 ? 2048 : 4096;
        int st = (int)lrint (v);
        if (st < 0) st = 0;
        if (st > 4) st = 4;
        snprintf (b, sizeof (b), st ? "%.0f ms" : "Off  %.0f ms", (base << st) * 1000.0 / p->sr);
        return b;
    }
    case P_LOCK:   return lrint (v) ? "On" : "Off";
    case P_DETECT:
    {
        if (v < 0.02) return "Clear";
        if (v > 0.98) return "Loose";
        snprintf (b, sizeof (b), "%d %% loose", (int)lrint (v * 100));
        return b;
    }
    case P_HOLD:
    {
        static const char *hn [4] = { "Off", "0.5 s", "2 s", "Forever" };
        int i = (int)lrint (v);
        return hn [i < 0 ? 0 : i > 3 ? 3 : i];
    }
    case P_MODE:
    {
        int i = (int)lrint (v);
        return mode_names [i < 0 ? 0 : i > 2 ? 2 : i];
    }
    }
    return "";
}

static uint32_t params_count (const clap_plugin_t *) { return P_COUNT; }

static bool params_get_info (const clap_plugin_t *pl, uint32_t i, clap_param_info_t *info)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (i >= P_COUNT) return false;
    memset (info, 0, sizeof (*info));
    info->id = i;
    info->flags = CLAP_PARAM_IS_AUTOMATABLE;
    if (param_info [i].stepped) info->flags |= CLAP_PARAM_IS_STEPPED;
    if (i == P_MODE) info->flags |= CLAP_PARAM_IS_ENUM;
    strncpy (info->name, param_info [i].name, CLAP_NAME_SIZE - 1);
    info->min_value = param_info [i].min;
    info->max_value = i == P_SCALE ? p->nscales : param_info [i].max;
    info->default_value = param_info [i].def;
    return true;
}

static bool params_get_value (const clap_plugin_t *pl, clap_id id, double *v)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (id >= P_COUNT) return false;
    *v = p->val [id];
    return true;
}

static bool params_value_to_text (const clap_plugin_t *pl, clap_id id, double v, char *out, uint32_t n)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (id >= P_COUNT) return false;
    snprintf (out, n, "%s", plugin_param_text (p, id, v).c_str ());
    return true;
}

static bool params_text_to_value (const clap_plugin_t *pl, clap_id id, const char *text, double *v)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (id >= P_COUNT) return false;
    if (id == P_SCALE)
    {
        std::string t = lower (text);
        if (t == "12-tet") { *v = 0; return true; }
        for (int i = 0; i < p->nscales; i++)
            if (lower (p->names [i]) == t) { *v = i + 1; return true; }
        return false;
    }
    if (id == P_MODE)
    {
        for (int i = 0; i < 3; i++)
            if (!strcasecmp (text, mode_names [i])) { *v = i; return true; }
    }
    *v = atof (text);
    return true;
}

static void params_apply_in (Plugin *p, const clap_input_events_t *in);
static void params_flush_out (Plugin *p, const clap_output_events_t *out);

static void params_flush (const clap_plugin_t *pl, const clap_input_events_t *in, const clap_output_events_t *out)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    params_apply_in (p, in);
    params_flush_out (p, out);
}

static const clap_plugin_params_t params_ext = {
    params_count, params_get_info, params_get_value, params_value_to_text,
    params_text_to_value, params_flush,
};

static double clampd (double v, double lo, double hi) { return v < lo ? lo : v > hi ? hi : v; }

static void handle_event (Plugin *p, const clap_event_header_t *h)
{
    if (h->space_id != CLAP_CORE_EVENT_SPACE_ID) return;
    switch (h->type)
    {
    case CLAP_EVENT_PARAM_VALUE:
    {
        auto *e = (const clap_event_param_value_t *)h;
        if (e->param_id < P_COUNT)
        {
            double mx = e->param_id == P_SCALE ? p->nscales : param_info [e->param_id].max;
            p->val [e->param_id] = clampd (e->value, param_info [e->param_id].min, mx);
        }
        break;
    }
    case CLAP_EVENT_NOTE_ON:
    {
        auto *e = (const clap_event_note_t *)h;
        if (e->key >= 0 && e->key < 128) p->held [e->key]++;
        break;
    }
    case CLAP_EVENT_NOTE_OFF:
    {
        auto *e = (const clap_event_note_t *)h;
        if (e->key >= 0 && e->key < 128 && p->held [e->key] > 0) p->held [e->key]--;
        break;
    }
    case CLAP_EVENT_MIDI:
    {
        auto *e = (const clap_event_midi_t *)h;
        int st = e->data [0] & 0xf0, k = e->data [1] & 127;
        if (st == 0x90 && e->data [2] > 0) p->held [k]++;
        else if (st == 0x80 || st == 0x90) { if (p->held [k] > 0) p->held [k]--; }
        else if (st == 0xb0 && (k == 123 || k == 120)) memset (p->held, 0, sizeof (p->held));
        break;
    }
    }
}

static void params_apply_in (Plugin *p, const clap_input_events_t *in)
{
    uint32_t n = in->size (in);
    for (uint32_t i = 0; i < n; i++) handle_event (p, in->get (in, i));
}

static void params_flush_out (Plugin *p, const clap_output_events_t *out)
{
    unsigned r = p->q_r.load (std::memory_order_relaxed);
    while (r != p->q_w.load (std::memory_order_acquire))
    {
        const GuiMsg &m = p->q [r & 255];
        if (m.kind == 1)
        {
            clap_event_param_value_t e;
            memset (&e, 0, sizeof (e));
            e.header.size = sizeof (e);
            e.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            e.header.type = CLAP_EVENT_PARAM_VALUE;
            e.param_id = m.id;
            e.note_id = e.port_index = e.channel = e.key = -1;
            e.value = m.value;
            out->try_push (out, &e.header);
        }
        else
        {
            clap_event_param_gesture_t e;
            memset (&e, 0, sizeof (e));
            e.header.size = sizeof (e);
            e.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
            e.header.type = m.kind == 0 ? CLAP_EVENT_PARAM_GESTURE_BEGIN : CLAP_EVENT_PARAM_GESTURE_END;
            e.param_id = m.id;
            out->try_push (out, &e.header);
        }
        r++;
    }
    p->q_r.store (r, std::memory_order_release);
}

// Called by the GUI thread.
void plugin_gui_param (Plugin *p, int kind, uint32_t id, double value)
{
    if (kind == 1 && id < P_COUNT)
    {
        double mx = id == P_SCALE ? p->nscales : param_info [id].max;
        value = clampd (value, param_info [id].min, mx);
        if (param_info [id].stepped) value = lrint (value);
        p->val [id] = value;
    }
    unsigned w = p->q_w.load (std::memory_order_relaxed);
    if (w - p->q_r.load (std::memory_order_acquire) >= 256) return; // queue full: drop
    p->q [w & 255] = { kind, id, value };
    p->q_w.store (w + 1, std::memory_order_release);
    if (p->host_params) p->host_params->request_flush (p->host);
}

// ----------------------------------------------------------------------------
// Audio

static uint32_t audio_ports_count (const clap_plugin_t *, bool) { return 1; }

static bool audio_ports_get (const clap_plugin_t *, uint32_t i, bool is_input, clap_audio_port_info_t *info)
{
    if (i) return false;
    memset (info, 0, sizeof (*info));
    info->id = is_input ? 0 : 1;
    strcpy (info->name, is_input ? "Input" : "Output");
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    info->channel_count = 2;
    info->port_type = CLAP_PORT_STEREO;
    info->in_place_pair = CLAP_INVALID_ID;
    return true;
}

static const clap_plugin_audio_ports_t audio_ports_ext = { audio_ports_count, audio_ports_get };

static uint32_t note_ports_count (const clap_plugin_t *, bool is_input) { return is_input ? 1 : 0; }

static bool note_ports_get (const clap_plugin_t *, uint32_t i, bool is_input, clap_note_port_info_t *info)
{
    if (i || !is_input) return false;
    info->id = 0;
    info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
    info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
    strcpy (info->name, "Target notes");
    return true;
}

static const clap_plugin_note_ports_t note_ports_ext = { note_ports_count, note_ports_get };

static uint32_t latency_get (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    return p->latency;
}

static const clap_plugin_latency_t latency_ext = { latency_get };

static bool plug_init (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    p->host_params = (const clap_host_params_t *)p->host->get_extension (p->host, CLAP_EXT_PARAMS);
    p->host_latency = (const clap_host_latency_t *)p->host->get_extension (p->host, CLAP_EXT_LATENCY);
    load_dir_settings (p);
    scan_scales (p);
    p->quit = false;
    p->loader = std::thread (loader_main, p);
    return true;
}

static void plug_destroy (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    gui_destroy_if_any (p);
    p->quit = true;
    if (p->loader.joinable ()) p->loader.join ();
    {
        std::lock_guard<std::mutex> lk (fftw_lock);
        delete p->rt [0];
        delete p->rt [1];
    }
    delete p;
}

static bool plug_activate (const clap_plugin_t *pl, double sr, uint32_t minf, uint32_t maxf)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    {
        std::lock_guard<std::mutex> lk (fftw_lock);
        delete p->rt [0];
        delete p->rt [1];
        p->sr = sr;
        p->rt [0] = new Retuner ((int)sr);
        p->rt [1] = new Retuner ((int)sr);
    }
    p->maxframes = maxf < 64 ? 64 : maxf > 8192 ? 8192 : maxf;
    for (int c = 0; c < 2; c++) p->scratch [c].assign (p->maxframes, 0.f);
    p->look_applied = (int)lrint (p->val [P_LOOK]);
    p->rt [0]->set_lookahead (p->look_applied);
    p->rt [1]->set_lookahead (p->look_applied);
    p->latency = p->rt [0]->latency ();
    memset (p->held, 0, sizeof (p->held));
    p->tab_valid = false;
    p->seen_gen = 0;
    return true;
}

static void plug_deactivate (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    std::lock_guard<std::mutex> lk (fftw_lock);
    delete p->rt [0]; delete p->rt [1];
    p->rt [0] = p->rt [1] = nullptr;
}

static bool plug_start (const clap_plugin_t *) { return true; }
static void plug_stop (const clap_plugin_t *) {}
static void plug_reset (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    memset (p->held, 0, sizeof (p->held));
}

static clap_process_status plug_process (const clap_plugin_t *pl, const clap_process_t *pr)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (!p->rt [0]) return CLAP_PROCESS_ERROR;

    params_apply_in (p, pr->in_events);
    params_flush_out (p, pr->out_events);

    if (pr->audio_inputs_count < 1 || pr->audio_outputs_count < 1) return CLAP_PROCESS_CONTINUE;
    const clap_audio_buffer_t *ib = &pr->audio_inputs [0];
    const clap_audio_buffer_t *ob = &pr->audio_outputs [0];
    uint32_t nch = ib->channel_count < ob->channel_count ? ib->channel_count : ob->channel_count;
    if (nch > 2) nch = 2;

    // Scale table: refresh when the loader published a new scale or the root moved.
    int root = (int)lrint (p->val [P_ROOT]);
    if (p->mtx.try_lock ())
    {
        if (p->cur_gen != p->seen_gen)
        {
            p->local = p->cur;
            p->seen_gen = p->cur_gen;
            p->tab_valid = false;
        }
        p->mtx.unlock ();
    }
    if (!p->tab_valid && p->seen_gen == 0) { p->local.twelve_tet (); }
    if (!p->tab_valid || root != p->tab_root)
    {
        TabInfo ti = scl_build (&p->local, root, p->tab, TAB_MAX);
        p->tab_n = ti.n;
        p->tab_kmin = ti.kmin;
        p->tab_root = root;
        p->tab_valid = true;
        // The largest gap to a held note class is half a period; allow that.
        double period = p->local.oct [p->local.n - 1];
        p->maxdist = (float)(period / 2 + 0.02 > 1.0 ? period / 2 + 0.02 : 1.0);
        for (uint32_t c = 0; c < 2; c++)
            if (p->rt [c]) p->rt [c]->set_table (p->tab, p->tab_n);
    }

    // Allowed notes. A held MIDI key stands for a note of the scale in every octave
    // (its pitch class): step = key - root, counted within one period.
    int mode = (int)lrint (p->val [P_MODE]);
    const int ns = p->local.n;
    bool any = false;
    memset (p->cls, 0, ns);
    for (int m = 0; m < 128; m++)
        if (p->held [m])
        {
            int r = (((m - root) % ns) + ns) % ns;
            p->cls [r] = 1;
            any = true;
        }
    const int words = (p->tab_n + 31) / 32;
    if (mode == 2 || (mode == 0 && !any))
        for (int w = 0; w < words; w++) p->act [w] = 0xffffffffu;
    else
    {
        memset (p->act, 0, words * sizeof (unsigned int));
        for (int e = 0; e < p->tab_n; e++)
        {
            int r = (((p->tab_kmin + e) % ns) + ns) % ns;
            if (p->cls [r]) p->act [e >> 5] |= 1u << (e & 31);
        }
    }

    int look = (int)lrint (p->val [P_LOOK]);
    if (look != p->look_applied)
    {
        p->look_applied = look;
        p->rt [0]->set_lookahead (look);
        p->rt [1]->set_lookahead (look);
        p->latency = p->rt [0]->latency ();
        p->latency_dirty = true;
        p->host->request_callback (p->host);   // the host is told from the main thread
    }

    for (uint32_t c = 0; c < nch; c++)
    {
        Retuner *r = p->rt [c];
        r->set_refpitch ((float)p->val [P_TUNE]);
        r->set_notebias ((float)p->val [P_BIAS]);
        r->set_corrfilt ((float)p->val [P_SMOOTH]);
        r->set_corrgain ((float)p->val [P_CORR]);
        r->set_corroffs ((float)p->val [P_OFFS]);
        r->set_range ((float)p->val [P_MINHZ], (float)p->val [P_MAXHZ]);
        r->set_threshold ((float)p->val [P_THRESH]);
        r->set_lock (lrint (p->val [P_LOCK]) != 0);
        r->set_looseness ((float)p->val [P_DETECT]);   // 0 = Clear ... 1 = Loose
        {
            static const float hold_s [4] = { 0.f, 0.5f, 2.f, -1.f };
            int hi = (int)lrint (p->val [P_HOLD]);
            r->set_hold (hold_s [hi < 0 ? 0 : hi > 3 ? 3 : hi]);
        }
        r->set_active (p->act);
        r->set_maxdist (p->maxdist);
    }

    uint32_t done = 0, total = pr->frames_count;
    while (done < total)
    {
        uint32_t n = total - done;
        if (n > p->maxframes) n = p->maxframes;
        for (uint32_t c = 0; c < nch; c++)
        {
            memcpy (p->scratch [c].data (), ib->data32 [c] + done, n * sizeof (float));
            p->rt [c]->process (n, p->scratch [c].data (), ob->data32 [c] + done);
        }
        done += n;
    }
    for (uint32_t c = nch; c < ob->channel_count; c++)
        memset (ob->data32 [c], 0, total * sizeof (float));

    for (uint32_t c = 0; c < nch; c++)
    {
        Retuner *r = p->rt [c];
        int e = r->get_note ();   // table entry -> the key that plays that step
        p->d_note [c] = e >= 0 ? root + p->tab_kmin + e : NOTE_NONE;
        p->d_err [c] = r->get_error ();
        p->d_cycle [c] = r->get_cycle ();
        p->d_voiced [c] = r->voiced () ? 1 : 0;
    }
    return CLAP_PROCESS_CONTINUE;
}

// ----------------------------------------------------------------------------
// State: simple text lines "key=value".

static bool state_save (const clap_plugin_t *pl, const clap_ostream_t *s)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    std::string t = "scalatuner 1\n";
    char b [256];
    int si = (int)lrint (p->val [P_SCALE]);
    t += "scale=" + std::string (si > 0 && si <= p->nscales ? p->names [si - 1] : "12-TET") + "\n";
    for (int i = 1; i < P_COUNT; i++)
    {
        snprintf (b, sizeof (b), "p%d=%.9g\n", i, (double)p->val [i]);
        t += b;
    }
    const char *d = t.data ();
    size_t left = t.size ();
    while (left)
    {
        int64_t w = s->write (s, d, left);
        if (w <= 0) return false;
        d += w;
        left -= w;
    }
    return true;
}

static bool state_load (const clap_plugin_t *pl, const clap_istream_t *s)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    std::string t;
    char buf [1024];
    int64_t r;
    while ((r = s->read (s, buf, sizeof (buf))) > 0) t.append (buf, r);
    size_t pos = 0;
    while (pos < t.size ())
    {
        size_t e = t.find ('\n', pos);
        if (e == std::string::npos) e = t.size ();
        std::string line = t.substr (pos, e - pos);
        pos = e + 1;
        size_t eq = line.find ('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr (0, eq), v = line.substr (eq + 1);
        if (k == "scale")
        {
            int idx = 0;
            for (int i = 0; i < p->nscales; i++)
                if (p->names [i] == v) { idx = i + 1; break; }
            p->val [P_SCALE] = idx;
        }
        else if (k.size () >= 2 && k [0] == 'p')
        {
            int i = atoi (k.c_str () + 1);
            if (i > 0 && i < P_COUNT)
                p->val [i] = clampd (atof (v.c_str ()), param_info [i].min, param_info [i].max);
        }
    }
    return true;
}

static const clap_plugin_state_t state_ext = { state_save, state_load };

static const void *plug_get_extension (const clap_plugin_t *, const char *id)
{
    if (!strcmp (id, CLAP_EXT_AUDIO_PORTS)) return &audio_ports_ext;
    if (!strcmp (id, CLAP_EXT_NOTE_PORTS)) return &note_ports_ext;
    if (!strcmp (id, CLAP_EXT_PARAMS)) return &params_ext;
    if (!strcmp (id, CLAP_EXT_LATENCY)) return &latency_ext;
    if (!strcmp (id, CLAP_EXT_STATE)) return &state_ext;
    if (!strcmp (id, CLAP_EXT_GUI)) return gui_extension ();
    if (!strcmp (id, CLAP_EXT_TIMER_SUPPORT)) return gui_timer_extension ();
    return nullptr;
}

static void plug_on_main_thread (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (p->latency_dirty.exchange (false) && p->host_latency) p->host_latency->changed (p->host);
}

// ----------------------------------------------------------------------------
// Factory

static const char *features [] = { CLAP_PLUGIN_FEATURE_AUDIO_EFFECT, CLAP_PLUGIN_FEATURE_PITCH_CORRECTION, CLAP_PLUGIN_FEATURE_STEREO, nullptr };

static const clap_plugin_descriptor_t descriptor = {
    CLAP_VERSION, "org.tjingboem.scalatuner", "ScalaTuner", "tjingboem productions",
    "", "", "",
    "0.1.0",
    "Stereo autotuner that snaps pitch to Scala (.scl) scales. Core: zita-at1 by Fons Adriaensen.",
    features
};

static const clap_plugin_t *factory_create (const clap_plugin_factory_t *, const clap_host_t *host, const char *id)
{
    if (strcmp (id, descriptor.id)) return nullptr;
    Plugin *p = new Plugin ();
    p->host = host;
    p->host_params = nullptr;
    p->host_latency = nullptr;
    for (int i = 0; i < P_COUNT; i++) p->val [i] = param_info [i].def;
    p->nscales = 0;
    p->cur_gen = 0; p->cur_index = 0; p->cur_ok = true;
    p->cur.twelve_tet ();
    p->local.twelve_tet ();
    p->rt [0] = p->rt [1] = nullptr;
    p->sr = 48000; p->maxframes = 0; p->seen_gen = 0; p->tab_valid = false; p->tab_root = 60; p->latency = 0; p->latency_dirty = false; p->look_applied = 0;
    memset (p->held, 0, sizeof (p->held));
    p->q_w = p->q_r = 0;
    for (int c = 0; c < 2; c++) { p->d_note [c] = NOTE_NONE; p->d_err [c] = 0; p->d_cycle [c] = 0; p->d_voiced [c] = 0; }
    p->gui = nullptr;
    p->quit = false;
    p->plugin.desc = &descriptor;
    p->plugin.plugin_data = p;
    p->plugin.init = plug_init;
    p->plugin.destroy = plug_destroy;
    p->plugin.activate = plug_activate;
    p->plugin.deactivate = plug_deactivate;
    p->plugin.start_processing = plug_start;
    p->plugin.stop_processing = plug_stop;
    p->plugin.reset = plug_reset;
    p->plugin.process = plug_process;
    p->plugin.get_extension = plug_get_extension;
    p->plugin.on_main_thread = plug_on_main_thread;
    return &p->plugin;
}

static uint32_t factory_count (const clap_plugin_factory_t *) { return 1; }
static const clap_plugin_descriptor_t *factory_desc (const clap_plugin_factory_t *, uint32_t i) { return i ? nullptr : &descriptor; }
static const clap_plugin_factory_t factory = { factory_count, factory_desc, factory_create };

static bool entry_init (const char *) { return true; }
static void entry_deinit (void) {}
static const void *entry_get_factory (const char *id)
{
    return !strcmp (id, CLAP_PLUGIN_FACTORY_ID) ? &factory : nullptr;
}

extern "C" CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION, entry_init, entry_deinit, entry_get_factory
};
