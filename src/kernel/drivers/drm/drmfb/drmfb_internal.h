// /* drmfb_internal.h - internal defs for drmfb driver */
// #ifndef DRMFB_INTERNAL_H
// #define DRMFB_INTERNAL_H

// #include <linux/module.h>
// #include <linux/platform_device.h>
// #include <linux/io.h>
// #include <linux/slab.h>
// #include <linux/mm.h>
// #include <linux/highmem.h>
// #include <linux/mutex.h>

// #include <drm/drm_drv.h>
// #include <drm/drm_plane.h>
// #include <drm/drm_crtc.h>
// #include <drm/drm_gem.h>
// #include <drm/drm_modeset_helper_vtables.h>
// #include <drm/drm_connector.h>
// #include <drm/drm_atomic.h>

// /* Forward declarations for your kernel's fb device structure.
//    If fb_dev is declared elsewhere, include the header that defines it.
//    For now we forward-declare fields we use. */
// struct fb_dev {
//     unsigned int width;
//     unsigned int height;
//     unsigned int pitch;
//     unsigned int bpp;
//     phys_addr_t framebuffer; /* physical address provided by bootloader */
// };

// /* External symbol: your kernel already sets this; driver will reference it.
//    If the symbol name differs, update the extern below to match your kernel. */
// extern struct fb_dev fb_dev;

// /* Driver private */
// struct drmfb_device {
//     struct drm_device *drm;
//     struct device *dev;

//     /* KMS objects */
//     struct drm_plane primary_plane;
//     struct drm_crtc crtc;
//     struct drm_connector connector;

//     /* framebuffer mapping (ioremap of fb_dev.framebuffer) */
//     void __iomem *fb_virt;
//     size_t fb_size;

//     /* GEM/dumb buffer bookkeeping */
//     struct mutex gem_lock;
//     /* (optional) list of allocated gem objects, etc. */

//     /* basic state */
//     bool enabled;
// };

// #define DRMFB_NAME "drmfb"

// #endif /* DRMFB_INTERNAL_H */
