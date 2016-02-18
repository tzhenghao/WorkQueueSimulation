#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/param.h>
#include <linux/stat.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

#define MSECS 500 /* 500 ms */

/* Prototypes */
static void thread_task_handler(struct work_struct *work);
ssize_t device_show(struct device *dev, char *buf);
ssize_t device_store(struct device *dev, char *buf, size_t count);
static int __init init_driver(void);
static void __exit deinit_driver(void);

static DEVICE_ATTR(code_test_workqueue, S_IWUSR | S_IRUGO, 
					device_show, device_store);

static DECLARE_WORK(print_task, thread_task_handler);

static struct timer_list timeList; /* timer functionality */

struct device myDevice;

/* Timer ISR */
static void print_task_handler(int num) {
	static int count = 0;
	schedule_work(&print_task);
	printk("<1> code_test_workqueue: counter:%d\n", count++);
}

/* Timer registration */
static void register_timer(struct timer_list * timeList, unsigned int timeover) {

	init_timer(timeList);
	timeList->data = timeover;
	timeList->expires =  jiffies + (MSECS * HZ / 1000);
	timeList->function = (void *)print_task_handler;
	add_timer(timeList);
}

static void thread_task_handler(struct work_struct *work) {
	register_timer(&timeList, 3);
}

/* Init function */
static int __init init_driver(void) {

	printk("<1> Initializing driver...\n");

	int retval = -1;

	// retval = device_create_file(&myDevice, &dev_attr_code_test_workqueue);
	// if (retval) {
	// 	printk("<1> Unable to create device file\n");
	// 	goto fail;
	// }

	register_timer(&timeList, 3);
	
	return 0;

	fail:
		deinit_driver();
		return retval;
}

/* Exit function */
static void __exit deinit_driver(void) {
	
	del_timer_sync(&timeList);

	printk("<1> Deinitializing driver...\n");

	// device_remove_file(&myDevice, &dev_attr_code_test_workqueue);
}

ssize_t device_show(struct device *dev, char *buf) {

	return scnprintf(buf, PAGE_SIZE, "%s\n");
}

ssize_t device_store(struct device *dev, char *buf, size_t count) {

	static uint8_t toggle = 0;
	
	if (toggle) {
		del_timer_sync(&timeList);
		toggle = 0;
	}
	else {
		register_timer(&timeList, 3);
		toggle = 1;
	}

	return count;
}

module_init(init_driver);
module_exit(deinit_driver);