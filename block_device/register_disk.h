/**
 * Register a Remote Block Device under current OS.
 * 
 * The connection is based on RDMA over InfiniBand.
 * 
 */




#define RMEM_SECT_SIZE							512 // disk sector size, bytes ??
//#define RMEM_REQUEST_QUEUE_NUM     2  // for debug, use the online_cores
#define RMEM_QUEUE_DEPTH           	16  // [?]  1 - (-1U), what's the good value ? 
#define RMEM_SIZE_IN_BYTES					2014*1024*1024*8  // 8GB


static int rbd_major_num;
static int online_cores;


/**
 * Used for storing RDMA connection information 
 * 
 * 
 */
struct rmem_rdma_connection	{

  int tmp;  // empty.

};


// Send with the i/o request to the remote memory server ??
//
struct rmem_rdma_request {
	//struct nvme_request	req;
	struct ib_mr		*mr;
	//struct nvme_rdma_qe	sqe;
	//struct ib_sge		sge[1 + NVME_RDMA_MAX_INLINE_SEGMENTS];
	u32			num_sge;
	int			nents;
	bool			inline_data;
	struct ib_reg_wr	reg_wr;
	struct ib_cqe		reg_cqe;
	//struct nvme_rdma_queue  *queue;
	struct sg_table		sg_table;
	//struct scatterlist	first_sgl[];
};



/**
 * [?] What's the purpose of this queue ??
 * used for RDMA connection
 * 
 */
struct rmem_rdma_queue {
	unsigned int		                  queue_depth;
	struct rmem_rdma_connection	      *rbd_conn;

	struct rmem_device_control	        *rbd_ctrl; /* pointer to parent, the remote bd controll */
};






/**
 * Block device information
 * 
 * 
 * [?] One rbd_device_control should record all the connection_queues ??
 * 
 */
struct rmem_device_control {
	int			     							fd;
	int			     							major; /* major number from kernel */
	//struct r_stat64		     stbuf; /* remote file stats*/
	char			     						file_name[MAX_IS_DEV_NAME];       // [?] Do we need a file name ??
	struct list_head	     		list;           /* next node in list of struct IS_file */    // Why do we need such a list ??
	struct gendisk		    		*disk;           // [?] the real disk ?? For NBD, points to ?
	struct request_queue	    *queue; /* The device request queue */
	struct blk_mq_tag_set	    tag_set;
	
  //[?] What's  this queue used for ?
  struct rmem_rdma_queue	    		*queues;
	unsigned int		      	queue_depth;      //[?] How to set these numbers ?
	unsigned int		      	nr_queues;
	int			              	index; /* drive idx */
	char			            	dev_name[MAX_IS_DEV_NAME];
	//struct rdma_connection	    **IS_conns;
	//struct config_group	     dev_cg;
	spinlock_t		        	state_lock;
	//enum IS_dev_state	     state;	
};




/**
 * Store all the information need for Block device, RDMA connection ??
 * 
 */
struct rmem_control {

  // information for the remote block device
  struct rmem_device_control* rmem_deb;

  // information for RDMA


};


