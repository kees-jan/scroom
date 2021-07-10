install(
  FILES scroom/assertions.hh
        scroom/async-deleter.hh
        scroom/bitmap-helpers.hh
        scroom/blockallocator.hh
        scroom/bookkeeping.hh
        scroom/cairo-helpers.hh
        scroom/color.hh
        scroom/colormappable.hh
        scroom/compiler.hh
        scroom/dont-delete.hh
        scroom/function-additor.hh
        scroom/global.hh
        scroom/gtk-helpers.hh
        scroom/interface.hh
        scroom/layeroperations.hh
        scroom/linearsegment.hh
        scroom/memoryblobs.hh
        scroom/observable.hh
        scroom/imagemdinterface.hh
        scroom/opentiledbitmapinterface.hh
        scroom/plugininformationinterface.hh
        scroom/point.hh
        scroom/presentationinterface.hh
        scroom/progressinterface.hh
        scroom/progressinterfacehelpers.hh
        scroom/rectangle.hh
        scroom/resizablepresentationinterface.hh
        scroom/scroominterface.hh
        scroom/scroomplugin.hh
        scroom/semaphore.hh
        scroom/stuff.hh
        scroom/threadpool.hh
        scroom/tile.hh
        scroom/tiledbitmapinterface.hh
        scroom/tiledbitmaplayer.hh
        scroom/timer.hh
        scroom/transformpresentation.hh
        scroom/unused.hh
        scroom/utilities.hh
        scroom/viewinterface.hh
        scroom/imagemdinterface.hh
  DESTINATION include/scroom)

install(FILES scroom/impl/bookkeepingimpl.hh scroom/impl/threadpoolimpl.hh DESTINATION include/scroom/impl)

install(FILES scroom/gmock/colormappable.hh scroom/gmock/presentationinterface.hh DESTINATION include/scroom/gmock)
