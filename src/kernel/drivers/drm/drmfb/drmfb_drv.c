// /* drmfb_drv.c
//  * Top-level driver registration and bind/unbind using component model.
//  * This code follows the typical drm_dev_alloc -> drm_mode_config_init -> drm_dev_register
//  */

// #include "drmfb_internal.h"

// /* driver info */
// static const struct drm_driver drmfb_drm_driver = {
//     .driver_features = DRIVER_MODESET | DRIVER_GEM,
//     .release = drm_dev_put,
//     .name = DRMFB_NAME,
//     .desc = "drmfb - framebuffer-backed DRM driver",
//     .date = "20251129",
//     .major = 1,
//     .minor = 0,
// };

// /* forward */
// static int drmfb_bind(struct device *dev, struct device *master, void *data);
// static void drmfb_unbind(struct device *dev, struct device *master, void *data);

// static const struct component_ops drmfb_component_ops = {
//     .bind = drmfb_bind,
//     .unbind = drmfb_unbind,
// };

// /* Simple platform probe/register to add component */
// static int drmfb_probe(struct platform_device *pdev)
// {
//     return component_add(&pdev->dev, &drmfb_component_ops);
// }

// static int drmfb_remove(struct platform_device *pdev)
// {
//     component_del(&pdev->dev, &drmfb_component_ops);
//     return 0;
// }

// /* Platform driver skeleton - register platform device in board code or DT */
// static struct platform_driver drmfb_platform_driver = {
//     .probe = drmfb_probe,
//     .remove = drmfb_remove,
//     .driver = {
//         .name = "drmfb",
//         .owner = THIS_MODULE,
//     },
// };

// module_platform_driver(drmfb_platform_driver);

// /* --- component bind/unbind --- */
// static int drmfb_bind(struct device *dev, struct device *master, void *data)
// {
//     struct drmfb_device *drmfb;
//     struct drm_device *drm;
//     int ret;

//     drmfb = kzalloc(sizeof(*drmfb), GFP_KERNEL);
//     if (!drmfb)
//         return -ENOMEM;
//     drmfb->dev = dev;

//     /* Allocate drm_device */
//     drm = drm_dev_alloc(&drmfb_drm_driver, dev);
//     if (IS_ERR(drm)) {
//         kfree(drmfb);
//         return PTR_ERR(drm);
//     }
//     drmfb->drm = drm;

//     /* initialize mode config limits */
//     drm->mode_config.min_width = 1;
//     drm->mode_config.min_height = 1;
//     drm->mode_config.max_width = fb_dev.width;
//     drm->mode_config.max_height = fb_dev.height;
//     drm->mode_config.funcs = NULL; /* we use helper funcs in drmfb_kms.c */
//     drm_mode_config_init(drm);

//     mutex_init(&drmfb->gem_lock);

//     /* ioremap the physical framebuffer so we can write it from kernel */
//     drmfb->fb_size = (size_t)fb_dev.pitch * fb_dev.height;
//     drmfb->fb_virt = ioremap((phys_addr_t)fb_dev.framebuffer, drmfb->fb_size);
//     if (!drmfb->fb_virt) {
//         dev_err(dev, "drmfb: ioremap failed for fb addr %pa\n", &fb_dev.framebuffer);
//         ret = -ENOMEM;
//         goto err_put;
//     }

//     /* store drmfb pointer in drm->dev_private so other files can find it */
//     drm->dev_private = drmfb;

//     /* register device node */
//     ret = drm_dev_register(drm, 0);
//     if (ret) {
//         dev_err(dev, "drm_dev_register failed: %d\n", ret);
//         goto err_iounmap;
//     }

//     platform_set_drvdata(to_platform_device(dev), drmfb);

//     dev_info(dev, "drmfb: bound and registered as /dev/dri/cardN\n");
//     return 0;

// err_iounmap:
//     iounmap(drmfb->fb_virt);
// err_put:
//     drm_dev_put(drm);
//     kfree(drmfb);
//     return ret;
// }

// static void drmfb_unbind(struct device *dev, struct device *master, void *data)
// {
//     struct drmfb_device *drmfb = platform_get_drvdata(to_platform_device(dev));
//     struct drm_device *drm = drmfb->drm;

//     drm_dev_unregister(drm);
//     iounmap(drmfb->fb_virt);
//     drm_dev_put(drm);
//     kfree(drmfb);
// }
// MODULE_LICENSE("GPL");
