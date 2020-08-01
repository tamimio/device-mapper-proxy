#include <linux/device-mapper.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "dmp"

MODULE_DESCRIPTION(DM_NAME "Device mapper proxy");
MODULE_LICENSE("GPL");

/* --------------------------------------------------------------- local data */
/* 
 * structure dmp_statistics
 * stores statistics of performed on the device operations
 */
struct dmp_statistics
{
  unsigned long int wr_cnt,    /* number of write requests */
                    rd_cnt,    /* number of read requests */
                    wr_avg_sz, /* average size of block to write */
                    rd_avg_sz, /* average size of block to read */
                    total_cnt, /* total number of requests */
                    total_avg_sz; /* average size of block */
};

static struct dmp_statistics dmp_stats = {
  .wr_cnt=0,
  .rd_cnt=0,
  .wr_avg_sz=0,
  .rd_avg_sz=0,
  .total_cnt=0,
  .total_avg_sz=0
};

/* ------------------------------------------------------------ ctr, dtr, map */
/*
 * ctr
 * (constructor of device, create a new virtual block device)
 * 
 */
static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
  struct dm_dev *device;
  /* check arguments */
  if (argc != 2)
  {
    printk(KERN_CRIT "\n Invalid number of arguments.\n");
    ti->error = "Invalid argument count";
    return -EINVAL;
  }
  
  /* getting the permissions of device mapper table
   * (which gets created after dmsetup create) */
  if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &device))
  {
    ti->error = "Device lookup failed";
    return -EINVAL;
  }
  ti->private = device;
  
  /* init */
  dmp_stats.wr_cnt=0;
  dmp_stats.rd_cnt=0;
  dmp_stats.wr_avg_sz=0;
  dmp_stats.rd_avg_sz=0;
  dmp_stats.total_cnt=0;
  dmp_stats.total_avg_sz=0;
  
  printk(KERN_ALERT "dmp constructed\n");
	return 0; /* success */
}

/*
 * dtr
 * (destructor of device, stop using a virtual block device)
 * 
 */
static void dmp_dtr(struct dm_target *ti)
{
  struct dm_dev *device = (struct dm_dev *) ti->private;
  dm_put_device(ti, device);
  printk(KERN_ALERT "dmp destructed\n");
  return;
}
/*
 * map
 * receives incoming bio and calculates statistics
 */
static int dmp_map(struct dm_target *ti, struct bio *bio)
{
  struct dm_dev *device = (struct dm_dev *) ti->private;
  bio_set_dev(bio, device->bdev);/*bio->bi_bdev = device->bdev;*/
  
  unsigned long int bi_size=0;
  /*if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))*/
    bi_size = bio->bi_iter.bi_size;
  
  /* switch type of operation */
	switch (bio_op(bio))
  {
    case REQ_OP_READ:
      dmp_stats.rd_cnt++;
      dmp_stats.rd_avg_sz += bi_size - dmp_stats.rd_avg_sz; /* ? */
      break;
    case REQ_OP_WRITE:
      dmp_stats.wr_cnt++;
      dmp_stats.wr_avg_sz += bi_size - dmp_stats.wr_avg_sz;
      break;
    default:
      return DM_MAPIO_KILL;
	}
  
  dmp_stats.total_cnt += dmp_stats.rd_cnt + dmp_stats.wr_cnt;
  dmp_stats.total_avg_sz += (bi_size - dmp_stats.total_avg_sz) / dmp_stats.total_cnt;
  
  submit_bio(bio);
  bio_endio(bio);
	/* accepted bio, don't make new request */
	return DM_MAPIO_SUBMITTED;
}

/* ------------------------------------------------------------------- target */
/*
 * device-mapper target
 */
static struct target_type dmp_target = {
	.name   = "dmp",
	.version = {0, 0, 1},
	.module = THIS_MODULE,
	.ctr    = dmp_ctr,
	.map    = dmp_map,
  .dtr    = dmp_dtr,
};

/* --------------------------------------------------------------- init, exit */
/*
 * init
 * function will be called when the module is loaded to the kernel
 */
static int __init dmp_init(void)
{
	int r = dm_register_target(&dmp_target);

	if (r < 0)
		DMERR("loading dmp failed: %d", r);
  else
    printk(KERN_ALERT "dmp is loaded to the kernel successfully\n");

	return r;
}

/*
 * exit
 * function will be called when the module is removed from the kernel
 */
static void __exit dmp_exit(void)
{
	dm_unregister_target(&dmp_target);
  printk(KERN_ALERT "dmp is removed from the kernel\n");
  return;
}

/* linking init and exit */
module_init(dmp_init)
module_exit(dmp_exit)
