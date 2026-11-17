ScalaTuner: five examples (rendered offline with the plug-in; the "after" files are not
corrected for the plug-in's delay: 21 ms, and 341 ms in example 2)

Mono 16-bit, 48 kHz.  "before" = the untouched input, "after" = the output of ScalaTuner.

1  ex1_drifting_line      synthetic line, 6 notes, 30-45 c out of tune, vibrato 25 c.
                          12-TET, Auto, Detector Clear, Smoothing 0.02.
2  ex2_klank_melody       accKlankM06.flac. Slendro (slendro5_1), MIDI mode, Scale Root C4,
                          Lookahead 340 ms, Hold Forever, Threshold -72 dB.
                          MIDI keys 60, 62, 61, 64, 63, 60 at 0.3 / 1.65 / 3.0 / 4.35 / 5.7 / 7.05 s.
3  ex3_noisy_glide        ringModMidi3_stems.wav, first 14 s. arist_enh3, Auto,
                          Detector all the way to Loose, Lock on.
4  ex4_bohlen_pierce_sweep  synthetic sweep 100-800 Hz in 12 s. bohlen-p_et (13 steps per 3:1),
                          Auto, Detector Clear, Smoothing 0.02.
5  ex5_scale_changes      the same sweep; Scale automated: slendro5_1 (0 s), partch_43 (4 s),
                          arist_enh3 (8 s). Other settings as in 4.

The files ex2_klank_melody_* and ex3_noisy_glide_* are made from the author's own samples
(accKlankM06.flac and the first 14 s of ringModMidi3_stems.wav). The same two examples, made
from synthetic sounds, are in synthetic/.


synthetic/  (made for the examples, from no one's recording: free to share)
  ex2_resonance.wav                  struck resonance on a C3 harmonic series, 6.4 s, long decay
  ex2_resonance_notes.mid            the five MIDI notes of example 2 (C4 D4 C#4 E4 D#4), 120 bpm
  ex2_resonance_slendro_midi.wav     the result: slendro5_1, MIDI mode, Scale Root C4, Lookahead 340 ms,
                                     Hold Forever, Threshold -72 dB. The sound moves by 0, +435, +231,
                                     -267 and -498 cents (measured: 0, +434, +232, -266, -499).
  ex3_noisy_glide.wav                noisy, gliding, flutter-modulated sound, 8 s
  ex3_noisy_glide_arist_clear.wav    arist_enh3, Auto, Detector Clear: it follows about half of the sound
  ex3_noisy_glide_arist_loose_lock.wav   arist_enh3, Auto, Detector Loose, Lock on: it follows all of it
Delay: the outputs are not shifted back (21 ms, and 341 ms for ex2_resonance_slendro_midi).
