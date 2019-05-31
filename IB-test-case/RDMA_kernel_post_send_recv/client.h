#ifndef CLIENT_H
#define CLIENT_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

//DMA related
#include <linux/pci-dma.h>

//network
#include <linux/inet.h>

//rdma
#include <rdma/ib_verbs.h>
#include <rdma/rdma_cm.h>


#endif  /* CLIENT_H */

#define DRV_NAME	"Test IB"
#define PFX		DRV_NAME ": "
#define DRV_VERSION	"0.0"

MODULE_AUTHOR("Chenxi Wang");
MODULE_DESCRIPTION("Do RDMA post send/recv operation in kernel mode.");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION(DRV_VERSION);


#define MAX_MR_SIZE_GB 32 



static void rdma_cq_event_handler(struct ib_cq * cq, void *ctx);





enum test_state { 
	IDLE = 1,
	CONNECT_REQUEST,
	ADDR_RESOLVED,
	ROUTE_RESOLVED,
	CONNECTED,		//5,  updated by IS_cma_event_handler()
	FREE_MEM_RECV,
	AFTER_FREE_MEM,
	RDMA_BUF_ADV,   // designed for server
	WAIT_OPS,
	RECV_STOP,    // 10
	RECV_EVICT,
	RDMA_WRITE_RUNNING,
	RDMA_READ_RUNNING,
	SEND_DONE,
	RDMA_DONE,     //15
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


struct message {
  	uint64_t buf[MAX_MR_SIZE_GB];
  	uint32_t rkey[MAX_MR_SIZE_GB];   // remote key ?  why not a local key ? because this is for receive data ?
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

  // DMA Receive buffer
  struct ib_recv_wr rq_wr;	/* recv work request record */
	struct ib_sge recv_sgl;		/* recv single SGE */
  //char recv_buf[8];           // use 8 chars as receive buffer.  
  struct message* recv_buf;
  u64 recv_dma_addr;
	DECLARE_PCI_UNMAP_ADDR(recv_mapping)
	struct ib_mr *recv_mr;

  // DMA send buffer
  struct ib_send_wr sq_wr;	/* send work requrest record */
	struct ib_sge send_sgl;
	//char send_buf[8];
	struct message* send_buf;
	u64 send_dma_addr;
	DECLARE_PCI_UNMAP_ADDR(send_mapping)
	struct ib_mr *send_mr;

  // [?] How to set this value ?
	int local_dma_lkey;		/* use 0 for lkey */


	// For RDMA read/write
	struct ib_mr *rdma_mr;

  uint64_t remote_len;		/* remote guys LEN */
	struct remote_chunk_g_list remote_chunk;     

};




static void IS_free_qp(struct kernel_cb *cb)
{

	if (cb == NULL)
		return;

	if(cb->qp != NULL)
		ib_destroy_qp(cb->qp);
	
	if(cb->cq != NULL)
		ib_destroy_cq(cb->cq);

	if(cb->pd != NULL)
		ib_dealloc_pd(cb->pd);
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


