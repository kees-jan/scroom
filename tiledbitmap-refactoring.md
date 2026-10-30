# Tiled bitmap refactoring plan

Status: step 0 complete and accepted by the user. Step 1 implemented and
validated; awaiting user review.

## Goal and boundary

Give callers at the `OpenTiledBitmapAsPresentation::open()` boundary public tools
to construct and populate a layer stack independently of its visualization:

- Choose a bottom-layer rectangle, including a nonzero or negative origin.
- Choose whether reduced layers are populated by automatic reduction or left
  unpopulated. Callers do not supply pixels.
- Supply alternative layer and tile implementations.
- Read and reuse the stack without creating a view, including as input to a
  future composite image implementation.

The existing presentation adapter will use the default construction and loading
path. It will not expose new UI or exercise custom policies in normal operation.
Relocation preserves bitmap dimensions. Cropping, padding, compositing itself,
arbitrary reduction factors, and arbitrary tile sizes are outside this milestone. The default remains 4096-pixel tiles and factor-8
reduction. Implementation names below are provisional.

## Current code and obstacles

- `plugins/tiff/src/tiff.cc` and `plugins/pnm/src/pnm.cc` implement
  `OpenTiledBitmapInterface::open()`. Both allocate a concrete bottom `Layer` and
  return metadata plus a loading callback capturing that layer. The caller
  cannot replace the destination through that callback.
- `libs/tiled-bitmap/src/tiledbitmappresentation.cc` adapts the result into a
  presentation. It constructs `TiledBitmap` before invoking the loading callback.
  Both presentation classes in this file are private to the translation unit;
  external alternatives need public factories rather than private headers.
- `TiledBitmap::initialize()` creates the smaller layers and `connect()` builds
  one `LayerCoordinator` per target tile, observing up to 64 source tiles.
  Dimensions are rounded upward after each division by 8. A reduced layer is
  created only while its largest dimension exceeds `TILESIZE / 4`.
- `TiledBitmapInterface` combines loading, bottom-layer access, drawing, view
  attachment, cache control, and context access. Its implementation also owns
  the layers, reduction coordinators, progress, and task registrations.
- `LayerOperations` combines pixel format (`getBpp()`), reduction, drawing, and
  rendering caches. `LayerSpecForBitmap()` sometimes changes format between
  layers; separating operations must preserve those transitions and colormaps.
- `Layer` and `CompressedTile` inherit `Viewable`; `CompressedTile` owns per-view
  `TileViewState` objects. Extracting only `TiledBitmap::redraw()` would leave the
  data structure coupled to rendering and prevent independent renderer caches.
- `Layer` and `CompressedTile` have private constructors and concrete,
  nonvirtual data APIs. `Tile`/`ConstTile` are public-field pixel buffers tied to
  `RawPageData`. The substitution boundary must distinguish persistent tile
  storage from borrowed mutable/const pixel access.
- `Layer::getRect()` always returns a zero-origin rectangle, although
  `BitmapMetaData::rect` includes an origin. Arithmetic is duplicated in layer
  allocation, reduction wiring, loading, redraw, and pipette sampling.
- Existing arithmetic assumes positive dimensions and zero-based grids. Signed
  division, fractional redraw areas, rounding, validity at image edges, and integer
  overflow require explicit contracts before supporting alternative origins.
- `DataFetcher` fills complete horizontal tile rows. TIFF and PNM sources use
  file-local coordinates; PNM source implementations read sequentially and
  ignore `startLine`. Relocation must preserve sequential source consumption.
- `LayerCoordinator::unfinishedSourceTiles` and `TiledBitmap` completion counts
  are not reset for another load. Reduction work is posted outside the source
  loading queue. Target data initialization/access also needs a concurrency
  audit. These are risks to verify with tests, not established runtime diagnoses.
- Observer registration can invoke callbacks immediately. Ownership, locking,
  destruction, and cancellation must follow Scroom's asynchronous RAII model.
  Source-reading completion and completion of all reductions are different events.
- All plugins are now in-tree. API changes may migrate all consumers together;
  preserving old plugin binaries and changing the relic API version are not
  requirements. Public headers and CMake exports still need to be usable by
  alternative adapters.
- Existing tiled-bitmap tests cover sample iteration and destruction of a large
  stack, but do not characterize the stack geometry or its reduction lifecycle.
  Performance tools and pipette code also consume the current API.

### Reduction and coverage review

Preserve the current outward rounding at both levels:
`TiledBitmap::initialize()` uses `(dimension + 7) / 8` for reduced pixel sizes,
and `Layer` uses `(dimension + TILESIZE - 1) / TILESIZE` for tile counts. For
relocated rectangles, round the scaled pixel bounds outward, then round coverage
outward to the tile grid. Use floor for lower bounds and ceiling for upper bounds,
including negative coordinates, rather than rounding the width alone.

Reduction operates on full tile buffers, including boundary tiles. Reducers are
not passed the valid image rectangle; for example, the 8-bit reducer averages
all 64 samples, including zero-initialized storage outside the bitmap. Preserve
that behavior in this refactoring rather than introducing boundary normalization
or discarding incomplete blocks.

Keep allocated coverage separate from image bounds. Caller-selected coverage can
contain tiles outside the image even with outward rounding at both levels. Such
tiles must not be treated as image data or prevent completion. This is distinct
from deliberately leaving a layer unpopulated.

## Agreed constraints and remaining design work

1. Support relocation only, preserving dimensions. Pixels outside the bitmap
   are invalid. Cropping and padding are separate features outside this work.
2. Pixels originate in a bitmap decoder or automatic reduction, never in the
   caller. A layer may be left unpopulated; constructing fewer layers and
   constructing unpopulated layers are distinct choices.
3. Change APIs freely while migrating all in-tree plugins and consumers. Ignore
   API versioning and old plugin binary compatibility.
4. All layers share a coordinate origin. Caller-supplied offsets place the image
   relative to it; the origin need not coincide with the image's top-left or
   lie inside it. Do not anchor a new grid at the image's top-left as a shortcut.
   Keep the aligned source-to-target tile mapping; extend boundary handling to
   top/left as well as right/bottom. Distinguish allocated tile coverage from
   valid image bounds. Round reduced pixel bounds and tile coverage outward;
   do not infer validity from allocation. Empty
   tiles inside caller-selected coverage are legitimate and must not be drawn
   as image data or leave reduction completion waiting forever.
5. A layer's source is fixed once assigned: bitmap data (including reloads) or
   reduction from a fixed lower layer (including repeated reductions). Reloads
   do not replace sources or switch population policies. Construction may leave
   a layer without an assigned source; define that state explicitly.
6. Introduce generic tile-data access independent of the compressed backend.
   Decide its lifetime, mutability, format, stride, and valid-region contracts
   from actual decoder/reducer/renderer needs rather than exposing `RawPageData`.
7. Reuse `rectangle.hh` and `rounding.hh`, especially `roundOutward()`, rectangle
   intersections, and floor/ceiling division. Add reusable rounding operations
   there when needed. First extract existing arithmetic unchanged; adopt corrected
   rounding in a separate tested change where semantics differ.

A stage may be split into smaller reviewable changes, each subject to the same
completion gate. Resolve the remaining coordinate/data-access design details in
those reviews; do not introduce cropping, external pixel writers, or source
replacement as a consequence of those details.

## Test priorities

Test image fidelity and coordinate transformations: the correct source pixels
must reach the correct location at every layer and zoom, without gaps, overlaps,
axis swaps, shifts, or invalid pixels being presented as valid. Include pixel
values and format conversions where geometry-only tests cannot prove fidelity.
Use asymmetric patterns so incorrect mappings cannot pass through symmetry.
Derive redraw requests from integer screen rectangles at power-of-two zooms
and integer offsets. Do not require arbitrary fractional viewport sizes or
non-power-of-two zoom factors.

Treat tile size and pyramid cutoff as implementation choices. Express tile-edge
cases relative to the configured tile size or a helper's tile-size argument;
use several small sizes where the abstraction permits. Changing the production
tile size must not require rewriting expected pixel locations or test fixtures.
Test mapping between supplied levels without requiring a particular automatically
chosen stack height. Do not assert today's cutoff, exact default layer count, or
incidental allocation/callback counts. Test counts only when they establish a
correctness property, such as complete coverage or completion without duplicates.
Every test should protect observable correctness or a necessary public contract.

## Completion gate for every step

Each numbered checkbox represents a separately reviewable, working change. Mark
it complete only after its implementation and tests are finished, both
`gcc-debug` and `clang22-debug` build, all registered tests pass in both presets,
and the user has reviewed and accepted the changes. Stop between steps for that
review. Record validation results and the reviewed commit/change beside the step.
Known failures must be resolved or explicitly reflected in a revised plan;
an intermediate broken tree is not a completed step.

Follow `AGENTS.md` for configuration, headless displays, formatting, and test
parallelism. Changes to public interfaces must rebuild all plugins and consumers;
retain Ubuntu and Windows support and verify affected platform CI. Use the
generated development environment for GUI smoke tests. Formatting and relevant
clang-tidy checks apply to touched code. Do not run builds merely for this draft.

## Steps

- [x] **0. Extract small geometry abstractions before writing class tests.**
  Validation: both compiler presets build and pass all nine registered test
  suites directly on the desktop, without Xvfb. Clang-tidy checked the four
  changed source files; its blocking member-initialization diagnostic was fixed
  and the affected geometry source rechecked successfully. Nonfatal warnings
  remain. Formatting and whitespace checks pass. User review accepted on
  2026-10-30, including the `tileCount(size, tileSize)` parameter rename and
  the `computeRowLayout()` function rename.

  Move existing computations into functions over rectangles, points, dimensions,
  tile indices, and zoom values, with explicit arguments and value results. Wire
  existing callers to them without changing the arithmetic or behavior. Extract
  incrementally; avoid a general geometry framework or dependencies on `Layer`,
  `CompressedTile`, Cairo contexts, views, thread pools, or memory providers.
  Extracted functions in `tiled-bitmap-geometry.hh`, grouped by responsibility:

  - **Pyramid layout:** `pyramidSizes()` computes the sequence of layer sizes,
    preserving upward division and the current stopping threshold. This is a
    description of topology, not allocation of layers or operation objects.
  - **Tile-grid geometry:** `tileCount(size, tileSize)`, `tileOrigin()`, and
    `tileRectangle()` compute tile counts and positions. `redrawTileRange()`
    and `samplingTileRange()` retain their separate rounding implementations
    until tests establish their differences.
  - **Reduction mapping:** `reductionMapping()` computes the destination tile
    index and offset from a source tile index, independently of coordinator
    creation and observer registration.
  - **Layer selection and transforms:** `selectLayer()` selects a layer from
    zoom and available levels, returning residual zoom and the transformed
    rectangle, independently of fetching data or drawing it.
  - **Visible-region mapping:** `clippedDrawArea()`, `tileLocalIntersection()`,
    and `tileDrawAreas()` intersect requested/image/tile rectangles and compute
    tile-local source and view-local destination rectangles. Pipette sampling
    reuses `tileLocalIntersection()` without drawing.
  - **Row layout:** `computeRowLayout()` computes the tile-row start and valid
    line count, independently of fetching buffers or invoking a decoder.

  Keep these responsibilities separate even if related functions share a small
  tile-grid value type. Name coordinate spaces in arguments/results. Review the
  extracted interfaces and their existing call sites before growing them.
  Step 0 is extraction only; retain existing tests and apply the normal build,
  full-test, and user-review gate without first adding large class fixtures.

- [ ] **1. Test the extracted geometry for image fidelity.** Test the
  functions from step 0 with small value inputs, without constructing stacks,
  views, or pixel buffers. Prioritize parent/subregion mapping, residual zoom,
  visible source/destination rectangles, pipette intersections, and row layouts
  from `computeRowLayout()`. Check that mappings cover valid pixels exactly and preserve their
  positions between layers. Include narrow images, partial edges, screen-derived
  requests at power-of-two zooms, and empty/disjoint intersections. Describe boundary cases relative
  to the supplied tile size and reduction factor, rather than literal production
  dimensions. Do not lock down the pragmatic pyramid cutoff or default stack
  height. Use independently calculated examples and useful coverage/round-trip
  properties rather than duplicating implementation formulas. Record unsupported
  inputs and suspected defects instead of enshrining them as desired behavior.
  Add class-level tests later only for interactions that pure functions cannot
  establish.

  Implementation: `test/tiled-bitmap-geometry-tests.cc` covers tile coverage,
  sampling at tile edges, tile-local intersections, screen-derived source/view
  rectangles at power-of-two zooms, image-edge clipping, reduction subregions, row layouts, and
  layer selection with residual zoom. Tile-dependent cases run with three
  small supplied tile sizes; no production tile size or pyramid cutoff is
  asserted. Production geometry is unchanged.

  Validation: both compiler presets build and pass all nine registered suites
  directly on the desktop. Clang-tidy checked the new test source without
  errors; nonfatal style warnings remain. Formatting and whitespace checks
  pass. User review is pending.

- [ ] **2. Establish reusable rounding and coordinate contracts.** Build on the
  extracted functions and their tests. Define half-open bounds, floor/ceiling
  behavior, valid regions, numeric ranges, checked arithmetic, and empty/invalid
  input handling. Use `Rectangle`, `roundOutward()`, and `rounding.hh` rather
  than maintaining parallel rounding formulas. Add general rounding helpers to
  those utilities where useful. Test negative coordinates, fractional bounds,
  exact edges, and overflow rejection. Make any behavior corrections explicit
  and reviewable against step 1; do not enable relocated stacks yet. Establish
  the common-origin transform contract for the later relocation step.

  Replace `TileRange` with `Rectangle<int>` after adding tests for requests
  wholly outside the image, especially to the left or above. Preserve empty
  iteration ranges by clamping negative sizes to zero rather than allowing
  rectangle normalization to turn them into nonempty ranges. Keep `TileRange`
  during step 0 so that extraction does not change this behavior.

- [ ] **3. Separate reduction and pixel-format contracts from drawing.** Introduce
  data-side layer format/reduction descriptions and renderer-side operations.
  Adapt existing operation classes and `LayerSpecForBitmap()` so the default
  path preserves format transitions, reduction results, colormaps, and pipette
  capabilities. Keep a compatibility facade where needed so this is a complete
  migration step. Test representative packed, 8-bit, 16-bit, RGB, CMYK, and
  colormapped transitions; reduction must work without drawing callbacks.

- [ ] **4. Extract a reusable layer-stack owner.** Move layer topology,
  construction, reduction wiring, loading registrations, and data progress out
  of the renderer into a public stack abstraction. Expose read access to every
  layer, its format, and geometry, with source-driven population access. Have the
  existing `TiledBitmapInterface` delegate to the new stack and renderer so
  existing callers continue working. Test constructing, loading, reading, and
  destroying a stack without a view. Keep UI progress forwarding in an adapter.

- [ ] **5. Move view state and rendering caches out of data objects.** Make the
  renderer own view attachments, tile-view state, subscriptions, and caches.
  Remove data-side dependence on `Viewable`, `ViewInterface`, and Cairo-facing
  operation contracts. Reuse one stack in two independent renderers and in a
  nonvisual consumer. Test cache isolation, colormap invalidation, tile residency,
  redraw notification, and closing one renderer while the others retain data.
  Keep any unavoidable GTK header dependency inherited from utility types
  distinct from a runtime requirement for a view or GTK main loop.

- [ ] **6. Introduce substitutable layers and tile storage.** Define minimal
  interfaces for layer lookup/iteration, tile state and notifications, and safe
  mutable/const pixel access. Wrap the existing compressed implementation as the
  default backend; keep memory-provider policy specific to that backend. Inject
  factories rather than hard-coding `Layer::create()` and
  `CompressedTile::create()`. Replace mutable concrete-grid exposure with the
  operations actually needed by consumers. Add a simple in-memory alternative
  layer/tile implementation in tests and run loading, reduction, rendering, and
  sampling against both. This must prove runtime substitution, not just factory
  callbacks returning the same concrete classes.

- [ ] **7. Define and implement repeatable population sessions.** Add an explicit
  session/generation boundary for source-driven loading, tile completion, reduction scheduling,
  progress, errors, and cancellation. Ensure completion means all requested
  population/reduction work is complete; report read completion separately if
  useful. Reject/ignore stale work according to the documented contract, reset
  generation-specific counters, and invalidate affected renderer caches. Resolve
  coordinator target-data synchronization and callback reentrancy. Test repeated
  loads, duplicate completion, cancellation during reduction, failure, destruction
  with queued work, and late completion from an earlier session. Use controlled
  scheduling/synchronization rather than timing sleeps.

- [ ] **8. Make topology and population policies caller-controlled.** Add public
  construction options for a bottom-only or default pyramid topology, caller
  supplied compatible layer implementations, and automatic or no reduced-layer
  population. Bind each populated layer to its fixed bitmap or reduction source.
  Validate adjacent geometry and format compatibility before starting work.
  Unpopulated layers must not
  acquire automatic reduction observers or prevent progress from finishing.
  Define renderer behavior for unavailable levels: select an available level or
  draw an explicit missing-data state, without reading uninitialized pixels.
  Test every mode, partial edge dependencies, invalid configurations, and reloads
  preserving source bindings. Reduction cannot complete from an unpopulated
  source layer; reject inconsistent policies or report their unavailability.

- [ ] **9. Support relocated bitmap rectangles end to end.** Apply the shared
  origin and offset convention through construction, reduction transforms, data access,
  redraw, and pipette. Separate valid image bounds from full tile allocation and
  invalid pixels outside the bitmap. Test positive and negative offsets unaligned
  to the reduction factor or configured tile size, with the shared origin both inside and outside the image,
  multiple reduction levels, odd sizes, fractional viewports, and clipping on
  all sides. Verify translated stacks produce equivalent pixels and selections
  after transforming coordinates. Handle valid regions on all four boundaries
  without changing the aligned tile mapping. Test tiles whose intersection with
  reduced image bounds is empty because caller-selected coverage includes areas
  outside the image. Verify outward rounding of pixel bounds and tile coverage on all
  four sides. Ensure these tiles
  do not contribute invalid pixels or block progress/completion. Keep source-file
  coordinates separate from
  stack coordinates; metadata, sampling, and displayed bounds must agree.

- [ ] **10. Decouple plugin decoding from destination allocation.** Provide an
  opening result/source contract that lets the presentation caller select the
  destination stack before loading starts. Preserve decoder lifetime, format
  metadata, aspect ratio, colormap, cancellation, and reset validation. Migrate
  both TIFF and all PNM variants together, including plugin registration. Support
  relocation without changing source dimensions or sequential consumption,
  including packed pixels. Test failed opens, unsupported formats, changed
  metadata on reload, and relocated placement. Ignore API versioning.

- [ ] **11. Publish the complete default adapter and extension path.** Expose
  installed headers/factories for stack building, population, renderer creation,
  and presentation adaptation. Make `OpenTiledBitmapAsPresentation::open()` use
  those public tools with defaults only, removing its dependence on private
  `TiledBitmap` internals. Migrate performance tools, pipette, mocks, and other
  consumers; remove transitional facades once their users are migrated. Add an
  public-API consumer build test using the installed/exported API so an alternative
  adapter does not need private source headers or a registration workaround.

- [ ] **12. Verify the milestone as a whole.** Add integration coverage using a
  test-only alternative adapter: relocated rectangle, alternate layer/tile backend,
  automatic/unpopulated reduced layers, and nonvisual stack reads. Use
  TIFF and binary/ASCII/packed PNM fixtures. Confirm the normal adapter still
  uses default behavior; smoke-test image loading, zoom, pan, pipette, colormaps,
  aspect ratio, and closing during loading. Compare large-image memory use and
  loading/zoom responsiveness with the baseline to catch eager pixel allocation
  or lost reduction throttling. Document the public contracts and a concise
  example for future adapters, with no production compositing implementation.

## Final acceptance

The milestone is complete when an alternative presentation adapter can select
geometry, storage, topology, and population policies entirely through public
APIs, while the normal adapter follows the default path. A nonvisual client can
retain and inspect the stack without a renderer. All steps have passed their
completion gate and have been reviewed by the user.
