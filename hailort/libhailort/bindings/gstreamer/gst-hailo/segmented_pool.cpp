#include "segmented_pool.hpp"
#include "gst/allocators/gstdmabuf.h"
#include "gst/gstbuffer.h"
#include "gst/gstinfo.h"
#include "gst/gstmemory.h"
#include "gst/gstpad.h"
#include "gst/gststructure.h"
#include "gst/video/gstvideopool.h"

/* bufferpool */
struct _GstCustomVideoBufferPoolPrivate {
  GstVideoInfo info;
  GstVideoAlignment video_align;
  gboolean add_videometa;
  gboolean need_alignment;
  GstAllocator *allocator;
  GstAllocationParams params;
};

static void gst_custom_video_buffer_pool_finalize(GObject *object);

#define gst_custom_video_buffer_pool_parent_class parent_class
G_DEFINE_TYPE_WITH_PRIVATE(GstCustomVideoBufferPool,
                           gst_custom_video_buffer_pool, GST_TYPE_BUFFER_POOL);

static const gchar **custom_video_buffer_pool_get_options(GstBufferPool *pool) {
  static const gchar *options[] = {GST_BUFFER_POOL_OPTION_VIDEO_META,
                                   GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT,
                                   NULL};
  return options;
}

static gboolean custom_video_buffer_pool_set_config(GstBufferPool *pool,
                                                    GstStructure *config) {
  GstCustomVideoBufferPool *vpool = GST_CUSTOM_VIDEO_BUFFER_POOL_CAST(pool);
  GstCustomVideoBufferPoolPrivate *priv = vpool->priv;
  GstVideoInfo info;
  GstCaps *caps;
  guint size, min_buffers, max_buffers;
  gint width, height;
  GstAllocator *allocator;
  GstAllocationParams params;

  if (!gst_buffer_pool_config_get_params(config, &caps, &size, &min_buffers,
                                         &max_buffers))
    goto wrong_config;

  if (caps == NULL)
    goto no_caps;

  /* now parse the caps from the config */
  if (!gst_video_info_from_caps(&info, caps))
    goto wrong_caps;

  if (size < info.size)
    goto wrong_size;

  if (!gst_buffer_pool_config_get_allocator(config, &allocator, &params))
    goto wrong_config;

  width = info.width;
  height = info.height;

  GST_LOG_OBJECT(pool, "%dx%d, caps %" GST_PTR_FORMAT, width, height, caps);

  priv->params = params;
  if (priv->allocator)
    gst_object_unref(priv->allocator);
  if ((priv->allocator = allocator))
    gst_object_ref(allocator);

  /* enable metadata based on config of the pool */
  priv->add_videometa = gst_buffer_pool_config_has_option(
      config, GST_BUFFER_POOL_OPTION_VIDEO_META);

  /* parse extra alignment info */
  priv->need_alignment = gst_buffer_pool_config_has_option(
      config, GST_BUFFER_POOL_OPTION_VIDEO_ALIGNMENT);

  if (priv->need_alignment && priv->add_videometa) {
    guint max_align, n;

    gst_buffer_pool_config_get_video_alignment(config, &priv->video_align);

    /* ensure GstAllocationParams alignment is compatible with video alignment
     */
    max_align = priv->params.align;
    for (n = 0; n < GST_VIDEO_MAX_PLANES; ++n)
      max_align |= priv->video_align.stride_align[n];

    for (n = 0; n < GST_VIDEO_MAX_PLANES; ++n)
      priv->video_align.stride_align[n] = max_align;

    /* apply the alignment to the info */
    if (!gst_video_info_align(&info, &priv->video_align))
      goto failed_to_align;

    gst_buffer_pool_config_set_video_alignment(config, &priv->video_align);

    if (priv->params.align < max_align) {
      GST_WARNING_OBJECT(
          pool,
          "allocation params alignment %u is smaller "
          "than the max specified video stride alignment %u, fixing",
          (guint)priv->params.align, max_align);
      priv->params.align = max_align;
      gst_buffer_pool_config_set_allocator(config, allocator, &priv->params);
    }
  }
  info.size = MAX(size, info.size);
  priv->info = info;

  gst_buffer_pool_config_set_params(config, caps, info.size, min_buffers,
                                    max_buffers);

  return GST_BUFFER_POOL_CLASS(parent_class)->set_config(pool, config);

  /* ERRORS */
wrong_config: {
  GST_WARNING_OBJECT(pool, "invalid config");
  return FALSE;
}
no_caps: {
  GST_WARNING_OBJECT(pool, "no caps in config");
  return FALSE;
}
wrong_caps: {
  GST_WARNING_OBJECT(pool, "failed getting geometry from caps %" GST_PTR_FORMAT,
                     caps);
  return FALSE;
}
wrong_size: {
  GST_WARNING_OBJECT(
      pool, "Provided size is to small for the caps: %u < %" G_GSIZE_FORMAT,
      size, info.size);
  return FALSE;
}
failed_to_align: {
  GST_WARNING_OBJECT(pool, "Failed to align");
  return FALSE;
}
}

static gsize gst_get_plane_data_size(GstVideoInfo *info,
                                     GstVideoAlignment *align, guint plane) {
  gint padded_height;
  gsize plane_size;

  padded_height = info->height;

  if (align)
    padded_height += align->padding_top + align->padding_bottom;

  padded_height =
      GST_VIDEO_FORMAT_INFO_SCALE_HEIGHT(info->finfo, plane, padded_height);

  plane_size = GST_VIDEO_INFO_PLANE_STRIDE(info, plane) * padded_height;

  return plane_size;
}

static GstFlowReturn
custom_video_buffer_pool_alloc(GstBufferPool *pool, GstBuffer **buffer,
                               GstBufferPoolAcquireParams *params) {
  GstCustomVideoBufferPool *vpool = GST_CUSTOM_VIDEO_BUFFER_POOL_CAST(pool);
  GstCustomVideoBufferPoolPrivate *priv = vpool->priv;
  GstVideoInfo *info;
  GstMemory *mem;

  info = &priv->info;

  GST_DEBUG_OBJECT(pool, "alloc %" G_GSIZE_FORMAT, info->size);
  if (info->finfo->n_planes > 1 && GST_IS_DMABUF_ALLOCATOR(priv->allocator)) {
    GST_INFO_OBJECT(pool,"Using split allocations");
    *buffer = gst_buffer_new();
    for (guint i = 0; i < info->finfo->n_planes; i++) {
      mem = gst_allocator_alloc(
          priv->allocator, gst_get_plane_data_size(info, &priv->video_align, i),
          NULL);
      if(!mem){
        goto no_memory;
      }
      gst_buffer_append_memory(*buffer, mem);
    }
  } else {
    *buffer =
        gst_buffer_new_allocate(priv->allocator, info->size, &priv->params);
  }
  if (*buffer == NULL)
    goto no_memory;

  if (priv->add_videometa) {
    GST_DEBUG_OBJECT(pool, "adding GstVideoMeta");

    gst_buffer_add_video_meta_full(
        *buffer, GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_INFO_FORMAT(info),
        GST_VIDEO_INFO_WIDTH(info), GST_VIDEO_INFO_HEIGHT(info),
        GST_VIDEO_INFO_N_PLANES(info), info->offset, info->stride);
  }

  return GST_FLOW_OK;

  /* ERROR */
no_memory: {
  GST_WARNING_OBJECT(pool, "can't create memory");
  return GST_FLOW_ERROR;
}
}

/**
 * gst_custom_video_buffer_pool_new:
 *
 * Create a new bufferpool that can allocate video frames. This bufferpool
 * supports all the video bufferpool options.
 *
 * Returns: (transfer full): a new #GstBufferPool to allocate video frames
 */
GstBufferPool *gst_custom_video_buffer_pool_new() {
  GstCustomVideoBufferPool *pool;

  pool = (GstCustomVideoBufferPool *)g_object_new(
      GST_TYPE_CUSTOM_VIDEO_BUFFER_POOL, NULL);
  gst_object_ref_sink(pool);

  GST_LOG_OBJECT(pool, "new video buffer pool %p", pool);

  return GST_BUFFER_POOL_CAST(pool);
}

static void
gst_custom_video_buffer_pool_class_init(GstCustomVideoBufferPoolClass *klass) {
  GObjectClass *gobject_class = (GObjectClass *)klass;
  GstBufferPoolClass *gstbufferpool_class = (GstBufferPoolClass *)klass;

  gobject_class->finalize = gst_custom_video_buffer_pool_finalize;

  gstbufferpool_class->get_options = custom_video_buffer_pool_get_options;
  gstbufferpool_class->set_config = custom_video_buffer_pool_set_config;
  gstbufferpool_class->alloc_buffer = custom_video_buffer_pool_alloc;
}

static void gst_custom_video_buffer_pool_init(GstCustomVideoBufferPool *pool) {
  pool->priv = (GstCustomVideoBufferPoolPrivate *)
      gst_custom_video_buffer_pool_get_instance_private(pool);
}

static void gst_custom_video_buffer_pool_finalize(GObject *object) {
  GstCustomVideoBufferPool *pool = GST_CUSTOM_VIDEO_BUFFER_POOL_CAST(object);
  GstCustomVideoBufferPoolPrivate *priv = pool->priv;

  GST_LOG_OBJECT(pool, "finalize video buffer pool %p", pool);

  if (priv->allocator)
    gst_object_unref(priv->allocator);

  G_OBJECT_CLASS(gst_custom_video_buffer_pool_parent_class)->finalize(object);
}
