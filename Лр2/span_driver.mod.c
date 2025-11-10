#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x35dbe77b, "nf_unregister_net_hook" },
	{ 0x2435d559, "strncmp" },
	{ 0x3b9eb4f2, "dev_get_by_name" },
	{ 0xee761a2b, "skb_copy" },
	{ 0x223d0dba, "__dev_queue_xmit" },
	{ 0x1b3db703, "param_ops_charp" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x340bcc69, "init_net" },
	{ 0xe7d4a967, "nf_register_net_hook" },
	{ 0x70eca2ca, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0x35dbe77b,
	0x2435d559,
	0x3b9eb4f2,
	0xee761a2b,
	0x223d0dba,
	0x1b3db703,
	0xd272d446,
	0xe8213e80,
	0x340bcc69,
	0xe7d4a967,
	0x70eca2ca,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__x86_return_thunk\0"
	"nf_unregister_net_hook\0"
	"strncmp\0"
	"dev_get_by_name\0"
	"skb_copy\0"
	"__dev_queue_xmit\0"
	"param_ops_charp\0"
	"__fentry__\0"
	"_printk\0"
	"init_net\0"
	"nf_register_net_hook\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "58EAEEEB409A0F19BDEFCD7");
