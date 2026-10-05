# Maze Agent Learning Workplan

## Purpose

Build agents that navigate finite mazes from local observations, and evaluate
them against a deterministic reference procedure. A completed run alone is not
evidence of a general solution: the input domain, available memory,
transition rules, and halting conditions must all be explicit.

This plan is based on Chapter 3, “An Algorithm for Finding Paths in a
Labyrinth,” in B. A. Trakhtenbrot’s *Algorithms and Automatic Computing
Machines*, and the accompanying project notes in
`references/trakhtenbrot-algorithms-and-automatic-computing-machines-notes.md`.

## 1. Define the Maze Contract

- Represent a maze as a finite, undirected graph of junctions and corridors.
- Record a cyclic ordering of exits at each junction, a start junction, and a
  target junction.
- Define observations, legal actions, available agent memory, terminal states,
  and a fixed tie-break convention. For example: choose the first unvisited
  exit clockwise from the corridor used to enter the junction.
- Convert procedural layouts, including Entombed-generated layouts, to this
  graph representation only after validating their entrances, exits, and
  connectivity.

**Deliverable:** a versioned `Maze` schema and deterministic, seeded map
generation.

## 2. Build the Reference Solver

Implement the Chapter 3 Theseus procedure:

- Mark corridors as untraversed (green), active path (yellow), or exhausted
  (red).
- At each junction, apply this priority order: target, loop, untraversed
  corridor, origin, then backtrack.
- Never traverse a red corridor.
- Emit a trace for every step: observation, controller state, selected
  corridor, marking change, position, and halt reason.

**Acceptance criteria:** the solver halts on every finite fixture, never
traverses a red corridor, and its yellow corridors always form the current
simple path from the start to the agent.

## 3. Create a Benchmark Curriculum

Introduce structural difficulty gradually:

1. Trees and single-route mazes.
2. Dead ends and unreachable targets.
3. Loops and several routes to the target.
4. Larger maps with varying cyclic exit order.
5. Held-out procedural and Entombed-derived layouts.

For each benchmark, preserve the seed, graph, reference trace, reachability
result, path length, corridor-traversal count, and expected halt reason.

## 4. Separate Learning Tracks

Keep three experiments distinct:

- **Imitation learning:** reproduce the reference controller from local
  observations and memory.
- **Reinforcement learning:** learn from reward; label this as learned and
  potentially stochastic behavior rather than a deterministic algorithm.
- **Memory ablations:** compare reactive agents, bounded recurrent memory, and
  agents with explicit corridor-marking memory.

This identifies which memory model is necessary, instead of measuring only
whether an agent eventually succeeds.

## 5. Evaluate Behavior, Not Only Completion

Measure:

- Reachability accuracy and false “unreachable” outcomes.
- Steps and corridor traversals against the reference solver.
- Invalid actions and red-corridor violations.
- Repeatability for an identical maze and seed.
- Generalization to unseen sizes, topologies, and corridor orderings.
- Trace agreement with the reference policy.

Every failure should retain a replayable trace.

## 6. Add Visual Inspection

Extend the existing maze display with:

- A junction-and-corridor graph view.
- Green, yellow, and red corridor states.
- Agent and target positions.
- Step, play, pause, and reset controls.
- Reference-versus-agent trace replay and export.

The current graphical renderer is a useful map-display starting point; the
next display should expose search state as well.

## 7. Milestones

| Milestone | Outcome |
| --- | --- |
| M1 | Maze schema, graph validator, and deterministic fixtures |
| M2 | Chapter 3 reference solver, traces, and invariant tests |
| M3 | Dataset generator and benchmark manifests |
| M4 | Imitation baseline and memory ablations |
| M5 | Reinforcement-learning baseline, held-out evaluation, and replay UI |
| M6 | Results report describing generalization, failures, and evidence |

## Guardrail

The book’s standard should remain the project standard: call a procedure an
algorithm only when its allowed inputs, deterministic transition rules,
representation or memory, and termination conditions are specified. Use the
learned-agent experiments to test behavior against that specification, not to
replace it.
