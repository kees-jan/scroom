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

## Reuse and abstractions

Look for existing, tested helpers before writing lower-level code. Reuse them
when they express the intended operation clearly and make the code easier to
understand.

Propose new or improved abstractions when they would clarify responsibilities
or simplify the implementation. Apply improvements within the authorized scope;
discuss changes beyond that scope before proceeding.

Add convenience overloads when they make calling code clearer by accepting
inputs in the form callers naturally have. Forward them to one implementation
so behavior stays consistent. Weigh the simpler calls against ambiguity and API
complexity; callers constructing objects merely to unpack them again can indicate
a useful overload.

Use defaults sparingly. Require callers to supply important policy choices in
low-level APIs; select application defaults at the layer responsible for that
policy.

## Headers and implementation hiding

Prefer including a type's defining header over forward declarations. Making a
dependency public is acceptable when it simplifies the design; public/private
boundaries need not be preserved solely to hide implementation details.

Keep a concrete class's state in the class. Introduce implementation hiding
when it serves a concrete need; when hiding is warranted, prefer an interface and a
factory returning an owning pointer over pImpl.

## Language features and consistency

Use modern C++ features supported by the project's configured standard and
supported platforms. Prefer them when they improve the code rather than
imitating legacy idioms in surrounding code. The CMake `project_options` target
is authoritative for the language standard.

When introducing a language feature in a file, use it at every relevant
opportunity in that file where it expresses the same intent. Prefer class
template argument deduction for constructions whose arguments establish the
intended type, such as points and rectangles. Keep explicit template arguments
when they specify an intended conversion or numeric precision.

## Function definition spacing

Leave a blank line before and after a function definition, including its attached
comments or attributes. Definitions that fit entirely on the declaration line
may remain grouped with adjacent declarations or other one-line definitions.

## Constants and enumerations

Use named `constexpr` constants when the numeric value is the meaning, such as
an index, size, offset, or numeric parameter. Scope related constants in an
appropriate namespace; keep scenario-specific constants local.

Use `enum class` for categorical alternatives with meaningful type identity.
Choose an enumeration to represent those alternatives, rather than as a
convenient way to number constants.

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
