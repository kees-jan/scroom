---
name: writing-tests
description: Apply Scroom's test-design preferences when writing, modifying, or reviewing C++ unit tests.
---

# Writing tests

Write tests as explanations of behavior. A maintainer should be able to understand
the scenario, why the expectation is correct, and what a failure suggests from
the test itself.

Group tests by the behavior or class under test. Give each test one behavior and
a name that explains it.

Use fixtures for repeated or elaborate setup that the tests actually use.
Construct simple, scenario-specific objects directly in the test when that makes
the scenario clearer. When tests repeatedly bypass fixture-provided objects,
reconsider the grouping: use plain tests, a shared construction helper, or
separate fixtures for distinct setups. Use fixture inheritance when the derived
scenarios genuinely reuse the base setup.

Weigh shared setup against the distance it creates between inputs and
expectations. Prefer local setup when it makes their relationship apparent,
even if that repeats a few construction lines. When shared values are useful,
give them names that make expectations understandable without inspecting the
fixture.

## Coverage

Choose the smallest set of scenarios that protects meaningful behavior and
distinguishes plausible defects. Add cases for distinct implementation paths
or changes in scale or structure that exercise additional behavior, such as
producing several outputs instead of one. Select representative cases rather
than taking Cartesian products of available inputs.

## Values and expectations

Name scenario-specific input values and reuse them in expectations. Derive
related positions, sizes, and expected values from those named inputs rather
than supplying independently calculated literals. Prefer simple relationships
such as equal halves, constant regions, and explicit frequency counts, so the
reason for the expectation is visible at the test site.

Use deliberate asymmetry to distinguish axes, channels, or positions, with only
the variation needed for the scenario. Choose patterns and tables whose
input/output relationship is apparent without decoding an arbitrary formula.
Naming a constant or helper is insufficient if the reader must still reconstruct
that relationship.

Choose names that explain each value's role. Use `aPoint` when its exact value
does not matter; use a more specific name when a boundary, coordinate space,
size, or direction matters. Distinguish values from objects operating on them;
apply the naming guidance in [code-changes](../code-changes/SKILL.md).

Share constants only when their meaning is independent of a particular test.
Keep scenario-specific inputs and expectations together. Helpers should remove
mechanical setup while leaving the scenario and the reason for its expected
result visible in the test.

Use parameterization when the same explanation applies clearly to every case.
Prefer separate named tests when cases need different explanations. Avoid packed
cases and conditional assertions that hide distinct scenarios.

## Failure diagnostics

Keep assertions at the test site. Prefer Google Mock matchers for reusable
comparisons instead of helper functions that contain assertions: `EXPECT_THAT`
retains the assertion's source location. Compose named field matchers so a
failure identifies the differing endpoint or coordinate. Choose floating-point
comparison semantics appropriate to the behavior being tested.

## Parents and children

Reuse tested helpers to express the test at the appropriate level, following
[code-changes](../code-changes/SKILL.md#reuse-and-abstractions). If relevant helper
behavior lacks coverage, establish it with focused helper tests before relying
on it in parent tests. Keep test dependencies acyclic: test a helper independently
of the class whose tests use it.

Pick the simplest suitable child and test the parent's responsibility, including
what it does with the child's output.
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
