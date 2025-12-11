#include "memfile.h"
#include "hal/vfs.h"
#include "kmalloc.h"
#include "string.h"

static int memfile_open(void *handle)
{
    // nothing to do
    return 0;
}

static int memfile_read(void *handle, void *buffer, uint32_t size)
{
    memfile_handle_t *h = (memfile_handle_t *)handle;
    uint32_t remaining = h->mf->size - h->offset;
    uint32_t chunk = (size < remaining) ? size : remaining;

    memcpy(buffer, h->mf->data + h->offset, chunk);
    h->offset += chunk;
    return chunk;
}

static int memfile_close(void *handle)
{
    return 0;
}

struct file_operations memfile_fops = {
    .open  = memfile_open,
    .read  = memfile_read,
    .write = NULL,
    .close = memfile_close,
};
