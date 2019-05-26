/*
*	We can't invoke the usr space API, ibv_* interfaces, provided by MLNX_OFED directly. 
* Kernel has implemented some kernel space RDMA API which invokes the MLNX_OFED low level driver directly.
* So we need to figure out how to use thes kernel-version-specigic RDMA API.
*/

#include "client.h"


#define DRV_NAME	"Test IB"
#define PFX		DRV_NAME ": "
#define DRV_VERSION	"0.0"

MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("Do RDMA post send/recv operation in kernel mode.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRV_VERSION);


#define MAX_MR_SIZE_GB 32 

enum test_state { 
	IDLE = 1,
	CONNECT_REQUEST,
	ADDR_RESOLVED,
	ROUTE_RESOLVED,
	CONNECTED,		// updated by IS_cma_event_handler()
	FREE_MEM_RECV,
	AFTER_FREE_MEM,
	RDMA_BUF_ADV,   // designed for server
	WAIT_OPS,
	RECV_STOP,
	RECV_EVICT,
	RDMA_WRITE_RUNNING,
	RDMA_READ_RUNNING,
	SEND_DONE,
	RDMA_DONE,
	RDMA_READ_ADV,	// updated by IS_cq_event_handler()
	RDMA_WRITE_ADV,
	CM_DISCONNECT,
	ERROR
};

enum mem_type {
	DMA = 1,
	FASTREG = 2,
	MW = 3,
	MR = 4
};


struct message {
  	uint64_t buf[MAX_MR_SIZE_GB];
  	uint32_t rkey[MAX_MR_SIZE_GB];   // remote key ?  why not a local key ? because this is for receive data ?
  	int size_gb;	
	enum {
		DONE = 1,
		INFO,
		INFO_SINGLE,
		FREE_SIZE,
		EVICT,
		ACTIVITY,
		STOP,
		BIND,
		BIND_SINGLE,
		QUERY
	} type;
};



struct kernel_cb {

	// cm events
	struct rdma_cm_id *cm_id;	/* connection on client side,*/

  // ib events 
  struct ib_cq *cq;
	struct ib_pd *pd;
	struct ib_qp *qp;


  // For infiniband connection rdma_cm operation
  uint16_t port;			/* dst port in NBO */
	u8 addr[16];			/* dst addr in NBO */
  uint8_t addr_type;		/* ADDR_FAMILY - IPv4/V6 */
  int txdepth;			/* SQ depth */  // [?] What's this used for ??

  enum test_state state;		/* used for cond/signalling */
  wait_queue_head_t sem;      // semaphore for wait/wakeup


  // Buffer information
  struct ib_mr *dma_mr;  // [?] receive mr to be registered ??
  enum mem_type mem;

  // Receive buffer
  struct ib_recv_wr rq_wr;	/* recv work request record */
	struct ib_sge recv_sgl;		/* recv single SGE */
  //char recv_buf[8];           // use 8 chars as receive buffer.  
  struct message* recv_buf;
  //DECLARE_PCI_UNMAP_ADDR(recv_mapping)
	struct ib_mr *recv_mr;

  
  struct ib_send_wr sq_wr;	/* send work requrest record */
	struct ib_sge send_sgl;
	//char send_buf[8];
	struct message* send_buf;
	u64 send_dma_addr;
	//DECLARE_PCI_UNMAP_ADDR(send_mapping)
	struct ib_mr *send_mr;

  



};



static int IS_cma_event_handler(struct rdma_cm_id *cma_id, struct rdma_cm_event *event)
 {

	 printk("Fake function \n");
	 
	 return 0;
 }


// The rdma CM handler function
// CMA Event handler  && cq_event_handler , 2 different functions.
// static int IS_cma_event_handler(struct rdma_cm_id *cma_id,
// 				   struct rdma_cm_event *event)
// {
// 	int ret;
// 	struct kernel_cb *cb = cma_id->context;

// 	pr_info("cma_event type %d cma_id %p (%s)\n", event->event, cma_id,
// 		  (cma_id == cb->cm_id) ? "parent" : "child");

// 	switch (event->event) {
// 	case RDMA_CM_EVENT_ADDR_RESOLVED:
// 		cb->state = ADDR_RESOLVED;
// 		ret = rdma_resolve_route(cma_id, 2000);
// 		if (ret) {
// 			printk(KERN_ERR PFX "rdma_resolve_route error %d\n", 
// 			       ret);
// 			wake_up_interruptible(&cb->sem);
// 		}
// 		break;

// 	case RDMA_CM_EVENT_ROUTE_RESOLVED:
// 		cb->state = ROUTE_RESOLVED;

//     printk("Received rdma_cm_event : RDMA_CM_EVENT_ROUTE_RESOLVED, wake up kernel_cb->sem\n ");
// 		wake_up_interruptible(&cb->sem);
// 		break;

// 	case RDMA_CM_EVENT_CONNECT_REQUEST:
// 		cb->state = CONNECT_REQUEST;
// 		//cb->child_cm_id = cma_id;
// 		//pr_info("child cma %p\n", cb->child_cm_id);
// 		//wake_up_interruptible(&cb->sem);

//     printk(KERN_ERR "Not Handle \n");

// 		break;

// 	case RDMA_CM_EVENT_ESTABLISHED:
	
// 		cb->state = CONNECTED;
		
//     pr_info("ESTABLISHED, wake up kernel_cb->sem\n");
//     wake_up_interruptible(&cb->sem);
// 		// last connection establish will wake up the IS_session_create()
		
//     //if (atomic_dec_and_test(&cb->IS_sess->conns_count)) {
// 	  //		pr_debug("%s: last connection established\n", __func__);
// 	  //		complete(&cb->IS_sess->conns_wait);
// 	  //	}
// 		break;

// 	case RDMA_CM_EVENT_ADDR_ERROR:
// 	case RDMA_CM_EVENT_ROUTE_ERROR:
// 	case RDMA_CM_EVENT_CONNECT_ERROR:
// 	case RDMA_CM_EVENT_UNREACHABLE:
// 	case RDMA_CM_EVENT_REJECTED:
// 		printk(KERN_ERR PFX "cma event %d, error %d\n", event->event,
// 		       event->status);
// 		cb->state = ERROR;
// 		wake_up_interruptible(&cb->sem);
// 		break;

// 	case RDMA_CM_EVENT_DISCONNECTED:	//should get error msg from here
// 		printk(KERN_ERR PFX "DISCONNECT EVENT...\n");
// 		cb->state = CM_DISCONNECT;
// 		// RDMA is off
// 		//IS_disconnect_handler(cb);
// 		break;

// 	case RDMA_CM_EVENT_DEVICE_REMOVAL:	//this also should be treated as disconnection, and continue disk swap
// 		printk(KERN_ERR PFX "cma detected device removal!!!!\n");
// 		return -1;
// 		break;

// 	default:
// 		printk(KERN_ERR PFX "oof bad type!\n");
// 		wake_up_interruptible(&cb->sem);
// 		break;
// 	}
// 	return 0;
// }



// Network bind
// socket address ip and port
// static void fill_sockaddr(struct sockaddr_storage *sin, struct kernel_cb *cb)
// {
// 	memset(sin, 0, sizeof(*sin));

// 	if (cb->addr_type == AF_INET) {

//     //debug
//     printk("Use sockaddr_in6\n");

// 		struct sockaddr_in *sin4 = (struct sockaddr_in *)sin;
// 		sin4->sin_family = AF_INET;
// 		memcpy((void *)&sin4->sin_addr.s_addr, cb->addr, 4);   // copy 32bits/ 4bytes from cb->addr to sin4->sin_addr.s_addr
// 		sin4->sin_port = cb->port;                             // assign cb->port to sin4->sin_port
// 	} else if (cb->addr_type == AF_INET6) {

//     //debug
//     printk("Use sockaddr_in6\n");

// 		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sin;
// 		sin6->sin6_family = AF_INET6;
// 		memcpy((void *)&sin6->sin6_addr, cb->addr, 16);
// 		sin6->sin6_port = cb->port;
// 	}
// }


// Resolve the dest ip to the dest IB device.
// Can't use the IP to connect to remote IB derictly ?
// There is a Route Table in local IB driver to record this ?
// Source adress is NULL ?
// static int IS_bind_client(struct kernel_cb *cb)
// {
// 	struct sockaddr_storage sin;
// 	int ret;

// 	fill_sockaddr(&sin, cb);

// 	ret = rdma_resolve_addr(cb->cm_id, NULL, (struct sockaddr *)&sin, 2000); // timeout 2000ms
// 	if (ret) {
// 		printk(KERN_ERR PFX "rdma_resolve_addr error %d\n", ret);
// 		return ret;
// 	}
  
//   // rdma_resolve_addr send a message to remote server pointed by sin.
//   // Remote sever will return a message to CM queue, handled by IS_cma_event_handler, which will wakeup kernel_cb->sem.
// 	wait_event_interruptible(cb->sem, cb->state >= ROUTE_RESOLVED);   //[?] Wait on cb->sem ?? Which process will wake up it.
// 	if (cb->state != ROUTE_RESOLVED) {
// 		printk(KERN_ERR PFX 
// 		       "addr/route resolution did not resolve: state %d\n",
// 		       cb->state);
// 		return -EINTR;
// 	}
// 	printk("rdma_resolve_addr - rdma_resolve_route successful\n");
// 	return 0;
// }

// Create a QP,
// 1) Set attributes
// 2) Bind pd to this qp
// static int IS_create_qp(struct kernel_cb *cb)
// {
// 	struct ib_qp_init_attr init_attr;
// 	int ret;

// 	memset(&init_attr, 0, sizeof(init_attr));
// 	init_attr.cap.max_send_wr = cb->txdepth; /*FIXME: You may need to tune the maximum work request */
// 	init_attr.cap.max_recv_wr = cb->txdepth;  
// 	init_attr.cap.max_recv_sge = 1;
// 	init_attr.cap.max_send_sge = 1;
// 	init_attr.sq_sig_type = IB_SIGNAL_REQ_WR;     // Receive WR ?
// 	init_attr.qp_type = IB_QPT_RC;                // RC Queue Pair ?
// 	init_attr.send_cq = cb->cq;
// 	init_attr.recv_cq = cb->cq;

// 	ret = rdma_create_qp(cb->cm_id, cb->pd, &init_attr);
// 	if (!ret){
// 		cb->qp = cb->cm_id->qp;
//   }else{
//     printk(KERN_ERR "Create QP falied.");
//   }


// 	return ret;
// }



// Kernel structure
//struct ib_cq_init_attr {
//	unsigned int	cqe;
//	int		comp_vector;
//	u32		flags;
//};




// Create Queue Pair : pd, cq, qp, 
// 
// static int IS_setup_qp(struct kernel_cb *cb, struct rdma_cm_id *cm_id)
// {
// 	int ret;

// 	struct ib_cq_init_attr init_attr;
// 	cb->pd = ib_alloc_pd(cm_id->device, IB_ACCESS_LOCAL_WRITE|
//                                             IB_ACCESS_REMOTE_READ|
//                                             IB_ACCESS_REMOTE_WRITE );    // No local read ??  [?] What's the cb->pd used for ?

// 	if (IS_ERR(cb->pd)) {
// 		printk(KERN_ERR PFX "ib_alloc_pd failed\n");
// 		return PTR_ERR(cb->pd);
// 	}
// 	pr_info("created pd %p\n", cb->pd);

// 	memset(&init_attr, 0, sizeof(init_attr));
// 	init_attr.cqe = cb->txdepth * 2;     //[?]  completion queue depth ??
// 	init_attr.comp_vector = 0;
	
// 	cb->cq = ib_create_cq(cm_id->device, rdma_cq_event_handler, NULL, cb, &init_attr);

// 	if (IS_ERR(cb->cq)) {
// 		printk(KERN_ERR PFX "ib_create_cq failed\n");
// 		ret = PTR_ERR(cb->cq);
// 		goto err1;
// 	}
// 	pr_info("created cq %p\n", cb->cq);

//   // Request a notification, if an event arrives on CQ ?
// 	ret = ib_req_notify_cq(cb->cq, IB_CQ_NEXT_COMP);   
// 	if (ret) {
// 		printk(KERN_ERR PFX "ib_create_cq failed\n");
// 		goto err2;
// 	}

// 	ret = IS_create_qp(cb);
// 	if (ret) {
// 		printk(KERN_ERR PFX "IS_create_qp failed: %d\n", ret);
// 		goto err2;
// 	}
// 	pr_info("created qp %p\n", cb->qp);
// 	return 0;

// err2:
// 	//ib_destroy_cq(cb->cq);
//   printk(KERN_ERR "Error in IS_setup_qp \n");
// err1:
// 	//ib_dealloc_pd(cb->pd);
//   printk(KERN_ERR "Error in IS_setup_qp \n");
// 	return ret;
// }



// [?] What's the purpose of this function ?
// Assign cb->qp info to cb field ??
// static void IS_setup_wr(struct kernel_cb *cb)
// {
// 	cb->recv_sgl.addr = cb->recv_dma_addr;
// 	cb->recv_sgl.length = sizeof cb->recv_buf;
// 	if (cb->local_dma_lkey)
// 		cb->recv_sgl.lkey = cb->qp->device->local_dma_lkey;
// 	else if (cb->mem == DMA)
// 		cb->recv_sgl.lkey = cb->dma_mr->lkey;
// 	cb->rq_wr.sg_list = &cb->recv_sgl;
// 	cb->rq_wr.num_sge = 1;

// 	cb->send_sgl.addr = cb->send_dma_addr;
// 	cb->send_sgl.length = sizeof cb->send_buf;
// 	if (cb->local_dma_lkey)
// 		cb->send_sgl.lkey = cb->qp->device->local_dma_lkey;
// 	else if (cb->mem == DMA)
// 		cb->send_sgl.lkey = cb->dma_mr->lkey;
// 	cb->sq_wr.opcode = IB_WR_SEND;
// 	cb->sq_wr.send_flags = IB_SEND_SIGNALED;
// 	cb->sq_wr.sg_list = &cb->send_sgl;
// 	cb->sq_wr.num_sge = 1;

// }



// Bind the receive buffer to IB
// RDMA ? DMA ?
// 
// static int IS_setup_buffers(struct kernel_cb *cb)
// {
// 	int ret;

// 	pr_info(PFX "IS_setup_buffers called on cb %p\n", cb);

// 	pr_info(PFX "size of IS_rdma_info %lu\n", sizeof(cb->recv_buf));


// 	cb->recv_dma_addr = dma_map_single(&cb->pd->device->dev, 
// 				   &cb->recv_buf, sizeof(cb->recv_buf), DMA_BIDIRECTIONAL);

// 	pci_unmap_addr_set(cb, recv_mapping, cb->recv_dma_addr);


// 	cb->send_dma_addr = dma_map_single(&cb->pd->device->dev, 
// 				   &cb->send_buf, sizeof(cb->send_buf), DMA_BIDIRECTIONAL);	

// 	pci_unmap_addr_set(cb, send_mapping, cb->send_dma_addr);
// 	pr_info(PFX "cb->mem=%d \n", cb->mem);

// 	if (cb->mem == DMA) {
// 		pr_info(PFX "IS_setup_buffers, in cb->mem==DMA \n");
// 		cb->dma_mr = cb->pd->device->get_dma_mr(cb->pd, IB_ACCESS_LOCAL_WRITE|
// 							        IB_ACCESS_REMOTE_READ|
// 							        IB_ACCESS_REMOTE_WRITE);

// 		if (IS_ERR(cb->dma_mr)) {
// 			pr_info(PFX "reg_dmamr failed\n");
// 			ret = PTR_ERR(cb->dma_mr);
// 			goto bail;
// 		}
// 	} 
	
// 	IS_setup_wr(cb);
// 	pr_info(PFX "allocated & registered buffers...\n");
// 	return 0;
// bail:

// 	if (cb->rdma_mr && !IS_ERR(cb->rdma_mr))
// 		ib_dereg_mr(cb->rdma_mr);
// 	if (cb->dma_mr && !IS_ERR(cb->dma_mr))
// 		ib_dereg_mr(cb->dma_mr);
// 	if (cb->recv_mr && !IS_ERR(cb->recv_mr))
// 		ib_dereg_mr(cb->recv_mr);
// 	if (cb->send_mr && !IS_ERR(cb->send_mr))
// 		ib_dereg_mr(cb->send_mr);
	
// 	return ret;
// }





int main(int argc, char* argv[]){


  struct rdma_cm_id *cm_id;   // device ID ?


  struct kernel_cb * cb = (struct kernel_cb *)kzalloc(sizeof(struct kernel_cb), GFP_KERNEL);

  //1) init kernel_cb


  //[?] How to define the init_net ??
  // @net: The network namespace in which to create the new id.
  // [?] Bind a event_handler to the IB deivce ??
  cb->cm_id = rdma_create_id(NULL, IS_cma_event_handler, cb, RDMA_PS_TCP, IB_QPT_RC);  // TCP, RC, reliable IB connection 
  cb->state = IDLE;

  // The number of  on-the-fly wr ?? every cores handle one ?? 
  cb->txdepth = 256 * num_online_cpus() + 1; //[?] What's this used for ? What's meaning of the value ?
  

  // Setup socket information
  cb->port=htons((uint16_t)9400);
  in4_pton("10.0.0.2", -1, cb->addr, -1, NULL);   // char* to ipv4 address ?
  printk("kernel_cb->port: %d , kernel_cb->addr : %s \n",cb->port, cb->addr);

  cb->addr_type = AF_INET;  //ipv4

  //put a flag into the receive buffer
  // GFP_DMA - Allocation suitable for DMA. Should only be used for kmalloc caches. Otherwise, use a slab created with SLAB_DMA ??
  cb->recv_buf = kmalloc(sizeof(struct message),GFP_DMA);
	//debug
	cb->recv_buf->type = 1;

	cb->send_buf = kmalloc(sizeof(struct message),GFP_DMA);
	cb->mem = DMA;

  // Initialize the queue.
  init_waitqueue_head(&cb->sem);

  printk("Initialized kernel_cb\n");


  //2) Bind to remote server
  // int ret;
  // ret = IS_bind_client(cb);
	// if (ret){
	// 	printk ("bind socket error \n");
  //   return ret;
  // }else{
  //   printk("Bind to remote server.\n");
  // }


  // 3) Create the QP
  //ret = IS_setup_qp(cb, cb->cm_id);

  //if(ret){

  //}

  //4) Bing a Buffer to IB
  //ret = IS_setup_buffers(cb);

	return 0;
}



// invoked by insmod 
static int __init IS_init_module(void)
{
  printk("Init self build kernel. \n");

	printk("Do nothing for now. \n");

	return 0;
}

// invoked by rmmod 
static void __exit IS_cleanup_module(void)
{

  printk("!! Do nothing at this moment. Pleas restart.\n");

}

module_init(IS_init_module);
module_exit(IS_cleanup_module);