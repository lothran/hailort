#ifndef SGEMENTED_POOL_H_
#define SGEMENTED_POOL_H_

#include "gst/gstmemory.h"
#include "gst/video/gstvideopool.h"
#include <gst/gst.h>
#include <gst/video/video.h>

G_BEGIN_DECLS

/* video bufferpool */
typedef struct _GstCustomVideoBufferPool GstCustomVideoBufferPool;
typedef struct _GstCustomVideoBufferPoolClass GstCustomVideoBufferPoolClass;
typedef struct _GstCustomVideoBufferPoolPrivate GstCustomVideoBufferPoolPrivate;

#define GST_TYPE_CUSTOM_VIDEO_BUFFER_POOL                                      \
  (gst_custom_video_buffer_pool_get_type())
#define GST_IS_CUSTOM_VIDEO_BUFFER_POOL(obj)                                   \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_CUSTOM_VIDEO_BUFFER_POOL))
#define GST_CUSTOM_VIDEO_BUFFER_POOL(obj)                                      \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_CUSTOM_VIDEO_BUFFER_POOL,        \
                              GstCustomVideoBufferPool))
#define GST_CUSTOM_VIDEO_BUFFER_POOL_CAST(obj)                                 \
  ((GstCustomVideoBufferPool *)(obj))

struct _GstCustomVideoBufferPool {
  GstBufferPool bufferpool;

  GstCustomVideoBufferPoolPrivate *priv;
};

struct _GstCustomVideoBufferPoolClass {
  GstBufferPoolClass parent_class;
};

GST_VIDEO_API
GType gst_custom_video_buffer_pool_get_type(void);

GST_VIDEO_API
GstBufferPool *gst_custom_video_buffer_pool_new(void);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(GstCustomVideoBufferPool, gst_object_unref)

G_END_DECLS

#endif
