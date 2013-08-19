//FFFS - Fernando Ferraz File System :)

//#include <linux/kernel.h>
//#include <linux/init.h>
//#include <linux/module.h>
//#include <linux/pagemap.h>
//#include <linux/fs.h>
//#include <asm/uaccess.h>
//#include <asm/atomic.h>

//static int fs_fill_super (struct super_block *sb, void *data, int silent) {
//    return 0;
//}

//static struct dentry *fs_get_super(struct file_system_type *fst,
//                                   int flags, const char *devname, void *data) {
//	return mount_bdev(fst, flags, devname, data, fs_fill_super);
//}

//static struct file_system_type lfs_type = {
//	.owner 		= THIS_MODULE,
//	.name		= "fffs",
//	.mount		= fs_get_super,
//	.kill_sb	= kill_litter_super,
//};

//static int __init fs_init(void)
//{
//	return register_filesystem(&fs_type);
//}

//static void __exit fs_exit(void)
//{
//	unregister_filesystem(&fs_type);
//}
