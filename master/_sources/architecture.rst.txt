Architecture
============

Overview
--------

.. uml::
   :caption: Overview of Scrooms libraries

   package includes
   package boost_test_helper

   package libs {
      package sanity_tests
      package util
      package threadpool
      package memory_manager
      package libscroom
      package tiled_bitmap
      
      memory_manager --> threadpool
      memory_manager --> util
      libscroom --> util
      tiled_bitmap --> util
      tiled_bitmap --> threadpool
      tiled_bitmap --> memory_manager
      tiled_bitmap --> libscroom
   }

   package plugins{
      package example
      package transparentoverlay
      package tiff
      package colormap

      example --> libscroom
      transparentoverlay --> util
      transparentoverlay --> libscroom
      tiff --> tiled_bitmap
      tiff --> libscroom
   }

   package gui

   plugins --> includes
   libs --> includes
   libs --> boost_test_helper
   gui --> util
   gui --> includes



Solid principles
----------------
