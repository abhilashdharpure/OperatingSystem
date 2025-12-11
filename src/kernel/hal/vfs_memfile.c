// #include <stdint.h>
// #include <stddef.h>
// #include <string.h>
// #include "vfs_memfile.h"
// #include "hal/file.h"

// /* open: called when open() is issued */
// static int memfile_open(struct file *file) {
//     /* nothing special to do */
//     return 0;
// }

// /* read: copy from memfile->data + file->position */
// static int memfile_read(struct file *file, void *buf, size_t size) {
//     if (!file || !file->private_data) return -1;
//     memfile_t *mf = (memfile_t*) file->private_data;

//     if (file->position >= mf->size) return 0; /* EOF */

//     size_t remain = mf->size - file->position;
//     size_t tocopy = (size < remain) ? size : remain;

//     memcpy(buf, mf->data + file->position, tocopy);
//     file->position += tocopy;
//     return (int)tocopy;
// }

// /* close: optional cleanup (we won't free memfile here if we want to keep file persistent) */
// static int memfile_close(struct file *file) {
//     /* do NOT free mf here if the memfile is shared. If you allocated per-open, free here. */
//     return 0;
// }

// /* file ops structure */
// struct file_operations memfile_fops = {
//     .open  = memfile_open,
//     .read  = memfile_read,
//     .write = NULL,
//     .close = memfile_close,
// };
