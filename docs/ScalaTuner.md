# ScalaTuner

## What it does

ScalaTuner is an autotuner that pulls the pitch of a sound onto a scale that you choose. The scale can be any Scala (.scl) file: any number of notes, and any period, so scales that repeat at the octave and scales that do not, such as Bohlen-Pierce, both work.

It follows one pitch at a time in each channel. Give it a voice, a bowed string, a whistle, a single synth line: it finds the pitch, looks for the nearest note of the scale, and moves the sound there. It can be used to correct a singer gently, or as an effect that forces a sound onto a grid, with every sliding note snapping from one scale note to the next.

The plug-in is stereo. The left and the right channel each follow their own pitch, so two different sounds can sit in the two channels. It is built for CLAP on Linux.

The pitch tracking and the resynthesis are the zita-at1 autotuner by Fons Adriaensen. ScalaTuner adds the Scala scales, the stereo setup and the window.

## Getting the scales

ScalaTuner reads the scale files while it runs. It can use several folders at once, and shows the scales of all of them in one list. The folders come from three places:

- the environment variable SCALATUNER_SCALES, if it is set; several folders are separated by a colon;
- the folders on the Settings page; they are kept in the file ~/.config/scalatuner/scales_dir, one folder per line.

The first time ScalaTuner runs, it puts the first of ~/scl, ~/scales and ~/.scala/scl that exists in that list, so you start with your scale archive in place. It is an ordinary entry: you can remove it like any other folder.

The Scala scale archive, more than 5200 files, works well in ~/scl. You can download it from www.huygens-fokker.org/docs/scales.zip (the Download page of Scala is www.huygens-fokker.org/scala/downloads.html). To search the archive on the web, use the ScalaVista Online Scale Archive of H-Pi Instruments, scalavista.zentralzone.com/scalearchive.php. It finds scales by name, description, number of tones and more, and each entry links to its .scl file. Note the name of a scale there, then look for that name in the ScalaTuner list. The file www.huygens-fokker.org/docs/scalesdir.txt is a plain list of every scale with a short description, which you can also search in a browser or text editor. To use your own scales as well, put them in a folder of your own and add that folder as described under Settings. Scales in all folders are sorted together by name.

If no folder is found, the list holds only 12-TET, the ordinary equal-tempered scale.

## The window

The window has the scale list on the left. On the right, two tabs at the top choose the page. The Main page shows, from the top, the grid strip, the two pitch meters, the sliders and the Mode buttons. The Settings page holds the pitch detection settings; see Settings, below. At the top, next to the name, a line gives the name of the current scale, how many notes it has and the size of its period in cents.

The window is made for hosts that embed plug-in windows on Linux (X11). In REAPER, for example, it opens from the plug-in in the FX chain. If a host shows only a plain list of sliders instead, use that host's button for the plug-in's own interface.

## Choosing a scale

Click a name in the list to choose that scale. Turn the mouse wheel over the list, or drag the thin bar at its right edge, to move through it. The arrow keys step through the scales: Up chooses the scale above the current one, Down the scale below it. Page Up and Page Down move a page at a time, Home and End go to the first and the last scale. When a search has narrowed the list, the keys move through the scales that are left. Click in the window first, so that the keys reach ScalaTuner and not the host; some hosts keep the keys for themselves anyway. The first entry is always 12-TET.

To find a scale, click the search box and type. The list shrinks to the names that contain what you typed; the number at the right of the box tells you how many are left. Backspace removes a letter, Escape clears the box, and Return chooses the scale if only one is left. Some hosts keep the keyboard for themselves; if typing does nothing, click the box again.

The scale is saved with the project by its name, not by its place in the list. So adding files to the folder later does not change which scale a project uses. If the file is missing when the project is opened, ScalaTuner uses 12-TET.

## The grid strip

The strip shows one period of the scale, starting at the Scale Root. Each thin blue line is a note of the scale. The first line, on the far left, is the first note (1/1); the white line on the far right is the same note, one period higher. A scale with 24 notes or fewer has the number of each note under its line.

The sound you play in is shown on the strip while ScalaTuner hears a pitch: a blue marker for the left channel, an orange marker for the right. The marker shows where the pitch sits inside the period, whatever the octave. A marker standing on a line is in tune; between two lines is where it will be pulled from.

## The meters

Each channel has a meter. It shows the name of the note that the pitch is being pulled to, for example C#4, and how far off the sound is in cents. The bar runs from -100 cents on the left to +100 cents on the right. A bar to the right means the sound is above the note and is being pulled down.

When ScalaTuner hears no pitch, the note name shows a dash and the bar is empty. The sound then passes through without correction.

## The controls

Click or drag a slider to set it. Hold Shift while you drag for fine adjustment: the value then moves ten times slower than the pointer, from where it was when you pressed Shift. Turn the mouse wheel over a slider for small steps; with Shift held, the steps are smaller still. Double-click a slider to set it back to its default.

| Control | Range | Default | What it does |
|---|---|---|---|
| Scale Root | C-1 to G9 | C4 | The note that carries the first note (1/1) of the scale. The Hz value next to it is how high that note sounds. See below. |
| A Reference | 400 to 480 Hz | 440 Hz | The pitch of A4. Every note follows from it. |
| Bias | 0 to 1 | 0.5 | How much the current note is preferred. At 0 the sound jumps to the nearest note as soon as it is nearer. Higher, and it may drift further off before it jumps. |
| Smoothing | 0.02 to 0.5 | 0.1 | How slowly the correction moves while the note stays the same. Low is fast and hard; high is slow and keeps more of the natural movement. A new note always jumps at once. |
| Correction | 0 to 100 % | 100 % | How much of the error is corrected. At 100 % the pitch lands on the note. At 50 % it goes half way. |
| Offset | -2 to +2 semitones | 0 | Moves the result up or down. With Correction at 0 %, it is a plain pitch shift. |
| Mode | Auto, MIDI, Manual | Auto | Where the target notes come from. See below. |

## Settings

The Settings tab holds the settings for the pitch detection. They are for fitting ScalaTuner to the sound, and most of the time they can stay as they are.

| Control | Range | Default | What it does |
|---|---|---|---|
| Lowest Pitch | 50 to 500 Hz | 75 Hz | Pitches below this are not followed. |
| Highest Pitch | 200 to 4000 Hz | 1200 Hz | Pitches above this are not followed. |
| Threshold | -90 to -20 dB | -50 dB | Sound below this level is taken for silence. 0 dB is a full-scale sine wave. |
| Lookahead | Off, 43 ms, 85 ms | Off | Extra delay, so that the pitch is known before a note is heard. See Delay and Lookahead, below. |

Set the range around the sound that you play in. A bass line can have the range 40 to 300 Hz, and a high flute 250 to 3000 Hz. A narrower range makes it harder for the detector to mistake a harmonic for the pitch, which is the cause of a note jumping an octave. It also keeps ScalaTuner from reacting to rumble or hiss.

The detector cannot hear below about 47 Hz, whatever the setting, and Highest Pitch should stay well under a quarter of the sample rate. Pitches outside the range are not corrected: the sound passes through as it is, and the meters show a dash.

Raise the Threshold if background noise or the tail of a note makes ScalaTuner jump about. Lower it for a very quiet recording, so that soft notes are followed too.

The Settings page also holds the scale folders. Under SCALE FOLDERS it lists every folder in use, and how many scale files they hold together.

- To add a folder, click the field at the bottom, type the path (a path starting with ~/ is fine) or paste it with Ctrl+V, and press Return. If the folder exists, its scales appear in the list at once, and the scale you were using stays selected. If it does not, the field shows a short message in red. Some hosts keep the keyboard for themselves; if typing does nothing, click the field again.
- To remove a folder, click the x at the right of its line. The files are not touched. A folder that you removed does not come back by itself.
- Folders from the environment variable have no x, because they are not part of the file in ~/.config. They go away when you unset the variable.

The folders are the same for every ScalaTuner in the project and in other projects. The scale a project uses is saved by name; if the file is no longer there when the project is opened, ScalaTuner uses 12-TET.

All four settings can be automated by the host, like the sliders on the Main page.

## What happens to the sound

The pitch detector and the sound you hear are separate. The detector works on a copy of the input in which the spectrum rolls off above about 8 kHz, so that hiss and high harmonics do not confuse it. None of this reaches the output.

The output is your input, played back faster or slower to move the pitch, with short cross-fades where it jumps from one stretch of the input to another. There is no filter or equaliser in that path. At 44.1 and 48 kHz the input is first resampled to double the rate inside the plug-in; that filter cuts only close to the top of the band. At 88.2 kHz and higher the input is used directly.

## Scale Root and A Reference

These are two different things. A Reference says how high A is: 440 Hz in most cases. Scale Root says on which note the scale starts.

Every Scala scale begins with 1/1, the starting note, and gives all other notes as steps above it. The Scale Root decides which note that is. With Scale Root on C4 and A Reference at 440 Hz, 1/1 sounds at 261.6 Hz, and the other notes are placed above it by the ratios or cents in the file. The Hz value next to the Scale Root slider always shows this.

For a scale that starts on A, set Scale Root to A4: 1/1 is then 440 Hz. Moving the Scale Root moves the whole grid, and the scale keeps its shape. Moving A Reference changes the pitch of everything a little, for example to follow an orchestra that tunes to 432 Hz or 442 Hz.

The Scale Root is a MIDI note number, so it also sets how MIDI notes find their place in the scale; see below.

## Changing scales over time

The scale can be changed while the music plays, by automation. This works in any host that automates plug-in parameters; as an example, REAPER:

- Open the plug-in's FX window, click Param, choose Scale, and choose Show track envelope. A lane with the envelope opens under the track.
- Draw points on the envelope. Each height is one scale. Use square points (the shape of a step) so that the scale changes at the point and does not slide through the scales in between.
- The box that shows the value of a point gives the name of the scale, for example pyth_12, in hosts that show the text of a parameter. At the bottom of the range is 12-TET.

A few things to know:

- The value of the Scale parameter is the place of the scale in the list, counted from 12-TET at 0. The list is sorted by name and holds the scales of all folders together. A REAPER envelope keeps its values precisely enough to tell thousands of numbers apart, so every scale in even a long list can be set exactly: edit the value of the envelope point, or drag the point until it shows the scale you want. The same number always gives the same scale, as long as the folders do not change.
- That makes the numbers usable as material in themselves: step through neighbouring scales, jump to a scale far away, or draw an envelope that visits scales in an order that you would not choose by ear. A folder of your own with only the scales for the piece gives small numbers that are easy to remember.
- When you add or remove a folder, the places of the scales change, and an envelope that was drawn before then points at other scales. Settle the folders before you draw the envelope.
- The scale changes within a fraction of a second after the point, not on the exact sample. The new scale is read from its file when it is called for.
- The pitch that is being followed jumps to the nearest note of the new scale. A note that was held can move when the scale changes under it.
- The Scale Root and the other sliders can be automated in the same way. A change of Scale Root with a steady scale moves the whole grid.
- The scale you chose in the window is the starting value; automation takes over while the host plays the envelope.

## Mode and MIDI notes

ScalaTuner can take notes to say which notes of the scale are allowed. Notes arrive through the plug-in's note input. In REAPER, for example, send MIDI to the track.

MIDI notes are placed on the scale one note per step, starting at the Scale Root. The Scale Root note is 1/1. The note above it is the second note of the scale, and so on. On a scale of 19 notes, 19 keys in a row cover one period, and the 20th key is 1/1 again, one period higher. Notes below the Scale Root count downward in the same way.

A held key allows its note of the scale in every octave, not at one exact pitch. This is the pitch class of the key: its place within one period of the scale. In 12-TET, holding C# allows every C#, and a sound that is heard as C#2, C#3 or C#5 is pulled to the nearest C#. In a scale of 19 notes, the key that is step 5 allows step 5 in every period. Two keys that are one period apart allow the same note. The sound always moves the shortest way to the nearest allowed note, so the octave of the key that you play does not matter, and neither does the octave in which the detector hears the sound. This keeps the correction steady if the detector flips between a note and its octave, which happens with sounds that are rich in partials.

You play, or draw in the MIDI editor, only the notes that you want to allow, in any octave. If the keys of the editor are not those of your scale, think in steps: the step of a key is its distance from the Scale Root, counted within one period.

- **Auto:** while notes are held, only those notes are allowed. With no note held, all notes of the scale are allowed. This is the usual setting: it works as a plain autotuner, and playing notes steers it.
- **MIDI:** only held notes are allowed. With no note held, the sound is not corrected.
- **Manual:** all notes of the scale are always allowed. MIDI notes are ignored.

## Stereo

Each channel has its own pitch tracker, so the two channels do not have to hold the same note. Two voices, one on each side, are tuned each to its own nearest note.

The settings are shared: both channels use the same scale, the same root and the same sliders. The result is that a mono sound that is sent to both channels is tuned twice in the same way. If the left and right channels carry a stereo recording of one single sound, they may land on different notes and the stereo image can shift. If that happens, tune the sound before it is made stereo.

## Delay and Lookahead

ScalaTuner needs a little time to hear the pitch. With Lookahead off, the sound comes out 1024 samples late at 44.1 and 48 kHz, which is about 21 to 23 milliseconds, and 2048 samples at 88.2 and 96 kHz. The plug-in tells the host this delay, and a host with plug-in delay compensation lines everything up again. For playing live through the plug-in, the delay is audible.

Hearing the pitch also takes time: the detector looks at about 43 milliseconds of sound, and on a clean, steady tone it knows the pitch after about 20 milliseconds. On a sound with a noisy or inharmonic start, it takes longer. During that time the start of the note is played at its original pitch, and then the pitch moves to the grid. That is the short glide you hear at the start of a note.

Lookahead removes it. It adds delay on purpose, so that the plug-in knows the pitch before it plays the start of the note. Off is the normal delay, 43 ms doubles it, 85 ms quadruples it. The setting shows the delay in milliseconds at the sample rate of the project. The host is told about the new delay and compensates for it, so tracks stay lined up; the only price is that the sound is later, which is fine for sounds that are not played live.

- Choose a longer Lookahead when the start of a note glides, or when the pitch needs time to settle at the start of a sample.
- A change of Lookahead changes the delay of the plug-in. Some hosts need a moment to recalculate the compensation, or restart playback.
- A very long, noisy start, such as the attack of a bell or a struck resonator, may need the longest setting, and may still not be heard as one pitch.

## Good to know

- ScalaTuner follows one pitch in each channel. Chords and several voices in one channel are not tuned note by note; the strongest pitch is followed.
- It hears pitches between 75 and 1200 Hz, and a sound below -50 dB is taken for silence. Both can be changed on the Settings page. Noise without a clear pitch is left alone.
- The scale is available over the whole range from about 27 Hz to 7 kHz, whatever the number of notes. How far a note can be pulled is limited to half a period of the scale, and at least one octave; a pitch further from every allowed note than that is left alone.
- On a scale with few notes, such as 5 or 7 per octave, the jumps between notes are large. The meter bar can stay at its end while a note is far from the next one. That is not a fault.
- The sound is changed by playing it faster or slower between cuts, as in a tape machine, with short cross-fades. Very large corrections, and unvoiced sounds like consonants, are the places where this can be heard.
- The scale list is read when the plug-in is loaded and whenever you add or remove a folder, and a scale is read from its file when it is chosen. A very large file with more than 1000 notes is not accepted; the plug-in uses 12-TET and shows a warning in red next to the scale name.
- All sliders, the Mode and the Settings can be automated by the host. The Scale can be automated as well; its value is its place in the list.

## Credits

The pitch tracking and resynthesis come from zita-at1 0.8.2 by Fons Adriaensen, who also wrote the resampler used here. ScalaTuner is distributed under the GNU General Public License, version 3. The source is at github.com/tjingboem/ScalaTuner.
