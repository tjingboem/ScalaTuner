# ScalaTuner

## What it does

ScalaTuner is an autotuner that pulls the pitch of a sound onto a scale that you choose. The scale can be any Scala (.scl) file: any number of notes, and any period, so scales that repeat at the octave and scales that do not, such as Bohlen-Pierce, both work.

It follows one pitch at a time in each channel. Give it a voice, a bowed string, a whistle, a single synth line: it finds the pitch, looks for the nearest note of the scale, and moves the sound there. It can be used to correct a singer gently, or as an effect that forces a sound onto a grid, with every sliding note snapping from one scale note to the next.

The plug-in is stereo. The left and the right channel each follow their own pitch, so two different sounds can sit in the two channels. It is built for CLAP on Linux.

The pitch tracking and the resynthesis are the zita-at1 autotuner by Fons Adriaensen. ScalaTuner adds the Scala scales, the stereo setup and the window.

## Getting the scales

ScalaTuner reads the scale files while it runs. It looks for a folder in this order:

- the environment variable SCALATUNER_SCALES, if it is set;
- the path written in the file ~/.config/scalatuner/scales_dir;
- the folders ~/scl, ~/scales and ~/.scala/scl.

Put your .scl files in the first one that exists, or write the path of your own folder into scales_dir. The Scala scale archive, about 5000 files, works well. The list is read when the plug-in is loaded, so after adding files, load the plug-in again.

If no folder is found, the list holds only 12-TET, the ordinary equal-tempered scale.

## The window

The window has the scale list on the left. On the right, from the top: the grid strip, the two pitch meters, the sliders and the Mode buttons. At the top, next to the name, a line gives the name of the current scale, how many notes it has and the size of its period in cents.

The window is made for hosts that embed plug-in windows on Linux (X11). In REAPER, for example, it opens from the plug-in in the FX chain. If a host shows only a plain list of sliders instead, use that host's button for the plug-in's own interface.

## Choosing a scale

Click a name in the list to choose that scale. Turn the mouse wheel over the list, or drag the thin bar at its right edge, to move through it. The first entry is always 12-TET.

To find a scale, click the search box and type. The list shrinks to the names that contain what you typed; the number at the right of the box tells you how many are left. Backspace removes a letter, Escape clears the box, and Return chooses the scale if only one is left. Some hosts keep the keyboard for themselves; if typing does nothing, click the box again.

The scale is saved with the project by its name, not by its place in the list. So adding files to the folder later does not change which scale a project uses. If the file is missing when the project is opened, ScalaTuner uses 12-TET.

## The grid strip

The strip shows one period of the scale, starting at the Scale Root. Each thin blue line is a note of the scale. The first line, on the far left, is the first note (1/1); the white line on the far right is the same note, one period higher. A scale with 24 notes or fewer has the number of each note under its line.

The sound you play in is shown on the strip while ScalaTuner hears a pitch: a blue marker for the left channel, an orange marker for the right. The marker shows where the pitch sits inside the period, whatever the octave. A marker standing on a line is in tune; between two lines is where it will be pulled from.

## The meters

Each channel has a meter. It shows the name of the note that the pitch is being pulled to, for example C#4, and how far off the sound is in cents. The bar runs from -100 cents on the left to +100 cents on the right. A bar to the right means the sound is above the note and is being pulled down.

When ScalaTuner hears no pitch, the note name shows a dash and the bar is empty. The sound then passes through without correction.

## The controls

Click or drag a slider to set it. Turn the mouse wheel over a slider for fine steps. Double-click a slider to set it back to its default.

| Control | Range | Default | What it does |
|---|---|---|---|
| Scale Root | C-1 to G9 | C4 | The note that carries the first note (1/1) of the scale. The Hz value next to it is how high that note sounds. See below. |
| A Reference | 400 to 480 Hz | 440 Hz | The pitch of A4. Every note follows from it. |
| Bias | 0 to 1 | 0.5 | How much the current note is preferred. At 0 the sound jumps to the nearest note as soon as it is nearer. Higher, and it may drift further off before it jumps. |
| Smoothing | 0.02 to 0.5 | 0.1 | How slowly the correction moves while the note stays the same. Low is fast and hard; high is slow and keeps more of the natural movement. A new note always jumps at once. |
| Correction | 0 to 100 % | 100 % | How much of the error is corrected. At 100 % the pitch lands on the note. At 50 % it goes half way. |
| Offset | -2 to +2 semitones | 0 | Moves the result up or down. With Correction at 0 %, it is a plain pitch shift. |
| Mode | Auto, MIDI, Manual | Auto | Where the target notes come from. See below. |

## Scale Root and A Reference

These are two different things. A Reference says how high A is: 440 Hz in most cases. Scale Root says on which note the scale starts.

Every Scala scale begins with 1/1, the starting note, and gives all other notes as steps above it. The Scale Root decides which note that is. With Scale Root on C4 and A Reference at 440 Hz, 1/1 sounds at 261.6 Hz, and the other notes are placed above it by the ratios or cents in the file. The Hz value next to the Scale Root slider always shows this.

For a scale that starts on A, set Scale Root to A4: 1/1 is then 440 Hz. Moving the Scale Root moves the whole grid, and the scale keeps its shape. Moving A Reference changes the pitch of everything a little, for example to follow an orchestra that tunes to 432 Hz or 442 Hz.

The Scale Root is a MIDI note number, so it also sets how MIDI notes find their place in the scale; see below.

## Mode and MIDI notes

ScalaTuner can take notes to say which notes of the scale are allowed. Notes arrive through the plug-in's note input. In REAPER, for example, send MIDI to the track.

MIDI notes are placed on the scale one note per step, starting at the Scale Root. The Scale Root note is 1/1. The note above it is the second note of the scale, and so on. On a scale of 19 notes, 19 keys in a row cover one period, and the 20th key is 1/1 again, one period higher. Notes below the Scale Root count downward in the same way.

- **Auto:** while notes are held, only those notes are allowed. With no note held, all notes of the scale are allowed. This is the usual setting: it works as a plain autotuner, and playing notes steers it.
- **MIDI:** only held notes are allowed. With no note held, the sound is not corrected.
- **Manual:** all notes of the scale are always allowed. MIDI notes are ignored.

## Stereo

Each channel has its own pitch tracker, so the two channels do not have to hold the same note. Two voices, one on each side, are tuned each to its own nearest note.

The settings are shared: both channels use the same scale, the same root and the same sliders. The result is that a mono sound that is sent to both channels is tuned twice in the same way. If the left and right channels carry a stereo recording of one single sound, they may land on different notes and the stereo image can shift. If that happens, tune the sound before it is made stereo.

## Delay

ScalaTuner needs a little time to hear the pitch. The sound comes out 1024 samples late at 44.1 and 48 kHz, which is about 21 to 23 milliseconds, and 2048 samples at 88.2 and 96 kHz. The plug-in tells the host this delay, and a host with plug-in delay compensation lines everything up again. For playing live through the plug-in, the delay is audible.

## Good to know

- ScalaTuner follows one pitch in each channel. Chords and several voices in one channel are not tuned note by note; the strongest pitch is followed.
- It hears pitches between 75 and 1200 Hz. A very quiet sound, below about -50 dB, is taken for silence, and noise without a clear pitch is left alone.
- On a scale with few notes, such as 5 or 7 per octave, the jumps between notes are large. The meter bar can stay at its end while a note is far from the next one. That is not a fault.
- The sound is changed by playing it faster or slower between cuts, as in a tape machine, with short cross-fades. Very large corrections, and unvoiced sounds like consonants, are the places where this can be heard.
- The scale list is read when the plug-in is loaded, and the scale is read from its file when it is chosen. A very large file with more than 1000 notes is not accepted; the plug-in uses 12-TET and shows a warning in red next to the scale name.
- All sliders and the Mode can be automated by the host. The Scale can be automated as well; its value is its place in the list.

## Credits

The pitch tracking and resynthesis come from zita-at1 0.8.2 by Fons Adriaensen, who also wrote the resampler used here. ScalaTuner is distributed under the GNU General Public License, version 3. The source is at github.com/tjingboem/ScalaTuner.
