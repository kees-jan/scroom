Tiled bitmap
============

Scroom visualizes bitmaps that are so large they do not (or at least they used to) fit comfortably
in memory. And even if it did fit, scaling down a multi-gigabyte bitmap to a size that fits on your
screen would take significant amounts of cpu time. To work around this, Scroom stores your image in pre-
scaled :scroom:`layers <Layer>`, each layer a factor 8 smaller than the one below it. Layers are then
further subdivided into :scroom:`tiles <Tile>`, such that only the area of the image you are currently
looking at needs to be in memory.

When you first open a bitmap, there's a preprocessing phase, in which all tiles in all layers are
initialized, scaling down the original bitmap as needed.

Requirements
------------

There's really only one: Render large bitmaps as faithfully as possible, as fast as possible.
See :ref:`be fast <be-fast>`. The pre-scaling we do into different layers gives rise to a ridiculous
amount of coordinate systems. To make things somewhat manageable, all coordinate systems share the same origin
(with a few exceptions, unfortunately). Let's go over all coordinate systems:

Presentation coordinates
  These are inherited from Scroom. They are the true-size coordinates, without any scaling. The bitmap
  might be located at an offset, though. Presentation coordinates are identical to the coordinates of the
  bottom most layer.

Source image coordinates
  These are coordinates within the bitmap being loaded, at true-size. They are Presentation coordinates,
  minus the offset (if any).

View coordinates
  These are coordinates of the destination window. It has, by necessity, a different origin, and it is scaled.
  When scroom calls our :scroom:`redraw <PresentationInterface::redraw>` method, it wants us to draw the
  given ``presentationArea`` at the requested zoomfactor, such that the top left of the ``presentationArea``
  lands at ``(0, 0)``. The size of the ``viewArea`` follows from scaling ``presentationArea.size()`` up or down
  as requested.

Layer coordinates
  These are coordinates inside one of the pre-scaled layers. They are Presentation coordinates, but with
  x and y scaled with the zoomfactor belonging to the layer

Grid coordinates or tile indices
  These coordinates are are for indexing tiles within a layer. For positive x and y, they'd be Layer coordinates
  ``div`` the tile size. Because ``div`` rounds towards zero, this breaks for negative coordinates, but you get
  the picture

Tile coordinates
  These are coordinates within a given tile. The origin is the top left corner of the tile. So for positive
  x and y, they'd be Layer coordinates ``mod`` the tile size. Again, this unfortunately breaks down for
  negative coordinates

Naming conventions
------------------

In many cases, the coordinate system will be obvious. ``presentationArea`` is an area in presentation coordinates.
``viewArea`` is in view coordinates. In case of ambiguity, for example after coordinate conversions, we suffix
variable names with an ``InXXXCoordinates``. For example, ``presentationAreaInLayerCoordinates`` might be
a useful thing to have. Though here too, there might be sane defaults. Like a layer might take layer coordinates
by default, and a tile might use tile coordinates. No need for suffixes there.

Conversion functions are typically called ``xxxCoordinatesFromYyyCoordinates()``. This matches well with the
naming conventions for variables. A line might read:

.. code-block:: cpp

  auto tileIndex = tileIndexFromLayerCoordinates(areaInLayerCoordinates);
  auto areaInTileCoordinates = tileCoordinatesFromLayerCoordinates(tileIndex, areaInLayerCoordinates);

Note how, lexically, the two ``tileCoordinates`` are close together, just as the two ``layerCoordinates``. That
makes for easy reviewing