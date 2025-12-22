#include "input.h"
#include "time/time.h"
#include "stdint.h"
#include "stdio.h"
#include "debug.h"
#include "hal/file.h"
#include <stdio.h>
#include "stddef.h"
#include <arch/x86_64/irq.h>

static inline unsigned long irq_save_flags_cli(void)
{
    unsigned long flags;
    __asm__ volatile("pushf; pop %0; cli" : "=g"(flags) : : "memory");
    return flags;
}

static inline void irq_restore_flags(unsigned long flags)
{
    __asm__ volatile("push %0; popf" : : "g"(flags) : "memory");
}

void input_push_event(struct input_device *dev, InputEvent* inputEvent)
{
    if (!dev || !inputEvent)
    {
        return;
    }

    // Lock
    unsigned long flags = irq_save_flags_cli();

    if (dev->count < INPUT_BUFFER_SIZE) {
        InputEvent *ev = &dev->buffer[dev->head];
        ev->time = inputEvent->time;
        ev->type = inputEvent->type;
        ev->code = inputEvent->code;
        ev->value = inputEvent->value;

        dev->head = (dev->head + 1) % INPUT_BUFFER_SIZE;
        dev->count++;
    }

    //Unlock
    irq_restore_flags(flags);
}

int input_pop_event(struct input_device *dev, InputEvent *out)
{
    if (!dev || !out)
    {
        return 0;
    }

    // Lock
    unsigned long flags = irq_save_flags_cli();

    if (dev->count == 0)
    {
        irq_restore_flags(flags);
        return 0; // no event available
    }
        
    *out = dev->buffer[dev->tail];
    dev->tail = (dev->tail + 1) % INPUT_BUFFER_SIZE;
    dev->count--;

    //Unlock
    irq_restore_flags(flags);
    return 1;
}

static int input_open(struct file *file) 
{
    // Called when /dev/input/event0 is opened
    return 0;
}

static int input_read(struct file *file, void *buf, size_t size)
{
    struct input_device *dev = (struct input_device *)file->private_data;

    if (dev->count == 0)
        return 0; // no events available

    if (size < sizeof(InputEvent))
        return -1;

    // InputEvent *src = &dev->buffer[dev->tail];
    // memcpy(buf, src, sizeof(InputEvent));

    // dev->tail = (dev->tail + 1) % INPUT_BUFFER_SIZE;
    // dev->count--;

    // return sizeof(InputEvent);

    InputEvent *out = (InputEvent *)buf;
    int events_read = 0;

    while (events_read * sizeof(InputEvent) < size)
    {
        InputEvent ev;
        if (!input_pop_event(dev, &ev))
            break; // no more events available

        out[events_read++] = ev;
    }

    return events_read * sizeof(InputEvent);
}

static int input_write(struct file *file, const void *buf, size_t size)
{
    // Optional: handle force feedback, LED indicators, etc.
    return size;
}

struct file_operations input_fops = {
    .open  = input_open,
    .read  = input_read,
    .write = input_write,
};

void register_input_device(struct input_device *dev, const char *name)
{
    strncpy(dev->name, name, sizeof(dev->name));
    dev->head = dev->tail = dev->count = 0;

    char path[64];
    ksnprintf(path, sizeof(path), "/dev/input/%s", name);
    VFS_RegisterDevice(path, &input_fops, dev);
}

void debug_poll_input(struct input_device *dev)
{
    InputEvent ev;
    while (input_pop_event(dev, &ev))
    {
        log_info("Input", "Event: type=%u, code=%u, value=%d, time=%llu\n",
                 ev.type, ev.code, ev.value, ev.time);
    }
}

// TODO: For /dev/null
static int null_open(struct file *file) { return 0; }
static int null_read(struct file *file, void *buf, size_t size) { return 0; }
static int null_write(struct file *file, const void *buf, size_t size) { return size; }

static struct file_operations null_fops = {
    .open = null_open,
    .read = null_read,
    .write = null_write,
};

void dev_null_init()
{
    VFS_RegisterDevice("/dev/null", &null_fops, NULL);
}

void InitializeDevNull()
{
    dev_null_init();

    int fd = VFS_Open("/dev/null", 0);
    const char *msg = "Hello OS!";
    // VFS_Write(fd, msg, strlen(msg));
    VFS_Write(fd, (uint8_t*)msg, strlen(msg));
}

