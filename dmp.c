#include <linux/device-mapper.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>

#define DM_MSG_PREFIX "dmp"

/* 
 * structure dmp_statistics
 * stores statistics of performed on the device operations
 */
struct dmp_statistics
{
    int wr_cnt,    /* number of write requests */
        rd_cnt,    /* number of read requests */
        wr_avg_sz, /* average size of block to write */
        rd_avg_sz, /* average size of block to read */
        total_cnt, /* total number of requests */
        total_avg; /* average size of block */
} stats;

/*
 * TODO
 * //Construct a dummy mapping that only returns zeros
 */
static int dmp_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
	if (argc != 0) {
		ti->error = "No arguments required";
		return -EINVAL;
	}

	/*
	 * Silently drop discards, avoiding -EOPNOTSUPP.
	 */
	ti->num_discard_bios = 1;

	return 0;
}

/*
 * map
 * receives incoming bio and calculates statistics
 */
static int dmp_map(struct dm_target *ti, struct bio *bio)
{
	switch (bio_op(bio))
  {
    case REQ_OP_READ:
      stats.rd_cnt++;
      stats.rd_avg_sz += bio.bi_size - stats.rd_avg_sz; /* ? */
      break;
    case REQ_OP_WRITE:
      stats.wr_cnt++;
      stats.wr_avg_sz += bio.bi_size - stats.wr_avg_sz;
      break;
    default:
      return DM_MAPIO_KILL;
	}

	bio_endio(bio);
  
  stats.total cnt += stats.rd_cnt + stats.wr_cnt;
  stats.total_avg += (bio_size - stats.total_avg) / stats.total cnt;

	/* accepted bio, don't make new request */
	return DM_MAPIO_SUBMITTED;
}

/*
 * device-mapper target
 */
static struct target_type dmp_target = {
	.name   = "dmp",
	.version = {0, 0, 1},
	.module = THIS_MODULE,
	.ctr    = dmp_ctr,
	.map    = dmp_map,
};

/*
 * init
 */
static int __init dm_dmp_init(void)
{
	int r = dm_register_target(&dmp_target);

	if (r < 0)
		DMERR("register failed %d", r);
  else
  printk(KERN_ALERT "dmp init\n");

	return r;
}

/*
 * exit
 */
static void __exit dm_dmp_exit(void)
{
	dm_unregister_target(&dmp_target);
  printk(KERN_ALERT "dmp exit\n");
}

module_init(dm_dmp_init)
module_exit(dm_dmp_exit)

MODULE_DESCRIPTION(DM_NAME "Device mapper proxy");
MODULE_LICENSE("GPL");