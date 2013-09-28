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
#include <linux/vfs.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <asm/uaccess.h>

/* seguindo outros exemplos que vi, é só adicionar um valor aleatório. */

#define FFFS_MAGIC 0xFF1234

static int inode_number;

const struct inode_operations fffs_file_inode_operations = {
    .getattr    = simple_getattr,
};

/* inicializando operações de super_block */
static struct super_operations fffs_super_operations = {
  .statfs	= simple_statfs,
  .drop_inode	= generic_delete_inode,
};

/* armazeno nesta struct as variaveis necessárias para calcular fibonacci. */
struct fibonacci_counter {
  int fibonacci_index;
  int fibonacci_last;
  int fibonacci_last2;
  int fibonacci_current;
};

static struct fibonacci_counter f_counter = {
   .fibonacci_index = 0,
   .fibonacci_last = 0,
   .fibonacci_last2 = 0,
   .fibonacci_current = 0,
};

//static int make_fibonacci_dir(struct super_block *sb,
//                              struct inode* parent_inode,
//                              int fibonacci_index, int current_number) {

//  struct inode* inode;

//  if (current_number == 0 and fibonacci_index == 0) {
//    return 0;
//  }

//  inode = fffs_create_inode(sb, S_IFDIR | 0777);

//  return 0;
//}

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

/*lê um arquivo no file system (atualmente fixo
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

  len = snprintf(return_string, 25, "value: %d\n", fibonacci_pt->fibonacci_current);

  //adicionando esta checagem pro futuro...
  if (count > len - *offset) {
    count = len;
  }

  if (copy_to_user(buf, return_string, count)) {
    return -EFAULT;
  }

  *offset += count;
  return count;

}

/* escreve em um arquivo no file system
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

static struct file_operations fffs_file_operations = {
    .open	= fffs_open,
    .read 	= fffs_read_file,
    .write      = fffs_write_file,
};

static int fffs_mkdir(struct inode * dir, struct dentry *dentry,
        int mode);

static struct inode_operations fffs_dir_inode_operations = {
    .lookup     = simple_lookup,
    .mkdir      = fffs_mkdir,
};

struct inode * fffs_create_inode(struct super_block *sb, int mode) {

  struct inode * inode = new_inode(sb);

  if (inode) {
      inode->i_mode = mode;
      inode->i_uid = current->cred->fsuid;
      inode->i_gid = current->cred->fsgid;
      //inode->i_blocks = 0;
      inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
      //inode->i_ino = ++inode_number;
      /* G: Voce esta usando o inode number que foi retornado nessa funcao. Apesar de nao ter nada de errado nisso,
            isso ignora completamente o inode_number global do islenefs. Entao voce poderia te-lo apagado.
           -> na verdade eu esqueci de acrescentar a linha acima, agora tem sentido a variável global */
      switch (mode & S_IFMT) {
        case S_IFREG:
          inode->i_op = &fffs_file_inode_operations;
          inode->i_fop = &fffs_file_operations;
          break;
        case S_IFDIR:
          inode->i_op = &fffs_dir_inode_operations;
          inode->i_fop = &simple_dir_operations;
          //inc_nlink(inode);
          break;
        }
    }
  return inode;
}

static struct dentry *fffs_create_dir (struct super_block *sb,
                                       struct dentry *parent,
                                       int mode,
                                       int fibonacci_index,
                                       int fibonnaci_number) {

  struct dentry *dentry;
  struct inode *inode;
  struct qstr qname;
  const char name[5];

  snprintf(&name, 5, "%d", fibonnaci_number);

  qname.name = name;
  qname.len = strlen (name);
  qname.hash = full_name_hash(name, qname.len);
  dentry = d_alloc(parent, &qname);
  if (! dentry)
    goto out;

  inode = fffs_create_inode(sb, mode);
  if (! inode)
    goto out_dput;

  d_add(dentry, inode);
  return dentry;

out_dput:
  dput(dentry);
out:
  return 0;

};

static int fffs_mkdir(struct inode * dir, struct dentry *dentry,
        int mode) {

    struct inode *inode;

    mode |= S_IFDIR;

    inode = fffs_create_inode(dir->i_sb, mode);

    if (inode) {
        if (dir->i_mode & S_ISGID) {
            inode->i_gid = dir->i_gid;
            if (S_ISDIR(mode))
                inode->i_mode |= S_ISGID;
        }

        d_instantiate(dentry, inode);
        //dget(dentry);
        dir->i_mtime = dir->i_ctime = CURRENT_TIME;
        //inc_nlink(dir);

    }

    fffs_create_dir(dir->i_sb, dentry, mode, 0, 1);
    fffs_create_dir(dir->i_sb, dentry, mode, 0, 0);

    return 0;
}


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

  //inode = fffs_create_inode(sb, S_IFREG | 0777);
  //  inode->i_fop = &fffs_file_operations;
  //  inode->i_op = &fffs_file_inode_operations;

  inode = fffs_create_inode(sb,  S_IFREG | 0777);

  inode->i_blocks = 0;
  inode->i_uid = inode->i_gid = 0;
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
  root->i_op = &fffs_dir_inode_operations;
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
  .kill_sb	= fffs_kill_sb
};

/* chamada na inicialização do modulo */
static int __init fffs_init(void) {
  return register_filesystem(&fffs_fs_type);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fernando Ferraz");

/* declarando a funcção de inicialização do lo de kernel */
module_init(fffs_init)

