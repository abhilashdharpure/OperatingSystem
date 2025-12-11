// /* drmfb_gem.c - simple dumb buffer + gem backing using alloc_pages */
// #include "drmfb_internal.h"
// #include <drm/drm_ioctl.h>
// #include <drm/drm_modes.h>
// #include <drm/drm_gem.h>
// #include <drm/drm_gem_cma_helper.h>

// /* Create dumb: allocate pages and attach to drm_gem_object */
// int drmfb_gem_create_dumb(struct drm_device *dev,
//                           struct drm_file *file_priv,
//                           struct drm_mode_create_dumb *args)
// {
//     struct drm_gem_object *obj;
//     size_t size = args->height * args->pitch;
//     unsigned int order;
//     struct page *pages;
//     void *vaddr;

//     /* round up to page size */
//     size = PAGE_ALIGN(size);

//     /* allocate kernel pages for buffer - for large buffers, alloc_pages() might be better */
//     pages = alloc_pages(GFP_KERNEL | __GFP_ZERO, get_order(size));
//     if (!pages)
//         return -ENOMEM;

//     /* create gem object with the pages as backing */
//     obj = drm_gem_object_alloc(dev, size);
//     if (!obj) {
//         __free_pages(pages, get_order(size));
//         return -ENOMEM;
//     }

//     /* store pages in obj->import_attach or custom field per your design.
//        For simplicity we store virtual mapping in obj->driver_private (NOT ideal) */
//     vaddr = kmap(pages);
//     if (!vaddr) {
//         drm_gem_object_release(obj);
//         __free_pages(pages, get_order(size));
//         return -ENOMEM;
//     }
//     obj->driver_private = vaddr;

//     args->handle = drm_gem_handle_create(file_priv, obj, &dev->object_name_idr);
//     drm_gem_object_unreference_unlocked(obj);

//     return 0;
// }

// /* Map dumb to userspace - support mmap */
// int drmfb_gem_mmap(struct drm_gem_object *obj,
//                    struct vm_area_struct *vma)
// {
//     void *kaddr = obj->driver_private;
//     unsigned long size = vma->vm_end - vma->vm_start;
//     if (size > obj->size) return -EINVAL;

//     /* map kernel memory to userspace - kernel-specific path required (remap_pfn_range etc.) */
//     return remap_pfn_range(vma, vma->vm_start, virt_to_phys(kaddr) >> PAGE_SHIFT, size, vma->vm_page_prot);
// }

// /* Free gem object */
// void drmfb_gem_free_object(struct drm_gem_object *obj)
// {
//     void *kaddr = obj->driver_private;
//     if (kaddr) {
//         kunmap((struct page *)kaddr);
//         /* free pages: find order from size */
//         __free_pages(virt_to_page(kaddr), get_order(obj->size));
//     }
//     drm_gem_object_release(obj);
// }

// /* Hook these into the DRM driver's GEM ops / ioctl handlers as needed.
//    For example, implement drm_ioctl functions and use drm_gem_handle_create,
//    drm_gem_object_init, etc. The exact glue depends on your DRM core version. */
