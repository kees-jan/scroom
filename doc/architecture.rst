Architecture
============

Overview
--------

.. uml::
   :caption: Overview of Scrooms libraries

   package lib {
      component util as lib_util
      component threadpool as lib_threadpool
      component memory_manager as lib_memory_manager
      component plugin_interfaces as lib_plugin_interfaces
      component scroom as lib_scroom_lib
      component tiledbitmap as lib_tiledbitmap
      component ruler as lib_ruler
   }
   component scroom

   lib_plugin_interfaces --> lib_util
   lib_scroom_lib --> lib_plugin_interfaces
   lib_threadpool --> lib_util
   lib_memory_manager --> lib_threadpool
   lib_memory_manager --> lib_util
   lib_tiledbitmap --> lib_memory_manager
   lib_tiledbitmap --> lib_plugin_interfaces
   lib_tiledbitmap --> lib_scroom_lib
   lib_tiledbitmap --> lib_threadpool
   lib_ruler --> lib_util

   scroom --> lib_ruler
   scroom --> lib_scroom_lib
   scroom --> lib_tiledbitmap
   scroom --> lib_util

.. uml::
   :caption: Overview of Scrooms plugins

   package lib {
      component threadpool as lib_threadpool
      component plugin_interfaces as lib_plugin_interfaces
      component scroom as lib_scroom_lib
      component tiledbitmap as lib_tiledbitmap
   }
   package plugin {
      component colormap as plugin_spcolormap
      component example as plugin_spexample
      component measure as plugin_spmeasure
      component metadata as plugin_spmetadata
      component pipette as plugin_sppipette
      component tiff as plugin_sptiff
      component transparentoverlay as plugin_sptransparentoverlay
   }

   plugin_spcolormap --> lib_plugin_interfaces
   plugin_spexample --> lib_plugin_interfaces
   plugin_spexample --> lib_scroom_lib
   plugin_spmeasure --> lib_plugin_interfaces
   plugin_spmeasure --> lib_scroom_lib
   plugin_spmetadata --> lib_plugin_interfaces
   plugin_spmetadata --> lib_threadpool
   plugin_sppipette --> lib_plugin_interfaces
   plugin_sppipette --> lib_scroom_lib
   plugin_sppipette --> lib_threadpool
   plugin_sptiff --> lib_scroom_lib
   plugin_sptiff --> lib_tiledbitmap
   plugin_sptransparentoverlay --> lib_plugin_interfaces
   plugin_sptransparentoverlay --> lib_scroom_lib

No startup or shutdown code
---------------------------

Some systems have dedicated code to start up and shut down the application. 
Not Scroom. Scroom deals with documents (more accurately :scroom:`presentations <PresentationInterface>`) that
allocate huge amounts of resources. These can be opened at any time, and 
resources should be released as soon as the document is closed, not at shutdown.
And if that happens, startup and shutdown code really has nothing to do.

.. _asynchronous:

Asynchronous
------------

Scroom is largely asynchronous. We start some action, and when it finishes, that
generates an event which triggers further actions, such as updating the UI. We
do not explicitly wait for actions to finish (but many are cancelable), not even
when closing a document or even the application.

This means that it is normal for your code to still be running on some thread, even
when the document, or scroom itself, is being closed. To deal with this, classes
need to keep alive everything they need to function, because otherwise things might 
be destroyed because the document is closed. This explains why shared pointers are used extensively.

