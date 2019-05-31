/*
*	We can't invoke the usr space API, ibv_* interfaces, provided by MLNX_OFED directly. 
* Kernel has implemented some kernel space RDMA API which invokes the MLNX_OFED low level driver directly.
* So we need to figure out how to use thes kernel-version-specigic RDMA API.
*/

#include "client.h"



// External symbols
//extern struct rdma_cm_id *rdma_create_id(struct net *net, rdma_cm_event_handler event_handler, void *context, enum rdma_port_space ps, enum ib_qp_type qp_type);





// Structure defined in OFED driver
//
// struct ib_pd {
//   u32     local_dma_lkey;
//   u32     flags;
//   struct ib_device       *device;
//   struct ib_uobject      *uobject;
//   atomic_t            usecnt; /* count all resources */

//   u32     unsafe_global_rkey;

//   /*
//    * Implementation details of the RDMA core, don't use in drivers:
//    */
//   struct ib_mr         *__internal_mr;
//   struct rdma_restrack_entry res; 
// };








// The rdma CM handler function
// CMA Event handler  && cq_event_handler , 2 different functions.
static int IS_cma_event_handler(struct rdma_cm_id *cma_id,
				   struct rdma_cm_event *event)
{
	int ret;
	struct kernel_cb *cb = cma_id->context;

	pr_info("cma_event type %d cma_id %p (%s)\n", event->event, cma_id,
		  (cma_id == cb->cm_id) ? "parent" : "child");

	switch (event->event) {
	case RDMA_CM_EVENT_ADDR_RESOLVED:
		cb->state = ADDR_RESOLVED;
		ret = rdma_resolve_route(cma_id, 2000);
		if (ret) {
			printk(KERN_ERR PFX "rdma_resolve_route error %d\n", 
			       ret);
			wake_up_interruptible(&cb->sem);
		}
		break;

	case RDMA_CM_EVENT_ROUTE_RESOLVED:
		cb->state = ROUTE_RESOLVED;

    printk("Received rdma_cm_event : RDMA_CM_EVENT_ROUTE_RESOLVED, wake up kernel_cb->sem\n ");
		wake_up_interruptible(&cb->sem);
		break;

	case RDMA_CM_EVENT_CONNECT_REQUEST:
		cb->state = CONNECT_REQUEST;
		//cb->child_cm_id = cma_id;
		//pr_info("child cma %p\n", cb->child_cm_id);
		//wake_up_interruptible(&cb->sem);

    printk(KERN_ERR "Receive but Not Handle : RDMA_CM_EVENT_CONNECT_REQUEST \n");

		break;

	case RDMA_CM_EVENT_ESTABLISHED:
	
		cb->state = CONNECTED;
		
    pr_info("ESTABLISHED, wake up kernel_cb->sem\n");
    wake_up_interruptible(&cb->sem);
		// last connection establish will wake up the IS_session_create()
		
    //if (atomic_dec_and_test(&cb->IS_sess->conns_count)) {
	  //		pr_debug("%s: last connection established\n", __func__);
	  //		complete(&cb->IS_sess->conns_wait);
	  //	}
		break;

	case RDMA_CM_EVENT_ADDR_ERROR:
	case RDMA_CM_EVENT_ROUTE_ERROR:
	case RDMA_CM_EVENT_CONNECT_ERROR:
	case RDMA_CM_EVENT_UNREACHABLE:
	case RDMA_CM_EVENT_REJECTED:
		printk(KERN_ERR PFX "cma event %d, error %d\n", event->event,
		       event->status);
		cb->state = ERROR;
		wake_up_interruptible(&cb->sem);
		break;

	case RDMA_CM_EVENT_DISCONNECTED:	//should get error msg from here
		printk(KERN_ERR PFX "DISCONNECT EVENT...\n");
		cb->state = CM_DISCONNECT;
		// RDMA is off
		//IS_disconnect_handler(cb);
		break;

	case RDMA_CM_EVENT_DEVICE_REMOVAL:	//this also should be treated as disconnection, and continue disk swap
		printk(KERN_ERR PFX "cma detected device removal!!!!\n");
		return -1;
		break;

	default:
		printk(KERN_ERR PFX "oof bad type!\n");
		wake_up_interruptible(&cb->sem);
		break;
	}
	return 0;
}



// Network bind
// socket address ip and port
static void fill_sockaddr(struct sockaddr_storage *sin, struct kernel_cb *cb)
{
	memset(sin, 0, sizeof(*sin));

	if (cb->addr_type == AF_INET) {

    //debug
    printk("Use sockaddr_in6\n");

		struct sockaddr_in *sin4 = (struct sockaddr_in *)sin;
		sin4->sin_family = AF_INET;
		memcpy((void *)&sin4->sin_addr.s_addr, cb->addr, 4);   // copy 32bits/ 4bytes from cb->addr to sin4->sin_addr.s_addr
		sin4->sin_port = cb->port;                             // assign cb->port to sin4->sin_port
	} else if (cb->addr_type == AF_INET6) {

    //debug
    printk("Use sockaddr_in6\n");

		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sin;
		sin6->sin6_family = AF_INET6;
		memcpy((void *)&sin6->sin6_addr, cb->addr, 16);
		sin6->sin6_port = cb->port;
	}
}


// Resolve the dest ip to the dest IB device.
// Can't use the IP to connect to remote IB derictly ?
// There is a Route Table in local IB driver to record this ?
// Source adress is NULL ?
// [?] Only to resolve ip:port, but not built the TCP-IP connection ?
static int IS_bind_client(struct kernel_cb *cb)
{
	struct sockaddr_storage sin;
	int ret;

	fill_sockaddr(&sin, cb);

	ret = rdma_resolve_addr(cb->cm_id, NULL, (struct sockaddr *)&sin, 2000); // timeout 2000ms
	if (ret) {
		printk(KERN_ERR PFX "rdma_resolve_addr error %d\n", ret);
		return ret;
	}else{
		printk("IS_bind_client - rdma_resolve_addr success.\n");
	}
  
  // rdma_resolve_addr send a message to remote server pointed by sin.
  // Remote sever will return a message to CM queue, handled by IS_cma_event_handler, which will wakeup kernel_cb->sem.
	wait_event_interruptible(cb->sem, cb->state >= ROUTE_RESOLVED);   //[?] Wait on cb->sem ?? Which process will wake up it.
	if (cb->state != ROUTE_RESOLVED) {
		printk(KERN_ERR PFX 
		       "addr/route resolution did not resolve: state %d\n",
		       cb->state);
		return -EINTR;
	}
	printk("IS_bind_client - rdma_resolve_route successful\n");
	return 0;
}

//Create a QP,
//1) Set attributes
//2) Bind pd to this qp
static int IS_create_qp(struct kernel_cb *cb)
{
	struct ib_qp_init_attr init_attr;
	int ret;

	memset(&init_attr, 0, sizeof(init_attr));
	init_attr.cap.max_send_wr = cb->txdepth; /*FIXME: You may need to tune the maximum work request */
	init_attr.cap.max_recv_wr = cb->txdepth;  
	init_attr.cap.max_recv_sge = 1;
	init_attr.cap.max_send_sge = 1;
	init_attr.sq_sig_type = IB_SIGNAL_REQ_WR;     // Receive WR ?
	init_attr.qp_type = IB_QPT_RC;                // RC Queue Pair ?
	init_attr.send_cq = cb->cq;
	init_attr.recv_cq = cb->cq;

	ret = rdma_create_qp(cb->cm_id, cb->pd, &init_attr);
	if (!ret){
		cb->qp = cb->cm_id->qp;
  }else{
    printk(KERN_ERR "Create QP falied.");
  }


	return ret;
}






// Create Queue Pair : pd, cq, qp, 
// And wait for CQ mesesage
static int IS_setup_qp(struct kernel_cb *cb, struct rdma_cm_id *cm_id)
{
	int ret;

	struct ib_cq_init_attr init_attr;
	cb->pd = ib_alloc_pd(cm_id->device, IB_ACCESS_LOCAL_WRITE|
                                            IB_ACCESS_REMOTE_READ|
                                            IB_ACCESS_REMOTE_WRITE );    // No local read ??  [?] What's the cb->pd used for ?

	if (IS_ERR(cb->pd)) {
		printk(KERN_ERR PFX "ib_alloc_pd failed\n");
		return PTR_ERR(cb->pd);
	}
	pr_info("created pd %p\n", cb->pd);

	memset(&init_attr, 0, sizeof(init_attr));
	init_attr.cqe = cb->txdepth * 2;     //[?]  completion queue depth ??
	init_attr.comp_vector = 0;
	
	cb->cq = ib_create_cq(cm_id->device, rdma_cq_event_handler, NULL, cb, &init_attr);

	if (IS_ERR(cb->cq)) {
		printk(KERN_ERR PFX "ib_create_cq failed\n");
		ret = PTR_ERR(cb->cq);
		goto err1;
	}
	pr_info("created cq %p\n", cb->cq);

  // Request a notification, if an event arrives on CQ ?
	ret = ib_req_notify_cq(cb->cq, IB_CQ_NEXT_COMP);   
	if (ret) {
		printk(KERN_ERR PFX "ib_create_cq failed\n");
		goto err2;
	}

	ret = IS_create_qp(cb);
	if (ret) {
		printk(KERN_ERR PFX "IS_create_qp failed: %d\n", ret);
		goto err2;
	}
	pr_info("created qp %p\n", cb->qp);
	return 0;

err2:
	//ib_destroy_cq(cb->cq);
  printk(KERN_ERR "Error in IS_setup_qp \n");
err1:
	//ib_dealloc_pd(cb->pd);
  printk(KERN_ERR "Error in IS_setup_qp \n");
	return ret;
}





// [?] What's the purpose of this function ?
// Assign cb->qp info to cb field ??
static void IS_setup_wr(struct kernel_cb *cb)
{
	cb->recv_sgl.addr = cb->recv_dma_addr;             // bus/dma address
	cb->recv_sgl.length = sizeof(struct message);
	if (cb->local_dma_lkey)                            // check ?
		cb->recv_sgl.lkey = cb->qp->device->local_dma_lkey;
	else if (cb->mem == DMA)
		cb->recv_sgl.lkey = cb->dma_mr->lkey;
	cb->rq_wr.sg_list = &cb->recv_sgl;
	cb->rq_wr.num_sge = 1;

	cb->send_sgl.addr = cb->send_dma_addr;
	cb->send_sgl.length = sizeof(struct message);
	if (cb->local_dma_lkey)
		cb->send_sgl.lkey = cb->qp->device->local_dma_lkey;
	else if (cb->mem == DMA)
		cb->send_sgl.lkey = cb->dma_mr->lkey;
	cb->sq_wr.opcode = IB_WR_SEND;
	cb->sq_wr.send_flags = IB_SEND_SIGNALED;
	cb->sq_wr.sg_list = &cb->send_sgl;
	cb->sq_wr.num_sge = 1;

}



// Bind the receive buffer to IB
// RDMA ? DMA ?
// 
static int IS_setup_buffers(struct kernel_cb *cb)
{
	int ret;

	printk( "IS_setup_buffers called on cb %p\n", cb);
	printk("size of IS_rdma_info %lu\n", sizeof(struct message));


//	cb->recv_dma_addr = dma_map_single(&cb->pd->device->dev, 
//				   cb->recv_buf, sizeof(struct message), DMA_BIDIRECTIONAL);   //Map a Driver (kernal) virtual address to DMA/bus address, which is used by device.

	// ib_dma_map_single is a ofed funtion.
//static inline u64 ib_dma_map_single(struct ib_device *dev,
// 				    void *cpu_addr, size_t size,
// 				    enum dma_data_direction direction)
// {
// #ifndef HAVE_DEVICE_DMA_OPS
// 	if (dev->dma_ops)
// 		return dev->dma_ops->map_single(dev, cpu_addr, size, direction);
// #endif
// 	return dma_map_single(dev->dma_device, cpu_addr, size, direction);
// }
	//
	cb->recv_dma_addr = ib_dma_map_single(cb->pd->device, cb->recv_buf, sizeof(struct message), DMA_BIDIRECTIONAL);
	printk("Got dma/bus address by ib_dma_map_single: %lx \n",cb->recv_dma_addr);


	pci_unmap_addr_set(cb, recv_mapping, cb->recv_dma_addr);   // [?] pci unmap ??


//	cb->send_dma_addr = dma_map_single(&cb->pd->device->dev, 
//				   &cb->send_buf, sizeof(struct message), DMA_BIDIRECTIONAL);	

	cb->send_dma_addr = ib_dma_map_single(cb->pd->device, cb->send_buf, sizeof(struct message), DMA_BIDIRECTIONAL);	
	printk("Got dma/bus address by ib_dma_map_single: %lx \n",cb->send_dma_addr);


	pci_unmap_addr_set(cb, send_mapping, cb->send_dma_addr);
	pr_info(PFX "cb->mem=%d \n", cb->mem);

	if (cb->mem == DMA) {
		pr_info(PFX "IS_setup_buffers, in cb->mem==DMA \n");
		cb->dma_mr = cb->pd->device->get_dma_mr(cb->pd, IB_ACCESS_LOCAL_WRITE|
							        IB_ACCESS_REMOTE_READ|
							        IB_ACCESS_REMOTE_WRITE);

		if (IS_ERR(cb->dma_mr)) {
			pr_info(PFX "reg_dmamr failed\n");
			ret = PTR_ERR(cb->dma_mr);
			goto bail;
		}
	} 
	
	IS_setup_wr(cb);
	pr_info(PFX "allocated & registered buffers...\n");
	return 0;
bail:

	printk(KERN_ERR "Bind DMA buffer error. \n");

	// if (cb->rdma_mr && !IS_ERR(cb->rdma_mr))
	// 	ib_dereg_mr(cb->rdma_mr);
	// if (cb->dma_mr && !IS_ERR(cb->dma_mr))
	// 	ib_dereg_mr(cb->dma_mr);
	// if (cb->recv_mr && !IS_ERR(cb->recv_mr))
	// 	ib_dereg_mr(cb->recv_mr);
	// if (cb->send_mr && !IS_ERR(cb->send_mr))
	// 	ib_dereg_mr(cb->send_mr);
	
	return ret;
}




static int IS_connect_client(struct kernel_cb *cb)
{
	struct rdma_conn_param conn_param;
	int ret;

	memset(&conn_param, 0, sizeof conn_param);
	conn_param.responder_resources = 1;
	conn_param.initiator_depth = 1;
	conn_param.retry_count = 10;

	ret = rdma_connect(cb->cm_id, &conn_param);
	if (ret) {
		printk(KERN_ERR PFX "rdma_connect error %d\n", ret);
		return ret;
	}else{
		printk("Send : rdma_connect(cb->cm_id, &conn_param) \n");
	}

	wait_event_interruptible(cb->sem, cb->state >= CONNECTED);
	if (cb->state == ERROR) {
		printk(KERN_ERR PFX "wait for CONNECTED state %d\n", cb->state);
		return -1;
	}

	pr_info("rdma_connect successful\n");
	return 0;
}




// Handle the received RDMA evetn.
static int client_recv(struct kernel_cb *cb, struct ib_wc *wc)
{
	if (wc->byte_len != sizeof(struct message)) {         // Check the length of received message
		printk(KERN_ERR PFX "Received bogus data, size %d\n", 
		       wc->byte_len);
		return -1;
	}	
	if (cb->state < CONNECTED){
		printk(KERN_ERR PFX "cb is not connected\n");	
		return -1;
	}

	// debug
	printk(KERN_ERR "client_recv, cb->state : %d  \n", cb->state );
	printk(KERN_ERR "client_recv, cb->recv_buf->size_gb : %d, cb->recv_buf->type : %d \n", cb->recv_buf->size_gb, cb->recv_buf->type );


	switch(cb->recv_buf->type){
		case FREE_SIZE:
			cb->remote_chunk.target_size_g = cb->recv_buf->size_gb;
			cb->state = FREE_MEM_RECV;	
			
			//debug
			printk(KERN_ERR "Receive  message, type : FREE_SIZE, avaible size : %d GB \n ", cb->remote_chunk.target_size_g = cb->recv_buf->size_gb);
			
			break;
		case INFO:
		//	cb->IS_sess->cb_state_list[cb->cb_index] = CB_MAPPED;
			cb->state = WAIT_OPS;
			//IS_chunk_list_init(cb);

			printk(KERN_ERR "Recieve cb->recv_buf->type: INFO \n");

			break;
		case INFO_SINGLE:
		//	cb->IS_sess->cb_state_list[cb->cb_index] = CB_MAPPED;
			cb->state = WAIT_OPS;

			printk(KERN_ERR "Recieve cb->recv_buf->type: INFO_SINGLE \n");
			//IS_single_chunk_init(cb);
			break;
		case EVICT:
			cb->state = RECV_EVICT;

			printk(KERN_ERR "Recieve cb->recv_buf->type: EVICT \n");
			//client_recv_evict(cb);
			break;
		case STOP:
			cb->state = RECV_STOP;	
			
			printk(KERN_ERR "Recieve cb->recv_buf->type: STOP \n");
			//client_recv_stop(cb);
			break;
		default:
			printk(KERN_ERR "client receives unknown msg\n");
			return -1; 	
	}
	return 0;
}





// Kernel structure
// 
// struct ib_cq_init_attr {
// 	unsigned int	cqe;
// 	int		comp_vector;
// 	u32		flags;
// };


// Handle the received message 
static void rdma_cq_event_handler(struct ib_cq * cq, void *ctx)    // cq : kernel_cb->cq;  ctx : cq->context, just the kernel_cb
{
	bool stop_waiting_on_cq = false;
	struct kernel_cb *cb=ctx;
	struct ib_wc wc;
	struct ib_recv_wr * bad_wr;
	int ret;
	BUG_ON(cb->cq != cq);
	if (cb->state == ERROR) {
		printk(KERN_ERR PFX "cq completion in ERROR state\n");
		return;
	}
	ib_req_notify_cq(cb->cq, IB_CQ_NEXT_COMP);

	//[?] When find a WC (Work Completion) in the CQ, where to find the data ?
	// The registered buffer of Receiver ??
	while ((ret = ib_poll_cq(cb->cq, 1, &wc)) == 1) {   //ib_poll_cq, check the CQ for completion Work Request.
		if (wc.status) {
			if (wc.status == IB_WC_WR_FLUSH_ERR) {
				pr_info("cq flushed\n");
				continue;
			} else {
				printk(KERN_ERR PFX "cq completion failed with "
				       "wr_id %Lx status %d opcode %d vender_err %x\n",
					wc.wr_id, wc.status, wc.opcode, wc.vendor_err);
				goto error;
			}
		}	

		switch (wc.opcode){
			case IB_WC_RECV:

				printk(KERN_ERR "Got IB_WC_RECV \n");

				ret = client_recv(cb, &wc);
			  if (ret) {
				 	printk(KERN_ERR PFX "recv wc error: %d\n", ret);
				 	goto error;
				 }

				 // debug
				 // Stop waiting for message.
				 stop_waiting_on_cq = true;
				 cb->state = SEND_MESSAGE;
				 wake_up_interruptible(&cb->sem);

				// ret = ib_post_recv(cb->qp, &cb->rq_wr, &bad_wr);
				// if (ret) {
				// 	printk(KERN_ERR PFX "post recv error: %d\n", 
				// 	       ret);
				// 	goto error;
				// }
				// if (cb->state == RDMA_BUF_ADV || cb->state == FREE_MEM_RECV || cb->state == WAIT_OPS){
				// 	wake_up_interruptible(&cb->sem);
				// }
				 break;
			case IB_WC_SEND:
				// ret = client_send(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "send wc error: %d\n", ret);
				// 	goto error;
				// }
				 break;
			case IB_WC_RDMA_READ:
				// ret = client_read_done(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "read wc error: %d, cb->state=%d\n", ret, cb->state);
				// 	goto error;
				// }
				break;
			case IB_WC_RDMA_WRITE:
				// ret = client_write_done(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "write wc error: %d, cb->state=%d\n", ret, cb->state);
				// 	goto error;
				// }
				break;
			default:
				printk(KERN_ERR PFX "%s:%d Unexpected opcode %d, Shutting down\n", __func__, __LINE__, wc.opcode);
				goto error;
		}

		if(stop_waiting_on_cq){
			printk("Stop waiting on CQ \n");
			break;
		}else{
			printk("rdma_cq_event_handler: Waiting on ib_poll_cq(). ");
		}


	}
	if (ret){
		printk(KERN_ERR PFX "poll error %d\n", ret);
		goto error;
	}
	return;
error:
	cb->state = ERROR;
}





static int send_messaget_to_remote(struct kernel_cb *cb, int messge_type  , int size_gb)
{
	int ret = 0;
	struct ib_send_wr * bad_wr;
	cb->send_buf->type = messge_type;
	cb->send_buf->size_gb = size_gb; 

	printk("Send a Message to Remote memory server. cb->send_buf->type  \n", messge_type);

	ret = ib_post_send(cb->qp, &cb->sq_wr, &bad_wr);
	if (ret) {
		printk(KERN_ERR PFX "BIND_SINGLE MSG send error %d\n", ret);
		return ret;
	}
	return 0;	
}




// Declare a global variables. 
// Initialize in main().
 struct kernel_cb * cb;


int main(int argc, char* argv[]){


  struct rdma_cm_id *cm_id;   // device ID ?
	int ret;


  //1) init kernel_cb
	cb = (struct kernel_cb *)kzalloc(sizeof(struct kernel_cb), GFP_KERNEL);

  cb->cm_id = rdma_create_id(&init_net, IS_cma_event_handler, cb, RDMA_PS_TCP, IB_QPT_RC);  // TCP, RC, reliable IB connection 
  cb->state = IDLE;

  // The number of  on-the-fly wr ?? every cores handle one ?? 
  cb->txdepth = 256 * num_online_cpus() + 1; //[?] What's this used for ? What's meaning of the value ?
  

  // Setup socket information
  cb->port = htons((uint16_t)9400);  // After transffer to big endian, the decimal value is 47140
	char ip[] = "10.0.0.2";
  ret= in4_pton(ip, strlen(ip), cb->addr, -1, NULL);   // char* to ipv4 address ?
  if(ret == 0){  // kernel 4.11.0 , success 1; failed 0.
		printk("Assign ip %s to  cb->addr : %s failed.\n",ip, cb->addr );
	}
	
	//cb->addr is null, because  big endian ??
	printk("kernel_cb->port(network big endian): %d , kernel_cb->addr : %s \n",cb->port, cb->addr);

  cb->addr_type = AF_INET;  //ipv4

  //put a flag into the receive buffer
  // GFP_DMA - Allocation suitable for DMA. Should only be used for kmalloc caches. Otherwise, use a slab created with SLAB_DMA ??
  cb->recv_buf = kmalloc(sizeof(struct message),GFP_DMA);  //[?] Or do we need to allocate DMA memory by get_dma_addr ???
	//debug
	cb->recv_buf->type = 1;

	cb->send_buf = kmalloc(sizeof(struct message),GFP_DMA);
	cb->mem = DMA;

  // Initialize the queue.
  init_waitqueue_head(&cb->sem);

  printk("Initialized kernel_cb\n");


  //2) Bind to remote server

   ret = IS_bind_client(cb);
	 if (ret){
	 	printk ("bind socket error \n");
     return ret;
   }else{
     printk("PASS: Bind to remote server.\n");
   }


  // 3) Create the QP
  ret = IS_setup_qp(cb, cb->cm_id);

  //if(ret){

  //}

  //4) Bing a Buffer to IB
	// !! Do nothing for now.
  ret = IS_setup_buffers(cb);
	if(ret){
		printk(KERN_ERR "Bind DMA buffer error\n");
	}else{
		printk("PASS: Bind DMA buffer successfully\n");
	}


	//5) Build the connection to Remote
	ret = IS_connect_client(cb);
	if(ret){
		printk(KERN_ERR "Connect to remote server error\n");
	}else{
		printk("Connect to remote server successfully\n");
	}

	//6) Get free memory information from Remote Mmeory Server

	// Post the WR to CQ to wait for WC.
	struct ib_recv_wr *bad_wr;
	ret = ib_post_recv(cb->qp, &cb->rq_wr, &bad_wr); 

	//7) Post a message to Remote memory server.
	wait_event_interruptible(cb->sem, cb->state == SEND_MESSAGE);
	printk("Receive message down, wake up to send message.\n");
	send_messaget_to_remote(cb, 9, 2);  // 9 : bind_single
	//send_messaget_to_remote(cb, 6, 2);  // 6 : activity


	printk("Exit the main() function.\n");

	return 0;
}



// invoked by insmod 
static int __init IS_init_module(void)
{
  printk("Init self build kernel. \n");

	//printk("Do nothing for now. \n");

	main(0,NULL);

	return 0;
}

// invoked by rmmod 
static void __exit IS_cleanup_module(void)
{

  printk(" Exit Kernel Space IB test module .\n");
	printk(" Destroy the built InfiniBand resource \n");

	//[?] Should send a disconnect event to remote memory server?

	IS_free_qp(cb);
	//IS_free_buffers(cb);  //!! DO Not invoke this.
	//if(cb != NULL && cb->cm_id != NULL){
	//	rdma_disconnect(cb->cm_id);
	//}

}

module_init(IS_init_module);
module_exit(IS_cleanup_module);