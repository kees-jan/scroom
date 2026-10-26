---
name: documentation-and-commenting
description: Apply concise, nonredundant writing preferences when creating or editing technical documentation or code comments, including documentation accompanying code changes.
---

# Documentation and commenting

Assume the reader is capable with the tools and language. Keep documentation
concise and focused on project-specific knowledge, decisions, and constraints.
Omit basic tool instructions and command examples unless explicitly requested
or needed to convey a non-obvious requirement.

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

- Explain why a choice is necessary when the reason is non-obvious and is not
  already documented elsewhere.
- Rely on clear code and appropriately named functions to convey what happens
  and how it works; avoid comments that narrate either.
- Leave existing comments untouched unless also editing their associated code.
  An explicit request to edit those comments takes precedence.
