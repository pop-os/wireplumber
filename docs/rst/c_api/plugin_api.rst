.. _plugin_api:

Plugins
=======
.. graphviz::
  :align: center

   digraph inheritance {
      rankdir=LR;
      GObject -> WpObject;
      WpObject -> WpPlugin;
   }

.. doxygenstruct:: WpPlugin
   :project: WirePlumber

.. doxygenstruct:: _WpPluginClass
   :project: WirePlumber

.. doxygengroup:: wpplugin
   :project: WirePlumber
   :content-only:
