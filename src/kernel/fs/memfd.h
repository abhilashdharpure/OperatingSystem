#pragma once

struct file_operations memfd_fops = {
    .read = memfd_read,
    .write = memfd_write,
    .lseek = memfd_lseek,
    .truncate = memfd_truncate,
    .mmap = memfd_mmap,   // optional for now
};
