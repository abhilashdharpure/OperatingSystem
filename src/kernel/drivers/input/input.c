#include "input.h"
#include "time/time.h"
#include "stdint.h"
#include "stdio.h"
#include "debug.h"
#include "hal/file.h"
#include <stdio.h>
#include "stddef.h"
#include <arch/x86_64/irq.h>
#include "arch/x86_64/events/input_event.h"

// extern void boot_mark(int, uint32_t);

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

static void ring_push_locked(struct input_device *dev, const InputEvent *src)
{
    if (dev->count < INPUT_BUFFER_SIZE) {
        InputEvent *ev = &dev->buffer[dev->head];
        ev->time = src->time; ev->type = src->type; ev->code = src->code; ev->value = src->value;
        dev->head = (dev->head + 1) % INPUT_BUFFER_SIZE;
        dev->count++;
    } else {
        dev->dropped++;
    }
}

static void flush_motion_locked(struct input_device *dev)
{
    if (!dev->acc_x && !dev->acc_y) return;
    if (INPUT_BUFFER_SIZE - dev->count < 3) return;              /* no room: keep accumulating */
    InputEvent e = {0};
    e.time.tv_sec = get_system_time();
    if (dev->acc_x) { e.type = EV_REL; e.code = REL_X; e.value = dev->acc_x; ring_push_locked(dev, &e); }
    if (dev->acc_y) { e.type = EV_REL; e.code = REL_Y; e.value = dev->acc_y; ring_push_locked(dev, &e); }
    e.type = EV_SYN; e.code = 0; e.value = 0; ring_push_locked(dev, &e);
    dev->acc_x = dev->acc_y = 0;
}

void input_push_event(struct input_device *dev, InputEvent *inputEvent)
{
    if (!dev || !inputEvent) return;
    unsigned long flags = irq_save_flags_cli();

    if (dev->coalesce) {
        if (inputEvent->type == EV_REL && (inputEvent->code == REL_X || inputEvent->code == REL_Y)) {
            if (inputEvent->code == REL_X) dev->acc_x += inputEvent->value; else dev->acc_y += inputEvent->value;
            irq_restore_flags(flags);
            return;
        }
        if (inputEvent->type == EV_SYN) { irq_restore_flags(flags); return; }     /* a SYN is generated when the motion is flushed */
        flush_motion_locked(dev);                                                 /* a button: keep it ordered after earlier motion */
        ring_push_locked(dev, inputEvent);
        InputEvent syn = {0};
        syn.time = inputEvent->time; syn.type = EV_SYN;
        ring_push_locked(dev, &syn);
    } else {
        ring_push_locked(dev, inputEvent);
    }
    irq_restore_flags(flags);
}

int input_pop_event(struct input_device *dev, InputEvent *out)
{
    if (!dev || !out) return 0;
    unsigned long flags = irq_save_flags_cli();
    if (dev->coalesce) flush_motion_locked(dev);
    if (dev->count == 0) { irq_restore_flags(flags); return 0; }
    *out = dev->buffer[dev->tail];
    dev->tail = (dev->tail + 1) % INPUT_BUFFER_SIZE;
    dev->count--;
    irq_restore_flags(flags);
    return 1;
}

static int input_can_read(struct file *f)
{
    ps2_poll();
    xhci_service();
    struct input_device *d = f->private_data;
    return d->count > 0 || d->acc_x || d->acc_y;
}

static int input_open(struct file *file) 
{
    // Called when /dev/input/event0 is opened
    return 0;
}


static int input_read(struct file *file, void *buf, size_t size)
{
    ps2_poll();
    xhci_service();

    struct input_device *dev = file->private_data;
    if (size < sizeof(InputEvent)) return -22;            /* EINVAL */
    InputEvent *out = buf;
    size_t n = 0;
    while ((n + 1) * sizeof(InputEvent) <= size) {
        InputEvent ev;
        if (!input_pop_event(dev, &ev)) break;
        out[n++] = ev;
    }

    // if (n && dev->name[5] == '1') { static bool t; t = !t; boot_mark(14, t ? 0x00FF00FF : 0x00303030); }   /* event1 read by a consumer */
    return n ? (int)(n * sizeof(InputEvent)) : -11;       /* EAGAIN when empty */
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
    .can_read = input_can_read,   
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

