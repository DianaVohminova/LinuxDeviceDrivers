// span_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/etherdevice.h>

static struct nf_hook_ops nfho_in;
static struct nf_hook_ops nfho_out;

static char *src_if = "eth0";     // Интерфейс, с которого зеркалим
static char *dst_if = "tap0";     // Интерфейс, на который зеркалим
module_param(src_if, charp, 0644);
module_param(dst_if, charp, 0644);

// Функция для отправки копии skb на другой интерфейс
static void mirror_packet(struct sk_buff *skb, const char *out_ifname)
{
    struct sk_buff *nskb;
    struct net_device *dev;

    dev = dev_get_by_name(&init_net, out_ifname);
    if (!dev) {
        printk(KERN_ERR "SPAN: destination interface %s not found\n", out_ifname);
        return;
    }

    // Создаём копию skb
    nskb = skb_copy(skb, GFP_ATOMIC);
    if (!nskb) {
        dev_put(dev);
        return;
    }

    // Подготавливаем для отправки
    nskb->dev = dev;
    nskb->protocol = htons(ETH_P_IP);
    nskb->pkt_type = PACKET_OUTGOING;
    skb_reset_network_header(nskb);

    // Отправляем
    dev_queue_xmit(nskb);
    dev_put(dev);
}

// Хук для входящих пакетов
static unsigned int hook_in(void *priv, struct sk_buff *skb,
                           const struct nf_hook_state *state)
{
    if (strncmp(skb->dev->name, src_if, IFNAMSIZ) == 0) {
        mirror_packet(skb, dst_if);
    }
    return NF_ACCEPT;
}

// Хук для исходящих пакетов
static unsigned int hook_out(void *priv, struct sk_buff *skb,
                            const struct nf_hook_state *state)
{
    if (strncmp(skb->dev->name, src_if, IFNAMSIZ) == 0) {
        mirror_packet(skb, dst_if);
    }
    return NF_ACCEPT;
}

static int __init span_init(void)
{
    printk(KERN_INFO "SPAN driver loaded. Mirroring %s → %s\n", src_if, dst_if);

    nfho_in.hook     = hook_in;
    nfho_in.hooknum  = NF_INET_PRE_ROUTING;
    nfho_in.pf       = PF_INET;
    nfho_in.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho_in);

    nfho_out.hook    = hook_out;
    nfho_out.hooknum = NF_INET_POST_ROUTING;
    nfho_out.pf      = PF_INET;
    nfho_out.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho_out);

    return 0;
}

static void __exit span_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho_in);
    nf_unregister_net_hook(&init_net, &nfho_out);
    printk(KERN_INFO "SPAN driver unloaded.\n");
}

module_init(span_init);
module_exit(span_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SPAN-like packet mirroring driver");
MODULE_AUTHOR("Vohminova Diana, 410B-22");