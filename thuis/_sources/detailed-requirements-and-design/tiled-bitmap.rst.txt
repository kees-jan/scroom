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
