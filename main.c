/*
   Example of a minimal character device driver 
*/
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <asm/uaccess.h> 

#define DEV_DATA_LENGTH  100
#define NUM_DEVICES      6
#define DEVICE_NAME      "huadeng"
#define MAJOR_NUM        88

static struct class *huadeng_class;


struct huadeng_dev {
	unsigned short current_pointer; /* current pointer within the device data region */
	unsigned int size;            /* size of the bank */
	int number;                   /* device number */
	struct cdev lll_cdev;         /* the cdev structure */ 
	struct device* lll_device;
	char name[10];                /* name of the device */
	char data[DEV_DATA_LENGTH];
} *hd_devp[NUM_DEVICES];



/**********************procfs lab ********************************************/
static struct proc_dir_entry *proc_lll_entry = NULL;
#if 0
static ssize_t proc_lll_read(struct file* filep, 
                             char __user* buf,
                             size_t count,
                             loff_t* offp )
{
    int n = 0, ret;
    char secrets[100];
    printk(KERN_INFO "proc_lll_read is called file %p, buf %p, count %d, off %llx\n",
                      filep, buf, count, *offp );

    sprintf(secrets, "kernel secrects blah blah %s...\n", filep->f_path.dentry->d_iname );
    n = strlen(secrets);
    if (*offp < n )
    {
        copy_to_user(buf, secrets, n+1);
        *offp = n + 1;
        ret = n + 1;
    }
    else
    {
        ret = 0;
    }

    return ret;
}

static const struct file_operations proc_lll_fops = {
.owner = THIS_MODULE,
.read  = proc_lll_read
};
#endif

static int lll_proc_show( struct seq_file *m, void *v )
{
    seq_printf(m, "kernel secrets balabala llaolao...\n");
    return 0;
}

static int lll_proc_open(struct inode* inode, struct file *file )
{
    return single_open( file, lll_proc_show, NULL );
}

static const struct file_operations lll_proc_fops = {
    .open = lll_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release
};

/**********************debugfs lab ********************************************/
/* static variables for debugfs*/

struct lll_profile_struct
{
    int age;
    int gender;
};

static struct dentry *df_dir = NULL;
static struct lll_profile_struct* lll_profile = NULL;
static int
lll_age_set(void *data, u64 val )
{
    struct lll_profile_struct *lp = (struct lll_profile_struct *)data;
    lp->age = val;

    return 0;
}

static int
lll_age_get(void* data, u64* val )
{
    struct lll_profile_struct *lp = (struct lll_profile_struct *)data;
    *val = lp->age;
    return 0;
}

/* macro from linux/fs.h */
DEFINE_SIMPLE_ATTRIBUTE(df_age_fops, lll_age_get, lll_age_set, "%llu\n" );

static int huadeng_open(struct inode *inode, struct file *file )
{
	struct huadeng_dev *hd_devp;
	pr_info("%s\n", __func__);
	printk("i'm being openned!\n" );

	/* Get the per-device structure that contains this cdev */
	hd_devp = container_of(inode->i_cdev, struct huadeng_dev, lll_cdev );

	/* Easy access to hd_devp from rest of the entry points */
	file->private_data = hd_devp;

	/* Initiailize some fileds */
	hd_devp->size = DEV_DATA_LENGTH;
	hd_devp->current_pointer = 0;
	return 0;
}

static ssize_t huadeng_read(struct file *file, char *buffer, size_t count, loff_t *offset)
{
	struct huadeng_dev *hd_devp = file->private_data;
	pr_info("%s count %u, +%llu\n", __func__, count, *offset );

	if (*offset >= hd_devp->size) {
		return 0; /*EOF*/
	}

        /* Adjust count if its edges past the end of the data region */
	if (*offset + count > hd_devp->size ) {
		count = hd_devp->size - *offset;
	}

	/* Copy the read bits to the user buffer */
	if (copy_to_user(buffer, (void*)(hd_devp->data + *offset),count) != 0 ) {
		return -EIO;
	}

	*offset += count;
	
	return count;
}

static ssize_t huadeng_write(struct file *file, const char *buffer, size_t length, loff_t *offset )
{
	struct huadeng_dev* hp_devp = file->private_data;
	pr_info("%s %u + %llx\n", __func__, length, *offset);
	if (*offset >= hp_devp->size) {
		return 0;
	}
	if (*offset + length > hp_devp->size) {
		length = hp_devp->size - *offset;
	}

	if (copy_from_user(hp_devp->data + *offset, buffer, length ) != 0) {
		printk(KERN_ERR "copy_from_user failed\n");
		return -EFAULT;
	}

	return length;
}

static int huadeng_release(struct inode *inode, struct file* file )
{
	return 0;
}

struct file_operations huadeng_fops = {
	.owner   = THIS_MODULE,
	.open    = huadeng_open,
	.release = huadeng_release,
	.read    = huadeng_read,
	.write   = huadeng_write,
};


static void llaolao_cleanup_devs( void )
{
	int i;
	//struct huadeng_dev* pdev;
	for ( i = 0; i < NUM_DEVICES; ++i ) {
		if (hd_devp[i]) {
			cdev_del(&hd_devp[i]->lll_cdev);
			device_del(hd_devp[i]->lll_device);
		}
		kfree(hd_devp[i]);
	}
}

static int __init llaolao_init(void)
{
	int n = 0x1937;
	int i = 0;
	dev_t first_dev_num; /*first device number*/

	printk(KERN_INFO "Hi, I am llaolao at address 0x%p stack 0x%p.\n", llaolao_init, &n);
	printk(KERN_INFO "symbol: 0x%pS\n", llaolao_init );
	printk(KERN_INFO "stack : %pB\n", llaolao_init );
	printk(KERN_INFO "first 16bytes: %*phC\n", 16, llaolao_init );

	/* create /proc/llaolao */
	proc_lll_entry = proc_create("llaolao", 0, NULL, &lll_proc_fops );

	df_dir = debugfs_create_dir("llaolao", 0 );
	if ( !df_dir )
	{
		printk( KERN_ERR "create dir under debugfs failed\n");
		return -1;
	}

	debugfs_create_file("age", 0222, df_dir, &lll_profile, &df_age_fops );

	huadeng_class = class_create(THIS_MODULE, DEVICE_NAME);
	/* Allocate memory for the per-device structure */
	for ( i = 0; i < NUM_DEVICES; ++i ) {
		hd_devp[i] = kmalloc(sizeof(struct huadeng_dev), GFP_KERNEL);
		if (!hd_devp[i]) {
			printk("allocate huadeng dev struct failed\n");
			return -ENOMEM;
		}
		/* Connect the file operations with the cdev */
		cdev_init(&hd_devp[i]->lll_cdev, &huadeng_fops);
		first_dev_num = MKDEV(MAJOR_NUM, i );
		if ( cdev_add( &hd_devp[i]->lll_cdev, first_dev_num, 1 ) ) { 
			goto out_err;
		}

		hd_devp[i]->lll_cdev.owner = THIS_MODULE;
		/* Send uevents to udev, so it'll create /dev nodes */
		hd_devp[i]->lll_device = device_create(huadeng_class, NULL, first_dev_num, NULL, "huadeng%d", i);
	}
	return 0;

out_err:
	llaolao_cleanup_devs();
	return -1;
}





static void __exit llaolao_exit(void)
{
    printk("Exiting from 0x%p... Bye, GEDU friends\n", llaolao_exit);
    if (proc_lll_entry)
        proc_remove(proc_lll_entry);

    if( df_dir )
        debugfs_remove_recursive( df_dir );

    llaolao_cleanup_devs();
     if (huadeng_class)
	     class_destroy(huadeng_class);
}

module_init(llaolao_init);
module_exit(llaolao_exit);

MODULE_AUTHOR("GEDU lab");
MODULE_DESCRIPTION("LKM example - llaolao");
MODULE_LICENSE("GPL");
