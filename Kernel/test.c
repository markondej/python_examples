#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Marcin Kondej <markondej@gmail.com>");
MODULE_DESCRIPTION("Kernel test module");

static int __init test_init(void) {
    pr_info("Test module init\n");
    return 0;
}

static void __exit test_exit(void) {
    pr_info("Test module exit\n");
}

module_init(test_init);
module_exit(test_exit);