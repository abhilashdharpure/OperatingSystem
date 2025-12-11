// /* drmfb_kms.c - minimal kms setup for drmfb */

// #include "drmfb_internal.h"

// /* Helper prototypes */
// static int drmfb_plane_atomic_check(struct drm_plane *plane, struct drm_atomic_state *state);
// static void drmfb_plane_atomic_update(struct drm_plane *plane, struct drm_atomic_state *state);
// static const struct drm_plane_helper_funcs drmfb_plane_helper_funcs = {
//     .atomic_check = drmfb_plane_atomic_check,
//     .atomic_update = drmfb_plane_atomic_update,
// };
// static const struct drm_plane_funcs drmfb_plane_funcs = {
//     .destroy = drm_plane_cleanup,
// };

// static const struct drm_crtc_helper_funcs drmfb_crtc_helper_funcs = {
//     .atomic_check = drm_atomic_helper_crtc_check,
//     .atomic_commit = drm_atomic_helper_crtc_commit,
// };

// static const struct drm_crtc_funcs drmfb_crtc_funcs = {
//     .destroy = drm_crtc_cleanup,
// };

// /* connector impl */
// static enum drm_connector_status drmfb_connector_detect(struct drm_connector *connector, bool force)
// {
//     return connector->status = connector_status_connected;
// }

// static void drmfb_connector_fill_modes(struct drm_connector *connector, void *data)
// {
//     struct drmfb_device *drmfb = connector->dev->dev_private;
//     struct drm_display_mode *mode;

//     mode = drm_mode_create(connector->dev);
//     if (!mode) return;

//     mode->hdisplay = fb_dev.width;
//     mode->vdisplay = fb_dev.height;
//     mode->vrefresh = 60;
//     mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
//     drm_mode_set_name(mode);
//     drm_mode_probed_add(connector, mode);
// }

// static const struct drm_connector_funcs drmfb_connector_funcs = {
//     .detect = drmfb_connector_detect,
//     .fill_modes = drmfb_connector_fill_modes,
//     .destroy = drm_connector_cleanup,
// };

// /* Atomic plane functions - VERY simple: we accept XRGB8888 and copy on commit */
// static int drmfb_plane_atomic_check(struct drm_plane *plane, struct drm_atomic_state *state)
// {
//     /* TODO: validate format, src/dst sizes etc. For now accept basic case */
//     return 0;
// }

// static void drmfb_plane_atomic_update(struct drm_plane *plane, struct drm_atomic_state *state)
// {
//     struct drm_fb_helper_surface_size size;
//     struct drm_plane_state *new_state;
//     struct drmfb_device *drmfb = plane->dev->dev_private;
//     struct drm_gem_object *gem;
//     void *obj_vaddr = NULL;
//     unsigned int i;

//     new_state = drm_atomic_get_new_plane_state(state, plane);
//     if (!new_state || !new_state->fb)
//         return;

//     /* get backing vaddr from GEM (we allocate using our simple kernel allocator) */
//     gem = drm_gem_fb_get_obj(new_state->fb, 0);
//     if (!gem) return;

//     /* Map gem pages into kernel (simple approach: use drm_gem_vmap if available) */
// #ifdef CONFIG_DRM_GEM_VMAP
//     obj_vaddr = drm_gem_vmap(gem);
// #else
//     /* fallback: if gem->page_count exists, map pages individually */
//     /* TODO: implement if needed for your kernel */
// #endif

//     if (!obj_vaddr) {
//         pr_warn("drmfb: can't vmap gem\n");
//         return;
//     }

//     /* Copy the framebuffer content into real fb mem (ioremap'd) */
//     {
//         struct drmfb_device *d = drmfb;
//         size_t line_bytes = fb_dev.width * (fb_dev.bpp / 8);
//         void __iomem *dst = d->fb_virt;
//         unsigned int y;
//         for (y = 0; y < fb_dev.height; ++y) {
//             void *srow = obj_vaddr + y * new_state->fb->pitches[0];
//             void __iomem *drow = dst + y * fb_dev.pitch;
//             memcpy_toio(drow, srow, line_bytes);
//         }
//     }

// #ifdef CONFIG_DRM_GEM_VMAP
//     drm_gem_vunmap(gem, obj_vaddr);
// #endif
// }

// /* simple drmfb init of plane/crtc/connector */
// int drmfb_kms_init(struct drm_device *dev)
// {
//     struct drmfb_device *drmfb = dev->dev_private;
//     int ret;

//     /* primary plane */
//     ret = drm_universal_plane_init(dev, &drmfb->primary_plane, 0,
//                                    &drmfb_plane_funcs, NULL, 0, NULL, DRM_PLANE_TYPE_PRIMARY, NULL);
//     if (ret) return ret;
//     drm_plane_helper_add(&drmfb->primary_plane, &drmfb_plane_helper_funcs);

//     /* crtc init */
//     ret = drm_crtc_init_with_planes(dev, &drmfb->crtc, &drmfb->primary_plane, NULL, &drmfb_crtc_funcs, NULL);
//     if (ret) return ret;
//     drm_crtc_helper_add(&drmfb->crtc, &drmfb_crtc_helper_funcs);

//     /* connector */
//     ret = drm_connector_init(dev, &drmfb->connector, &drmfb_connector_funcs, DRM_MODE_CONNECTOR_Unknown);
//     if (ret) return ret;

//     drm_connector_helper_add(&drmfb->connector, &drm_simple_connector_helper_funcs);

//     /* add one mode representing current fb_dev resolution */
//     {
//         struct drm_display_mode *mode = drm_mode_create(dev);
//         if (!mode) return -ENOMEM;
//         mode->hdisplay = fb_dev.width;
//         mode->vdisplay = fb_dev.height;
//         mode->vrefresh = 60;
//         mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
//         drm_mode_set_name(mode);
//         drm_mode_probed_add(&drmfb->connector, mode);
//     }

//     /* match connector -> crtc (simple) */
//     drm_mode_connector_attach_encoder(&drmfb->connector, drm_encoder_create(dev, NULL, DRM_MODE_ENCODER_NONE));
//     return 0;
// }

// void drmfb_kms_fini(struct drm_device *dev)
// {
//     struct drmfb_device *drmfb = dev->dev_private;
//     drm_connector_cleanup(&drmfb->connector);
//     drm_crtc_cleanup(&drmfb->crtc);
//     drm_plane_cleanup(&drmfb->primary_plane);
// }
