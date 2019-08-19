/* WirePlumber
 *
 * Copyright © 2019 Collabora Ltd.
 *    @author Julian Bouzas <julian.bouzas@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

/**
 * module-pw-bluez provides bluetooth device detection through pipewire
 * and automatically creates pipewire audio nodes to play and capture audio
 */

#include <spa/utils/names.h>
#include <spa/monitor/monitor.h>
#include <pipewire/pipewire.h>
#include <wp/wp.h>

struct monitor {
  struct spa_handle *handle;
  struct spa_monitor *monitor;
  struct spa_list device_list;
};

struct impl {
  WpModule *module;
  WpRemotePipewire *remote_pipewire;

  /* The bluez monitor */
  struct monitor monitor;
};

struct device {
  struct impl *impl;
  struct spa_list link;
  uint32_t id;

  struct pw_properties *props;

  struct spa_handle *handle;
  struct pw_proxy *proxy;
  struct spa_device *device;
  struct spa_hook device_listener;

  struct spa_list node_list;
};

struct node {
  struct impl *impl;
  struct device *device;
  struct spa_list link;
  uint32_t id;

  struct pw_properties *props;

  struct pw_node *adapter;
  struct pw_proxy *proxy;
};

static struct node *
create_node(struct impl *impl, struct device *dev, uint32_t id,
    const struct spa_device_object_info *info)
{
  struct node *node;
  const char *str;
  struct pw_properties *props = NULL;
  struct pw_factory *factory = NULL;
  struct pw_node *adapter = NULL;
  struct pw_proxy *proxy = NULL;

  /* Check if the type is a node */
  if (info->type != SPA_TYPE_INTERFACE_Node)
    return NULL;

  /* Create the properties */
  props = pw_properties_new_dict(info->props);
  str = pw_properties_get(dev->props, SPA_KEY_DEVICE_DESCRIPTION);
  if (str == NULL)
    str = pw_properties_get(dev->props, SPA_KEY_DEVICE_NAME);
  if (str == NULL)
      str = pw_properties_get(dev->props, SPA_KEY_DEVICE_NICK);
  if (str == NULL)
    str = pw_properties_get(dev->props, SPA_KEY_DEVICE_ALIAS);
  if (str == NULL)
    str = "bluetooth-device";
  pw_properties_setf(props, PW_KEY_NODE_NAME, "%s.%s", info->factory_name, str);
  pw_properties_set(props, PW_KEY_NODE_DESCRIPTION, str);
  pw_properties_set(props, "factory.name", info->factory_name);

  /* Find the factory */
  factory = wp_remote_pipewire_find_factory(impl->remote_pipewire, "adapter");
  g_return_val_if_fail (factory, NULL);

  /* Create the adapter */
  adapter = pw_factory_create_object(factory, NULL, PW_TYPE_INTERFACE_Node,
      PW_VERSION_NODE_PROXY, props, 0);
  g_return_val_if_fail (adapter, NULL);

  /* Create the proxy */
  proxy = wp_remote_pipewire_export(impl->remote_pipewire,
      PW_TYPE_INTERFACE_Node, props, adapter, 0);
  g_return_val_if_fail (proxy, NULL);

  /* Create the node */
  node = g_slice_new0(struct node);
  node->impl = impl;
  node->device = dev;
  node->id = id;
  node->props = props;
  node->adapter = adapter;
  node->proxy = proxy;

  /* Add the node to the list */
  spa_list_append(&dev->node_list, &node->link);

  return node;
}

static void
update_node(struct impl *impl, struct device *dev, struct node *node,
    const struct spa_device_object_info *info)
{
  /* Just update the properties */
  pw_properties_update(node->props, info->props);
}

static void destroy_node(struct impl *impl, struct device *dev, struct node *node)
{
  /* Remove the node from the list */
  spa_list_remove(&node->link);

  /* Destroy the proxy node */
  pw_proxy_destroy(node->proxy);

  /* Destroy the node */
  g_slice_free (struct node, node);
}

static struct node *
find_node(struct device *dev, uint32_t id)
{
  struct node *node;

  /* Find the node in the list */
  spa_list_for_each(node, &dev->node_list, link) {
    if (node->id == id)
      return node;
  }

  return NULL;
}

static void
device_object_info(void *data, uint32_t id,
  const struct spa_device_object_info *info)
{
  struct device *dev = data;
  struct impl *impl = dev->impl;
  struct node *node = NULL;

  /* Find the node */
  node = find_node(dev, id);

  if (info) {
    /* Just update the node if it already exits, otherwise create it */
    if (node)
      update_node(impl, dev, node, info);
    else
      create_node(impl, dev, id, info);
  } else {
    /* Just remove the node if it already exists */
    if (node)
      destroy_node(impl, dev, node);
  }
}

static const struct spa_device_events device_events = {
  SPA_VERSION_DEVICE_EVENTS,
  .object_info = device_object_info
};

static struct device*
create_device(struct impl *impl, uint32_t id,
  const struct spa_monitor_object_info *info) {

  struct device *dev;
  struct spa_handle *handle;
  int res;
  void *iface;

  /* Check if the type is a device */
  if (info->type != SPA_TYPE_INTERFACE_Device)
    return NULL;

  /* Load the device handle */
  handle = (struct spa_handle *)wp_remote_pipewire_load_spa_handle (
      impl->remote_pipewire, info->factory_name, info->props);
  if (!handle)
    return NULL;

  /* Get the handle interface */
  res = spa_handle_get_interface(handle, info->type, &iface);
  if (res < 0) {
    pw_unload_spa_handle(handle);
    return NULL;
  }

  /* Create the device */
  dev = g_slice_new0(struct device);
  dev->impl = impl;
  dev->id = id;
  dev->handle = handle;
  dev->device = iface;
  dev->props = pw_properties_new_dict(info->props);
  dev->proxy = wp_remote_pipewire_export (impl->remote_pipewire, info->type, dev->props, dev->device, 0);
  if (!dev->proxy) {
    pw_unload_spa_handle(handle);
    return NULL;
  }
  spa_list_init(&dev->node_list);

  /* Add device listener for events */
  spa_device_add_listener(dev->device, &dev->device_listener, &device_events,
      dev);

  /* Add the device to the list */
  spa_list_append(&impl->monitor.device_list, &dev->link);

  return dev;
}

static void
update_device(struct impl *impl, struct device *dev,
    const struct spa_monitor_object_info *info)
{
  /* Update the properties of the device */
  pw_properties_update(dev->props, info->props);
}

static void
destroy_device(struct impl *impl, struct device *dev)
{
  /* Remove the device from the list */
  spa_list_remove(&dev->link);

  /* Remove the device listener */
  spa_hook_remove(&dev->device_listener);

  /* Destroy the device proxy */
  pw_proxy_destroy(dev->proxy);

  /* Unload the device handle */
  pw_unload_spa_handle(dev->handle);

  /* Destroy the object */
  g_slice_free (struct device, dev);
}

static struct device *
find_device(struct impl *impl, uint32_t id)
{
  struct device *dev;

  /* Find the device in the list */
  spa_list_for_each(dev, &impl->monitor.device_list, link) {
    if (dev->id == id)
      return dev;
  }

  return NULL;
}

static int
monitor_object_info(gpointer data, uint32_t id,
    const struct spa_monitor_object_info *info)
{
  struct impl *impl = data;
  struct device *dev = NULL;

  /* Find the device */
  dev = find_device(impl, id);

  if (info) {
    /* Just update the device if it already exits, otherwise create it */
    if (dev)
      update_device(impl, dev, info);
    else
      if (!create_device(impl, id, info))
        return -ENOMEM;
  } else {
    /* Just remove the device if it already exists, otherwise return error */
    if (dev)
      destroy_device(impl, dev);
    else
      return -ENODEV;
  }

  return 0;
}

static const struct spa_monitor_callbacks monitor_callbacks =
{
  SPA_VERSION_MONITOR_CALLBACKS,
  .object_info = monitor_object_info,
};

static void
start_monitor (WpRemotePipewire *remote, WpRemoteState state, gpointer data)
{
  struct impl *impl = data;
  struct spa_handle *handle;
  int res;
  void *iface;

  /* Load the monitor handle */
  handle = (struct spa_handle *)wp_remote_pipewire_load_spa_handle (
      impl->remote_pipewire, SPA_NAME_API_BLUEZ5_MONITOR, NULL);
  g_return_if_fail (handle);

  /* Get the handle interface */
  res = spa_handle_get_interface(handle, SPA_TYPE_INTERFACE_Monitor, &iface);
  if (res < 0) {
    g_critical ("module-pw-alsa-udev cannot get monitor interface");
    pw_unload_spa_handle(handle);
    return;
  }

  /* Init the monitor data */
  impl->monitor.handle = handle;
  impl->monitor.monitor = iface;
  spa_list_init(&impl->monitor.device_list);

  /* Set the monitor callbacks */
  spa_monitor_set_callbacks(impl->monitor.monitor, &monitor_callbacks, impl);
}

static void
module_destroy (gpointer data)
{
  struct impl *impl = data;

  /* Set to NULL module and remote pipewire as we don't own the reference */
  impl->module = NULL;
  impl->remote_pipewire = NULL;

  /* Clean up */
  g_slice_free (struct impl, impl);
}

void
wireplumber__module_init (WpModule * module, WpCore * core, GVariant * args)
{
  struct impl *impl;
  WpRemotePipewire *rp;

  /* Make sure the remote pipewire is valid */
  rp = wp_core_get_global (core, WP_GLOBAL_REMOTE_PIPEWIRE);
  if (!rp) {
    g_critical ("module-pw-bluez cannot be loaded without a registered "
        "WpRemotePipewire object");
    return;
  }

  /* Create the module data */
  impl = g_slice_new0(struct impl);
  impl->module = module;
  impl->remote_pipewire = rp;

  /* Set destroy callback for impl */
  wp_module_set_destroy_callback (module, module_destroy, impl);

  /* Add the spa lib */
  wp_remote_pipewire_add_spa_lib (rp, "api.bluez5.*", "bluez5/libspa-bluez5");

  /* Start the monitor when the connected callback is triggered */
  g_signal_connect(rp, "state-changed::connected", (GCallback)start_monitor, impl);
}