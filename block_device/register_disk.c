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


// Self defined headers
#include "register_disk.h"



MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("RMEM, remote memory paging over RDMA");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0");


/**
 * 1) RDMA connection
 */




/**
 * 2) Define device i/o operation
 * 
 */

/**
 * hctx :  
 *      The hardware dispatch queue context.
 * 
 * data : who assigns this parater ??
 *      seems the data is the rdd_device_control, the driver controller/context.
 * 
 * hw_index : 
 *      The index for the dispatch queue ??
 *      [?] The problem is that every hardware core will invoke blk_mq_ops->.init_hctx  ??
 * 
 * 
 */
static int rmem_init_hctx(struct blk_mq_hw_ctx *hctx, void *data, unsigned int hw_index){

  struct rmem_device_control* rmem_ctrl = data;
  struct rmem_rdma_queue*  rmem_queue = &rmem_ctrl->rdma_queues[hw_index];

  // do some initialization for the  rmem_rdma_queues
  // 
  //

  hctx->driver_data = rmem_queue;

  return 0;
}


/**
 * Dirver gets request fomr the hardware dispatch queue ??
 * blk_mq_ops->queue_rq is the regsitered function to handle this request. 
 * 
 * For this remote_memory_pool device, we seend the read/write request to remote mmeory pool.
 * 
 * blk_mq_queeu_data : stores the requet gotten from staging queue.
 * blk_mq_hw_ctx
 * 
 */
static int rmem_queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd){

  int ret = 0;

  struct rmem_rdma_queue* rmem_rdma_q = hctx->driver_data;   // blk_mq_hw_ctx->driver_data  stores the RDMA connection context.
  struct request *rq = bd->rq;

  //
  // TO BE DONE
  //

  printk("rmem_queue_rq: get a request.  \n");

out:
  return ret;

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
	.map_queues     = blk_mq_map_queues,      // Map hardware dispatch queues and available cores.
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
 * [?] Used to initialize hardware dispatch queue
 * 
 * [?] For the staging queue, set the information within it directly ?
 * 
 */
static int init_blk_mq_tag_set(struct rmem_device_control* rmem_dev_ctrl){

  struct blk_mq_tag_set* tag_set = &(rmem_dev_ctrl->tag_set);
  int err = 0;

  if(!tag_set){
    printk("init_blk_mq_tag_set : pass a null pointer in. \n");
    err = -1;
    goto out;
  }

  tag_set->ops = &rmem_mq_ops;
  tag_set->nr_hw_queues = rmem_dev_ctrl->nr_queues;   // hardware dispatch queue == software staging queue == avaible cores
  tag_set->queue_depth = rmem_dev_ctrl->queue_depth;  // [?] staging / dispatch queues have the same queue depth ?? or only staging queues have queue depth ?? 
  tag_set->numa_node  = NUMA_NO_NODE;
  tag_set->cmd_size = sizeof(struct rmem_rdma_request);     // [?] Send a rdma_request with the normal i/o request to remote memory server ??
  tag_set->flags = BLK_MQ_F_SHOULD_MERGE;                   // [?]merge the i/o requets ??
  tag_set->driver_data = rmem_dev_ctrl;                     // The driver controller, the context 


  err = blk_mq_alloc_tag_set(tag_set);      // Check & correct the value within the blk_mq_tag_set.
	if (err){
    pr_err("blk_mq_alloc_tag_set error. \n");
		goto out;
  }


out:
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
  sector_t remote_mem_size = RMEM_SIZE_IN_BYTES;

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
  memcpy(rmem_dev_ctrl->disk->disk_name, rmem_dev_ctrl->dev_name, DEVICE_NAME_LEN);

  sector_div(remote_mem_size, RMEM_SECT_SIZE);            // remote_mem_size /=RMEM_SECT_SIZE, return remote_mem_size%RMEM_SECT_SIZE 
	set_capacity(rmem_dev_ctrl->disk, remote_mem_size);     // size is in remote file state->size, add size info into block device
 
   //debug
  printk("init_gendisk : initialize disk %s done. \n", rmem_dev_ctrl->disk->disk_name);

alloc_disk:
  blk_cleanup_queue(rmem_dev_ctrl->queue);

  return ret;
}

/**
 * Initialize the fields of Driver controller/context.
 * 
 */
int init_rmem_device_control(char* dev_name, struct rmem_device_control* rmem_dev_ctrl){
  
  int ret = 0;

  // if driver controller is null, create a new driver context.
  //if(!rmem_dev_ctrl){

    //
    // [?] Can we don the allocation during kernel booting time ??
    // 
    //rmem_dev_ctrl = (struct rmem_device_control *)kzalloc(sizeof(struct rmem_device_control), GFP_KERNEL);  // kzlloc, initialize memory to zero.
    if(rmem_dev_ctrl == NULL){
      ret = -1;
      pr_err("Allocate struct rmem_device_control failed \n");
      goto out;
    }
 // }

  if(dev_name != NULL ){
    //Use assigned name
    int len = strlen(dev_name) >= DEVICE_NAME_LEN ? DEVICE_NAME_LEN : strlen(dev_name);
    memcpy(rmem_dev_ctrl->dev_name, dev_name, len);   // string copy or memory copy ?
  }else{
    strcpy(rmem_dev_ctrl->dev_name,"rmempool");       // rmem_dev_ctrl->dev_name = "xx" is wrong. Try to change the value(address) of a char pointer.
  }

  rmem_dev_ctrl->major = rbd_major_num;

  rmem_dev_ctrl->queue_depth  = RMEM_QUEUE_DEPTH; // 
  rmem_dev_ctrl->nr_queues    = online_cores;     // staging queue = dispatch queue = avaible cores.

  //
  // blk_mq_tag_set, request_queue, rmem_rdma_queue are waiting to be initialized later. 
  //

out:
  return ret;
}




/**
 * The main function to create the block device. 
 * 
 * [?] who assigns the value of rmem_dev_ctrl ??
 * 
 * return 0 if success.
 */
int RMEM_create_device(char* dev_name, struct rmem_device_control* rmem_dev_ctrl ){

  int ret = 0;  
  unsigned long page_size; 

  //
  //  1) Initiaze the fields of rmem_device_control structure 
  //
  if(rmem_dev_ctrl == NULL ){    
    pr_err("Can not pass a null pointer to initialize. \n");
    ret = -1;
    goto out;
  }

  ret = init_rmem_device_control( dev_name, rmem_dev_ctrl);
  if(ret){
    pr_err("Intialize rmem_device_control error \n");
    goto out;
  }

  // 2) Intialize thte blk_mq_tag_set
  ret = init_blk_mq_tag_set(rmem_dev_ctrl);
  if(ret){
    printk("RBD_create_device : Allocate blk_mq_tag_set failed. \n");
   goto out;
  }


  //
  // 3) allocate & intialize the software staging queue
  //
  rmem_dev_ctrl->queue = blk_mq_init_queue(&rmem_dev_ctrl->tag_set);   // Build the block i/o queue.
	if (IS_ERR(rmem_dev_ctrl->queue )) {
		ret = PTR_ERR(rmem_dev_ctrl->queue );
    printk("RBD_create_device : create the software staging reqeust queue failed. \n");
		goto out;
	}
  
  //
  // * set some device queue information
  // i.e. Cat get these information from : /sys/block/sda/queue/*
  // logical block size   : 4KB, The granularity of kernel read/write. From disk buffer to memory ?
  // physical block size  : 512B aligned. The granularity read from hardware disk.  
  // 
  
  
  page_size = PAGE_SIZE;           // alignment to RMEM_SECT_SIZE

  blk_queue_logical_block_size(rmem_dev_ctrl->queue, PAGE_SIZE);          // logical block size, 4KB, performance tuning. 
	blk_queue_physical_block_size(rmem_dev_ctrl->queue, RMEM_SECT_SIZE);    // physical block size, 512B
	sector_div(page_size, RMEM_SECT_SIZE);                  // page_size /=RMEM_SECT_SIZE
	blk_queue_max_hw_sectors(rmem_dev_ctrl->queue, RMEM_QUEUE_MAX_SECT_SIZE);    // [?] 256kb for current /dev/sda

  
  //
  // 4) initiate the gendisk and request queue information
  //
  ret = init_gendisk(rmem_dev_ctrl);
  if(ret){
    pr_err("Init_gendisk failed \n");
    goto out;
  }



  //debug
  printk("RMEM_create_device done. \n");

out:
  return ret;

}





static int  RMEM_init_module(void){

  int ret = 0;
	//debug
  printk("Load kernel module : register remote block device. \n");

	rbd_major_num = register_blkdev(0, "rmempool");
	if (rbd_major_num < 0){
		return rbd_major_num;
  }

  //number of staging and dispatch queues, equal to available cores
  online_cores = num_online_cpus();
  
  //debug
  printk("Got block device major number: %d \n", rbd_major_num);
  printk(" online cores : %d \n", online_cores);
  

  //debug
  // create the block device here
  // Create block information within functions
  
  ret = RMEM_create_device(NULL, &rmem_dev_ctl_global);  
  if(ret){
    pr_err("Crate block device error.\n");
    goto out;
  }


out:
	return ret;
}

// module function
static void RMEM_cleanup_module(void){

	unregister_blkdev(rbd_major_num, "rmempool");

}






module_init(RMEM_init_module);
module_exit(RMEM_cleanup_module);
