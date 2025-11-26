#include "compositor_helper.h"

wl_resource* get_focused_keyboard(LumaCompositor* compositor)
{
    if (!compositor->focused_surface)
    {
        return nullptr;
    }

    wl_client* focused_client = wl_resource_get_client(compositor->focused_surface);

    wl_resource* keyboard_res;
    wl_list_for_each(keyboard_res, &compositor->seat->keyboards, link)
    {
        if (wl_resource_get_client(keyboard_res) == focused_client)
        {
            return keyboard_res;
        }
    }

    return nullptr;
}

wl_resource* get_focused_pointer(LumaCompositor* compositor)
{
    if (!compositor->pointer_focused_surface)
    {
        return nullptr;
    }

    wl_client* focused_client = wl_resource_get_client(compositor->pointer_focused_surface);

    wl_resource* pointer_res;
    wl_list_for_each(pointer_res, &compositor->seat->pointers, link)
    {
        if (wl_resource_get_client(pointer_res) == focused_client)
        {
            return pointer_res;
        }
    }

    return nullptr;
}
