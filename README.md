# ScalaTuner

**ScalaTune** is a stereo CLAP autotuner that pulls the pitch of a sound onto a
[Scala](https://www.huygens-fokker.org/scala/) (`.scl`) scale: any number of notes per period,
any period (octave, tritave, ...). The left and right channels are tuned independently.
It has its own GUI, with a scale browser that reads your `.scl` files at runtime.

It is monophonic: one detected pitch per channel is moved to the nearest note of the scale.
Good for single voices or single lines, used either as a corrector or as a creative
"push this onto a grid" effect.

The pitch tracking and resynthesis core is **zita-at1 0.8.2** by Fons Adriaensen. Only the note
matching step was replaced, with a 128-note lookup table built from the scale.

## Features
- Any Scala scale, loaded at runtime from a folder (thousands are fine)
- Stereo, with independent pitch tracking per channel
- GUI: scale browser with type-to-search, one-period grid strip with live L/R pitch markers,
  pitch-error meters, sliders
- Modes: **Auto** (held MIDI notes if any, otherwise the whole scale), **MIDI** (held notes only),
  **Manual** (whole scale)
- **Scale Root** sets which MIDI note carries the scale's 1/1 (its sounding Hz is shown next to it); **A Reference** sets the pitch of A4. MIDI notes map one per scale degree
- Reports its latency to the host (1024 samples at 48 kHz)
- Saves the scale by name, so projects survive re-ordering of your scale folder

## Manual
See [`docs/ScalaTune.odt`](docs/ScalaTune.odt) (source text: `docs/ScalaTune.md`; rebuilt with `tools/make_manual.py`).

## Where the scales come from
In this order:
1. the `SCALATUNE_SCALES` environment variable
2. the path in the file `~/.config/scalatune/scales_dir`
3. `~/scl`, `~/scales` or `~/.scala/scl`

The scale list is read when the plugin is created. A good source is the Scala scale archive
(about 5000 `.scl` files).

## Build (Linux, X11)
Needs a C++ compiler, `fftw3f`, `x11` and `cairo` development packages.

```
make
cp build/ScalaTune.clap ~/.clap/
```

The CLAP headers are included in `include/`. It was developed and tested against REAPER on Linux Mint 22.
The GUI uses X11, so there is no Wayland-native window; other hosts and platforms are untested.

## Status
Early. The DSP and GUI were tested with a synthetic test host (pitch snapping, latency, state save/load, GUI
interaction). Listening tests on real voices and testing in more hosts are still to come.

## Credits and licence
- Pitch tracking core: **zita-at1** 0.8.2, (C) 2010-2024 Fons Adriaensen, GPL v3
- Resampler: from Fons Adriaensen's zita-resampler, via x42's fat1.lv2 (GPL)
- CLAP headers: free-audio/clap, MIT (see `include/CLAP-LICENSE`)

ScalaTune is distributed under the **GNU General Public License v3** (see `COPYING`).
