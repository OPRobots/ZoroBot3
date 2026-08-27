"""
Comprehensive analysis of all turn pair combinations.
Checks:
1. Which pairs share the same exit/entry line (direct chain possible)
2. Reference point distances and .start/.end consistency
3. Which pairs need intermediate straight movement
"""

import sys
sys.path.insert(0, '.')

from micromouse_sinusoidal_turn_profiles_aux import (
    Maze, RobotPhysics, Simulator, Line, pi, lines_intersection
)
from numpy import array, tan, cos, sin, sign
from numpy.linalg import norm
import numpy as np
from math import sqrt


# ─── Setup ───────────────────────────────────────────────────────────────────
classic = Maze(cell=0.18, post=0.012)
robot = RobotPhysics(
    mass=0.070, moment_of_inertia=(0.070 * 0.05 ** 2) / 2,
    width=0.0702, wheels_separation=0.062, max_angular_velocity=40,
)
sim = Simulator(robot=robot, maze=classic, time_period=0.00001)
cell = 0.18
CELL_HALF_DIAGONAL = cell * sqrt(2) / 2  # 127.28 mm
CELL_DIAGONAL = cell * sqrt(2)           # 254.56 mm

# ─── Define all turn types ────────────────────────────────────────────────────
# Using NORMAL speed as reference
turn_defs = {
    'MOVE_90': {
        'entry': (0, -.5, 0), 'exit': (.5, 0, pi/2),
        'radius': 0.135, 'shift': None, 'lv': 1.696,
    },
    'MOVE_180': {
        'entry': (0, -.5, 0), 'exit': (0, .5, pi),
        'radius': 0.09, 'shift': (-0.045, 0), 'lv': 1.350,
    },
    'MOVE_TO_45': {
        'entry': (0, -.5, 0), 'exit': (.5, 0, pi/4),
        'radius': 0.12, 'shift': None, 'lv': 1.644,
    },
    'MOVE_TO_135': {
        'entry': (0, -.5, 0), 'exit': (0, .5, 3*pi/4),
        'radius': 0.07456, 'shift': None, 'lv': 1.118,
    },
    'MOVE_45_TO_45': {
        'entry': (0, -.5, pi/4), 'exit': (0, .5, 3*pi/4),
        'radius': 0.06364, 'shift': None, 'lv': 0.955,
    },
    'MOVE_FROM_45': {
        'entry': (0, -.5, pi/4), 'exit': (.5, 0, pi/2),
        'radius': 0.12, 'shift': None, 'lv': 1.644,
    },
    'MOVE_FROM_45_180': {
        'entry': (0, -.5, pi/4), 'exit': (0, .5, pi),
        'radius': 0.07456, 'shift': None, 'lv': 1.118,
    },
}

# ─── Generate all profiles ────────────────────────────────────────────────────
profiles = {}
for name, params in turn_defs.items():
    p = sim.slalom(
        entry=params['entry'], exit=params['exit'],
        radius=params['radius'], shift=params['shift'],
        linear_velocity=params['lv'])
    profiles[name] = p

# ─── Extract line info for each turn's entry and exit ─────────────────────────
def line_key(line):
    """Return a canonical key for a line: (slope_rounded, intercept_rounded)."""
    slope = round(line.slope, 6)
    intercept = round(line.intercept, 6)
    return (slope, intercept)

def line_eq_str(line):
    """Human-readable line equation."""
    s = line.slope
    b = line.intercept
    # Detect vertical lines (near-infinite slope from tan(pi/2))
    if abs(s) > 1e10:
        x_int = line.reference[0]
        return f"x = {x_int*1000:.1f} mm"
    s = round(s, 4)
    b = round(b, 4)
    if s == 0:
        return f"y = {b*1000:.1f} mm"
    elif abs(s - 1) < 0.001:
        return f"y = x + {b*1000:.1f} mm"
    elif abs(s + 1) < 0.001:
        return f"y = -x + {b*1000:.1f} mm"
    else:
        return f"y = {s:.4f}x + {b*1000:.1f} mm"

def ref_str(ref):
    return f"({ref[0]*1000:.1f}, {ref[1]*1000:.1f})"

def angle_name(rad):
    """Human-readable angle name."""
    deg = np.degrees(rad) % 360
    names = {0: 'E', 45: 'NE', 90: 'N', 135: 'NW', 180: 'W', 225: 'SW', 270: 'S', 315: 'SE'}
    for k, v in names.items():
        if abs(deg - k) < 0.1:
            return v
    return f'{deg:.0f}deg'

print("=" * 90)
print("  COMPREHENSIVE TURN CHAINING ANALYSIS")
print("=" * 90)

# ═══════════════════════════════════════════════════════════════════════════════
# PART 1: All entry and exit lines
# ═══════════════════════════════════════════════════════════════════════════════
print("\n--- PART 1: Entry and Exit Lines for Each Turn ---\n")

entry_lines = {}  # line_key -> list of turn names
exit_lines = {}   # line_key -> list of turn names

for name, p in profiles.items():
    ek = line_key(p.entry)
    xk = line_key(p.exit)
    entry_lines.setdefault(ek, []).append(name)
    exit_lines.setdefault(xk, []).append(name)
    print(f"  {name:22s}  ENTRY: {line_eq_str(p.entry):20s}  ref={ref_str(p.entry.reference):16s}  ({angle_name(p.entry.angle)})")
    print(f"  {'':22s}  EXIT:  {line_eq_str(p.exit):20s}  ref={ref_str(p.exit.reference):16s}  ({angle_name(p.exit.angle)})")
    print()

print(f"\n  Unique entry lines: {len(entry_lines)}")
for k, turns in entry_lines.items():
    print(f"    {k}: {turns}")
print(f"\n  Unique exit lines: {len(exit_lines)}")
for k, turns in exit_lines.items():
    print(f"    {k}: {turns}")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 2: Direct chain pairs (exit line of A == entry line of B)
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n--- PART 2: Direct Chain Pairs (Exit[A] line == Entry[B] line) ---\n")

direct_chains = []
for name_a, p_a in profiles.items():
    for name_b, p_b in profiles.items():
        if name_a == name_b:
            continue
        if line_key(p_a.exit) == line_key(p_b.entry):
            direct_chains.append((name_a, name_b))

if direct_chains:
    for a, b in direct_chains:
        print(f"  {a:22s} -> {b:22s}  line: {line_eq_str(profiles[a].exit)}")
else:
    print("  (none)")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 3: For each direct chain, analyze reference point distance
#         and .start/.end consistency
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n--- PART 3: Direct Chain Analysis (offsets and gaps) ---\n")

def get_start_end(p):
    xy = p.profile[['x', 'y']]
    before = norm(xy.iloc[0].values - p.entry.reference)
    before *= np.sign(norm(xy.iloc[1].values - p.entry.reference) - before)
    after = norm(xy.iloc[-1].values - p.exit.reference)
    after *= np.sign(norm(xy.iloc[-2].values - p.exit.reference) - after)
    return float(before)*1000, float(after)*1000

for a, b in direct_chains:
    p_a = profiles[a]
    p_b = profiles[b]

    s_a, e_a = get_start_end(p_a)
    s_b, e_b = get_start_end(p_b)

    # Distance between exit ref of A and entry ref of B
    ref_dist = norm(p_a.exit.reference - p_b.entry.reference) * 1000

    # Physical position at end of A
    xy_a = p_a.profile[['x', 'y']]
    p_end_a = xy_a.iloc[-1].values

    # Physical position at start of B
    xy_b = p_b.profile[['x', 'y']]
    p_start_b = xy_b.iloc[0].values

    phys_gap = norm(p_end_a - p_start_b) * 1000

    # Direction along the shared line
    # For same-line chains, the exit direction of A and entry direction of B should
    # be the same (or opposite if going back)
    exit_angle = p_a.exit.angle
    entry_angle = p_b.entry.angle
    dir_diff = abs(exit_angle - entry_angle) % (2*pi)
    same_dir = dir_diff < 0.01 or abs(dir_diff - 2*pi) < 0.01
    opposite_dir = abs(dir_diff - pi) < 0.01

    # For same direction: the ref distance is the actual physical distance
    # between ref points along the line
    direction = "SAME" if same_dir else ("OPPOSITE" if opposite_dir else f"{np.degrees(dir_diff):.1f}deg")

    print(f"  {a:22s} -> {b}")
    print(f"    Shared line:     {line_eq_str(p_a.exit)}")
    print(f"    Exit[A] ref:     {ref_str(p_a.exit.reference)} (heading {angle_name(exit_angle)})")
    print(f"    Entry[B] ref:    {ref_str(p_b.entry.reference)} (heading {angle_name(entry_angle)})")
    print(f"    Heading diff:    {direction}")
    print(f"    Ref distance:    {ref_dist:.2f} mm")
    print(f"    A.end (offset):  {e_a:.2f} mm  (from exit ref)")
    print(f"    B.start (offset):{s_b:.2f} mm  (from entry ref)")
    print(f"    Physical gap:    {phys_gap:.2f} mm")
    print(f"    A.end physical:  [{p_end_a[0]*1000:.2f}, {p_end_a[1]*1000:.2f}] mm")
    print(f"    B.start physical:[{p_start_b[0]*1000:.2f}, {p_start_b[1]*1000:.2f}] mm")

    # The expected relationship:
    # If same direction: B.start position = A.end position + ref_dist
    #   -> B should be "ahead" of A by exactly ref_dist
    # But .start and .end are signed distances from their respective refs
    #   A.end = signed distance from A's exit ref to A's physical end
    #   B.start = signed distance from B's entry ref to B's physical start
    # For chaining: B's physical start should = A's physical end
    #   -> B.start (relative to B's entry ref) should account for the ref_dist

    # Key check: does the geometry work out?
    # A ends at: A_exit_ref + e_a/1000 * dir_vector_A
    # B starts at: B_entry_ref + s_b/1000 * dir_vector_B
    # For same dir: dir_vector_A = dir_vector_B = (cos(angle), sin(angle))
    # Gap = B_start - A_end (physical)

    dir_vec = array([cos(exit_angle), sin(exit_angle)])  # direction of travel on shared line
    a_end_phys_via_offset = p_a.exit.reference + (e_a / 1000) * dir_vec
    b_start_phys_via_offset = p_b.entry.reference + (s_b / 1000) * dir_vec
    computed_gap = norm(b_start_phys_via_offset - a_end_phys_via_offset) * 1000

    print(f"    A.end via offset: [{a_end_phys_via_offset[0]*1000:.2f}, {a_end_phys_via_offset[1]*1000:.2f}] mm")
    print(f"    B.start via offset:[{b_start_phys_via_offset[0]*1000:.2f}, {b_start_phys_via_offset[1]*1000:.2f}] mm")
    print(f"    Computed gap:     {computed_gap:.2f} mm")
    print(f"    Gap matches physical? {'YES' if abs(computed_gap - phys_gap) < 0.1 else 'NO'}")
    print()

# ═══════════════════════════════════════════════════════════════════════════════
# PART 4: Non-direct pairs - what connecting path is needed?
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n--- PART 4: Non-Direct Pairs (need intermediate path) ---\n")

all_names = list(turn_defs.keys())
non_direct = []
for name_a in all_names:
    for name_b in all_names:
        if name_a == name_b:
            continue
        if (name_a, name_b) not in direct_chains:
            non_direct.append((name_a, name_b))

# Group by exit/entry line combinations
from collections import defaultdict
line_pair_groups = defaultdict(list)
for a, b in non_direct:
    p_a = profiles[a]
    p_b = profiles[b]
    ek = line_key(p_a.exit)
    xk = line_key(p_b.entry)
    line_pair_groups[(ek, xk)].append((a, b))

print(f"  Total non-direct pairs: {len(non_direct)}")
print(f"  Unique line combinations: {len(line_pair_groups)}")
print()

for (ek, xk), pairs in sorted(line_pair_groups.items()):
    # Find representative
    a, b = pairs[0]
    p_a = profiles[a]
    p_b = profiles[b]

    s_a, e_a = get_start_end(p_a)
    s_b, e_b = get_start_end(p_b)

    ref_dist = norm(p_a.exit.reference - p_b.entry.reference) * 1000

    # Lines
    exit_line = p_a.exit
    entry_line = p_b.entry

    # Are they parallel? Perpendicular?
    try:
        intersection = lines_intersection(exit_line, entry_line)
        has_intersection = True
        int_dist_exit = norm(intersection - p_a.exit.reference) * 1000
        int_dist_entry = norm(intersection - p_b.entry.reference) * 1000
    except ValueError:
        has_intersection = False
        int_dist_exit = int_dist_entry = None

    angle_between = abs(exit_line.angle - entry_line.angle) % pi
    if angle_between > pi/2:
        angle_between = pi - angle_between
    is_parallel = abs(angle_between) < 0.01
    is_perpendicular = abs(angle_between - pi/4) < 0.01  # 45 deg

    # Actually, lines can be parallel, perpendicular, or at other angles
    angle_type = "PARALLEL" if is_parallel else (
        "PERPENDICULAR" if is_perpendicular else f"{np.degrees(angle_between):.0f}deg"
    )

    print(f"  Exit line:  {line_eq_str(exit_line):20s}  Turn types: {[p[0] for p in pairs]}")
    print(f"  Entry line: {line_eq_str(entry_line):20s}  Turn types: {[p[1] for p in pairs]}")
    print(f"  Angle between lines: {angle_type}")
    if has_intersection:
        print(f"  Intersection at: ({intersection[0]*1000:.2f}, {intersection[1]*1000:.2f}) mm")
        print(f"  Dist from exit ref:  {int_dist_exit:.2f} mm")
        print(f"  Dist from entry ref: {int_dist_entry:.2f} mm")
    else:
        print(f"  Lines are parallel (no intersection)")
    print(f"  Ref-to-ref distance: {ref_dist:.2f} mm")
    print()

# ═══════════════════════════════════════════════════════════════════════════════
# PART 5: Reference point distance matrix
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n--- PART 5: Reference Point Distance Matrix (mm) ---\n")
print(f"  {'':22s}", end="")
for name_b in all_names:
    print(f"  {name_b:>16s}", end="")
print()
for name_a in all_names:
    print(f"  {name_a:22s}", end="")
    for name_b in all_names:
        p_a = profiles[name_a]
        p_b = profiles[name_b]
        d = norm(p_a.exit.reference - p_b.entry.reference) * 1000
        if line_key(p_a.exit) == line_key(p_b.entry):
            marker = "*"
        else:
            marker = " "
        print(f"  {d:7.2f}{marker:>9s}", end="")
    print()

print(f"\n  * = same line (direct chain possible)")
print(f"  CELL = {cell*1000:.1f} mm")
print(f"  CELL_HALF_DIAGONAL = {CELL_HALF_DIAGONAL*1000:.2f} mm")
print(f"  CELL_DIAGONAL = {CELL_DIAGONAL*1000:.2f} mm")

# ═══════════════════════════════════════════════════════════════════════════════
# PART 6: Verify offsets for straight movement scenarios
# ═══════════════════════════════════════════════════════════════════════════════
print("\n\n--- PART 6: Straight Segment Analysis ---\n")
print("  For a straight segment between two turns at CONSECUTIVE posts:")
print("  - After turn A at post P1, robot exits along line L")
print("  - Travels straight along L for 1 CELL (180mm) to post P2")
print("  - Executes turn B at post P2, approaching along same line L")
print()
print("  The total straight distance = CELL - |A.end| + |B.start|")
print("  (adjusted for sign conventions)")
print()

for a, b in direct_chains:
    p_a = profiles[a]
    p_b = profiles[b]
    s_a, e_a = get_start_end(p_a)
    s_b, e_b = get_start_end(p_b)

    # Both refs are on the same line, 0.5 cells from their respective posts
    # The ref distance along the line:
    ref_dist = norm(p_a.exit.reference - p_b.entry.reference) * 1000

    # For consecutive posts: turn A at P1, straight segment, turn B at P2
    # P1 and P2 are 1 cell apart along the shared line
    # A's exit ref is 0.5 cells from P1, B's entry ref is 0.5 cells from P2
    # So ref_dist = 1 cell (if both are "forward" of their posts) or varies

    # The straight travel needed from A's physical end to B's physical start:
    xy_a = p_a.profile[['x', 'y']]
    xy_b = p_b.profile[['x', 'y']]
    p_end_a = xy_a.iloc[-1].values
    p_start_b = xy_b.iloc[0].values

    # If the two turns are at the SAME post, they share the same line
    # and the physical positions should ideally meet
    phys_gap = norm(p_end_a - p_start_b) * 1000

    print(f"  {a:22s} -> {b}")
    print(f"    Line:          {line_eq_str(p_a.exit)}")
    print(f"    Exit ref:      {ref_str(p_a.exit.reference)}")
    print(f"    Entry ref:     {ref_str(p_b.entry.reference)}")
    print(f"    Ref distance:  {ref_dist:.2f} mm")
    print(f"    A.end:         {e_a:.2f} mm")
    print(f"    B.start:       {s_b:.2f} mm")
    print(f"    Physical gap:  {phys_gap:.2f} mm")
    if phys_gap < 0.5:
        print(f"    -> Turns chain DIRECTLY (gap < 0.5mm)")
    else:
        # Check if this matches a known distance
        ratios = []
        for name, val in [('CELL', cell*1000), ('CELL_HALF_DIAG', CELL_HALF_DIAGONAL),
                          ('CELL_DIAG', CELL_DIAGONAL)]:
            ratio = phys_gap / val
            if abs(ratio - round(ratio)) < 0.05:
                ratios.append(f"{round(ratio)}x {name} ({val:.2f} mm)")
        if ratios:
            print(f"    -> Gap matches: {', '.join(ratios)}")
        else:
            print(f"    -> Needs straight segment of {phys_gap:.2f} mm")
    print()

print("=" * 90)
print("  SUMMARY")
print("=" * 90)
