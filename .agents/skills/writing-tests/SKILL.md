---
name: writing-tests
description: Apply Scroom's test-design preferences when writing, modifying, or reviewing C++ unit tests.
---

# Writing tests

Group tests by the class under test. Use a fixture that creates an instance of
that class. Give each test one behavior and a name that explains it.

## Values and expectations

Use explanatory variables and small helpers to make the scenario readable.
Construct related inputs and expected results from named base values rather
than supplying independently calculated numeric literals. For example,
`aBitAboveAndLeftOf(aGridPoint)` expresses a geometric relationship without
requiring the reader to compare coordinate literals.

Choose names that explain each value's role. Use `aPoint` when its exact value
does not matter; use a more specific name when a boundary, coordinate space,
size, or direction matters. Distinguish values from objects operating on them;
apply the naming guidance in [code-changes](../code-changes/SKILL.md).

Shared constants are appropriate when their names describe their meaning
independently of a particular test. Keep scenario-specific values local. Moving
a literal into a named constant is insufficient if the reader still has to
reverse-engineer its relationship to the test.

Use tables or parameterization when they clarify a common behavior. Avoid
packed cases and conditional assertions that hide distinct scenarios.

## Failure diagnostics

Keep assertions at the test site. Prefer Google Mock matchers for reusable
comparisons instead of helper functions that contain assertions: `EXPECT_THAT`
retains the assertion's source location. Compose named field matchers so a
failure identifies the differing endpoint or coordinate. Choose floating-point
comparison semantics appropriate to the behavior being tested.

## Parents and children

Assume a tested, functioning child. Pick the simplest suitable child and test
the parent's responsibility, including what it does with the child's output.
Avoid repeating child-behavior tests through the parent or exercising multiple
child implementations when that adds no coverage of the parent's behavior.

Use controlled child outputs when needed to isolate the parent. In particular,
an identity child can allow missing input and output conversions to cancel
unnoticed. Mock expectations should verify meaningful delegation and conversion
contracts rather than incidental implementation details.

Follow `AGENTS.md` for build, execution, and resource limits.

## Mock headers

Place mock declarations in a `gmock` directory next to the header declaring the
original types, using the same header filename. For example, mocks for
`inc/scroom/tweak-view.hh` belong in `inc/scroom/gmock/tweak-view.hh`.
See `libs/plugin-interfaces/inc/scroom/gmock/colormappable.hh` for the existing
convention. Keep unrelated test helpers separate from mock declarations.

Declare mocks in the same namespace as the types they mock. Private test helpers
need no named namespace; when a namespace is convenient because of the code they
call, use that code's namespace.
