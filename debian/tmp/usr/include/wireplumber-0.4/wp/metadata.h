/* WirePlumber
 *
 * Copyright © 2020 Collabora Ltd.
 *    @author Raghavendra Rao <raghavendra.rao@collabora.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef __WIREPLUMBER_METADATA_H__
#define __WIREPLUMBER_METADATA_H__

#include "global-proxy.h"

G_BEGIN_DECLS

/*!
 * \brief An extension of WpProxyFeatures for WpMetadata objects
 * \ingroup wpmetadata
 */
typedef enum { /*< flags >*/
  /*! caches metadata locally */
  WP_METADATA_FEATURE_DATA = (WP_PROXY_FEATURE_CUSTOM_START << 0),
} WpMetadataFeatures;

/*!
 * \brief The WpMetadata GType
 * \ingroup wpmetadata
 */
#define WP_TYPE_METADATA (wp_metadata_get_type ())

WP_API
G_DECLARE_DERIVABLE_TYPE (WpMetadata, wp_metadata, WP, METADATA, WpGlobalProxy)

struct _WpMetadataClass
{
  WpGlobalProxyClass parent_class;

  /*< private >*/
  WP_PADDING(4)
};

WP_API
WpIterator * wp_metadata_new_iterator (WpMetadata * self, guint32 subject);

WP_API
void wp_metadata_iterator_item_extract (const GValue * item, guint32 * subject,
    const gchar ** key, const gchar ** type, const gchar ** value);

WP_API
const gchar * wp_metadata_find (WpMetadata * self, guint32 subject,
    const gchar * key, const gchar ** type);

WP_API
void wp_metadata_set (WpMetadata * self, guint32 subject,
    const gchar * key, const gchar * type, const gchar * value);

WP_API
void wp_metadata_clear (WpMetadata * self);

/*!
 * \brief The WpImplMetadata GType
 * \ingroup wpmetadata
 */
#define WP_TYPE_IMPL_METADATA (wp_impl_metadata_get_type ())

WP_API
G_DECLARE_FINAL_TYPE (WpImplMetadata, wp_impl_metadata, WP, IMPL_METADATA, WpMetadata)

WP_API
WpImplMetadata * wp_impl_metadata_new (WpCore * core);

WP_API
WpImplMetadata * wp_impl_metadata_new_full (WpCore * core, const gchar *name,
    WpProperties *properties);

G_END_DECLS

#endif
