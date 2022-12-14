 .. _installing-wireplumber:

Installing WirePlumber
======================

Dependencies
------------

In order to compile WirePlumber you will need:

* GLib >= 2.62
* PipeWire 0.3 (>= 0.3.43)
* Lua 5.3 or 5.4

Lua is optional in the sense that if it is not found in the system, a bundled
version will be built and linked statically with WirePlumber. This is controlled
by the **system-lua** meson option.

For building gobject-introspection data, you will also need
`doxygen <https://www.doxygen.nl/>`_ and **g-ir-scanner**.
The latter is usually shipped together with the gobject-introspection
development files.

For building documentation, you will also need
`Sphinx <https://pypi.org/project/Sphinx/>`_,
`sphinx-rtd-theme <https://github.com/readthedocs/sphinx_rtd_theme>`_ and
`breathe <https://pypi.org/project/breathe/>`_.
It is recommended to install those using python's **pip** package manager.

Compilation
-----------

WirePlumber uses the `meson build system <https://mesonbuild.com/>`_

To configure the project, you need to first run `meson`.
The basic syntax is shown below:

.. code:: console

   meson [--prefix=/path] [...options...] [build directory] [source directory]

Assuming you want to build in a directory called 'build' inside the source
tree, you can run:

.. code:: console

   $ meson setup --prefix=/usr build
   $ meson compile -C build

Additional options
------------------

.. option:: -Dintrospection=[enabled|disabled|auto]

  Force enable or force disable building gobject-introspection data.

  The default value is **auto**, which means that g-i will be built
  if **doxygen** and **g-ir-scanner** are found and skipped otherwise.

.. option:: -Ddocs=[enabled|disabled|auto]

  Force enable or force disable building documentation.

  The default value is **auto**, which means that documentation will be built
  if **doxygen**, **sphinx** and **breathe** are found and skipped otherwise.

.. option:: -Dmodules=[true|false]

   Build the wireplumber modules.

   The default value is **true**.

.. option:: -Ddaemon=[true|false]

   Build the session manager daemon.

   The default value is **true**.

   Requires **modules** option to be **true**.

.. option:: -Dtools=[true|false]

   Enable or disable building tools.

   The default value is **true**.

   Requires **modules** option to be **true**.

.. option:: -Dsystem-lua=[enabled|disabled|auto]

   Force using lua from the system instead of the bundled one.

   The default value is **auto**, which means that system lua will be used
   if it is found, otherwise the bundled static version will be built silently.

   Use **disabled** to force using the bundled lua.

.. option:: -Dsystemd=[enabled|disabled|auto]

   Enables installing systemd units. The default is **auto**

   **enabled** and **auto** currently mean the same thing.

.. option:: -Dsystemd-system-service=[true|false]

   Enables installing systemd system service file. The default is **false**

.. option:: -Dsystemd-user-service=[true|false]

   Enables installing systemd user service file. The default is **true**

.. option:: -Dsystemd-system-unit-dir=[path]

   Directory for system systemd units.

.. option:: -Dsystemd-user-unit-dir=[path]

   Directory for user systemd units.

Installation
------------

To install, simply run the **install** target with ninja:

.. code:: console

   $ ninja -C build install

To revert the installation, there is also an **uninstall** target:

.. code:: console

   $ ninja -C build uninstall
