// ScalaTuner GUI: plain Xlib + cairo, embedded into the host via CLAP gui (x11).
#include "plugin.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cairo/cairo-xlib.h>
#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

enum { W = 760, H = 444 };

// Layout
static const int LX = 12, LY = 46, LW = 290, LH = 386;      // scale browser
static const int ROW_H = 22, SEARCH_H = 26;
static const int RX = 318, RW = 430;                         // right column

struct Slider { int param; int y; bool log; };
static const Slider sliders [] = {
    { P_ROOT,   236, false },
    { P_TUNE,   266, false },
    { P_BIAS,   296, false },
    { P_SMOOTH, 326, true  },
    { P_CORR,   356, false },
    { P_OFFS,   386, false },
};
static const int NSL = 6;
static const int SL_X = RX + 110, SL_W = RW - 110 - 120, SL_H = 10;

struct Gui
{
    Plugin *p;
    Display *dpy;
    Window win;
    bool floating;
    bool visible;
    cairo_surface_t *surf;
    clap_id timer_id;
    const clap_host_timer_support_t *host_timer;

    // browser state
    std::string filter;
    std::vector<int> shown;      // scale indexes (0 = 12-TET)
    int scroll;                  // first visible row
    bool search_focus;
    bool dragging_scroll;

    // slider drag
    int  drag;                   // slider number or -1
    int  last_click_slider;
    long last_click_ms;
    bool dirty_list;

    // cached current scale for drawing
    Scl     scl;
    uint32_t scl_gen;
    int     scl_index;
    bool    scl_ok;
};

static long now_ms (void)
{
    struct timeval tv;
    gettimeofday (&tv, 0);
    return tv.tv_sec * 1000L + tv.tv_usec / 1000;
}

static std::string lower_s (std::string s)
{
    for (auto &c : s) c = tolower ((unsigned char)c);
    return s;
}

static std::string scale_name (Plugin *p, int idx)
{
    return idx <= 0 || idx > p->nscales ? "12-TET" : p->names [idx - 1];
}

static void rebuild_list (Gui *g)
{
    g->shown.clear ();
    std::string f = lower_s (g->filter);
    for (int i = 0; i <= g->p->nscales; i++)
        if (f.empty () || lower_s (scale_name (g->p, i)).find (f) != std::string::npos)
            g->shown.push_back (i);
    int rows = (LH - SEARCH_H - 6) / ROW_H;
    int maxs = std::max (0, (int)g->shown.size () - rows);
    if (g->scroll > maxs) g->scroll = maxs;
    if (g->scroll < 0) g->scroll = 0;
}

static void scroll_to_selected (Gui *g)
{
    int cur = (int)lrint (g->p->val [P_SCALE]);
    int rows = (LH - SEARCH_H - 6) / ROW_H;
    for (size_t i = 0; i < g->shown.size (); i++)
        if (g->shown [i] == cur)
        {
            if ((int)i < g->scroll || (int)i >= g->scroll + rows)
                g->scroll = std::max (0, (int)i - rows / 2);
            break;
        }
    rebuild_list (g);
}

// ---------------------------------------------------------------- drawing

static void rrect (cairo_t *cr, double x, double y, double w, double h, double r)
{
    cairo_new_sub_path (cr);
    cairo_arc (cr, x + w - r, y + r, r, -M_PI / 2, 0);
    cairo_arc (cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc (cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_arc (cr, x + r, y + r, r, M_PI, 1.5 * M_PI);
    cairo_close_path (cr);
}

static void col (cairo_t *cr, unsigned rgb, double a = 1.0)
{
    cairo_set_source_rgba (cr, ((rgb >> 16) & 255) / 255.0, ((rgb >> 8) & 255) / 255.0, (rgb & 255) / 255.0, a);
}

static void text (cairo_t *cr, double x, double y, const char *s, double size, unsigned rgb, int align = 0)
{
    cairo_set_font_size (cr, size);
    col (cr, rgb);
    if (align)
    {
        cairo_text_extents_t e;
        cairo_text_extents (cr, s, &e);
        x -= align == 1 ? e.x_advance : e.x_advance / 2;
    }
    cairo_move_to (cr, x, y);
    cairo_show_text (cr, s);
}

static double slider_norm (const Slider &s, double v)
{
    const ParamInfo &pi = param_info [s.param];
    if (s.log) return (log (v) - log (pi.min)) / (log (pi.max) - log (pi.min));
    return (v - pi.min) / (pi.max - pi.min);
}

static double slider_value (const Slider &s, double n)
{
    const ParamInfo &pi = param_info [s.param];
    if (n < 0) n = 0;
    if (n > 1) n = 1;
    if (s.log) return exp (log (pi.min) + n * (log (pi.max) - log (pi.min)));
    return pi.min + n * (pi.max - pi.min);
}

static void draw (Gui *g, cairo_t *cr)
{
    Plugin *p = g->p;
    cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    // Refresh cached scale.
    if (p->mtx.try_lock ())
    {
        if (p->cur_gen != g->scl_gen)
        {
            g->scl = p->cur;
            g->scl_gen = p->cur_gen;
            g->scl_index = p->cur_index;
            g->scl_ok = p->cur_ok;
        }
        p->mtx.unlock ();
    }

    col (cr, 0x14161b); cairo_paint (cr);

    // Header
    char b [256];
    text (cr, 14, 28, "ScalaTuner", 20, 0xe8ecf4);
    int cur = (int)lrint (p->val [P_SCALE]);
    {
        double period = g->scl.oct [g->scl.n - 1];
        snprintf (b, sizeof (b), "%s   %d notes, period %.2f cents%s", scale_name (p, cur).c_str (), g->scl.n,
                  period * 1200.0, g->scl_ok ? "" : "   (file unreadable, using 12-TET)");
        text (cr, 140, 28, b, 13, g->scl_ok ? 0x9fd8ff : 0xff8a7a);
    }

    // ---- scale browser
    col (cr, 0x1d2027); rrect (cr, LX, LY, LW, LH, 6); cairo_fill (cr);
    col (cr, g->search_focus ? 0x2c3340 : 0x242830); rrect (cr, LX + 6, LY + 6, LW - 12, SEARCH_H - 4, 4); cairo_fill (cr);
    if (g->filter.empty ()) text (cr, LX + 14, LY + 24, "type to search scales", 13, 0x6c7380);
    else text (cr, LX + 14, LY + 24, (g->filter + (g->search_focus ? "|" : "")).c_str (), 13, 0xe8ecf4);
    snprintf (b, sizeof (b), "%d", (int)g->shown.size ());
    text (cr, LX + LW - 14, LY + 24, b, 11, 0x6c7380, 1);

    int rows = (LH - SEARCH_H - 6) / ROW_H;
    int ry0 = LY + SEARCH_H + 6;
    cairo_save (cr);
    cairo_rectangle (cr, LX, ry0, LW - 10, rows * ROW_H);
    cairo_clip (cr);
    for (int r = 0; r < rows; r++)
    {
        int k = g->scroll + r;
        if (k >= (int)g->shown.size ()) break;
        int idx = g->shown [k];
        int y = ry0 + r * ROW_H;
        if (idx == cur)
        {
            col (cr, 0x2a6fa8); rrect (cr, LX + 4, y, LW - 20, ROW_H - 2, 4); cairo_fill (cr);
        }
        text (cr, LX + 12, y + 16, scale_name (p, idx).c_str (), 13, idx == cur ? 0xffffff : 0xc4cad6);
    }
    cairo_restore (cr);
    // scrollbar
    if ((int)g->shown.size () > rows)
    {
        double th = (double)rows / g->shown.size ();
        double ty = (double)g->scroll / g->shown.size ();
        int sbh = rows * ROW_H;
        col (cr, 0x2a2e37); rrect (cr, LX + LW - 9, ry0, 5, sbh, 2); cairo_fill (cr);
        col (cr, 0x5a6578); rrect (cr, LX + LW - 9, ry0 + ty * sbh, 5, std::max (16.0, th * sbh), 2); cairo_fill (cr);
    }

    // ---- grid strip (one period)
    double period = g->scl.oct [g->scl.n - 1];
    int sy = 52, sh = 92;
    col (cr, 0x1d2027); rrect (cr, RX, sy - 6, RW, sh + 8, 6); cairo_fill (cr);
    text (cr, RX + 10, sy + 10, "GRID  (one period, root at left)", 11, 0x6c7380);
    int gx = RX + 14, gw = RW - 28, gy = sy + 22;
    col (cr, 0x2a2e37); cairo_rectangle (cr, gx, gy + 38, gw, 1); cairo_fill (cr);
    for (int i = 0; i < g->scl.n; i++)
    {
        double pos = i == 0 ? 0 : g->scl.oct [i - 1] / period;
        double x = gx + pos * gw;
        col (cr, 0x7fb0d8); cairo_rectangle (cr, x - 0.5, gy + 20, 1.5, 36); cairo_fill (cr);
        if (g->scl.n <= 24 && i > 0)
        {
            snprintf (b, sizeof (b), "%d", i);
            text (cr, x, gy + 68, b, 9, 0x6c7380, 2);
        }
    }
    col (cr, 0xe8ecf4); cairo_rectangle (cr, gx + gw - 0.5, gy + 20, 1.5, 36); cairo_fill (cr); // period = next 1/1
    double tune = p->val [P_TUNE];
    double rootlog = ((int)lrint (p->val [P_ROOT]) - 69) / 12.0;
    static const unsigned chcol [2] = { 0x5cc8ff, 0xffb35c };
    for (int c = 0; c < 2; c++)
    {
        if (!p->d_voiced [c] || p->d_cycle [c] <= 0) continue;
        double f = p->sr / p->d_cycle [c];
        double lg = log2 (f / tune) - rootlog;           // relative to 1/1
        double fr = lg / period;
        fr -= floor (fr);
        double x = gx + fr * gw;
        col (cr, chcol [c]); cairo_rectangle (cr, x - 1.5, gy + 6 + c * 4, 3, 56 - c * 4); cairo_fill (cr);
        text (cr, x, gy + 4 + c * 0, c ? "R" : "L", 10, chcol [c], 2);
    }

    // ---- meters
    int my = 160;
    col (cr, 0x1d2027); rrect (cr, RX, my, RW, 64, 6); cairo_fill (cr);
    for (int c = 0; c < 2; c++)
    {
        int y = my + 8 + c * 28;
        text (cr, RX + 12, y + 14, c ? "R" : "L", 13, chcol [c]);
        int note = p->d_note [c];
        char nn [16];
        if (p->d_voiced [c] && note >= 0) note_name (note, nn, sizeof (nn)); else snprintf (nn, sizeof (nn), "-");
        text (cr, RX + 34, y + 14, nn, 14, 0xe8ecf4);
        double err = p->d_voiced [c] ? p->d_err [c] * 100.0 : 0.0;   // cents
        snprintf (b, sizeof (b), p->d_voiced [c] ? "%+.0f c" : "", err);
        text (cr, RX + 84, y + 14, b, 12, 0x9aa3b4);
        int mx = RX + 140, mw = RW - 140 - 16;
        col (cr, 0x2a2e37); rrect (cr, mx, y + 3, mw, 12, 3); cairo_fill (cr);
        col (cr, 0x4a5262); cairo_rectangle (cr, mx + mw / 2 - 0.5, y + 1, 1.5, 16); cairo_fill (cr);
        if (p->d_voiced [c])
        {
            double e = std::max (-100.0, std::min (100.0, err)) / 100.0;
            col (cr, chcol [c]);
            double x0 = mx + mw / 2, x1 = x0 + e * mw / 2;
            cairo_rectangle (cr, std::min (x0, x1), y + 4, fabs (x1 - x0) + 1, 10); cairo_fill (cr);
        }
    }

    // ---- sliders
    col (cr, 0x1d2027); rrect (cr, RX, 222, RW, 212, 6); cairo_fill (cr);
    for (int i = 0; i < NSL; i++)
    {
        const Slider &s = sliders [i];
        double v = p->val [s.param];
        text (cr, RX + 12, s.y + 9, param_info [s.param].name, 13, 0xc4cad6);
        col (cr, 0x2a2e37); rrect (cr, SL_X, s.y, SL_W, SL_H, 4); cairo_fill (cr);
        double n = slider_norm (s, v);
        col (cr, g->drag == i ? 0x7fd6ff : 0x4aa8e0); rrect (cr, SL_X, s.y, std::max (6.0, n * SL_W), SL_H, 4); cairo_fill (cr);
        text (cr, RX + RW - 12, s.y + 9, plugin_param_text (p, s.param, v).c_str (), 12, 0xe8ecf4, 1);
    }

    // ---- mode buttons
    int by = 410, bw = 86;
    text (cr, RX + 12, by + 16, "Mode", 13, 0xc4cad6);
    int mode = (int)lrint (p->val [P_MODE]);
    static const char *mn [3] = { "Auto", "MIDI", "Manual" };
    for (int i = 0; i < 3; i++)
    {
        int x = SL_X + i * (bw + 6);
        col (cr, i == mode ? 0x2a6fa8 : 0x2a2e37); rrect (cr, x, by, bw, 24, 5); cairo_fill (cr);
        text (cr, x + bw / 2, by + 17, mn [i], 13, i == mode ? 0xffffff : 0xa8b0c0, 2);
    }
}

static void redraw (Gui *g)
{
    if (!g->surf) return;
    cairo_t *cr = cairo_create (g->surf);
    cairo_push_group (cr);
    draw (g, cr);
    cairo_pop_group_to_source (cr);
    cairo_paint (cr);
    cairo_destroy (cr);
    XFlush (g->dpy);
}

// ---------------------------------------------------------------- input

static int slider_at (int x, int y)
{
    for (int i = 0; i < NSL; i++)
        if (y >= sliders [i].y - 8 && y <= sliders [i].y + SL_H + 8 && x >= RX && x <= RX + RW) return i;
    return -1;
}

static void set_slider_from_x (Gui *g, int i, int x)
{
    const Slider &s = sliders [i];
    double n = (double)(x - SL_X) / SL_W;
    double v = slider_value (s, n);
    if (s.param == P_ROOT) v = lrint (v);
    plugin_gui_param (g->p, 1, s.param, v);
}

static void choose_scale (Gui *g, int idx)
{
    plugin_gui_param (g->p, 0, P_SCALE, idx);
    plugin_gui_param (g->p, 1, P_SCALE, idx);
    plugin_gui_param (g->p, 2, P_SCALE, idx);
}

static void on_button (Gui *g, XButtonEvent *e)
{
    int x = e->x, y = e->y;
    int rows = (LH - SEARCH_H - 6) / ROW_H;
    int ry0 = LY + SEARCH_H + 6;
    int maxs = std::max (0, (int)g->shown.size () - rows);
    bool in_list = x >= LX && x < LX + LW && y >= LY && y < LY + LH;

    if (e->button == 4 || e->button == 5)
    {
        int dir = e->button == 4 ? -1 : 1;
        if (in_list)
        {
            g->scroll = std::max (0, std::min (maxs, g->scroll + dir * 3));
        }
        else
        {
            int i = slider_at (x, y);
            if (i >= 0)
            {
                const Slider &s = sliders [i];
                double n = slider_norm (s, g->p->val [s.param]) - dir * 0.02;
                double v = slider_value (s, n);
                if (s.param == P_ROOT) v = lrint (g->p->val [P_ROOT]) - dir;
                plugin_gui_param (g->p, 0, s.param, v);
                plugin_gui_param (g->p, 1, s.param, v);
                plugin_gui_param (g->p, 2, s.param, v);
            }
        }
        return;
    }
    if (e->button != 1) return;

    if (in_list)
    {
        if (y < ry0)
        {
            g->search_focus = true;
            XSetInputFocus (g->dpy, g->win, RevertToParent, CurrentTime);
        }
        else if (x > LX + LW - 16 && (int)g->shown.size () > rows)
        {
            g->dragging_scroll = true;
            double f = (double)(y - ry0) / (rows * ROW_H);
            g->scroll = std::max (0, std::min (maxs, (int)(f * g->shown.size () - rows / 2)));
        }
        else
        {
            int k = g->scroll + (y - ry0) / ROW_H;
            if (k >= 0 && k < (int)g->shown.size ()) choose_scale (g, g->shown [k]);
            g->search_focus = false;
        }
        return;
    }

    int i = slider_at (x, y);
    if (i >= 0)
    {
        long t = now_ms ();
        const Slider &s = sliders [i];
        if (g->last_click_slider == i && t - g->last_click_ms < 350)
        {
            plugin_gui_param (g->p, 0, s.param, param_info [s.param].def);
            plugin_gui_param (g->p, 1, s.param, param_info [s.param].def);
            plugin_gui_param (g->p, 2, s.param, param_info [s.param].def);
            g->last_click_slider = -1;
            return;
        }
        g->last_click_slider = i;
        g->last_click_ms = t;
        g->drag = i;
        plugin_gui_param (g->p, 0, s.param, g->p->val [s.param]);
        set_slider_from_x (g, i, x);
        return;
    }
    // mode buttons
    int by = 410, bw = 86;
    if (y >= by && y < by + 24)
        for (int m = 0; m < 3; m++)
        {
            int bx = SL_X + m * (bw + 6);
            if (x >= bx && x < bx + bw)
            {
                plugin_gui_param (g->p, 0, P_MODE, m);
                plugin_gui_param (g->p, 1, P_MODE, m);
                plugin_gui_param (g->p, 2, P_MODE, m);
            }
        }
    g->search_focus = false;
}

static void on_motion (Gui *g, XMotionEvent *e)
{
    if (g->drag >= 0) set_slider_from_x (g, g->drag, e->x);
    else if (g->dragging_scroll)
    {
        int rows = (LH - SEARCH_H - 6) / ROW_H;
        int ry0 = LY + SEARCH_H + 6;
        int maxs = std::max (0, (int)g->shown.size () - rows);
        double f = (double)(e->y - ry0) / (rows * ROW_H);
        g->scroll = std::max (0, std::min (maxs, (int)(f * g->shown.size () - rows / 2)));
    }
}

static void on_key (Gui *g, XKeyEvent *e)
{
    if (!g->search_focus) return;
    char buf [16];
    KeySym ks;
    int n = XLookupString (e, buf, sizeof (buf) - 1, &ks, 0);
    if (ks == XK_BackSpace) { if (!g->filter.empty ()) g->filter.pop_back (); }
    else if (ks == XK_Escape) { g->filter.clear (); g->search_focus = false; }
    else if (ks == XK_Return && g->shown.size () == 1) { choose_scale (g, g->shown [0]); }
    else if (n > 0 && (unsigned char)buf [0] >= 32 && buf [0] != 127) g->filter.append (buf, n);
    else return;
    g->scroll = 0;
    rebuild_list (g);
}

// ---------------------------------------------------------------- CLAP gui

static Gui *G (const clap_plugin_t *pl) { return ((Plugin *)pl->plugin_data)->gui; }

static bool gui_is_api_supported (const clap_plugin_t *, const char *api, bool) { return !strcmp (api, CLAP_WINDOW_API_X11); }
static bool gui_get_preferred_api (const clap_plugin_t *, const char **api, bool *floating)
{
    *api = CLAP_WINDOW_API_X11;
    *floating = false;
    return true;
}

static void make_window (Gui *g, Window parent)
{
    int scr = DefaultScreen (g->dpy);
    XSetWindowAttributes a;
    memset (&a, 0, sizeof (a));
    a.background_pixel = 0x14161b;
    a.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask |
                   StructureNotifyMask;
    g->win = XCreateWindow (g->dpy, parent ? parent : RootWindow (g->dpy, scr), 0, 0, W, H, 0,
                            CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWEventMask, &a);
    if (!parent)
    {
        XStoreName (g->dpy, g->win, "ScalaTuner");
        XSizeHints *sh = XAllocSizeHints ();
        sh->flags = PMinSize | PMaxSize;
        sh->min_width = sh->max_width = W;
        sh->min_height = sh->max_height = H;
        XSetWMNormalHints (g->dpy, g->win, sh);
        XFree (sh);
    }
    g->surf = cairo_xlib_surface_create (g->dpy, g->win, DefaultVisual (g->dpy, scr), W, H);
}

static bool gui_create (const clap_plugin_t *pl, const char *api, bool floating)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    if (strcmp (api, CLAP_WINDOW_API_X11)) return false;
    Gui *g = new Gui ();
    g->p = p;
    g->dpy = XOpenDisplay (nullptr);
    if (!g->dpy) { delete g; return false; }
    g->win = 0;
    g->surf = nullptr;
    g->floating = floating;
    g->visible = false;
    g->timer_id = CLAP_INVALID_ID;
    g->host_timer = (const clap_host_timer_support_t *)p->host->get_extension (p->host, CLAP_EXT_TIMER_SUPPORT);
    g->scroll = 0;
    g->search_focus = false;
    g->dragging_scroll = false;
    g->drag = -1;
    g->last_click_slider = -1;
    g->last_click_ms = 0;
    g->scl.twelve_tet ();
    g->scl_gen = 0;
    g->scl_index = 0;
    g->scl_ok = true;
    rebuild_list (g);
    scroll_to_selected (g);
    p->gui = g;
    if (floating) make_window (g, 0);
    return true;
}

static void gui_destroy (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    Gui *g = p->gui;
    if (!g) return;
    if (g->host_timer && g->timer_id != CLAP_INVALID_ID) g->host_timer->unregister_timer (p->host, g->timer_id);
    if (g->surf) cairo_surface_destroy (g->surf);
    if (g->win) XDestroyWindow (g->dpy, g->win);
    XCloseDisplay (g->dpy);
    p->gui = nullptr;
    delete g;
}

void gui_destroy_if_any (Plugin *p)
{
    if (p->gui) gui_destroy (&p->plugin);
}

static bool gui_set_scale (const clap_plugin_t *, double) { return false; }
static bool gui_get_size (const clap_plugin_t *, uint32_t *w, uint32_t *h) { *w = W; *h = H; return true; }
static bool gui_can_resize (const clap_plugin_t *) { return false; }
static bool gui_get_resize_hints (const clap_plugin_t *, clap_gui_resize_hints_t *) { return false; }
static bool gui_adjust_size (const clap_plugin_t *, uint32_t *w, uint32_t *h) { *w = W; *h = H; return true; }
static bool gui_set_size (const clap_plugin_t *, uint32_t, uint32_t) { return true; }

static bool gui_set_parent (const clap_plugin_t *pl, const clap_window_t *w)
{
    Gui *g = G (pl);
    if (!g || strcmp (w->api, CLAP_WINDOW_API_X11)) return false;
    if (g->win)
    {
        if (g->surf) { cairo_surface_destroy (g->surf); g->surf = nullptr; }
        XDestroyWindow (g->dpy, g->win);
        g->win = 0;
    }
    make_window (g, (Window)w->x11);
    return true;
}

static bool gui_set_transient (const clap_plugin_t *pl, const clap_window_t *w)
{
    Gui *g = G (pl);
    if (!g || !g->win) return false;
    XSetTransientForHint (g->dpy, g->win, (Window)w->x11);
    return true;
}

static void gui_suggest_title (const clap_plugin_t *pl, const char *title)
{
    Gui *g = G (pl);
    if (g && g->win && g->floating) XStoreName (g->dpy, g->win, title);
}

static bool gui_show (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    Gui *g = p->gui;
    if (!g || !g->win) return false;
    XMapRaised (g->dpy, g->win);
    XFlush (g->dpy);
    g->visible = true;
    if (g->host_timer && g->timer_id == CLAP_INVALID_ID) g->host_timer->register_timer (p->host, 33, &g->timer_id);
    return true;
}

static bool gui_hide (const clap_plugin_t *pl)
{
    Plugin *p = (Plugin *)pl->plugin_data;
    Gui *g = p->gui;
    if (!g || !g->win) return false;
    XUnmapWindow (g->dpy, g->win);
    XFlush (g->dpy);
    g->visible = false;
    if (g->host_timer && g->timer_id != CLAP_INVALID_ID)
    {
        g->host_timer->unregister_timer (p->host, g->timer_id);
        g->timer_id = CLAP_INVALID_ID;
    }
    return true;
}

static const clap_plugin_gui_t gui_ext = {
    gui_is_api_supported, gui_get_preferred_api, gui_create, gui_destroy, gui_set_scale, gui_get_size,
    gui_can_resize, gui_get_resize_hints, gui_adjust_size, gui_set_size, gui_set_parent, gui_set_transient,
    gui_suggest_title, gui_show, gui_hide,
};

static void gui_on_timer (const clap_plugin_t *pl, clap_id)
{
    Gui *g = G (pl);
    if (!g || !g->win) return;
    while (XPending (g->dpy))
    {
        XEvent ev;
        XNextEvent (g->dpy, &ev);
        switch (ev.type)
        {
        case ButtonPress:   on_button (g, &ev.xbutton); break;
        case ButtonRelease:
            if (ev.xbutton.button == 1)
            {
                if (g->drag >= 0) plugin_gui_param (g->p, 2, sliders [g->drag].param, g->p->val [sliders [g->drag].param]);
                g->drag = -1;
                g->dragging_scroll = false;
            }
            break;
        case MotionNotify:
            // collapse queued motion events
            while (XCheckTypedWindowEvent (g->dpy, g->win, MotionNotify, &ev)) {}
            on_motion (g, &ev.xmotion);
            break;
        case KeyPress: on_key (g, &ev.xkey); break;
        default: break;
        }
    }
    if (g->visible) redraw (g);
}

static const clap_plugin_timer_support_t timer_ext = { gui_on_timer };

const clap_plugin_gui_t *gui_extension (void) { return &gui_ext; }
const clap_plugin_timer_support_t *gui_timer_extension (void) { return &timer_ext; }
