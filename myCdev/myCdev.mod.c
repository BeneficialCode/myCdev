#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7d09bae2, "module_layout" },
	{ 0xbe6a2279, "cdev_del" },
	{ 0xa3247381, "per_cpu__current_task" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x4f0b454f, "cdev_init" },
	{ 0x6980fe91, "param_get_int" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x2bb6fde2, "__kfifo_put" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0x45d11c43, "down_interruptible" },
	{ 0xef97c22d, "remove_proc_entry" },
	{ 0xcf20bd97, "device_destroy" },
	{ 0x6729d3df, "__get_user_4" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xff964b25, "param_set_int" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x3c2c5af5, "sprintf" },
	{ 0xffc7c184, "__init_waitqueue_head" },
	{ 0x3da5eb6d, "kfifo_alloc" },
	{ 0xb72397d5, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xb4390f9a, "mcount" },
	{ 0xb4ca9447, "__kfifo_get" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0xfda5d96e, "device_create" },
	{ 0x6087741d, "cdev_add" },
	{ 0x4292364c, "schedule" },
	{ 0x7c866d11, "create_proc_entry" },
	{ 0x642e54ac, "__wake_up" },
	{ 0x37a0cba, "kfree" },
	{ 0x33d92f9a, "prepare_to_wait" },
	{ 0x3f1899f1, "up" },
	{ 0xa8f3ff3d, "class_destroy" },
	{ 0x9ccb2622, "finish_wait" },
	{ 0xdecb46af, "__class_create" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x15ef2dd9, "kfifo_free" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "FE3D7A00F4E6350F5AD44E0");
