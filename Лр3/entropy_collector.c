#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/time.h>
#include <linux/random.h>

static void entropy_handler_func(struct input_handle *handle, unsigned int type, unsigned int code, int value)
{
    unsigned long entropy;

    // 1. Положение мыши
    if ((type == EV_REL || type == EV_ABS) && (code == ABS_X || code == ABS_Y)) {
        entropy = jiffies ^ value;
        add_device_randomness(&entropy, sizeof(entropy));

        printk(KERN_INFO "Entropy added from mouse: %lu (jiffies: %lu, value: %d)\n", entropy, jiffies, value);
    }

    // 2. Нажатие клавиш
    if (type == EV_KEY) {
        entropy = jiffies ^ value;
        add_device_randomness(&entropy, sizeof(entropy));

        printk(KERN_INFO "Entropy added from keyboard: %lu (jiffies: %lu, value: %d)\n", entropy, jiffies, value);
    }
}

static int input_connect(struct input_handler *handler, struct input_dev *dev, const struct input_device_id *id)
{
    struct input_handle *handle;
    handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
    if (!handle) return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "simple_entropy";

    if (input_register_handle(handle) || input_open_device(handle)) {
        input_unregister_handle(handle);
        kfree(handle);
        return -EIO;
    }

    printk(KERN_INFO "Connected to input device: %s\n", dev->name);
    return 0;
}

static void input_disconnect(struct input_handle *handle)
{
    input_close_device(handle);
    input_unregister_handle(handle);
    kfree(handle);

    printk(KERN_INFO "Disconnected from input device\n");
}

static const struct input_device_id entropy_ids[] = { { .driver_info = 1 }, { } };

static struct input_handler entropy_handler = {
    .event     = entropy_handler_func,
    .connect   = input_connect,
    .disconnect= input_disconnect,
    .name      = "simple_entropy",
    .id_table  = entropy_ids,
};

static int __init init_func(void)
{
    printk(KERN_INFO "Entropy collector driver loaded\n");
    return input_register_handler(&entropy_handler);
}

static void __exit exit_func(void)
{
    printk(KERN_INFO "Entropy collector driver unloaded\n");
    input_unregister_handler(&entropy_handler);
}

module_init(init_func);
module_exit(exit_func);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vohminova Diana, 410B-22");

/*
make clean
make
sudo insmod entropy_collector.ko
lsmod | grep entropy_collector
sudo dmesg | tail -5
./keypress
cat /tmp/entropy
sudo rmmod entropy_collector
*/