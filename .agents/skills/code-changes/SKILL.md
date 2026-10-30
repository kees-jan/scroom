---
name: code-changes
description: Apply Scroom's design and coding conventions when implementing, refactoring, or fixing code in this repository.
---

# Code changes

Work as a senior C++ developer who values modular design and clean abstractions.
Favor cohesive responsibilities, clear interfaces, and explicit ownership and
lifetimes. Choose abstractions that serve the change and fit the surrounding
architecture.

Before changing code, read these files relative to the repository root:

- `doc/architecture.rst`
- `doc/patterns.rst`
- `doc/problems-and-solutions.rst`
- `doc/coding-guidelines.rst`

Apply their guidance to the affected components. Follow `AGENTS.md` for build
and verification requirements.

When writing or modifying tests, apply
[writing-tests](../writing-tests/SKILL.md).

## Naming

Name types and variables by their actual responsibility. Before choosing a
name, distinguish a value from an object that computes, modifies, validates,
or otherwise operates on that value. The name must preserve that distinction.

For objects representing an action, prefer names expressing that action. For
example, an implementation of `ITweakPresentationPosition` that aligns positions
to pixels could be named `PixelAlignPresentationPositions`, with a variable
named `tweak` or `pixelAlign`. Calling the type
`PixelAlignedPresentationPosition` and the variable `position` misleadingly
suggests that the object is itself a position.

Review type names and variable names together. A misleading existing type name
does not justify a misleading variable name; flag the type name too. Flagging
an existing name does not authorize a rename beyond the requested scope.

For each new or changed public type and its principal variables, ask what a
reader would reasonably believe it represents from the name alone. If that
differs from its actual responsibility, correct the name or flag the
uncertainty. Apply the same check to other names introduced or changed in the
implementation.

If the responsibility or intended terminology is unclear, explain the ambiguity
and ask for help before introducing or propagating the name. Choose between
accurate names using these conventions without asking for routine naming
decisions.

## Build sources

When adding a header, register it as a source of the appropriate target in
`CMakeLists.txt`, including headers used only by tests. Keep source registration
separate from decisions about installation.

Keep source file entries in CMake lists sorted alphabetically by path.

## Source file headers

Start every source file, including headers and test sources, with a copyright
message and an SPDX license identifier. Follow
`libs/util/inc/scroom/assertions.hh` for the project's header format, copyright
holder, and license identifier.
