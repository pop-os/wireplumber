/* WirePlumber
 *
 * Copyright © 2019 Collabora Ltd.
 *    @author George Kiagiadakis <george.kiagiadakis@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include <wp/wp.h>
#include <pipewire/pipewire.h>

static void
test_properties_basic (void)
{
  g_autoptr (WpProperties) p = NULL;

  p = wp_properties_new_empty ();
  g_assert_nonnull (p);

  g_assert_cmpint (wp_properties_set (p, "foo.bar", "test-value"), ==, 1);
  g_assert_cmpstr (wp_properties_get (p, "foo.bar"), ==, "test-value");
  g_assert_cmpstr (wp_properties_get (p, "nonexistent"), ==, NULL);

  /* remove the key */
  g_assert_cmpint (wp_properties_set (p, "foo.bar", NULL), ==, 1);
  g_assert_cmpstr (wp_properties_get (p, "foo.bar"), ==, NULL);
  /* now returns 0 because it does not exist */
  g_assert_cmpint (wp_properties_set (p, "foo.bar", NULL), ==, 0);

  g_assert_true (wp_properties_ref (p) == p);
  wp_properties_unref (p);
}

static void
test_properties_wrap_dict (void)
{
  g_autoptr (WpProperties) p = NULL;
  const struct spa_dict_item dict_items[] = {
    { "key1", "value1" },
    { "key2", "value2" },
  };
  const struct spa_dict dict = SPA_DICT_INIT_ARRAY(dict_items);

  p = wp_properties_new_wrap_dict (&dict);
  g_assert_nonnull (p);

  g_assert_cmpint (wp_properties_get_count (p), ==,
      SPA_N_ELEMENTS (dict_items));
  g_assert_cmpstr (wp_properties_get (p, "key1"), ==, "value1");
  g_assert_cmpstr (wp_properties_get (p, "key2"), ==, "value2");
  g_assert_cmpstr (wp_properties_get (p, "key3"), ==, NULL);

  g_assert_true (wp_properties_peek_dict (p) == &dict);
}

static void
test_properties_copy_dict (void)
{
  g_autoptr (WpProperties) p = NULL;
  const struct spa_dict_item dict_items[] = {
    { "key1", "value1" },
    { "key2", "value2" },
  };
  const struct spa_dict dict = SPA_DICT_INIT_ARRAY(dict_items);

  p = wp_properties_new_copy_dict (&dict);
  g_assert_nonnull (p);

  g_assert_cmpstr (wp_properties_get (p, "key1"), ==, "value1");
  g_assert_cmpstr (wp_properties_get (p, "key2"), ==, "value2");
  g_assert_cmpstr (wp_properties_get (p, "key3"), ==, NULL);

  g_assert_true (wp_properties_peek_dict (p) != &dict);
}

static void
test_properties_wrap (void)
{
  g_autoptr (WpProperties) p = NULL;
  struct pw_properties *props;

  props = pw_properties_new ("key1", "value1", NULL);
  g_assert_nonnull (props);
  p = wp_properties_new_wrap (props);
  g_assert_nonnull (p);

  g_assert_true (wp_properties_peek_dict (p) == &props->dict);
  g_assert_cmpstr (wp_properties_get (p, "key1"), ==, "value1");

  wp_properties_unref (g_steal_pointer (&p));
  /* because wrap does not free the original object, this should not crash */
  pw_properties_free (props);
}

static void
test_properties_take (void)
{
  g_autoptr (WpProperties) p = NULL;
  struct pw_properties *props;

  props = pw_properties_new ("key1", "value1", NULL);
  g_assert_nonnull (props);
  p = wp_properties_new_take (props);
  g_assert_nonnull (p);

  g_assert_true (wp_properties_peek_dict (p) == &props->dict);
  g_assert_cmpstr (wp_properties_get (p, "key1"), ==, "value1");

  /* value changes should be reflected on both objects */
  g_assert_cmpint (wp_properties_setf (p, "foobar", "%d", 2), ==, 1);
  g_assert_cmpstr (pw_properties_get (props, "foobar"), ==, "2");

  g_assert_cmpint (pw_properties_setf (props, "test", "some-%s", "value"), ==, 1);
  g_assert_cmpstr (wp_properties_get (p, "test"), ==, "some-value");

  /* no leaks because p frees props */
}

static void
test_properties_to_pw_props (void)
{
  g_autoptr (WpProperties) p = NULL;
  struct pw_properties *props;

  p = wp_properties_new ("key1", "value1", NULL);
  g_assert_nonnull (p);
  g_assert_cmpstr (wp_properties_get (p, "key1"), ==, "value1");

  props = wp_properties_to_pw_properties (p);
  g_assert_nonnull (props);
  g_assert_cmpstr (pw_properties_get (props, "key1"), ==, "value1");

  /* we have different underlying objects */
  g_assert_true (wp_properties_peek_dict (p) != &props->dict);
  g_assert_cmpint (pw_properties_set (props, "test", "some-value"), ==, 1);
  g_assert_cmpstr (wp_properties_get (p, "test"), ==, NULL);

  pw_properties_free (props);
}

static void
test_properties_iterate (void)
{
  g_autoptr (WpProperties) p = NULL;

  p = wp_properties_new_empty ();
  g_assert_nonnull (p);
  g_assert_cmpint (wp_properties_set (p, "key0", "value0"), ==, 1);
  g_assert_cmpint (wp_properties_set (p, "key1", "value1"), ==, 1);
  g_assert_cmpint (wp_properties_set (p, "key2", "value2"), ==, 1);
  g_assert_cmpint (wp_properties_set (p, "key3", "value3"), ==, 1);
  g_assert_cmpint (wp_properties_set (p, "key4", "value4"), ==, 1);

  g_autoptr (WpIterator) it = NULL;
  g_auto (GValue) item = G_VALUE_INIT;
  gint i = 0;
  for (it = wp_properties_new_iterator (p);
          wp_iterator_next (it, &item);
          g_value_unset (&item)) {
    WpPropertiesItem *pi = g_value_get_boxed (&item);
    g_autofree gchar *expected_key = g_strdup_printf ("key%d", i);
    g_autofree gchar *expected_value = g_strdup_printf ("value%d", i);
    g_assert_cmpstr (expected_value, ==, wp_properties_item_get_value (pi));
    g_assert_cmpstr (expected_key, ==, wp_properties_item_get_key (pi));
    i++;
  }
  g_assert_cmpint (i, ==, 5);
}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_log_set_writer_func (wp_log_writer_default, NULL, NULL);

  g_test_add_func ("/wp/properties/basic", test_properties_basic);
  g_test_add_func ("/wp/properties/wrap_dict", test_properties_wrap_dict);
  g_test_add_func ("/wp/properties/copy_dict", test_properties_copy_dict);
  g_test_add_func ("/wp/properties/wrap", test_properties_wrap);
  g_test_add_func ("/wp/properties/take", test_properties_take);
  g_test_add_func ("/wp/properties/to_pw_props", test_properties_to_pw_props);
  g_test_add_func ("/wp/properties/iterate", test_properties_iterate);

  return g_test_run ();
}
