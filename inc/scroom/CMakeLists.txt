install(FILES
        assertions.hh
        async-deleter.hh
        bitmap-helpers.hh
        blockallocator.hh
        bookkeeping.hh
        cairo-helpers.hh
        color.hh
        colormappable.hh
        compiler.hh
        dont-delete.hh
        function-additor.hh
        global.hh
        gtk-helpers.hh
        layeroperations.hh
        linearsegment.hh
        memoryblobs.hh
        observable.hh
        plugininformationinterface.hh
        point.hh
        presentationinterface.hh
        progressinterface.hh
        progressinterfacehelpers.hh
        rectangle.hh
        resizablepresentationinterface.hh
        scroominterface.hh
        scroomplugin.hh
        semaphore.hh
        stuff.hh
        threadpool.hh
        tile.hh
        tiledbitmapinterface.hh
        tiledbitmaplayer.hh
        timer.hh
        transformpresentation.hh
        unused.hh
        utilities.hh
        viewinterface.hh
        DESTINATION include/scroom)

install(FILES
        impl/bookkeepingimpl.hh
        impl/threadpoolimpl.hh
        DESTINATION include/scroom/impl)

install(FILES
        gmock/colormappable.hh
        gmock/presentationinterface.hh
        DESTINATION include/scroom/gmock)
