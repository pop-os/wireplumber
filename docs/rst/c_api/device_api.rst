.. _device_api:

PipeWire Device
===============
.. graphviz::
  :align: center

   digraph inheritance {
      rankdir=LR;
      GObject -> WpObject;
      WpObject -> WpProxy;
      WpProxy -> WpGlobalProxy;
      WpGlobalProxy -> WpDevice;
      GInterface -> WpPipewireObject;
      WpPipewireObject -> WpDevice;
   }

.. doxygenstruct:: WpDevice
   :project: WirePlumber

.. doxygengroup:: wpdevice
   :project: WirePlumber
   :content-only:
