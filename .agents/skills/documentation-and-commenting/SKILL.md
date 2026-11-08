---
name: documentation-and-commenting
description: Apply concise, nonredundant writing preferences when creating or editing technical documentation or code comments, including documentation accompanying code changes.
---

# Documentation and commenting

Assume the reader is capable with the tools and language. Keep documentation
concise and focused on project-specific knowledge, decisions, and constraints.
Omit basic tool instructions and command examples unless explicitly requested
or needed to convey a non-obvious requirement.

In explanatory documentation and code comments, state the relevant facts,
rationale, and constraints. Let readers draw routine conclusions rather than
adding advice that merely restates what those facts imply.

Give instructions when guidance is the purpose of the text—for example, usage
documentation, procedures, coding guidelines, or architectural rules—or when a
necessary action is not apparent from the explanation.

State what is needed directly. Mention prohibited actions or approaches only
when there is a strong practical reason, such as a likely mistake with meaningful
consequences.

Keep each fact in one authoritative place. Refer to maintained documentation,
build scripts, CI workflows, or configuration instead of repeating information
that can be readily reconstructed from them. Fix an incorrect source when in
scope rather than documenting a workaround for the inconsistency. Omit obsolete
workflow details and historical environment notes that do not help current work.

Review what the documentation reveals about the underlying design or workflow.
If it seems needlessly complex, propose a cleanup. Keep changes within the
requested scope; ask about the rationale when it is unclear. The user's
explanation may reveal a constraint or decision worth documenting.

For code comments:

- Use comments sparingly: each must add information that is not obvious from
  the code, interface, or implementation and is expected to remain true. Consider
  likely upcoming changes when deciding whether a comment will earn its upkeep.
- Use established names from the code to explain concrete relationships. Keep
  comments within the responsibility being described rather than introducing
  terminology or hypothetical concerns from other responsibilities.
- Write comments for the final code. Explain non-obvious, lasting contracts and
  rationale rather than information already conveyed by types, previous
  implementations, or explanations that depend on the editing history.
- Assume normal lifecycle use. Describe behavior outside that lifecycle, such
  as accessing an object before initialization, only when the API deliberately
  supports it and callers need that information. An implementation's incidental
  behavior does not by itself merit documentation.
- Explain why a choice is necessary when the reason is non-obvious and is not
  already documented elsewhere.
- Rely on clear code and appropriately named functions to convey what happens
  and how it works; avoid comments that narrate either.
- Document non-obvious public API contracts with Doxygen on the operation they
  describe. Reserve type-level documentation for contracts applying to the type
  as a whole. Focus on behavior callers need to know beyond what signatures and
  standard conventions already establish.
- Put a blank line before standalone comments, including line comments and
  single-line or multiline block comments. Treat consecutive comment lines as
  one block. Trailing comments may remain on the code line without extra spacing.
- Treat a comment block and the declaration it documents as one group. Put a
  blank line after that declaration to make the comment's scope clear before
  the next declaration.
- Single-line Doxygen comments may use `/** ... */`. For multiline blocks, put
  `/**` on its own line and align subsequent `*` prefixes and the closing `*/`.
- Leave existing comments untouched unless also editing their associated code.
  An explicit request to edit those comments takes precedence.
