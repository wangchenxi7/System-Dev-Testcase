/**
 * Works like a block device driver:
 * 
 * 1) Register a block device under /dev/xxx
 * 
 * 2) Define read/write i/o operation
 * 
 * 3) i/o queue
 * 
 * 4) maintain the RDMA connection to remote server.
 * 
 * 
 */

// Linux header
#include <linux/kernel.h>
#include <linux/module.h>


// Self defined headers
#include "register_disk.h"



/**
 * 1) RDMA connection
 */




/**
 * 2) Define device i/o operation
 * 
 */

/**
 * hctx : ?? 
 *      some intermediate structure, used to pass information
 * 
 * data : who assigns this parater ??
 *      seems the data is the rdd_device_control 
 * 
 * index : 
 * 
 */
static int rmem_init_hctx(struct blk_mq_hw_ctx *hctx, void *data, unsigned int index){

  struct rbd_device_control* rbd = data;
  struct rbd_queue * rbd_conn_q;


  // What's  this used for ??
  // assign hctx->driver_data

  rbd_conn_q = &rbd->queues[index]
  // [??] Do we need to initiate its fields here ??

  htcx->driver_data = rbd_conn_q;

  return 0;
}


/**
 * the devicer operations
 * 
 * queue_rq : handle the queued i/o operation
 * map_queues : map the hardware dispatch queue to cpu
 * init_hctx : initialize the hardware dispatch queue ?? 
 * 
 */
static struct blk_mq_ops rmem_mq_ops = {
	.queue_rq       = rmem_queue_rq,
	.map_queues     = blk_mq_map_queues,  // Map hardware dispatch queues and available cores.
	.init_hctx	    = rmem_init_hctx,
};







/**
 * #################3
 * 3) Register Local Block Device (Interface)
 * 
 */


/**
 * blk_mq_tag_set stores all the disk information. Both hardware and software
 * 
 * 
 * 
 */
static int init_blk_mq_tag_set(struct rmem_device_control* rmem_dev_ctrl){

  struct blk_mq_tag_set* tag_set = rmem_dev_ctrl->tag_set;
  int err = 0;

  if(!tag_set){
    printk("init_blk_mq_tag_set : pass a null pointer in. \n");
    return -1;
  }

  tag_set->ops = &rmem_mq_ops;
  tag_set->nr_hw_queues = online_cores;   // hardware dispatch queue == software staging queue == avaible cores
  tag_set->queue_depth = RMEM_QUEUE_DEPTH;
  tag_set->numa_node  = NUMA_NO_NODE;
  tag_set->cmd_size = sizeof(struct rmem_rdma_request);
  tag_set->flags = BLK_MQ_F_SHOULD_MERGE;   // [?]merge the i/o requets ??
  tag_set->driver_data = rmem_dev_ctrl;     // The disk dievice information

  err = blk_mq_alloc_tag_set(tag_set);      // Check & correct the value within the blk_mq_tag_set.
	if (err)
		goto out;

out:
  printk("init_blk_mq_tag_set error.\n");

  return err;
}




/**
 * Allocate && Set the gendisk information 
 * 
 * 
 * // manual url : https://lwn.net/Articles/25711/
 * 
 * 
 * gendisk->fops : open/close a device ? 
 *        The difference with read/write i/o operation (blk_mq_ops)??
 * 
 * 
 */


static int rmem_dev_open(struct block_device *bd, fmode_t mode)
{
	pr_debug("%s called\n", __func__);
	return 0;
}

static void rmem_dev_release(struct gendisk *gd, fmode_t mode)
{
	pr_debug("%s called\n", __func__);
}

static int rmem_dev_media_changed(struct gendisk *gd)
{
	pr_debug("%s called\n", __func__);
	return 0;
}

static int rmem_dev_revalidate(struct gendisk *gd)
{
	pr_debug("%s called\n", __func__);
	return 0;
}

static int rmem_dev_ioctl(struct block_device *bd, fmode_t mode,
		      unsigned cmd, unsigned long arg)
{
	pr_debug("%s called\n", __func__);
	return -ENOTTY;
}


static struct block_device_operations rmem_device_ops = {
	.owner            = THIS_MODULE,
	.open 	          = rmem_dev_open,
	.release 	        = rmem_dev_release,
	.media_changed    = rmem_dev_media_changed,
	.revalidate_disk  = rmem_dev_revalidate,
	.ioctl	          = rmem_dev_ioctl
};


int init_gendisk(struct rmem_device_control* rmem_dev_ctrl ){

  int ret = 0;
  char disk_name[15] = "remote_mem";
  sector_t remote_mem_size = RMEM_SIZE_IN_BYTES;
  unsigned long page_size  = PAGE_SIZE;

  rmem_dev_ctrl->disk = alloc_disk_node(1, NUMA_NO_NODE); // minors =1, at most have one partition.
  if(!rmem_dev_ctrl->disk){
    pr_err("%s: Failed to allocate disk node\n", __func__);
		ret = -ENOMEM;
    goto alloc_disk;
  }

  rmem_dev_ctrl->disk->major  = rmem_dev_ctrl->major;
  rmem_dev_ctrl->disk->first_minor = 0;  // The partition id, start from 0.
  rmem_dev_ctrl->disk->fops   = &rmem_device_ops;  // Define device operations
  rmem_dev_ctrl->disk->queue  = rmem_dev_ctrl->queue;   //  Points to the software staging request queue
  rmem_dev_ctrl->disk->private_data = rmem_dev_ctrl;    // Driver controller/context. Reserved for disk driver.
  memcpy(rmem_dev_ctrl->disk->disk_name, disk_name, 15);

 
  //
  // * set some device information
  //
  blk_queue_logical_block_size(rmem_dev_ctrl->queue, RMEM_SECT_SIZE);    // logical block size = 512 bytes
	blk_queue_physical_block_size(rmem_dev_ctrl->queue, RMEM_SECT_SIZE);   // physical block size 512 bytes ??
	sector_div(page_size, RMEM_SECT_SIZE);                // page_size /=RMEM_SECT_SIZE
	blk_queue_max_hw_sectors(rmem_dev_ctrl->queue, page_size * MAX_SGL_LEN);   // [??]
	sector_div(remote_mem_size, RMEM_SECT_SIZE);          // remote_mem_size /=RMEM_SECT_SIZE, return remote_mem_size%RMEM_SECT_SIZE 
	set_capacity(rmem_dev_ctrl->disk, remote_mem_size);   // size is in remote file state->size, add size info into block device

   //debug
  printk("init_gendisk : initialize disk %s done. \n"rmem_dev_ctrl->disk->disk_name);






alloc_disk:
  blk_cleanup_queue(rmem_dev_ctrl->queue);

}



int RBD_create_device(char* dev_name, struct rmem_device_control* rmem_dev_ctrl ){

  int err = 0;

  if(dev_name == NULL ){
    //Use default name
    dev_name = (char*)malloc(sizeof(char)*10);
    dev_name = "remote_bd";
  }

  //
  //[?] which parameters have to be assigned ?
  //

  // a. assign device information
  //

  rmem_dev_ctrl->major = rbd_major_num;


  // * Check * correct thte blk_mq_tag_set
  err = init_blk_mq_tag_set(rmem_dev_ctrl);

  if(!err){
    printk("RBD_create_device : Allocate blk_mq_tag_set failed. \n");
    return err;
  }

  // * allocate & intialize the software staging queue
  rmem_dev_ctrl->queue = blk_mq_init_queue(&rmem_dev_ctrl->tag_set);   // Build the block i/o queue.
	if (IS_ERR(rmem_dev_ctrl->queue )) {
		err = PTR_ERR(rmem_dev_ctrl->queue );
    printk("RBD_create_device : create the software staging reqeust queue failed. \n");
		return err;
	}
  

  
  // * initiate the gendisk information


  rmem_dev_ctrl->disk->major = rmem_dev_ctrl->major;
  //rmem_dev_ctrl->disk->first_minor = ?


  return err;

}





static int  RBD_init_module(void){

	//debug
  printk("Load kernel module : register remote block device. \n");

	rbd_major_num = register_blkdev(0, "rmempool");
	if (rbd_major_num < 0){
		return rbd_major_num;
  }
  //debug
  printk("Got block device major number: %d \n", rbd_major_num);




  //number of staging and dispatch queues, equal to available cores
  online_cores = num_online_cpus();
  //debug
  printk(" online cores : %d \n", onlin_cores);
  



	return 0;
}

// module function
static void RBD_cleanup_module(void){

	unregister_blkdev(rbd_major_num, "infiniswap");

}






module_init(RBD_init_module);
module_exit(RBD_cleanup_module);
