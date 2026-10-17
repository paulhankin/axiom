# Axiom Pinball

A pinball table in 3D for macOS, in Axiom: the ball's physics, a table with the
usual things on it, its rules and display, its sounds, and Metal rendering with
physically based shading, HDR, bloom and shadows.

```
axiom build examples/pinball --target=aarch64-macos -O2 -o examples/pinball/pinball
examples/pinball/pinball
```

| Key | |
|---|---|
| Enter (or Space) | start a game |
| Left / Right arrows (or Z / M) | flippers (they also move the lit top lanes) |
| Space or Down (hold, let go) | the plunger: the longer you pull, the harder it shoots |
| N or Up; V and B | nudge the table (forward; left and right). Too much tilts it |
| Escape | quit |

A game controller works too: the shoulders or triggers flip, A plunges, Start
starts, B nudges.

`--autopilot` plays by itself. `--png out.png --frames N` plays N frames by itself
offscreen and saves the last; with `--multiball` it stops a few seconds into the
first multiball. `--trace` prints where the balls go.

## The table

From the bottom: two flippers over the drain; inlanes and outlanes; two
slingshots; three standup targets with a saucer (the lock) beside them; four drop
targets on the right; three pop bumpers under three rollover lanes (A X M); a
clear plastic ramp up the left that turns at the top onto a six-wire habitrail,
which carries the ball back down the left side to the left inlane; an orbit round
the top, with a spinner in its right lane; the shooter lane, with a gate.

## The rules

- Bumpers 1,000 (5,000 with SUPER BUMPERS lit), slingshots 110, spinner 100 a turn.
- The lanes: all three lit raise the bonus multiplier (to 5x). The lane flashing
  as you plunge is the skill shot, 50,000.
- The drop targets: the whole bank lights the lock. Two balls locked in the saucer
  start a three-ball multiball, in which the ramp scores the jackpot (250,000, and
  100,000 more each time).
- The ramp 20,000, the orbit 15,000; one straight after the other is a combo, doubled.
- The standups: all three light SUPER BUMPERS for twenty seconds.
- Ball save for the first ten seconds of each ball. Bonus at the end of each ball,
  times the multiplier. Three balls a game.

## How it works

| File | |
|---|---|
| `physics.ax` | The ball as a solid steel sphere with a spin, stepped 2400 times a second: impulses at contacts with restitution and Coulomb friction (sliding turns into rolling), rolling resistance; fences (walls, posts, rubbers, targets, one-way gates) that can kick and report hits; wires and triangles for the habitrail and the ramp; flippers as tapered bats driven by a solenoid against a spring, with their own inertia; the plunger; balls hitting balls; a grid to find what is near |
| `table.ax` | The layout in metres (54 x 107 cm, tilted 6.5 degrees), the ramp's and rail's curves (Catmull-Rom, spaced evenly), the switches and the lamps |
| `game.ax` | Rules, scoring, lamps (incandescent fade, flashing, the attract mode's waves), the display's screens, ball save, ball search, tilt, the autopilot |
| `render.ax`, `shaders.metal` | A shadow map; the scene into a half-float target with 4x MSAA; GGX shading with a lit room for reflections; inserts lit in the playfield's shader; a chrome ball that reflects the playfield under it; the clear ramp; bloom at two sizes; ACES tone mapping; the dot-matrix display |
| `scene.ax`, `mesh.ax` | The meshes, built from the table's description |
| `art.ax` | The playfield's artwork, painted with Quartz at startup |
| `sound.ax` | Every sound synthesized at startup: solenoids, chimes, the rolling ball, the rail, tunes, a music loop |
| `dmd.ax` | The 128 x 32 dot display and its font |

`src/shaders.ax` is generated from `src/shaders.metal` by `tools/embed.py`; edit the
`.metal` file, then run the script.
