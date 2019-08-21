/**
 * README
 * 
 * This is the RDMA client.
 * Use the daemon part of InfiniSwap as Remote Memory Server.
 * 
 * We can't invoke the usr space API, ibv_* interfaces, provided by MLNX_OFED directly. 
 * Kernel has implemented some kernel space RDMA API which invokes the MLNX_OFED low level driver directly.
 *		i.e. invoke into the mlx4 libraries. 
 * So we need to figure out how to use thes kernel-version-specigic RDMA API.
 */

#ifndef RDMA_CLIENT_H
#define RDMA_CLIENT_H

// Kernel development
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

// Block Layer
#include <linux/blk-mq.h>
#include <linux/blkdev.h>

// RDMA related
#include <linux/pci-dma.h>
#include <linux/inet.h>
#include <rdma/ib_verbs.h>
#include <rdma/rdma_cm.h>


#endif  /* RDMA_CLIENT_H */


MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("Do RDMA post send/recv operation in kernel mode.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0");






//
// Disk hardware information
//
#define RMEM_PHY_SECT_SIZE					512 	// physical sector seize, used by driver (to disk).
#define RMEM_LOGICAL_SECT_SIZE			4096	// logical sector seize, used by kernel (to i/o).
//#define RMEM_REQUEST_QUEUE_NUM     2  	// for debug, use the online_cores
#define RMEM_QUEUE_DEPTH           	16  	// [?]  1 - (-1U), what's the good value ? 
#define RMEM_QUEUE_MAX_SECT_SIZE		1024 	// The max number of sectors per request, /sys/block/sda/queue/max_hw_sectors_kb is 256
#define DEVICE_NAME_LEN							32


//
// RDMA related macros  
//

#define RMEM_SIZE_IN_BYTES  (unsigned long)1024*1024*1024*8  // 8GB
#define MAX_MR_SIZE_GB 32 
#define RDMA_READ_WRITE_QUEUE_DEPTH	16		// [?] connection with the Disk dispatch queue depth ??




// Enable debug information printing 
#define DEBUG_RDMA_CLIENT 1 



/**
 * Used for message passing control
 * For both CM event, data evetn.
 * RDMA data transfer is desinged in an asynchronous style. 
 */
enum rdma_session_context_state { 
	IDLE = 1,		 // Start from 1.
	CONNECT_REQUEST,
	ADDR_RESOLVED,
	ROUTE_RESOLVED,
	CONNECTED,		//5,  updated by IS_cma_event_handler()
	FREE_MEM_RECV,
	AFTER_FREE_MEM,
	RDMA_BUF_ADV,   // designed for server
	WAIT_OPS,
	RECV_STOP,    	// 10
	RECV_EVICT,
	RDMA_WRITE_RUNNING,
	RDMA_READ_RUNNING,
	SEND_DONE,
	RDMA_DONE,     	//15
	RDMA_READ_ADV,	// updated by IS_cq_event_handler()
	RDMA_WRITE_ADV,
	CM_DISCONNECT,
	ERROR,

	//self defined
	SEND_MESSAGE   
};



enum mem_type {
	DMA = 1,
	FASTREG = 2,
	MW = 3,
	MR = 4
};

/**
 * RDMA message structure.
 * We use 2-sieded RDMA communication to exchange information between Client and Server.
 * Both Client and Server have the same message structure. 
 */
struct message {
  	uint64_t buf[MAX_MR_SIZE_GB];		// Remote addr ??
  	uint32_t rkey[MAX_MR_SIZE_GB];   	// remote key
  	int size_gb;	
	enum {
		DONE = 1,
		INFO,
		INFO_SINGLE,
		FREE_SIZE,
		EVICT,        // 5
		ACTIVITY,
		STOP,
		BIND,
		BIND_SINGLE,
		QUERY         // 10
	} type;
};



enum chunk_list_state {
	C_IDLE,
	C_READY,
	C_EVICT,
	C_STOP,
	// C_OFFLINE	
};


// 1GB remote chunk struct	("chunk": we use the term "slab" in our paper)
struct remote_chunk_g {
	uint32_t remote_rkey;		/* remote guys RKEY */
	uint64_t remote_addr;		/* remote guys TO */
	//uint64_t remote_len;		/* remote guys LEN */
	int *bitmap_g;	//1GB bitmap
};


struct remote_chunk_g_list {
	struct remote_chunk_g **chunk_list;
	atomic_t *remote_mapped; 
	int chunk_size_g; //size = chunk_num * ONE_GB
	int target_size_g; // == future size of remote
	int shrink_size_g;
	int *chunk_map;	//cb_chunk_index to session_chunk_index
	struct task_struct *evict_handle_thread;
	char *evict_chunk_map;
	wait_queue_head_t sem;      	              //[?] What's the use for this semaphore ?
	enum chunk_list_state c_state;
};










//
// ################################ 			Structure definition of RDMA 		   ###################################
//
// This is the kernel level RDMA over IB.  
// Need to use the API defined in kernel headers. 
//

/**
 * Used to manage each RDMA message.
 * One rdma_context for each RDMA message.
 * 		i.e. Have registered RDMA buffer, used to send a wr to the QP.
 * 
 * Fields
 * 		ib_sge		: Store the data to be sent to remote memory.
 * 		ib_rdma_wr 	:
 */
struct rdma_context{

//	struct IS_connection 	*IS_conn;
//	struct free_ctx_pool 	*free_ctxs;  	// points to its upper structure, free_ctx_pool.
	//struct mutex ctx_lock;	

	struct ib_rdma_wr 	rdma_sq_wr;			// wr for RDMA write/send 

	struct ib_sge 		rdma_sgl;			// Points to the data addres
	char 				*rdma_buf;			// 
	u64  				rdma_dma_addr;		// DMA address of rdma_buf
	DECLARE_PCI_UNMAP_ADDR(rdma_mapping)    // [?]

	struct ib_mr 		*rdma_mr;			// The memory region 
	struct request 		*req;

	struct rmem_device_control 		*rmem_ctx;		// disk driver_data, blk_mq_ops->driver_data
	unsigned long 					offset;
	unsigned long 					len;


	int 				chunk_index;
//	struct remote_chunk_g 			*chunk_ptr;
//	atomic_t in_flight; //true = 1, false = 0

};



/**
 * Assign a rdma_queue to each dispatch queue to handle the poped request.
 * 
 * 
 * Fields
 * 		struct rdma_context* rdma_ctx_queue,  Prepare a rdma_context for each i/o request in the dispatch queue ?
 * 												RDMA meesage is also used in an asyncrhonous way ? is there a limitation ?	
 * 			
 * 
 */
struct rdma_queue {


	struct rdma_context* rdma_ctx_queue; 

};



/**
 * Mange the RDMA connection to a remote server. 
 * 
 * 	
 * Fields
 * 		rdma_cm_id		:  the RDMA device ?
 * 		Queue Pair (QP) : 
 * 		Completion Queue (CQ) :
 * 		
 */
struct rdma_session_context {

	// cm events
	struct rdma_cm_id *cm_id;	// IB device information ?

  	// ib events 
  	struct ib_cq *cq;
	struct ib_pd *pd;
	struct ib_qp *qp;


  // For infiniband connection rdma_cm operation
  	uint16_t port;			/* dst port in NBO */
	u8 addr[16];			/* dst addr in NBO */
  	uint8_t addr_type;		/* ADDR_FAMILY - IPv4/V6 */
  	int txdepth;			/* SQ depth */  // [?] receive and send queue depth  use this same depth ?? CQ entry depth x2 +1??

  	enum rdma_session_context_state 	state;		/* used for cond/signalling */
  	wait_queue_head_t 					sem;      	// semaphore for wait/wakeup

	//
  	// 3) DMA buffer for controll message
  	//
  	// DMA Receive buffer
  	struct ib_recv_wr rq_wr;	/* recv work request record */
	struct ib_sge recv_sgl;		/* recv single SGE */
  //char recv_buf[8];           // use 8 chars as receive buffer.  
  	struct message* recv_buf;
  	u64 recv_dma_addr;
	DECLARE_PCI_UNMAP_ADDR(recv_mapping)
	//struct ib_mr *recv_mr;

  	// DMA send buffer
	// ib_send_wr, used for posting a two-sided RDMA message 
	// ib_rdma_wr, used for posting a one-sided RDMA message 
	// 		=> remote_addr, rkey 
  	struct ib_send_wr sq_wr;	/* send work requrest record */
	struct ib_sge send_sgl;
	//char send_buf[8];
	struct message* send_buf;
	u64 send_dma_addr;
	DECLARE_PCI_UNMAP_ADDR(send_mapping)
	//struct ib_mr *send_mr;


	// Unknown porpuse ??
	//
	struct ib_mr *dma_mr;  // [?] receive mr to be registered ??
  	enum mem_type mem;		//  only when mem == DMA, we need allocate and intialize dma_mr.
	    // [?] How to set this value ?
	int local_dma_lkey;		/* use 0 for lkey */



	// For RDMA read/write
//	struct ib_mr *rdma_mr;

//  	uint64_t remote_len;		/* remote guys LEN */
	struct remote_chunk_g_list remote_chunk;    

};



//
// ################################## Below is the structure definition of Block Device ####################################
//


// Driver data for each request
// Attach this struct at the end of request and send it to the device.
// 	  i.e.	request will reserve size for this request command : requet->cmd_size
//					Acccess by : blk_mq_rq_to_pdu(request)
struct rmem_rdma_cmd {
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
 * Driver data for dispatch queue context(hctx).
 * One rmem_rdma_queue per hctx.
 * 		i.e. add to	 blk_mq_hw_ctx->driver_data
 * 
 * [?] We can store all the RDMA connection information here.
 * 
 */
struct rmem_rdma_queue {
	struct rmem_rdma_cmd	      			*rmem_conn;				// RDMA session connection 
	struct rmem_device_control	      *rmem_dev_ctrl;  	// pointer to parent, the device driver 

	// other fields

};






/**
 * Block device information
 * This structure is the driver context data.  
 * 		i.e. blk_mq_tag_set->driver_data
 * 
 * 
 * [?] One rbd_device_control should record all the connection_queues ??
 * 
 * [?] Finnaly, we need pass all these fields to  tag_set,  htcx, rdma_connections ?
 * 
 * 
 */
struct rmem_device_control {
	//int			     		fd;   // [?] Why do we need a file handler ??
	int			     			major; /* major number from kernel */
	//struct r_stat64		     stbuf; /* remote file stats*/
	//char			     						file_name[DEVICE_NAME_LEN];       // [?] Do we need a file name ??
	//struct list_head	     		list;           /* next node in list of struct IS_file */    // Why do we need such a list ??
	struct gendisk		    		*disk;        // [?] The disk information, logical/physical sectior size ? 
	struct request_queue	    *queue; 			// The software staging request queue
	struct blk_mq_tag_set	    tag_set;			// Used for information passing. Define blk_mq_ops.(block_device_operations is defined in gendisk.)
	
  //[?] What's  this queue used for ?
  struct rmem_rdma_queue	  *rdma_queues;			//  [?] The rdma connection session ?? one rdma session queue per software staging queue ??
	unsigned int		      		queue_depth;      //[?] How to set these numbers ?
	unsigned int		      		nr_queues;				// [?] pass to blk_mq_tag_set->nr_hw_queues ??
	//int			              		index; /* drive idx */
	char			            		dev_name[DEVICE_NAME_LEN];
	//struct rdma_connection	    **IS_conns;
	//struct config_group	     dev_cg;
	spinlock_t		        		rmem_ctl_lock;					// mutual exclusion
	//enum IS_dev_state	     state;	

	// Below fields are used for debug.
	//
	struct block_device *bdev_raw;			// Points to a local block device. 
	struct bio_list bio_list;
};







/**
 * ######## Resource Free Functions ##########
 */

static void octopus_free_qp(struct rdma_session_context *rdma_session)
{

	if (rdma_session == NULL)
		return;

	if(rdma_session->qp != NULL)
		ib_destroy_qp(rdma_session->qp);
	
	if(rdma_session->cq != NULL)
		ib_destroy_cq(rdma_session->cq);

	if(rdma_session->pd != NULL)
		ib_dealloc_pd(rdma_session->pd);
}



static int octopus_disconenct_and_collect_resource(struct rdma_session_context *rdma_session){

	int ret;

	ret = rdma_disconnect(rdma_session);
	if(ret){
		printk(KERN_ERR "%s, RDMA disconnect failed. \n",__func__);
	}

	// Free hardware resource.
		// !! TO BE DONE.

	return ret;
}



// If invoke this funtion after exiting  Remote Memory Server,
// it will cause kernel crash.
// static void IS_free_buffers(struct kernel_cb *cb)
// {
// 	if(cb != NULL){
// 		if (cb->dma_mr)
// 			ib_dereg_mr(cb->dma_mr);
// 		if (cb->send_mr)
// 			ib_dereg_mr(cb->send_mr);
// 		if (cb->recv_mr)
// 			ib_dereg_mr(cb->recv_mr);
// 		if (cb->rdma_mr)
// 			ib_dereg_mr(cb->rdma_mr);
	

// 		dma_unmap_single(&cb->pd->device->dev,
// 			 pci_unmap_addr(cb, recv_mapping),
// 			 sizeof(struct message), DMA_BIDIRECTIONAL);
// 		dma_unmap_single(&cb->pd->device->dev,
// 			 pci_unmap_addr(cb, send_mapping),
// 			 sizeof(struct message), DMA_BIDIRECTIONAL);
// 	}

// }






/**
 * ########## Function declaration ##########
 */
static	int 	octopus_RDMA_connect(struct rdma_session_context * rdma_session);
static 	void 	octopus_cq_event_handler(struct ib_cq * cq, void *rdma_session_context);
static 	int 	octopus_rdma_cm_event_handler(struct rdma_cm_id *cma_id, struct rdma_cm_event *event);





/** 
 * ########## Declare some global varibles ##########
 */

// Initialize in main().
// One rdma_session_context per memory server connected by IB. 
struct rdma_session_context * rdma_session_global;
static struct rmem_device_control  rmem_dev_ctl_global;



static int rmem_major_num;
static int online_cores;







/**
 * ########## Debug functions ##########
 * 
 */
	   
// Print the string of rdma_session_context state.
void rdma_session_context_state_print(int id){

	switch (id){

		case 1 :
			printk("IDLE \n");
			break;
		case 2 :
			printk("CONNECT_REQUEST \n");
			break;
		case 3 :
			printk("ADDR_RESOLVED \n");
			break;
		case 4 :
			printk("ROUTE_RESOLVED \n");
			break;
		case 5 :
			printk("CONNECTED \n");
			break;
		case 6 :
			printk("FREE_MEM_RECV \n");
			break;
		case 7 :
			printk("AFTER_FREE_MEM \n");
			break;
		case 8 :
			printk("RDMA_BUF_ADV \n");
			break;
		case 9 :
			printk("WAIT_OPS \n");
			break;
		case 10 :
			printk("RECV_STOP \n");
			break;
		case 11 :
			printk("RECV_EVICT \n");
			break;
		case 12 :
			printk("RDMA_WRITE_RUNNING \n");
			break;
		case 13 :
			printk("RDMA_READ_RUNNING \n");
			break;
		case 14 :
			printk("SEND_DONE \n");
			break;
		case 15 :
			printk("RDMA_DONE \n");
			break;
		case 16 :
			printk("RDMA_READ_ADV \n");
			break;
		case 17 :
			printk("RDMA_WRITE_ADV \n");
			break;
		case 18 :
			printk("CM_DISCONNECT \n");
			break;
		case 19 :
			printk("ERROR \n");
			break;
	
		default :
			printk("Un-defined rdma_session_context state. \n");
			break;
	}

	return;
}


