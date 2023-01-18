#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("GPLv3");

static int hello_world_init(void)
{
    printk(KERN_ALERT "Hello, world! This is My First Kernel Module(TM) by Playskool(TM)\n");
    return 0;
}

static void hello_world_exit(void)
{
    printk(KERN_ALERT "Goodbye, world! Remember to tip your waitresses\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);
