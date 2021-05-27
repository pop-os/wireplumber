.. _proxy_api:

PipeWire Proxy
==============
.. graphviz::
  :align: center

   digraph inheritance {
      rankdir=LR;
      GObject -> WpObject;
      WpObject -> WpProxy;
   }

.. doxygenstruct:: WpProxy
   :project: WirePlumber

.. doxygenstruct:: _WpProxyClass
   :project: WirePlumber

.. doxygengroup:: wpproxy
   :project: WirePlumber
   :content-only:
