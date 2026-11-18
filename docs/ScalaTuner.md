# ScalaTuner

## What it does

ScalaTuner brings a frequency structure to sound. It follows the pitch of a sound and pulls it onto a scale that you choose. The scale can be any Scala (.scl) file: any number of notes, and any period, so scales that repeat at the octave and scales that do not, such as Bohlen-Pierce, both work.

ScalaTuner is not only for voices. It started from an autotuner, and a voice, a bowed string or a single synth line can be corrected with it. But its purpose is wider: to give all kinds of sounds, tones and noises, a frequency structure. A gliding, noisy sound, a ringing resonance or a field recording can be given steps along a scale that it did not have before, and a sound with an unsteady pitch can be made to move from one note of the scale to the next.

How much structure a sound gets depends on how clear its pitch is. For a sound with a clear pitch the detector can be left at Clear, and the result is a precise tuning. For a sound with a weaker or unsteady pitch, move the Detector slider towards Loose; the sound then follows the grid for far more of its length, in steps. A scale with few notes per octave gives large steps and a strong effect; a scale with many notes gives small steps. See Clear and Loose, below.

It follows one pitch at a time in each channel: it finds the pitch, looks for the nearest scale note, and moves the sound there. With a sliding sound, every slide is cut into steps, from one scale note to the next.

The plug-in is stereo. The left and the right channel each follow their own pitch, so two different sounds can sit in the two channels. It is built for CLAP on Linux.

The pitch tracking and the resynthesis are the zita-at1 autotuner by Fons Adriaensen. ScalaTuner adds the Scala scales, the stereo setup and the window.

## Words used in this manual

Two kinds of notes meet in ScalaTuner, and they are not the same thing. This manual keeps them apart.

- **Scale note.** One of the notes of the scale that you chose: a thin blue line in the grid strip. A scale note is counted in steps from the first scale note, 1/1, which is step 0.
- **Target note.** The scale note that the sound is being pulled to at this moment. Its name is shown in the meter.
- **Allowed notes.** The scale notes that may become the target. In Auto and Manual mode every scale note is allowed, unless MIDI notes narrow the choice. See Mode and MIDI notes.
- **MIDI note.** A note that you send to the plug-in, a key of the keyboard or a note drawn in the MIDI editor. A MIDI note is not a scale note. It stands for one, and tells ScalaTuner that this scale note is allowed. A MIDI note is only used for that.
- **Pitch.** The frequency that ScalaTuner hears in the sound itself. When the manual speaks of a note of the sound, as in "the start of a note", it means a note in the music that you put in.

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

The strip shows one period of the scale, starting at the Scale Root. Each thin blue line is a scale note. The first line, on the far left, is the first scale note (1/1); the white line on the far right is the same scale note, one period higher. A scale with 24 notes or fewer has the step number of each scale note under its line.

Hovering the mouse over a line shows its size in cents from the root, at the top right of the strip.

The sound you play in is shown on the strip while ScalaTuner hears a pitch: a blue marker for the left channel, an orange marker for the right. The marker shows where the pitch sits inside the period, whatever the octave. A marker standing on a line is in tune; between two lines is where it will be pulled from.

## The meters

Each channel has a meter. It shows the name of the target note, the scale note that the sound is being pulled to, for example C#4. The name is that of the MIDI note that plays this scale note (see Mode and MIDI notes). On a scale that is not 12-TET it is only a label for the step, and the grid strip shows where the scale note really is. The meter also shows how far off the sound is, in cents. The bar runs from -100 cents on the left to +100 cents on the right. A bar to the right means the sound is above the target note and is being pulled down.

When ScalaTuner hears no pitch, the name of the target note shows a dash and the bar is empty. The sound then passes through without correction.

## The controls

Click or drag a slider to set it. Hold Shift while you drag for fine adjustment: the value then moves ten times slower than the pointer, from where it was when you pressed Shift. Turn the mouse wheel over a slider for small steps; with Shift held, the steps are smaller still. Double-click a slider to set it back to its default.

| Control | Range | Default | What it does |
|---|---|---|---|
| Scale Root | C-1 to G9 | C4 | The MIDI note that plays the first scale note (1/1). The Hz value next to it is how high that scale note sounds. See below. |
| A Reference | 400 to 480 Hz | 440 Hz | The pitch of A4. The pitch of every scale note follows from it. |
| Bias | 0 to 1 | 0.5 | How much the current target note is preferred. At 0 the sound jumps to the nearest scale note as soon as that note is nearer. Higher, and it may drift further off before it jumps. |
| Smoothing | 0.02 to 0.5 | 0.1 | How slowly the correction moves while the sound stays on the same target note. Low is fast and hard; high is slow and keeps more of the natural movement. A new target note always jumps at once. |
| Correction | 0 to 100 % | 100 % | How much of the error is corrected. At 100 % the pitch lands on the scale note. At 50 % it goes half way. |
| Offset | -2 to +2 semitones | 0 | Moves the result up or down. With Correction at 0 %, it is a plain pitch shift. |
| Mode | Auto, MIDI, Manual | Auto | Which scale notes are allowed as targets: all of them, or only those that held MIDI notes stand for. See Mode and MIDI notes, below. |

## Settings

The Settings tab holds the settings for the pitch detection. They are for fitting ScalaTuner to the sound, and most of the time they can stay as they are.

| Control | Range | Default | What it does |
|---|---|---|---|
| Lowest Pitch | 50 to 500 Hz | 75 Hz | Pitches below this are not followed. |
| Highest Pitch | 200 to 4000 Hz | 1200 Hz | Pitches above this are not followed. |
| Threshold | -90 to -20 dB | -50 dB | Sound below this level is taken for silence. 0 dB is a full-scale sine wave. |
| Detector | Clear to Loose | Clear | How clear a pitch the sound needs before it is followed. See Clear and Loose, below. |
| Lookahead | Off, 43, 85, 170, 340 ms | Off | Extra delay, so that the pitch is known before a note is heard. See Delay and Lookahead, below. |
| Lock | Off, On | Off | Keep the correction steady while the sound stays on the same target note (a scale note). See Lock, below. |
| Hold | Off, 0.5 s, 2 s, Forever | Off | How long the correction stays when the pitch is lost. See Hold, below. |

Set the range around the sound that you play in. A bass line can have the range 40 to 300 Hz, and a high flute 250 to 3000 Hz. A narrower range makes it harder for the detector to mistake a harmonic for the pitch, which is the cause of a note jumping an octave. It also keeps ScalaTuner from reacting to rumble or hiss.

The detector cannot hear below about 47 Hz, whatever the setting, and Highest Pitch should stay well under a quarter of the sample rate. Pitches outside the range are not corrected: the sound passes through as it is, and the meters show a dash.

Raise the Threshold if background noise or the tail of a note makes ScalaTuner jump about. Lower it for a very quiet recording, so that soft notes are followed too.

The Settings page also holds the scale folders. Under SCALE FOLDERS it lists every folder in use, and how many scale files they hold together.

- To add a folder, click the field at the bottom, type the path (a path starting with ~/ is fine) or paste it with Ctrl+V, and press Return. If the folder exists, its scales appear in the list at once, and the scale you were using stays selected. If it does not, the field shows a short message in red. Some hosts keep the keyboard for themselves; if typing does nothing, click the field again.
- To remove a folder, click the x at the right of its line. The files are not touched. A folder that you removed does not come back by itself.
- Folders from the environment variable have no x, because they are not part of the file in ~/.config. They go away when you unset the variable.

The folders are the same for every ScalaTuner in the project and in other projects. The scale a project uses is saved by name; if the file is no longer there when the project is opened, ScalaTuner uses 12-TET.

All seven settings can be automated by the host, like the sliders on the Main page.

## Clear and Loose

The Detector slider decides how clear a pitch a sound needs before ScalaTuner follows it. It runs from Clear, on the left, to Loose, on the right, with every step between. Between the ends the box shows how far towards Loose the slider is, in per cent. It is the same detector at every position; the slider only changes how strict it is.

- **Clear** is for sounds with a definite pitch, such as a voice, a bowed note or a single synthesiser line. The sound has to show a clear, steady repetition before it is followed. Anything less is left alone and passes through unchanged. This is how the original zita-at1 detector works.
- **Loose** accepts much weaker evidence. It follows noisy, gliding and unstable pitches too, so far more of the sound is pulled onto the grid. On a noisy sound whose pitch glides, the glide turns into a series of steps along the scale, and the sound gets a tuning structure it did not have. The price is more flicker between notes: with Loose the steps come more often, and the pitch it finds may be a partial more than a fundamental.

What the slider changes, from left to right:

- **The strength of the repetition.** The detector measures how strongly a sound repeats itself, on a scale from 0 to 1. At Clear it needs at least 0.6, which is a clear, steady pitch. Moving to the right lowers this to 0, so that any repetition counts.
- **The power in the spectrum.** At Clear, a peak is rejected if its frequency has almost no power in the spectrum, because it is probably a sub-harmonic. Towards Loose this rule fades out.
- **The weaker peak.** At Clear, a peak that is more than 20 dB below a stronger one is rejected, because it is probably not the fundamental. Towards Loose this rule fades out too.

At the far right, ScalaTuner follows every repetition that it can find. The Threshold and the pitch range on the Settings page still apply at every position: a sound that is too quiet, or outside the range, is still left alone. If part of a sound is not followed even at Loose, lower the Threshold or widen the pitch range.

Move the slider towards Loose until the sound follows the grid as much as you want. Because the slider can be automated, a sound can change from clear to loose and back over time.

An example of what it does. A noisy sound whose pitch glides, 12 seconds long, was run through a scale of five notes. The table shows how much of the sound was followed, and how many times per second the target note changed.

| Slider | Sound followed | Note changes per second |
|---|---|---|
| Clear | 46 % | 13.6 |
| 25 % | 63 % | 21.6 |
| 50 % | 71 % | 26.6 |
| 75 % | 76 % | 30.3 |
| Loose | 79 % | 32.7 |

The rest of the sound, about a fifth, was below the Threshold of -50 dB or outside the pitch range, and so was never followed. A clean tone with a clear pitch gave the same result at every position of the slider, so the slider does not disturb sounds that are already clear.

The sparser the scale, the stronger the effect is to hear. With a scale of five notes, the average correction in this example was about half a semitone. With a scale of twelve notes it was about a fifth of a semitone, and with a scale of 43 notes, a twelfth of a semitone, which is hardly heard.

Loose is meant for sound material of any kind, not for correcting a singer. Pitch class, Hold and Lookahead work in the same way at every position of the slider.

## Lock

With a sound that holds several pitches at once, the detector hops from one voice to another, because it follows the strongest pitch at each moment. Each hop gives a different amount of correction, and the Smoothing filter turns the changes into slides. All the voices of the sound are moved by the same correction, so they seem to glide between each other. This is an internal glide, and it comes from imposing a grid on a sound that has more than one pitch.

Lock switches that off. While the sound stays on the same target note, the correction is kept at the value that was found when that scale note was chosen, and it no longer follows every small change of the detected pitch. The whole sound is then moved by a steady amount for as long as it stays on that scale note, and it only jumps when the target note changes, that is, when the sound is pulled to another scale note. A change of the target note is heard as a step, not as a slide.

- Use Lock on dense or polyphonic sounds, and on sounds with an unsteady pitch, when the glides between the steps are not wanted.
- With Lock on, a sound whose own pitch moves slowly keeps its own movement on top of the steady correction. The correction no longer corrects it back onto the target note between two steps.
- Lock does not change which scale note is chosen as the target, only how the correction behaves while the target note stays the same. Smoothing has no effect while Lock is on, except at the start of a new target note.
- In a test with a noisy gliding sound, the movement of the correction within one target note became about 40 % smaller with Clear, and about 55 % smaller with Loose.

## Hold

When ScalaTuner can no longer hear a pitch, because the sound has fallen below the Threshold or has turned into noise, it normally lets go after about 50 milliseconds. The correction drops to zero and the sound snaps back to its original pitch. For a singer between two phrases that is right. For a ringing sound that slowly dies away it is not: the end of the sound jumps back off the grid.

Hold keeps the last correction for the time that you choose, 0.5 seconds, 2 seconds, or for ever. The tail of the sound then stays on the grid until it is gone. With Hold off, nothing changes from before.

- While Hold is in force, the sound is shifted by the last correction, even if it no longer has a pitch that the detector can follow. That is what makes a tail follow the grid.
- When a new pitch is heard, it is corrected at once, as always.
- If a new sound starts while the plug-in is still holding, the first 20 to 40 milliseconds of the new sound are shifted by the old correction, until the new pitch is found. With Lookahead, that part is corrected in time, as described above.
- For voices and other sounds with pauses, keep Hold off.

## What happens to the sound

The pitch detector and the sound you hear are separate. The detector works on a copy of the input in which the spectrum rolls off above about 8 kHz, so that hiss and high harmonics do not confuse it. None of this reaches the output.

The output is your input, played back faster or slower to move the pitch, with short cross-fades where it jumps from one stretch of the input to another. There is no filter or equaliser in that path. At 44.1 and 48 kHz the input is first resampled to double the rate inside the plug-in; that filter cuts only close to the top of the band. At 88.2 kHz and higher the input is used directly.

## Scale Root and A Reference

These are two different things. A Reference says how high A is: 440 Hz in most cases. Scale Root says which MIDI note plays the first scale note of the scale.

Every Scala scale begins with 1/1, the first scale note, and gives all other scale notes as steps above it. The Scale Root decides which pitch that is: the 12-TET pitch of the MIDI note that you choose. With Scale Root on C4 and A Reference at 440 Hz, 1/1 sounds at 261.6 Hz, and the other scale notes are placed above it by the ratios or cents in the file. The Hz value next to the Scale Root slider always shows this.

For a scale that starts on A, set Scale Root to A4: 1/1 is then 440 Hz. Moving the Scale Root moves the whole grid, and the scale keeps its shape. Moving A Reference changes the pitch of everything a little, for example to follow an orchestra that tunes to 432 Hz or 442 Hz.

The Scale Root is a MIDI note number, so it also decides which MIDI note is step 0 of the scale, and how the other MIDI notes find their place in it; see Mode and MIDI notes, below.

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
- The sound jumps to the nearest scale note of the new scale. A sound that was held on a scale note can move when the scale changes under it.
- The Scale Root and the other sliders can be automated in the same way. A change of Scale Root with a steady scale moves the whole grid.
- The scale you chose in the window is the starting value; automation takes over while the host plays the envelope.

## Mode and MIDI notes

ScalaTuner can take MIDI notes to say which scale notes are allowed as targets. MIDI notes arrive through the plug-in's note input. In REAPER, for example, send MIDI to the track. A MIDI note is a key of the keyboard. It is not a scale note, but it stands for one, as described next.

**How a MIDI note stands for a scale note.** MIDI notes are placed on the scale one MIDI note per step, starting at the Scale Root. The MIDI note that is the Scale Root stands for the first scale note, 1/1, which is step 0. The MIDI note above it stands for the second scale note, step 1, and so on. On a scale of 19 notes, 19 MIDI notes in a row cover one period, and the 20th MIDI note stands for 1/1 again, one period higher. MIDI notes below the Scale Root count downward in the same way.

**Every octave.** A held MIDI note allows its scale note in every octave, not at one exact pitch. This is the pitch class of the MIDI note: its place within one period of the scale. In 12-TET, holding a C# allows every C#, and a sound that is heard as C#2, C#3 or C#5 is pulled to the nearest C#. In a scale of 19 notes, the MIDI note that is step 5 allows scale step 5 in every period. Two MIDI notes that are one period apart allow the same scale note. The sound always moves the shortest way to the nearest allowed scale note, so the octave of the MIDI note that you play does not matter, and neither does the octave in which the detector hears the sound. This keeps the correction steady if the detector flips between a pitch and its octave, which happens with sounds that are rich in partials.

You play, or draw in the MIDI editor, only the MIDI notes for the scale notes that you want to allow, in any octave. If the keys of the editor are not those of your scale, think in steps: the step of a MIDI note is its distance from the Scale Root, counted within one period.

- **Auto:** while MIDI notes are held, only the scale notes that they stand for are allowed. With no MIDI note held, all scale notes are allowed. This is the usual setting: it works as a plain autotuner, and playing MIDI notes steers it.
- **MIDI:** only the scale notes of held MIDI notes are allowed. With no MIDI note held, the sound is not corrected.
- **Manual:** all scale notes are always allowed. MIDI notes are ignored.

## Stereo

Each channel has its own pitch tracker, so the two channels do not have to be on the same scale note. Two voices, one on each side, are each pulled to their own nearest scale note.

The settings are shared: both channels use the same scale, the same root and the same sliders. The result is that a mono sound that is sent to both channels is tuned twice in the same way. If the left and right channels carry a stereo recording of one single sound, they may land on different scale notes and the stereo image can shift. If that happens, tune the sound before it is made stereo.

## Delay and Lookahead

ScalaTuner needs a little time to hear the pitch. With Lookahead off, the sound comes out 1024 samples late at 44.1 and 48 kHz, which is about 21 to 23 milliseconds, and 2048 samples at 88.2 and 96 kHz. The plug-in tells the host this delay, and a host with plug-in delay compensation lines everything up again. For playing live through the plug-in, the delay is audible.

Hearing the pitch also takes time: the detector looks at about 43 milliseconds of sound, and on a clean, steady tone it knows the pitch after about 20 milliseconds. On a sound with a noisy or inharmonic start, it takes longer. During that time the start of the note is played at its original pitch, and then the pitch moves to the grid. That is the short glide you hear at the start of a note.

Lookahead removes it. It adds delay on purpose, so that the plug-in knows the pitch before it plays the start of the note. Off is the normal delay. The other steps are 2, 4, 8 and 16 times that: 43, 85, 170 and 340 ms. The setting shows the delay in milliseconds at the sample rate of the project. The host is told about the new delay and compensates for it, so tracks stay lined up; the only price is that the sound is later, which is fine for sounds that are not played live.

- Choose a longer Lookahead when the start of a note glides, or when the pitch needs time to settle at the start of a sample.
- A change of Lookahead changes the delay of the plug-in. Some hosts need a moment to recalculate the compensation, or restart playback.
- A sound with a long, noisy or inharmonic start, such as the attack of a bell or a struck resonator, may need the pitch for a quarter of a second or more before the detector is sure of it. Choose 170 or 340 ms for such a sound. Such a start may still never be heard as one pitch.
- A change of Lookahead while the sound plays moves the playback position at once, which may be heard as a click. Set it before you play.

## Five examples

These five examples are very different from each other. Each one gives the sound, the scale and the settings, and says what happens. The numbers come from tests with sounds made for the purpose, and with sounds of the kind described. The settings that are not mentioned stay at their defaults. Try the same settings on your own material, and change them from there.

### 1. Tuning a line that drifts

**The sound.** A single line that is out of tune and wanders: a voice, a violin, a synthesiser line with vibrato.

**The scale.** 12-TET, which is the first entry of the list. Any other scale of twelve notes gives another tuning of the same line.

**The settings.** Mode on Auto, Detector on Clear, Smoothing at its fastest (0.02). The rest at the defaults.

**What happens.** Each note of the line is pulled to the nearest scale note and kept there, and the drift and most of the vibrato disappear. In a test with a line that was up to 45 cents out of tune and moved by a vibrato of 25 cents, the average distance from the grid went from 29 cents to 8 cents. With Smoothing at its default of 0.1, part of the vibrato stays and the average distance is 16 cents. This is the classic use, and the one in which Clear is the right setting.

### 2. Playing a sound like an instrument

**The sound.** A struck resonance with a long decay, about 8 seconds, built on the harmonic series of a C.

**The scale.** Slendro, five notes to the octave, at 231, 435, 702 and 933 cents above the root.

**The settings.** Mode on MIDI, Scale Root on C4, Lookahead 340 ms, Hold on Forever, Threshold at -72 dB. Send the plug-in the MIDI notes C4, D4, C#4, E4, D#4 and C4 again, one after the other, each held for about a second and a half.

**What happens.** The whole sound moves to the scale note that the MIDI note stands for, in the intervals of the scale and not those of the keyboard. The MIDI notes are steps 0, 2, 1, 4, 3 and 0 of the scale. In a test the sound moved by 0, +431, +226, -273 and -502 cents, where the scale gives 0, +435, +231, -267 and -498 cents. Pitch class means that the octave of the MIDI note does not matter. Lookahead lets the start of each sound already be on the new scale note, and Hold lets the decay stay on that scale note until the sound has gone.

### 3. A structure for a noisy sound that glides

**The sound.** A noisy sound that slides about without a clear pitch, such as ring-modulated material.

**The scale.** arist_enh3, seven notes to the octave, in two tight clusters (0, 42, 83 cents and 700, 742, 783 cents) with large gaps between them.

**The settings.** Mode on Auto, Detector all the way to Loose, Lock on.

**What happens.** The slide is cut into plateaus, and the sound jumps over the large gaps of the scale. In a test with 14 seconds of such a sound, Clear followed 46 % of the sound, and Loose followed 79 %. The note changed about 32 times a second, and not 13. With Lock on, the movement of the correction within one note was more than half smaller than with Clear and Lock off. The sound gets a direction that it did not have, and the large steps of this scale are clearly heard.

### 4. A grid that is not an octave

**The sound.** A rising sweep, from 100 to 800 Hz in twelve seconds.

**The scale.** Bohlen-Pierce, in the version bohlen-p_et: 13 equal steps of 146 cents that together cover the 3:1 ratio, a tritave, and not an octave.

**The settings.** Mode on Auto, Detector on Clear, Smoothing at its fastest.

**What happens.** The smooth sweep turns into a staircase on the steps of the Bohlen-Pierce scale. In the test the sweep went through 25 scale notes, and the time it spent within 10 cents of a note rose from 14 % to 62 %. The average distance from the nearest note fell from 36 cents to 14 cents. The steps are 146 cents wide and do not repeat at the octave: this is a grid that the keyboard and the usual autotuners do not have.

### 5. A grid that changes over time

**The sound.** The same sweep as in the fourth example.

**The scale.** Automated: Slendro for the first four seconds, Partch's 43 notes for the next four, arist_enh3 for the last four. In REAPER, for example, this is an envelope on the Scale parameter; see Changing scales over time.

**The settings.** The same as in the fourth example.

**What happens.** The same sound is strongly structured in one part and hardly touched in another. With Slendro, the average distance from the scale notes went from 59 cents to 11 cents. With Partch's 43 notes it went from 7 cents to 6 cents: the notes are so close that every pitch is already near one, so there is almost nothing to pull. With arist_enh3 the distance went from 90 cents to 56 cents. A scale with few notes pulls hard, a scale with many notes hardly at all, so changing the scale over time moves the sound between steps and slides.

## Good to know

- ScalaTuner follows one pitch in each channel. Chords and several voices in one channel are not tuned note by note; the strongest pitch is followed. For sounds that are very dense, or have no pitch that the detector can find at all, the effect is limited; this is where Loose helps most.
- It hears pitches between 75 and 1200 Hz, and a sound below -50 dB is taken for silence. Both can be changed on the Settings page. Noise without a clear pitch is left alone.
- The scale is available over the whole range from about 27 Hz to 7 kHz, whatever the number of notes. How far a sound can be pulled is limited to half a period of the scale, and at least one octave; a pitch further from every allowed scale note than that is left alone.
- On a scale with few notes, such as 5 or 7 per octave, the jumps between scale notes are large. The meter bar can stay at its end while the sound is far from the next scale note. That is not a fault.
- The sound is changed by playing it faster or slower between cuts, as in a tape machine, with short cross-fades. Very large corrections, and unvoiced sounds like consonants, are the places where this can be heard.
- The scale list is read when the plug-in is loaded and whenever you add or remove a folder, and a scale is read from its file when it is chosen. A very large file with more than 1000 notes is not accepted; the plug-in uses 12-TET and shows a warning in red next to the scale name.
- All sliders, the Mode and the Settings can be automated by the host. The Scale can be automated as well; its value is its place in the list.

## Credits

The pitch tracking and resynthesis come from zita-at1 0.8.2 by Fons Adriaensen, who also wrote the resampler used here. ScalaTuner is distributed under the GNU General Public License, version 3. The source is at github.com/tjingboem/ScalaTuner.
