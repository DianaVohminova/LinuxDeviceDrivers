// entropy_collector.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/random.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/time.h>

static unsigned long last_keypress_jiffies = 0;

// Обработчик событий ввода
static void entropy_handler_func(struct input_handle *handle,
                                unsigned int type,
                                unsigned int code,
                                int value)
{
    unsigned long entropy;

    // 1. Положение мыши
    if ((type == EV_REL || type == EV_ABS) &&
        (code == ABS_X || code == ABS_Y || code == REL_X || code == REL_Y)) {
        entropy = jiffies ^ value;
        add_device_randomness(&entropy, sizeof(entropy));
        printk(KERN_INFO "Entropy from mouse: %lu (jiffies: %lu, value: %d)\n",
               entropy, jiffies, value);
    }

    // 2. Нажатие/отпускание клавиш + интервалы между нажатиями
    if (type == EV_KEY && (value == 0 || value == 1)) {
        unsigned long now = jiffies;
        entropy = now ^ value;
        add_device_randomness(&entropy, sizeof(entropy));

        // Интервал между нажатиями (только при нажатии)
        if (value == 1 && last_keypress_jiffies != 0) {
            unsigned long interval = now - last_keypress_jiffies;
            add_device_randomness(&interval, sizeof(interval));
            printk(KERN_INFO "Keypress interval entropy: %lu ticks\n", interval);
        }
        if (value == 1) {
            last_keypress_jiffies = now;
        }

        printk(KERN_INFO "Entropy from keyboard: %lu (jiffies: %lu, value: %d)\n",
               entropy, jiffies, value);
    }
}

// Подключение к устройствам ввода
static int input_connect(struct input_handler *handler,
                         struct input_dev *dev,
                         const struct input_device_id *id)
{
    struct input_handle *handle;
    handle = kzalloc(sizeof(*handle), GFP_KERNEL);
    if (!handle)
        return -ENOMEM;

    handle->dev = dev;
    handle->handler = handler;
    handle->name = "entropy_collector";

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

// Поддержка всех устройств ввода
static const struct input_device_id entropy_ids[] = {
    {
        .flags = INPUT_DEVICE_ID_MATCH_EVBIT,
        .evbit = { BIT_MASK(EV_KEY) | BIT_MASK(EV_REL) | BIT_MASK(EV_ABS) },
    },
    { } // завершающий элемент
};

static struct input_handler entropy_handler = {
    .event      = entropy_handler_func,
    .connect    = input_connect,
    .disconnect = input_disconnect,
    .name       = "entropy_collector",
    .id_table   = entropy_ids,
};

// Сетевой хук для ICMP-пакетов
static unsigned int net_entropy_hook(void *priv,
                                     struct sk_buff *skb,
                                     const struct nf_hook_state *state)
{
    if (!skb || !skb->data)
        return NF_ACCEPT;

    struct iphdr *iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_ICMP)
        return NF_ACCEPT;

    struct icmphdr *icmph = (struct icmphdr *)(skb->data + (iph->ihl * 4));
    if (!icmph)
        return NF_ACCEPT;

    unsigned long entropy = jiffies ^ (unsigned long)ntohl(iph->saddr) ^ icmph->type;
    add_device_randomness(&entropy, sizeof(entropy));
    printk(KERN_INFO "Network entropy added (ICMP from %pI4, type=%u): %lu\n",
           &iph->saddr, icmph->type, entropy);

    return NF_ACCEPT;
}

static struct nf_hook_ops nfho = {
    .hook     = net_entropy_hook,
    .pf       = PF_INET,
    .hooknum  = NF_INET_PRE_ROUTING,
    .priority = NF_IP_PRI_FIRST,
};

// Инициализация
static int __init init_func(void)
{
    int ret;
    ret = input_register_handler(&entropy_handler);
    if (ret) {
        printk(KERN_ERR "Failed to register input handler\n");
        return ret;
    }

    ret = nf_register_net_hook(&init_net, &nfho);
    if (ret) {
        printk(KERN_ERR "Failed to register netfilter hook\n");
        input_unregister_handler(&entropy_handler);
        return ret;
    }

    printk(KERN_INFO "Entropy collector driver loaded (with network support)\n");
    return 0;
}

// Выгрузка
static void __exit exit_func(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
    input_unregister_handler(&entropy_handler);
    printk(KERN_INFO "Entropy collector driver unloaded\n");
}

module_init(init_func);
module_exit(exit_func);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vohminova Diana, 410B-22");
MODULE_DESCRIPTION("Entropy collector from mouse, keyboard, and ICMP packets");