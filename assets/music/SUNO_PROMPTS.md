# Suno Music Prompts — Sky Fire Patrol

Generate tracks in [Suno](https://suno.com), download as MP3, and drop into
`assets/sounds/` using the filename listed for each track. The game already
references these filenames (with graceful fallbacks), so replacing the
placeholder file is all that's needed.

The campaign-1 reference style is:
> electro-industrial, dark synthwave, arcade techno. high energy 160bpm retro shoot-em up

---

## Campaign 2 — Marshall Islands  →  `bgm_marshall.mp3`

Turquoise tropical lagoon, white sandy atolls, an enemy supply line of boats
and carriers. Keep the arcade-techno backbone of campaign 1 but make it feel
hotter, more humid and exotic — sunlight on water with menace underneath.

**Style prompt (paste into Suno's "Style of Music" box):**
```
electro-industrial dark synthwave, arcade techno, tropical exotic lead synths,
steel-drum and marimba stabs over driving 16th-note bassline, humid reverb,
sunlit-but-menacing, naval war machine pulse, 165 BPM, instrumental,
retro shoot-em-up, high energy, no vocals
```

**Optional lyrics box (use as a structure guide; keep it instrumental):**
```
[Intro] sparse atoll shimmer, distant radar pings
[Build] 16th-note bass enters, industrial hats
[Drop] full arcade-techno groove, tropical lead over pounding kick
[Break] steel-drum melody, tension riser
[Drop 2] heavier, add detuned saw layer
[Outro] filtered fade, sonar echo
```

## Campaign 2 Boss — Ayako II  →  `bgm_boss_marshall.mp3`
The Marshall boss stage (25) is wired to play `bgm_boss_marshall.mp3` (currently a
placeholder copy of `bgm_boss.mp3`). Generate a dedicated faster cue and replace it:
```
relentless arcade-techno boss battle, dark industrial synthwave, double-time
175 BPM, alarm-siren leads, distorted bass, tropical-storm tension, instrumental,
retro bullet-hell finale, no vocals
```

---

### Notes
- Target length 1:30–2:30; the track loops in-game.
- Export at a consistent loudness with the other `bgm_*.mp3` files.
- DONE: `bgm_marshall.mp3` is now the real track "Lagoon Siege (Marshall Sector)"
  (source kept in `assets/music/`). The boss cue `bgm_boss_marshall.mp3` is still a
  placeholder pending its own track.
