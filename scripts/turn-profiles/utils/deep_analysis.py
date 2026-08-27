"""
Deep analysis of the turn profile generation logic.
Traces every step for 180-degree and 135-degree turn cases.
"""

import sys
sys.path.insert(0, '.')

from micromouse_sinusoidal_turn_profiles_aux import (
    Maze, RobotPhysics, Simulator, Line, pi, lines_intersection, turn_shift,
    turn_profile, complete_profile, complete_slalom_profile
)
from numpy import array, tan
from numpy.linalg import norm
import pandas as pd


# ─── Setup ───────────────────────────────────────────────────────────────────
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

# ═══════════════════════════════════════════════════════════════════════════════
# PART 1: Deep trace of 180-degree turn
# ═══════════════════════════════════════════════════════════════════════════════
print("=" * 70)
print("PART 1: 180-DEGREE TURN DEEP TRACE")
print("=" * 70)

# Entry/Exit definitions
entry_tuple = (0, -.5, 0)
exit_tuple = (0, .5, pi)  # CORREGIDO: exit al norte, no al este
linear_velocity = 1.350  # normal speed
radius = 0.09
manual_shift = (-0.045, 0)

entry = Line(x=entry_tuple[0]*cell, y=entry_tuple[1]*cell, angle=entry_tuple[2])
exit = Line(x=exit_tuple[0]*cell, y=exit_tuple[1]*cell, angle=exit_tuple[2])

print(f"\nEntry: ref=({entry.reference[0]:.6f}, {entry.reference[1]:.6f}), "
      f"angle={entry.angle:.6f}, slope={entry.slope:.6f}, intercept={entry.intercept:.6f}")
print(f"Exit:  ref=({exit.reference[0]:.6f}, {exit.reference[1]:.6f}), "
      f"angle={exit.angle:.6f}, slope={exit.slope:.6f}, intercept={exit.intercept:.6f}")

# Check if lines are parallel
if abs(entry.slope - exit.slope) < 1e-10:
    print(">>> ENTRY AND EXIT LINES ARE PARALLEL! turn_shift would fail.")
    print(">>> Manual shift is REQUIRED for 180-degree turns.")

# Compute physical parameters (as done in Simulator.slalom)
angle = exit.angle - entry.angle
print(f"\nTurn angle: {angle:.6f} rad ({angle*180/pi:.1f} degrees)")

force = (linear_velocity**2 * robot.mass) / (2 * radius)
max_angular_velocity = linear_velocity / radius
max_angular_velocity = min(max_angular_velocity, robot.max_angular_velocity)
max_angular_acceleration = force * robot.wheels_separation / robot.moment_of_inertia

print(f"Force: {force:.6f} N")
print(f"Max angular velocity (computed): {linear_velocity/radius:.4f} rad/s")
print(f"Max angular velocity (capped):    {max_angular_velocity:.4f} rad/s")
print(f"Max angular acceleration: {max_angular_acceleration:.4f} rad/s^2")

# Check transition feasibility
max_angular_velocity_transition = (abs(angle) / 2 * max_angular_acceleration)**0.5
print(f"Angular velocity from transition only: {max_angular_velocity_transition:.4f} rad/s")
print(f"Is transition-only sufficient? {max_angular_velocity_transition >= max_angular_velocity}")

# Generate profile step by step
profile = turn_profile(angle, max_angular_velocity, max_angular_acceleration, 0.00001)
profile = complete_profile(profile, entry.angle, radius, linear_velocity, robot, 0.00001)

print(f"\nUnshifted profile:")
print(f"  Start: ({profile['x'].iloc[0]:.6f}, {profile['y'].iloc[0]:.6f}), angle={profile['angle'].iloc[0]:.6f}")
print(f"  End:   ({profile['x'].iloc[-1]:.6f}, {profile['y'].iloc[-1]:.6f}), angle={profile['angle'].iloc[-1]:.6f}")
print(f"  Total duration: {profile.index[-1]:.6f}s")
print(f"  Net x displacement: {profile['x'].iloc[-1] - profile['x'].iloc[0]:.6f}m")
print(f"  Net y displacement: {profile['y'].iloc[-1] - profile['y'].iloc[0]:.6f}m")

# Compute what the ideal y-displacement should be
ideal_y_displacement = exit.reference[1] - entry.reference[1]
print(f"\n  Required y-displacement (exit_ref.y - entry_ref.y): {ideal_y_displacement:.6f}m")

# Manual shift
shift_array = array(manual_shift) + array(entry.reference)
print(f"\nApplied shift: {manual_shift} + entry.reference = ({shift_array[0]:.6f}, {shift_array[1]:.6f})")

# After applying shift to start point
shifted_start_y = profile['y'].iloc[0] + shift_array[1]
shifted_end_y = profile['y'].iloc[-1] + shift_array[1]
print(f"Shifted start y: {shifted_start_y:.6f} (should be {entry.reference[1]:.6f})")
print(f"Shifted end y:   {shifted_end_y:.6f} (should be {exit.intercept:.6f} [line y={exit.intercept:.6f}])")

# For the end to be on exit line y=0, we need:
# shifted_end_y = y_end_before_shift + shift_y = 0
# shift_y = -y_end_before_shift
correct_shift_y = -profile['y'].iloc[-1]
print(f"\nCorrect shift_y for exit alignment: {correct_shift_y:.6f}")
print(f"But then start_y would be: {profile['y'].iloc[0] + correct_shift_y:.6f} (should be {entry.reference[1]:.6f})")

# The entry and exit can't both be aligned with a pure y-shift
# because the natural y-displacement != required y-displacement
print(f"\n>>> FUNDAMENTAL ISSUE:")
print(f"    Profile natural y-displacement: {profile['y'].iloc[-1] - profile['y'].iloc[0]:.6f}m")
print(f"    Required y-displacement:        {ideal_y_displacement:.6f}m")
print(f"    Ratio: {(profile['y'].iloc[-1] - profile['y'].iloc[0]) / ideal_y_displacement:.4f}")
print(f"    The profile is ~2x too tall for the given entry/exit geometry!")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 2: Deep trace of TO_135 vs FROM_45_180
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("PART 2: TO_135 vs FROM_45_180 RELATIONSHIP")
print("=" * 70)

lv = 1.118  # normal speed
rad = 0.07456

# ── TO_135 ──
to135 = simulate.slalom(
    entry=(0, -.5, 0), exit=(0, .5, 3/4*pi),
    radius=rad, linear_velocity=lv)

# ── FROM_45_180 ──
from45_180 = simulate.slalom(
    entry=(0, -.5, pi/4), exit=(0, .5, pi),
    radius=rad, linear_velocity=lv)

# Get the unshifted profiles (before complete_slalom_profile)
angle1 = to135.exit.angle - to135.entry.angle  # 3pi/4
angle2 = from45_180.exit.angle - from45_180.entry.angle  # 3pi/4

print(f"\nTO_135 turn angle: {angle1:.6f} rad ({angle1*180/pi:.1f} deg)")
print(f"FROM_45_180 turn angle: {angle2:.6f} rad ({angle2*180/pi:.1f} deg)")
print(f"Both turns have the same angle! (135 degrees)")

# Get the shifts applied
shift1_raw = turn_shift(to135.entry, to135.exit,
    complete_profile(turn_profile(angle1, lv/rad,
        (lv**2 * robot.mass)/(2*rad) * robot.wheels_separation / robot.moment_of_inertia, 0.00001),
        to135.entry.angle, rad, lv, robot, 0.00001))

shift2_raw = turn_shift(from45_180.entry, from45_180.exit,
    complete_profile(turn_profile(angle2, lv/rad,
        (lv**2 * robot.mass)/(2*rad) * robot.wheels_separation / robot.moment_of_inertia, 0.00001),
        from45_180.entry.angle, rad, lv, robot, 0.00001))

print(f"\nTO_135 shift (from turn_shift): ({shift1_raw[0]:.6f}, {shift1_raw[1]:.6f})")
print(f"FROM_45_180 shift (from turn_shift): ({shift2_raw[0]:.6f}, {shift2_raw[1]:.6f})")

# Profile endpoints
xy1 = to135.profile[['x', 'y']]
xy2 = from45_180.profile[['x', 'y']]

p1_end = xy1.iloc[-1].values
p2_start = xy2.iloc[0].values

print(f"\nTO_135 profile ends at:   ({p1_end[0]:.6f}, {p1_end[1]:.6f})")
print(f"FROM_45_180 profile starts at: ({p2_start[0]:.6f}, {p2_start[1]:.6f})")
print(f"Distance between endpoints: {norm(p1_end - p2_start)*1000:.4f} mm")

# Are they on the same line?
print(f"\nTO_135 exit line:  y = {tan(3*pi/4):.2f}x + {to135.exit.intercept:.6f}")
print(f"FROM_45_180 entry line: y = {tan(pi/4):.2f}x + {from45_180.entry.intercept:.6f}")

# Check if p1_end is on TO_135 exit line
exit_line_y = tan(3*pi/4) * p1_end[0] + to135.exit.intercept
print(f"TO_135 exit line check at p_end: line y={exit_line_y:.6f}, actual y={p1_end[1]:.6f}, "
      f"error={abs(p1_end[1]-exit_line_y)*1000:.4f} mm")

# Check if p2_start is on FROM_45_180 entry line
entry2_line_y = tan(pi/4) * p2_start[0] + from45_180.entry.intercept
print(f"FROM_45_180 entry line check at p_start: line y={entry2_line_y:.6f}, actual y={p2_start[1]:.6f}, "
      f"error={abs(p2_start[1]-entry2_line_y)*1000:.4f} mm")

# The relationship between TO_135 end and FROM_45_180 start:
print(f"\n>>> RELATIONSHIP ANALYSIS:")
print(f"    TO_135 end point:   ({p1_end[0]*1000:.2f}, {p1_end[1]*1000:.2f}) mm")
print(f"    FROM_45_180 start:  ({p2_start[0]*1000:.2f}, {p2_start[1]*1000:.2f}) mm")
print(f"    Physical distance = {norm(p1_end - p2_start)*1000:.2f} mm")
print(f"    These are at DIFFERENT posts (mirror images across y=0)")
print(f"    CORRECTED: TO_135 uses exit=(0, .5, 3??/4), giving .end=+47.97mm")
print(f"    Now TO_135.end == FROM_45_180.start == +47.97mm ???")

# Now compare with TO_45/FROM_45 which works correctly
print(f"\n--- Reference: TO_45 / FROM_45 (WORKING pair) ---")
to45 = simulate.slalom(entry=(0, -.5, 0), exit=(.5, 0, pi/4), radius=0.12, linear_velocity=1.644)
from45 = simulate.slalom(entry=(0, -.5, pi/4), exit=(.5, 0, pi/2), radius=0.12, linear_velocity=1.644)

to45_end = to45.profile[['x', 'y']].iloc[-1].values
from45_start = from45.profile[['x', 'y']].iloc[0].values

print(f"TO_45 exit line:  y = {tan(pi/4):.2f}x + {to45.exit.intercept:.6f}")
print(f"FROM_45 entry line: y = {tan(pi/4):.2f}x + {from45.entry.intercept:.6f}")
print(f"TO_45 end:   ({to45_end[0]:.6f}, {to45_end[1]:.6f})")
print(f"FROM_45 start: ({from45_start[0]:.6f}, {from45_start[1]:.6f})")
print(f"Distance: {norm(to45_end - from45_start)*1000:.4f} mm")
print(f">>> TO_45 exit and FROM_45 entry are the SAME LINE (y = x - 0.09)")
print(f">>> This is why the pair works: the profiles naturally connect!")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 3: Analysis of describe() sign convention
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n" + "=" * 70)
print("PART 3: SIGN CONVENTION ANALYSIS")
print("=" * 70)

def analyze_sign_convention(name, slalom_profile):
    """Analyze how .start and .end signs work."""
    xy = slalom_profile.profile[['x', 'y']]
    entry = slalom_profile.entry
    exit = slalom_profile.exit

    p0 = xy.iloc[0].values
    p1 = xy.iloc[1].values
    p_last = xy.iloc[-1].values
    p_prev = xy.iloc[-2].values

    d0 = norm(p0 - entry.reference)
    d1 = norm(p1 - entry.reference)
    before = d0 * (1 if (d1 - d0) > 0 else -1)

    d_last = norm(p_last - exit.reference)
    d_prev = norm(p_prev - exit.reference)
    after = d_last * (1 if (d_prev - d_last) > 0 else -1)  # note: sign logic in code

    print(f"\n  [{name}]")
    print(f"    Entry angle: {entry.angle:.4f} rad")
    print(f"    Exit angle:  {exit.angle:.4f} rad")
    print(f"    d0={d0*1000:.4f}mm, d1={d1*1000:.4f}mm -> {'approaching' if d1<d0 else 'leaving'} ref")
    print(f"    d_prev={d_prev*1000:.4f}mm, d_last={d_last*1000:.4f}mm -> {'approaching' if d_last<d_prev else 'leaving'} ref")
    print(f"    .start (before) = {before*1000:.4f} mm")
    print(f"    .end (after)    = {after*1000:.4f} mm")

analyze_sign_convention("normal180",
    simulate.slalom(entry=(0,-.5,0), exit=(0,.5,pi), radius=0.09, shift=(-0.045,0), linear_velocity=1.350))
analyze_sign_convention("normal90",
    simulate.slalom(entry=(0,-.5,0), exit=(.5,0,pi/2), radius=0.135, linear_velocity=1.696))
analyze_sign_convention("normalTO135",
    simulate.slalom(entry=(0,-.5,0), exit=(0,.5,3*pi/4), radius=0.07456, linear_velocity=1.118))
analyze_sign_convention("normalFROM45_180",
    simulate.slalom(entry=(0,-.5,pi/4), exit=(0,.5,pi), radius=0.07456, linear_velocity=1.118))

print("\n\n>>> SIGN CONVENTION:")
print("    - Negative .start: profile starts BEFORE entry reference (approaching)")
print("    - Positive .start: profile starts AFTER entry reference (leaving)")
print("    - Negative .end:   profile ends BEFORE exit reference (approaching/hasn't passed)")
print("    - Positive .end:   profile ends AFTER exit reference (leaving/has passed)")
