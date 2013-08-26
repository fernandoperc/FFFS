/* FFFS - Fernando Ferraz File System :P
 * Este file system tem o intuito de servir como um ambiente de aprendizado
 * para o desenvolvimento de file systems sob linux VFS. Contribuições
 * são bem vindas tendo em vista que este código pode servir para aprendizado
 * de mais pessoas no futuro. Este File system segue como referência o
 * código publicado no artigo "Creating Linux virtual filesystems" por Jonathan
 * Corbet - http://lwn.net/Articles/57369/ (código disponível em
 * https://gist.github.com/prashants/3496839)
 *
 * Funcionalidades (todas dummy :P):
 *   /fibonacci:
 *     - read_file(): calcula próximo fibonnaci para cada operação de leitura
 *     no arquivo fibonacci.
 *     - write_file(): verifica se valor passado é um numero fibonacci, caso
 *     verdadeiro o valor é armazenado.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fernando Ferraz");

/* seguindo outros exemplos que vi, é só adicionar um valor aleatório. */
#define FFFS_MAGIC 0xFF1234

/* armazeno nesta struct as variaveis necessárias para calcular fibonacci. */
struct fibonacci_counter {
  int fibonacci_index;
  int fibonacci_last;
  int fibonacci_current;
};

static struct fibonacci_counter f_counter = {
  .fibonacci_index = 0,
  .fibonacci_last = 0,
  .fibonacci_current = 0,
};

/* inicializando operações de super_block */
static struct super_operations fffs_super_operations = {
  .statfs		= simple_statfs,
  .drop_inode	= generic_delete_inode,
};

/* calcula fibonacci baseado nos valores contidos em fibonacci_counter.
 * está sem otimização nenhuma, no caso eu só queria ver funcionando :P */
static int calculate_next_fibonacci(struct fibonacci_counter *fibonacci_cnt) {

  int last_value = 0;
  int return_value = 0;

  if (fibonacci_cnt->fibonacci_index == 0) {

    return_value =  0;

  } else if (fibonacci_cnt->fibonacci_index == 1) {

    fibonacci_cnt->fibonacci_last = 0;
    fibonacci_cnt->fibonacci_current = 1;
    return_value = 1;

  } else {

    last_value = fibonacci_cnt->fibonacci_current;

    fibonacci_cnt->fibonacci_current = fibonacci_cnt->fibonacci_current +
                                   fibonacci_cnt->fibonacci_last;

    fibonacci_cnt->fibonacci_last = last_value;

    return_value = fibonacci_cnt->fibonacci_current;

  }

  fibonacci_cnt->fibonacci_index++;
  return return_value;
}


static int fffs_open(struct inode *inode, struct file *filp) {

  filp->private_data = inode->i_private;
  return 0;
}

/* disponivel para o usuário, lê um arquivo no file system (atualmente fixo
 *para o meu gerador de fibonacci) */
static ssize_t fffs_read_file(struct file *filp, char *buf,
                              size_t count, loff_t *offset) {

  int len = 0;
  char return_string[25];
  struct fibonacci_counter *fibonacci_pt;

  if (*offset > 0) {
    return 0;
  }

  fibonacci_pt = (struct fibonacci_counter*)filp->private_data;
  calculate_next_fibonacci(fibonacci_pt);

  len = snprintf(return_string, 25, "%d\n", fibonacci_pt->fibonacci_current);

  if (count > len - *offset) {
    count = len;
  }

  if (copy_to_user(buf, return_string, count)) {
    return -EFAULT;
  }

  *offset += count;
  return count;
}

/* disponivel para o usuario, escreve em um arquivo no file system
 *(atualmente fixo para o meu gerador de fibonacci) */
static ssize_t fffs_write_file(struct file *filp, const char *buf,
                               size_t count, loff_t *offset) {

  int buffer_value = 0;
  char return_string[25];
  struct fibonacci_counter fibonacci_temp;
  struct fibonacci_counter *fibonacci_pt;

  fibonacci_pt = (struct fibonacci_counter*)filp->private_data;

  if (*offset > 0) {
    return 0;
  }

  if (count >= 25) {
    return -EINVAL;
  }

  memset(return_string, 0, 20);

  if (copy_from_user(return_string, buf, count)) {
    return -EFAULT;
  }

  buffer_value = simple_strtol(return_string, NULL, 10);

  fibonacci_temp.fibonacci_index = 0;
  fibonacci_temp.fibonacci_last = 0;
  fibonacci_temp.fibonacci_current = 0;

  do {

    calculate_next_fibonacci(&fibonacci_temp);

    if (fibonacci_temp.fibonacci_current > buffer_value) {

      return -EINVAL;

    } else if (fibonacci_temp.fibonacci_current == buffer_value) {

      fibonacci_pt->fibonacci_index = fibonacci_temp.fibonacci_index;
      fibonacci_pt->fibonacci_last = fibonacci_temp.fibonacci_last;
      fibonacci_pt->fibonacci_current = fibonacci_temp.fibonacci_current;

      break;
    }

  } while (true);

  return count;
}

/* não disponivel pra o usuario de file system, cria um novo inode para o meu
 * file system. */
static struct inode *fffs_create_inode(struct super_block *sb, int mode) {

  struct inode *ret = new_inode(sb);

  ret->i_mode = mode;
  ret->i_uid = ret->i_gid = 0;
  ret->i_blocks = 0;
  ret->i_atime = ret->i_mtime = ret->i_ctime = CURRENT_TIME;

  return ret;
}

/* inicializando struct contendo as operações de arquivo implementadas. */
static struct file_operations fffs_file_ops = {
    .open	= fffs_open,
    .read 	= fffs_read_file,
    .write      = fffs_write_file,
};

/* não disponivel para o usuario de file system, cria um novo arquivo no meu
 * file system. */
static struct dentry *fffs_create_file (struct super_block *sb,
                                        struct dentry *dir, const char *name) {

  struct dentry *dentry;
  struct inode *inode;
  struct qstr qname;

  qname.name = name;
  qname.len = strlen (name);
  qname.hash = full_name_hash(name, qname.len);

  dentry = d_alloc(dir, &qname);
  inode = fffs_create_inode(sb, S_IFREG | 0777);

  inode->i_fop = &fffs_file_ops;
  inode->i_private = &f_counter;

  d_add(dentry, inode);
  return dentry;

}

/* chamada quando o file system é montado, preenche o super_block e incializa
 * o inode e dentry do root */
static int fffs_fill_super(struct super_block *sb, void *data, int silent) {

  struct inode *root;
  struct dentry *root_dentry;

  sb->s_blocksize = PAGE_CACHE_SIZE;
  sb->s_blocksize_bits = PAGE_CACHE_SHIFT;
  sb->s_magic = FFFS_MAGIC;
  sb->s_op = &fffs_super_operations;

  root = fffs_create_inode(sb, S_IFDIR | 0777);
  root->i_op = &simple_dir_inode_operations;
  root->i_fop = &simple_dir_operations;

  root_dentry = d_make_root(root);
  sb->s_root = root_dentry;

  fffs_create_file(sb, root_dentry, "fibonacci");

  return 0;
}

/* assassino de superblocos :P */
static void fffs_kill_sb(struct super_block *sb) {
  kill_litter_super(sb);
}

/* chamada quando o FS é montado. */
static struct dentry *fffs_mount(struct file_system_type *fst,
                                 int flags, const char *devname, void *data) {

  return mount_bdev(fst, flags, devname, data, fffs_fill_super);
}

/* iniciali/zando struct contendo as informações do meu file system. */
static struct file_system_type fffs_fs_type = {
//  .owner 	= THIS_MODULE,
  .name		= "fffs",
  .mount	= fffs_mount,
  .kill_sb	= fffs_kill_sb,
};

/* chamada na inicialização do modulo */
static int __init fffs_init(void) {
  return register_filesystem(&fffs_fs_type);
}

/* declarando a funcção de inicialização do lo de kernel */
module_init(fffs_init)

