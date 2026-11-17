# ScalaTuner

**ScalaTuner** is a stereo CLAP plug-in that brings a frequency structure to sound. It pulls the pitch of a sound onto a [Scala](https://www.huygens-fokker.org/scala/) (`.scl`) scale: any number of notes per period, any period (octave, tritave, ...). The left and right channels are tuned independently, and it has its own GUI with a scale browser that reads your `.scl` files at runtime.

It is not only for voices. It can correct a singer or a single instrument, but its purpose is wider: to give all kinds of sounds, tones and noises, a frequency structure. With the Detector slider towards Loose, a noisy, gliding or unsteady sound follows the grid in steps along the scale, and a sparse scale makes the steps clear.

It follows one pitch per channel: that pitch is moved to the nearest note of the scale.

The pitch tracking and resynthesis core is **zita-at1 0.8.2** by Fons Adriaensen. ScalaTuner replaces
its note matching with a table of the scale over the whole audible range, and adds MIDI pitch class,
the adjustable detector, Lookahead and Hold.

## Features
- Any Scala scale, loaded at runtime from a folder (thousands are fine)
- Stereo, with independent pitch tracking per channel
- GUI: scale browser with type-to-search, one-period grid strip with live L/R pitch markers,
  pitch-error meters, sliders
- **Detector** slider, Clear to Loose: how clear a pitch a sound needs before it is followed
- **Lookahead** (up to 340 ms) and **Hold**: for sounds with a slow start or a decaying tail
- Modes: **Auto** (held MIDI notes if any, otherwise the whole scale), **MIDI** (held notes only),
  **Manual** (whole scale)
- **Scale Root** sets which MIDI note carries the scale's 1/1 (its sounding Hz is shown next to it); **A Reference** sets the pitch of A4. MIDI notes map one per scale degree
- Reports its latency to the host (1024 samples at 48 kHz)
- Saves the scale by name, so projects survive re-ordering of your scale folder

## Manual
See [`docs/ScalaTuner.odt`](docs/ScalaTuner.odt) (source text: `docs/ScalaTuner.md`; rebuilt with `tools/make_manual.py`).

## Examples
`ScalaTuner_examples/` has audio for the five examples of the manual, with the settings in its `README.txt`:
before and after files for the drifting line, the Bohlen-Pierce sweep and the scale changes, and in
`synthetic/` a resonance with a MIDI file (play it from MIDI, example 2) and a noisy gliding sound
(Clear against Loose with Lock, example 3). Examples 2 and 3 also exist in versions made from the
author's own samples (a struck resonance and a ring-modulated sound); the others use synthetic sounds.

## Where the scales come from
Several folders can be used at once; all their scales appear in one list:
1. folders in the `SCALATUNER_SCALES` environment variable (separated by `:`)
2. folders on the plug-in's Settings page (add and remove them there), stored one per line in `~/.config/scalatuner/scales_dir`

On first run the first of `~/scl`, `~/scales`, `~/.scala/scl` that exists is added to the list (an ordinary, removable entry).

A good source is the Scala scale archive (more than 5200 `.scl` files): [scales.zip](https://www.huygens-fokker.org/docs/scales.zip), with a [contents listing](https://www.huygens-fokker.org/docs/scalesdir.txt) that describes every scale. To search the archive on the web (by name, description, number of tones), use the [ScalaVista Online Scale Archive](https://scalavista.zentralzone.com/scalearchive.php) by H-Pi Instruments. Download page: https://www.huygens-fokker.org/scala/downloads.html

## Build (Linux, X11)
Needs a C++ compiler, `fftw3f`, `x11` and `cairo` development packages.

```
make
cp build/ScalaTuner.clap ~/.clap/
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

ScalaTuner is distributed under the **GNU General Public License v3** (see `LICENSE`).
