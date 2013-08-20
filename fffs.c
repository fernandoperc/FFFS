//FFFS - Fernando Ferraz File System :)

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>



static int fffs_fill_super(struct super_block *sb, void *data, int silent) {
  return 0;
}

static void fffs_kill_sb(struct super_block *sb) {

  kill_litter_super(sb);
}

static struct dentry *fffs_mount(struct file_system_type *fst,
                                   int flags, const char *devname, void *data) {

  return mount_bdev(fst, flags, devname, data, fffs_fill_super);
}

static struct file_system_type fffs_fs_type = {
  .name		= "fffs",
  .mount		= fffs_mount,
  .kill_sb	= fffs_kill_sb,
};

static int __init fffs_init(void) {
  return register_filesystem(&fffs_fs_type);
}

static void __exit fffs_exit(void) {
  unregister_filesystem(&fffs_fs_type);
}

module_init(fffs_init)
module_exit(fffs_exit)

