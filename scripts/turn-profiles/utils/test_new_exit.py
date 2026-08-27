"""
Test the proposed fix: change MOVE_180 exit from (.5, 0, pi) to (0, .5, pi).

Old exit: (.5, 0, pi)   -> exit line y=0 (horizontal through post)
New exit: (0, .5, pi)   -> exit line y=0.09 (horizontal, north of post, heading west)

The expected improvement:
- Vertical distance between entry (y=-0.09) and exit (y=0.09) is now 0.18m
- Profile natural y-displacement is ~0.1825m (with radius 0.09m)
- Ratio ~1.01 instead of ~2.03 -> much better symmetry
"""

import sys
sys.path.insert(0, '.')

from micromouse_sinusoidal_turn_profiles_aux import (
    Maze, RobotPhysics, Simulator, Line, pi, lines_intersection, turn_shift,
    turn_profile, complete_profile, complete_slalom_profile
)
from numpy import array, tan
from numpy.linalg import norm
import numpy as np


classic = Maze(cell=0.18, post=0.012)
robot = RobotPhysics(
    mass=0.070,
    moment_of_inertia=(0.070 * 0.05 ** 2) / 2,
    width=0.0702,
    wheels_separation=0.062,
    max_angular_velocity=40,
)
simulate = Simulator(robot=robot, maze=classic, time_period=0.00001)

cell = 0.18

speeds = {
    'normal': 1.350,
    'medium': 1.575,
    'fast':   1.800,
    'super':  2.025,
    'haki':   2.250,
}

# ═══════════════════════════════════════════════════════════════════════════════
# PART 1: Compare OLD vs NEW exit for 180-degree turns
# ═══════════════════════════════════════════════════════════════════════════════
print("=" * 70)
print("PART 1: OLD exit=(.5, 0, pi) vs NEW exit=(0, .5, pi)")
print("=" * 70)

for speed_name, lv in speeds.items():
    print(f"\n--- [{speed_name.upper()}] v={lv:.3f} m/s, radius=0.09, shift=(-0.045, 0) ---")

    # OLD exit
    old = simulate.slalom(
        entry=(0, -.5, 0), exit=(.5, 0, pi),
        radius=0.09, shift=(-0.045, 0), linear_velocity=lv)

    # NEW exit
    new = simulate.slalom(
        entry=(0, -.5, 0), exit=(0, .5, pi),
        radius=0.09, shift=(-0.045, 0), linear_velocity=lv)

    # Extract start/end from describe_profile logic
    def get_start_end(slalom):
        xy = slalom.profile[['x', 'y']]
        before = norm(xy.iloc[0].values - slalom.entry.reference)
        before *= np.sign(norm(xy.iloc[1].values - slalom.entry.reference) - before)
        after = norm(xy.iloc[-1].values - slalom.exit.reference)
        after *= np.sign(norm(xy.iloc[-2].values - slalom.exit.reference) - after)
        return float(before)*1000, float(after)*1000

    old_start, old_end = get_start_end(old)
    new_start, new_end = get_start_end(new)

    # Check exit line alignment
    old_xy = old.profile[['x', 'y']]
    new_xy = new.profile[['x', 'y']]

    old_exit_line_y = tan(old.exit.angle) * old_xy['x'].iloc[-1] + old.exit.intercept
    new_exit_line_y = tan(new.exit.angle) * new_xy['y'].iloc[-1]  # wrong, let me recalc
    # Exit line: y - ref.y = tan(angle) * (x - ref.x)
    # For angle=pi: tan(pi)=0, so y = ref.y
    old_exit_error = abs(old_xy['y'].iloc[-1] - old.exit.intercept) * 1000
    new_exit_error = abs(new_xy['y'].iloc[-1] - new.exit.intercept) * 1000

    # Entry line alignment
    old_entry_error = abs(old_xy['y'].iloc[0] - old.entry.intercept) * 1000
    new_entry_error = abs(new_xy['y'].iloc[0] - new.entry.intercept) * 1000

    print(f"  OLD: .start={old_start:.4f}mm  .end={old_end:.4f}mm  "
          f"|start|={abs(old_start):.4f}  |end|={abs(old_end):.4f}  "
          f"ratio={abs(old_end)/abs(old_start):.4f}  "
          f"exit_err={old_exit_error:.4f}mm  entry_err={old_entry_error:.4f}mm")
    print(f"  NEW: .start={new_start:.4f}mm  .end={new_end:.4f}mm  "
          f"|start|={abs(new_start):.4f}  |end|={abs(new_end):.4f}  "
          f"ratio={abs(new_end)/abs(new_start):.4f}  "
          f"exit_err={new_exit_error:.4f}mm  entry_err={new_entry_error:.4f}mm")

    old_ok = abs(abs(old_start) - abs(old_end)) < 0.1
    new_ok = abs(abs(new_start) - abs(new_end)) < 0.1
    print(f"  OLD symmetry: {'OK' if old_ok else 'FAIL'}  |  NEW symmetry: {'OK' if new_ok else 'FAIL'}")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 2: Find optimal shift for new exit
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("PART 2: Optimal shift search for new exit=(0, .5, pi)")
print("=" * 70)

# Use normal speed as reference
lv = 1.350
radius = 0.09

# Generate unshifted profile
entry = Line(x=0, y=-0.5*cell, angle=0)
exit_new = Line(x=0, y=0.5*cell, angle=pi)

angle = exit_new.angle - entry.angle
force = (lv**2 * robot.mass) / (2 * radius)
max_av = min(lv / radius, robot.max_angular_velocity)
max_aa = force * robot.wheels_separation / robot.moment_of_inertia

# Check
max_av_trans = (abs(angle) / 2 * max_aa)**0.5
print(f"\nMax angular velocity: {max_av:.4f} rad/s (transition-only would be {max_av_trans:.4f})")

profile_raw = turn_profile(angle, max_av, max_aa, 0.00001)
profile_raw = complete_profile(profile_raw, entry.angle, radius, lv, robot, 0.00001)

y_end_unshifted = profile_raw['y'].iloc[-1]
x_end_unshifted = profile_raw['x'].iloc[-1]
print(f"Unshifted profile: start=(0,0), end=({x_end_unshifted:.6f}, {y_end_unshifted:.6f})")
print(f"Net y-displacement: {y_end_unshifted:.6f}m")
print(f"Entry line: y = {entry.intercept:.6f}")
print(f"Exit line:  y = {exit_new.intercept:.6f}")
print(f"Required y-displacement: {exit_new.intercept - entry.intercept:.6f}m")

# Optimal shift to center profile between entry and exit lines
# We want: start on entry line, end on exit line
# start_y + shift_y = entry.intercept  => shift_y = entry.intercept (since start_y=0)
# end_y + shift_y = exit.intercept    => shift_y = exit.intercept - y_end_unshifted
# These give different values when y_end_unshifted != exit.intercept - entry.intercept

shift_y_from_start = entry.intercept
shift_y_from_end = exit_new.intercept - y_end_unshifted

print(f"\nShift y from start alignment: {shift_y_from_start:.6f}")
print(f"Shift y from end alignment:   {shift_y_from_end:.6f}")
print(f"Difference: {abs(shift_y_from_start - shift_y_from_end)*1000:.4f} mm")
print(f"(With old exit, this difference was ~92.5 mm!)")

# Best shift: average of the two (centers the error)
shift_y_optimal = (shift_y_from_start + shift_y_from_end) / 2
print(f"\nOptimal shift_y (average): {shift_y_optimal:.6f}")

# For x-shift, we want symmetry
# With entry angle 0 and exit angle pi, the unshifted profile should have
# start at x=0 heading east, end at some x_end heading west
# For a symmetric turn, the x-midpoint should be at x_mid
# Let's check the x-displacement
x_start_unshifted = profile_raw['x'].iloc[0]
print(f"Unshifted x: start={x_start_unshifted:.6f}, end={x_end_unshifted:.6f}")
print(f"Net x-displacement: {x_end_unshifted - x_start_unshifted:.6f}m")

# The turn's x-extent: from start to end
# For symmetry between entry ref (0, entry_intercept) and exit ref (0, exit_intercept):
# The profile should be centered at x=0
# x_midpoint = (x_start + shift_x + x_end + shift_x) / 2 = shift_x + (x_start + x_end)/2
# For centering: x_midpoint = 0 => shift_x = -(x_start + x_end)/2
shift_x_optimal = -(x_start_unshifted + x_end_unshifted) / 2
print(f"\nOptimal shift_x (centering): {shift_x_optimal:.6f}")
print(f"Old manual shift: (-0.045, 0) + entry.ref = ({-0.045 + entry.reference[0]:.6f}, {0 + entry.reference[1]:.6f})")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 3: Test with optimal shift vs current shift
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("PART 3: Testing different shift strategies for NEW exit")
print("=" * 70)

shift_strategies = {
    'current (-0.045, 0)': (-0.045, 0),
    'optimal (computed)': (shift_x_optimal, shift_y_optimal - entry.reference[1]),
    'optimal_y_only (-0.045, y_opt)': (-0.045, shift_y_optimal - entry.reference[1]),
    'no_shift (None)': None,
}

for strat_name, shift_val in shift_strategies.items():
    try:
        if shift_val is None:
            # Try automatic turn_shift
            try:
                p = simulate.slalom(
                    entry=(0, -.5, 0), exit=(0, .5, pi),
                    radius=radius, linear_velocity=lv)
                auto_used = True
            except ValueError as e:
                print(f"\n  [{strat_name}]: turn_shift failed (lines parallel) - {e}")
                continue
        else:
            p = simulate.slalom(
                entry=(0, -.5, 0), exit=(0, .5, pi),
                radius=radius, shift=shift_val, linear_velocity=lv)
            auto_used = False

        xy = p.profile[['x', 'y']]
        p_start = xy.iloc[0].values
        p_end = xy.iloc[-1].values

        # Compute start/end
        before = norm(p_start - p.entry.reference)
        before *= np.sign(norm(xy.iloc[1].values - p.entry.reference) - before)
        after = norm(p_end - p.exit.reference)
        after *= np.sign(norm(xy.iloc[-2].values - p.exit.reference) - after)

        # Line errors
        entry_err = abs(p_start[1] - p.entry.intercept) * 1000
        exit_err = abs(p_end[1] - p.exit.intercept) * 1000

        print(f"\n  [{strat_name}] {'(auto)' if auto_used else ''}")
        print(f"    .start = {before*1000:.4f} mm  .end = {after*1000:.4f} mm")
        print(f"    |start| = {abs(before)*1000:.4f}  |end| = {abs(after)*1000:.4f}  "
              f"ratio = {abs(after)/abs(before):.4f}")
        print(f"    Entry line error: {entry_err:.4f} mm")
        print(f"    Exit line error:  {exit_err:.4f} mm")
        print(f"    Profile start: [{p_start[0]:.6f}, {p_start[1]:.6f}]")
        print(f"    Profile end:   [{p_end[0]:.6f}, {p_end[1]:.6f}]")
        print(f"    Symmetry: {'OK' if abs(abs(before)-abs(after))*1000 < 0.2 else 'FAIL'}")

    except Exception as e:
        print(f"\n  [{strat_name}]: ERROR - {e}")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 4: Verify margin (distance to post) is still safe
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("PART 4: Safety check - margin to post")
print("=" * 70)

for speed_name, lv in speeds.items():
    # NEW exit with current shift
    new = simulate.slalom(
        entry=(0, -.5, 0), exit=(0, .5, pi),
        radius=0.09, shift=(-0.045, 0), linear_velocity=lv)

    min_margin = new.profile['margin'].min() * 1000
    print(f"  [{speed_name.upper()}] Min margin to post: {min_margin:.4f} mm")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 5: Summary
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("SUMMARY")
print("=" * 70)
print("""
  Old exit: (.5, 0, pi)   -> exit line y=0 (through post center)
  New exit: (0, .5, pi)   -> exit line y=0.09 (north cell center, heading west)

  Geometry fix:
  - Old: entry line y=-0.09, exit line y=0 -> vertical span = 0.09m
  - New: entry line y=-0.09, exit line y=0.09 -> vertical span = 0.18m
  - Profile natural y-displacement: ~0.1825m
  - With new exit, profile nearly fits the geometry naturally

  Shift considerations:
  - Lines remain parallel (both horizontal) -> manual shift still needed
  - Current shift (-0.045, 0) may need minor y-adjustment for perfect alignment
""")
