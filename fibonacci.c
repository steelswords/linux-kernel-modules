#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tristan Andrus");
MODULE_DESCRIPTION("A demonstration character device driver");
MODULE_VERSION("0.1.0");

static int majorNumber;
static dev_t deviceNumber;
static struct class *deviceClass = NULL;
static struct device *fibonacciDevice = NULL;

#define MAX_LENGTH 22 // Length of 18446744073709551615, plus 1 for null and 1 for newline
static char *messageBuffer;
static char simpleMessage[] = "Hello, character device world!";
static int currentNumber = 1;
static int previousNumber = 0;

static ssize_t fib_read(struct file* filep, char __user *buffer, size_t length, loff_t* offset)
{
    printk(KERN_INFO "Fibonacci: read() length = %d, offset = %d\n", length, *offset);

    // If the offset is already past where we're meant to transfer, return 0 to signal
    // the end of file.
    if (*offset >= MAX_LENGTH)
    {
        return 0;
    }

    // Copy our messageBuffer out to userland.
    int copyResult;
    copyResult = copy_to_user(buffer, messageBuffer, MAX_LENGTH);
    //copyResult = copy_to_user(buffer, simpleMessage, strlen(simpleMessage));
    

    if (0 == copyResult)
    {
        printk(KERN_INFO "Read successfully. %s\n", messageBuffer);
        //*offset += strlen(simpleMessage);
        //return strlen(simpleMessage);
        *offset += MAX_LENGTH;
        return MAX_LENGTH;
    }

    printk(KERN_INFO "ERROR: Could not read.\n");
    return -EFAULT;
}

static ssize_t fib_write(struct file *filp, const char __user *foo, size_t bar, loff_t *baz)
{
    printk(KERN_INFO "Fibonacci: write()\n");
    // TODO
    return 0;
}

static int fib_open(struct inode *i, struct file *filp)
{
    printk(KERN_INFO "Fibonacci: open()\n");
    // Print the current number into the messageBuffer
    snprintf(messageBuffer, MAX_LENGTH, "%d\n", currentNumber);

    // Update current and previous numbers
    uint64_t tmp = currentNumber;
    currentNumber += previousNumber;
    previousNumber = tmp;

    return 0;
}

static int fib_release(struct inode *i, struct file *filp)
{
    printk(KERN_INFO "Fibonacci: release()\n");
    return 0;
}

static struct file_operations fib_fops = {
        .owner =    THIS_MODULE,
        .read =   fib_read,
        .write =  fib_write,
        .open =   fib_open,
        .release = fib_release,
};

static int __init fibonacci_init(void)
{
    // Before anything else, let's initialize our buffer memory.
    messageBuffer = kzalloc(MAX_LENGTH, GFP_KERNEL);
    
    // First, allocate major and minor numbers for our device
    printk(KERN_ALERT "-> Initializing fibonacci char device.\n");

    majorNumber = register_chrdev(0, "fibonacci", &fib_fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "Failed to register character device!\n");
        return PTR_ERR(majorNumber);
    }
    printk(KERN_INFO "Registered character device with major number %d.\n", majorNumber);

    // Create the /sys class for it.
    deviceClass = class_create(THIS_MODULE, "fibonacci");
    if (IS_ERR(deviceClass))
    {
        unregister_chrdev(majorNumber, "fibonacci");
        // TODO: Clean up that magic string
        printk(KERN_ALERT "Could not register device class fibonacci");
        return PTR_ERR(deviceClass);
    }
    printk(KERN_INFO "Registered device class successfully.\n");

    // Create the device file at /dev/fibonacci0
    fibonacciDevice = device_create(deviceClass, NULL, MKDEV(majorNumber, 0), NULL, "fibonacci0");
    if (IS_ERR(fibonacciDevice))
    {
        class_destroy(deviceClass);
        unregister_chrdev(majorNumber, "fibonacci0");
        printk(KERN_ALERT "Failed to create fibonacci device!");
        return PTR_ERR(fibonacciDevice);
    }
    
    // Finally, if we got through all that the device should be registered!
    printk(KERN_INFO "-> Fibonacci device created successfully!\n");
    return 0;
}

static void __exit fibonacci_exit(void)
{
    device_destroy(deviceClass, MKDEV(majorNumber, 0));
    class_unregister(deviceClass);
    class_destroy(deviceClass);
    unregister_chrdev_region(majorNumber, "fibonacci0");
    kfree(messageBuffer);
    printk(KERN_INFO "Destroyed fibonacci device");
}

module_init(fibonacci_init);
module_exit(fibonacci_exit);
