#include "rdma_client.h"




// The rdma CM handler function
// CMA Event handler  && cq_event_handler , 2 different functions for CM event and normal RDMA message handling.
// struct rdma_cm_event : assigned by user.
//
static int octopus_rdma_cm_event_handler(struct rdma_cm_id *cma_id, struct rdma_cm_event *event)
{
	int ret;
	struct rdma_session_context *rdma_session = cma_id->context;

	// [?] What's the meaning of this ?? parent ?
	//
	pr_info("cma_event type %d cma_id %p (%s)\n", event->event, cma_id, (cma_id == rdma_session->cm_id) ? "parent" : "child");

	switch (event->event) {
	case RDMA_CM_EVENT_ADDR_RESOLVED:
		rdma_session->state = ADDR_RESOLVED;
		// Go to next step, resolve the rdma route.
		ret = rdma_resolve_route(cma_id, 2000);
		if (ret) {
			printk(KERN_ERR "%s,rdma_resolve_route error %d\n", __func__, ret);
		//	wake_up_interruptible(&rdma_session->sem);
		}
		break;

	case RDMA_CM_EVENT_ROUTE_RESOLVED:
		rdma_session->state = ROUTE_RESOLVED;
		// RDMA route is solved, wake up the main process  to continue.
    	printk("%s : RDMA_CM_EVENT_ROUTE_RESOLVED, wake up rdma_session->sem\n ",__func__);
		wake_up_interruptible(&rdma_session->sem);
		break;

	case RDMA_CM_EVENT_CONNECT_REQUEST:
		rdma_session->state = CONNECT_REQUEST;
		//cb->child_cm_id = cma_id;
		//pr_info("child cma %p\n", cb->child_cm_id);
		//wake_up_interruptible(&cb->sem);

    	printk("Receive but Not Handle : RDMA_CM_EVENT_CONNECT_REQUEST \n");

		break;

	case RDMA_CM_EVENT_ESTABLISHED:
	
		rdma_session->state = CONNECTED;
		
    	printk("ESTABLISHED, wake up kernel_cb->sem\n");
    	wake_up_interruptible(&rdma_session->sem);
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
		printk( "%s, cma event %d, error %d\n", __func__, event->event, event->status);
		rdma_session->state = ERROR;
		wake_up_interruptible(&rdma_session->sem);
		break;

	case RDMA_CM_EVENT_DISCONNECTED:	//should get error msg from here
		printk( "%s, DISCONNECT EVENT...\n",__func__);
		rdma_session->state = CM_DISCONNECT;
		// RDMA is off
		//IS_disconnect_handler(cb);
		break;

	case RDMA_CM_EVENT_DEVICE_REMOVAL:	//this also should be treated as disconnection, and continue disk swap
		printk(KERN_ERR "%s, cma detected device removal!!!!\n", __func__);
		return -1;
		break;

	default:
		printk(KERN_ERR "%s,oof bad type!\n",__func__);
		wake_up_interruptible(&rdma_session->sem);
		break;
	}

	return ret;
}




// Resolve the destination IB device by the destination IP.
// [?] Need to build some route table ?
// 
static int rdma_resolve_ip_to_ib_device(struct rdma_session_context *rdma_session)
{
	struct sockaddr_storage sin; 
	int ret;

	//fill_sockaddr(&sin, cb);
	// Assume that it's ipv6
	// [?]cast "struct sockaddr_storage" to "sockaddr_in" ??
	//
	struct sockaddr_in *sin4 = (struct sockaddr_in *)&sin;   
	sin4->sin_family = AF_INET;
	memcpy((void *)&(sin4->sin_addr.s_addr), rdma_session->addr, 4);   	// copy 32bits/ 4bytes from cb->addr to sin4->sin_addr.s_addr
	sin4->sin_port = rdma_session->port;                             		// assign cb->port to sin4->sin_port


	ret = rdma_resolve_addr(rdma_session->cm_id, NULL, (struct sockaddr *)&sin, 2000); // timeout 2000ms
	if (ret) {
		printk(KERN_ERR "%s, rdma_resolve_ip_to_ib_device error %d\n", __func__, ret);
		return ret;
	}else{
		printk("rdma_resolve_ip_to_ib_device - rdma_resolve_addr success.\n");
	}
	
	// Wait for the CM events to be finished:  handled by rdma_cm_event_handler()
	// 	1) resolve addr
	//	2) resolve route
	// Come back here and continue:
	//
	wait_event_interruptible(rdma_session->sem, rdma_session->state >= ROUTE_RESOLVED);   //[?] Wait on cb->sem ?? Which process will wake up it.
	if (rdma_session->state != ROUTE_RESOLVED) {
		printk(KERN_ERR  "%s, addr/route resolution did not resolve: state %d\n", __func__, rdma_session->state);
		return -EINTR;
	}
	printk("rdma_resolve_ip_to_ib_device -  resolve address and route successfully\n");
	return ret;
}



/**
 * Build the Queue Pair (QP).
 * 
 */
static int octopus_create_qp(struct rdma_session_context *rdma_session)
{
	struct ib_qp_init_attr init_attr;
	int ret;

	memset(&init_attr, 0, sizeof(init_attr));
	init_attr.cap.max_send_wr = rdma_session->txdepth; /*FIXME: You may need to tune the maximum work request */
	init_attr.cap.max_recv_wr = rdma_session->txdepth;  
	init_attr.cap.max_recv_sge = 1;					// [?] ib_sge ?  Each wr can only support one buffer ? not use the scatter/gather ?
	init_attr.cap.max_send_sge = 1;
	init_attr.sq_sig_type = IB_SIGNAL_REQ_WR;     // Receive WR ?
	init_attr.qp_type = IB_QPT_RC;                // Queue Pair connect type, Reliable Communication.  [?] Already assign this during create cm_id.

	// [?] Can both recv_cq and send_cq use the same cq ??
	init_attr.send_cq = rdma_session->cq;
	init_attr.recv_cq = rdma_session->cq;

	ret = rdma_create_qp(rdma_session->cm_id, rdma_session->pd, &init_attr);
	if (!ret){
		// Record this queue pair.
		rdma_session->qp = rdma_session->cm_id->qp;
  	}else{
    	printk(KERN_ERR "Create QP falied.");
  	}

	return ret;
}





// Prepare for building the Connection to remote IB servers. 
// Create Queue Pair : pd, cq, qp, 
static int octopus_create_rdma_queues(struct rdma_session_context *rdma_session, struct rdma_cm_id *cm_id)
{
	int ret = 0;

	struct ib_cq_init_attr init_attr;
	// 1) Build PD.
	// flags of Protection Domain, (ib_pd) : Protect the local OR remote memory region ? ?  
	// Local Read is default.
	rdma_session->pd = ib_alloc_pd(cm_id->device, IB_ACCESS_LOCAL_WRITE|
                                            		IB_ACCESS_REMOTE_READ|
                                            		IB_ACCESS_REMOTE_WRITE );    // No local read ??  [?] What's the cb->pd used for ?

	if (IS_ERR(rdma_session->pd)) {
		printk(KERN_ERR "%s, ib_alloc_pd failed\n", __func__);
		return PTR_ERR(rdma_session->pd);
	}
	pr_info("%s, created pd %p\n", __func__, rdma_session->pd);

	// 2) Build CQ
	memset(&init_attr, 0, sizeof(init_attr));
	init_attr.cqe = rdma_session->txdepth * 2;     // [?] The depth of cq. Number of completion queue entries.  ??
	init_attr.comp_vector = 0;					   // [?] What's the meaning of this ??
	
	// Set up the completion queues and the cq evnet handler.
	// Parameters
	// 		cq_context = qp_context = rdma_session_context.
	//
	// [?] receive cq and send cq are the same one ??
	// 
	rdma_session->cq = ib_create_cq(cm_id->device, octopus_cq_event_handler, NULL, rdma_session, &init_attr);

	if (IS_ERR(rdma_session->cq)) {
		printk(KERN_ERR "%s, ib_create_cq failed\n", __func__);
		ret = PTR_ERR(rdma_session->cq);
		goto err;
	}
	pr_info("%s, created cq %p\n", __func__, rdma_session->cq);

  	// Request a notification (IRQ), if an event arrives on CQ entry.
	ret = ib_req_notify_cq(rdma_session->cq, IB_CQ_NEXT_COMP);   
	if (ret) {
		printk(KERN_ERR "%s, ib_create_cq failed\n", __func__);
		goto err;
	}

	// 3) Build QP.
	ret = octopus_create_qp(rdma_session);
	if (ret) {
		printk(KERN_ERR  "%s, failed: %d\n", __func__, ret);
		goto err;
	}
	pr_info("%s, created qp %p\n", __func__, rdma_session->qp);
	return ret;

err:
	//ib_dealloc_pd(cb->pd);
  	printk(KERN_ERR "Error in %s \n", __func__);
	return ret;
}





/**
 * Reserve two RDMA wr for receive/send meesages
 * 		rdma_session_context->rq_wr
 * 		rdma_session_context->send_sgl
 * Post these 2 WRs to receive/send controll messages.
 */
static void octopus_setup_message_wr(struct rdma_session_context *rdma_context)
{
	// 1) Reserve a wr to receive RDMA message
	rdma_context->recv_sgl.addr = rdma_context->recv_dma_addr;      
	rdma_context->recv_sgl.length = sizeof(struct message);
	if (rdma_context->local_dma_lkey){                            // check ?
		rdma_context->recv_sgl.lkey = rdma_context->qp->device->local_dma_lkey;

		#ifdef DEBUG_RDMA_CLIENT
		printk("%s, get lkey from rdma_context->local_dma_lkey \n",__func__);
		#endif
	}else if (rdma_context->mem == DMA){
		rdma_context->recv_sgl.lkey = rdma_context->dma_mr->lkey;

		#ifdef DEBUG_RDMA_CLIENT
		printk("%s, get lkey from rdma_context->dma_mr->lkey \n",__func__);
		#endif
	}
	rdma_context->rq_wr.sg_list = &rdma_context->recv_sgl;
	rdma_context->rq_wr.num_sge = 1;


	// 2) Reserve a wr
	rdma_context->send_sgl.addr = rdma_context->send_dma_addr;
	rdma_context->send_sgl.length = sizeof(struct message);
	if (rdma_context->local_dma_lkey){
		rdma_context->send_sgl.lkey = rdma_context->qp->device->local_dma_lkey;
	}else if (rdma_context->mem == DMA){
		rdma_context->send_sgl.lkey = rdma_context->dma_mr->lkey;
	}
	rdma_context->sq_wr.opcode = IB_WR_SEND;		// ib_send_wr.opcode , passed to wc.
	rdma_context->sq_wr.send_flags = IB_SEND_SIGNALED;
	rdma_context->sq_wr.sg_list = &rdma_context->send_sgl;
	rdma_context->sq_wr.num_sge = 1;

}



/**
 * We reserve two WRs for send/receive RDMA messages in a 2-sieded way.
 * 	a. Allocate 2 buffers
 * 		dma_session->recv_buf
 * 		rdma_session->send_buf
 *	b. Bind their DMA/BUS address to  
 * 		rdma_context->recv_sgl
 * 		rdma_context->send_sgl
 * 	c. Bind the ib_sge to send/receive WR
 * 		rdma_context->rq_wr
 * 		rdma_context->sq_wr
 */
static int octopus_setup_buffers(struct rdma_session_context *rdma_session)
{
	int ret;

	// 1) Allocate some DMA buffers.
	// [x] Seems that any memory can be registered as DMA buffers, if they satisfy the constraints:
	// 1) Corresponding physical memory is allocated. The page table is built. 
	//		If the memory is allocated by user space allocater, malloc, we need to walk  through the page table.
	// 2) The physial memory is pinned, can't be swapt/paged out.
  	rdma_session->recv_buf = kmalloc(sizeof(struct message), GFP_KERNEL);  	//[?] Or do we need to allocate DMA memory by get_dma_addr ???
	rdma_session->send_buf = kmalloc(sizeof(struct message), GFP_KERNEL);  
	rdma_session->mem = DMA;   // [??] Is this useful ?


	// Get DMA/BUS address for the receive buffer
	rdma_session->recv_dma_addr = ib_dma_map_single(rdma_session->pd->device, rdma_session->recv_buf, sizeof(struct message), DMA_BIDIRECTIONAL);
	pci_unmap_addr_set(rdma_session, recv_mapping, rdma_session->recv_dma_addr);   // [?] pci unmap ??


//	cb->send_dma_addr = dma_map_single(&cb->pd->device->dev, 
//				   &cb->send_buf, sizeof(struct message), DMA_BIDIRECTIONAL);	

	rdma_session->send_dma_addr = ib_dma_map_single(rdma_session->pd->device, rdma_session->send_buf, sizeof(struct message), DMA_BIDIRECTIONAL);	
	pci_unmap_addr_set(rdma_session, send_mapping, rdma_session->send_dma_addr);

	#ifdef DEBUG_RDMA_CLIENT
	printk("Got dma/bus address 0x%llx, for the recv_buf 0x%llx \n",(unsigned long long)rdma_session->recv_dma_addr, (unsigned long long)rdma_session->recv_buf);
	printk("Got dma/bus address 0x%llx, for the send_buf 0x%llx \n",(unsigned long long)rdma_session->send_dma_addr, (unsigned long long)rdma_session->send_buf);
	#endif

	
	// 2) Allocate a DMA Memory Region.
	//pr_info(PFX "rdma_session->mem=%d \n", cb->mem);

	if (rdma_session->mem == DMA) {
		// [??] What's this region used for ?
		//		=> get a lkey from rdma_session->dma_mr->lkey
		// 
		// [x] RDMA read/write . But for this, we only need a remote addr AND rkey ?
		// 

		pr_info("%s, IS_setup_buffers, in cb->mem==DMA \n", __func__);
		rdma_session->dma_mr = rdma_session->pd->device->get_dma_mr(rdma_session->pd, IB_ACCESS_LOCAL_WRITE|
							        													IB_ACCESS_REMOTE_READ|
							        													IB_ACCESS_REMOTE_WRITE);

		if (IS_ERR(rdma_session->dma_mr)) {
			pr_info("%s, reg_dmamr failed\n", __func__);
			ret = PTR_ERR(rdma_session->dma_mr);
			goto error;
		}
	} 
	

	// 3) Add the allocated (DMA) buffer to reserved WRs
	octopus_setup_message_wr(rdma_session);
	pr_info("%s, allocated & registered buffers...\n", __func__);

	return ret;


error:

	printk(KERN_ERR "%s, Bind DMA buffer error. \n", __func__);

	// if (cb->rdma_mr && !IS_ERR(cb->rdma_mr))
	// 	ib_dereg_mr(cb->rdma_mr);
	 if (rdma_session->dma_mr && !IS_ERR(rdma_session->dma_mr))
	 	ib_dereg_mr(rdma_session->dma_mr);
	// if (cb->recv_mr && !IS_ERR(cb->recv_mr))
	// 	ib_dereg_mr(cb->recv_mr);
	// if (cb->send_mr && !IS_ERR(cb->send_mr))
	// 	ib_dereg_mr(cb->send_mr);
	
	return ret;
}



/**
 * All the PD, QP, CP are setted up, connect to remote IB servers.
 * This will send a CM event to remote IB server && get a CM event response back.
 */
static int octopus_connect_remote_memory_server(struct rdma_session_context *rdma_session)
{
	struct rdma_conn_param conn_param;
	int ret;

	// [?] meaning of these parameters ?
	memset(&conn_param, 0, sizeof conn_param);
	conn_param.responder_resources = 1;
	conn_param.initiator_depth = 1;
	conn_param.retry_count = 10;

	ret = rdma_connect(rdma_session->cm_id, &conn_param);  // RDMA CM event 
	if (ret) {
		printk(KERN_ERR "%s, rdma_connect error %d\n", __func__, ret);
		return ret;
	}else{
		printk("Send : rdma_connect(cb->cm_id, &conn_param) \n");
	}

	wait_event_interruptible(rdma_session->sem, rdma_session->state >= CONNECTED);
	if (rdma_session->state == ERROR) {
		printk(KERN_ERR "%s, wait for CONNECTED state %d\n", __func__, rdma_session->state);
		return ret;
	}

	pr_info("%s, rdma_connect successful\n", __func__);
	return ret;
}




/**
 * Receive a WC, IB_WC_RECV.
 * Read the data from the posted WR.
 * 		For this WR, its associated DMA buffer is rdma_session_context->recv_buf.
 * 
 * Action 
 * 		Change some rdma_session_context->state
 * 
 */
static int handle_recv_wr(struct rdma_session_context *rdma_session, struct ib_wc *wc)
{
	if (wc->byte_len != sizeof(struct message)){         // Check the length of received message
		printk(KERN_ERR "%s, Received bogus data, size %d\n", __func__,  wc->byte_len);
		return -1;
	}	

	if (rdma_session->state < CONNECTED){
		printk(KERN_ERR "%s, RDMA is not connected\n", __func__);	
		return -1;
	}

	// debug
	//printk(KERN_ERR "client_recv, rdma_session->state : %d  \n", rdma_session->state );
	//printk(KERN_ERR "client_recv, rdma_session->recv_buf->size_gb : %d, rdma_session->recv_buf->type : %d \n", rdma_session->recv_buf->size_gb, rdma_session->recv_buf->type );


	switch(rdma_session->recv_buf->type){
		case FREE_SIZE:
			rdma_session->remote_chunk.target_size_g = rdma_session->recv_buf->size_gb;
			rdma_session->state = FREE_MEM_RECV;	
			
			#ifdef DEBUG_RDMA_CLIENT
			printk( "%s, Received RDMA message, type: FREE_SIZE, avaible size : %d GB \n ", __func__, rdma_session->remote_chunk.target_size_g );
			#endif

			break;
		case INFO:
		//	cb->IS_sess->cb_state_list[cb->cb_index] = CB_MAPPED;
			rdma_session->state = WAIT_OPS;
			//IS_chunk_list_init(cb);

			#ifdef DEBUG_RDMA_CLIENT
			printk("%s, Recieved RDMA message: INFO \n",__func__);
			#endif

			break;
		case INFO_SINGLE:
		//	cb->IS_sess->cb_state_list[cb->cb_index] = CB_MAPPED;
			rdma_session->state = WAIT_OPS;
			#ifdef DEBUG_RDMA_CLIENT
			printk("%s, Recieved RDMA message: INFO_SINGLE \n",__func__);
			#endif

			//IS_single_chunk_init(cb);
			break;
		case EVICT:
			rdma_session->state = RECV_EVICT;

			#ifdef DEBUG_RDMA_CLIENT
			printk("%s, Recieved RDMA message: EVICT \n",__func__);
			#endif

			//client_recv_evict(cb);
			break;
		case STOP:
			rdma_session->state = RECV_STOP;	
			
			#ifdef DEBUG_RDMA_CLIENT
			printk("%s, Recieved RDMA message: STOP \n",__func__);
			#endif

			//client_recv_stop(cb);
			break;
		default:
			printk(KERN_ERR "%s, Recieved RDMA message UN-KNOWN \n",__func__);
			return -1; 	
	}
	return 0;
}





/**
 * RDMA  CQ event handler.
 * After invoke the cq_notify, everytime a wc is insert into completion queue entry, notify to the process by invoking "rdma_cq_event_handler".
 * 
 * 
 * [?] For the RDMA read/write, is  there also a WC to acknowledge the finish of this ??
 * 
 */
static void octopus_cq_event_handler(struct ib_cq * cq, void *rdma_ctx)    // cq : kernel_cb->cq;  ctx : cq->context, just the kernel_cb
{
	bool 							stop_waiting_on_cq 	= 	false;
	struct rdma_session_context 	*rdma_session		=	rdma_ctx;
	struct ib_wc 		wc;
	//struct ib_recv_wr 	*bad_wr;
	int ret;
	BUG_ON(rdma_session->cq != cq);
	if (rdma_session->state == ERROR) {
		printk(KERN_ERR "%s, cq completion in ERROR state\n", __func__);
		return;
	}

	// Get notified by the arriving of next WC.
	// The action is to trigger current function, rdma_cq_event_handler.
	ib_req_notify_cq(rdma_session->cq, IB_CQ_NEXT_COMP);

	// If current function, rdma_cq_event_handler, is invoked, a WC is on the CQE.
	// Get the SIGNAL, WC, by invoking ib_poll_cq.
	while ((ret = ib_poll_cq(rdma_session->cq, 1, &wc)) == 1) {   //ib_poll_cq, get the wc from cq.
		if (wc.status) {
			if (wc.status == IB_WC_WR_FLUSH_ERR) {
				pr_info("cq flushed\n");
				continue;
			} else {
				printk(KERN_ERR "cq completion failed with "
				       "wr_id %Lx status %d opcode %d vender_err %x\n",
					wc.wr_id, wc.status, wc.opcode, wc.vendor_err);
				goto error;
			}
		}	

		switch (wc.opcode){
			case IB_WC_RECV:
				// posted a recv wr, and it gets  the data back.
				// We can read data from wr's associated DMA buffer.

				ret = handle_recv_wr(rdma_session, &wc);
			  	if (ret) {
				 	printk(KERN_ERR "%s, recv wc error: %d\n", __func__, ret);
				 	goto error;
				 }

				 // debug
				 // Stop waiting for message.
				 stop_waiting_on_cq = true;
				 //rdma_session->state = SEND_MESSAGE;

				 // [?] Which function is waiting here ?
				 wake_up_interruptible(&rdma_session->sem);

				 //ret = ib_post_recv(rdma_session->qp, &rdma_session->rq_wr, &bad_wr);
				 //if (ret) {
				 //	printk(KERN_ERR PFX "post recv error: %d\n",    ret);
				 //	goto error;
				 //}
				// if (cb->state == RDMA_BUF_ADV || cb->state == FREE_MEM_RECV || cb->state == WAIT_OPS){
				// 	wake_up_interruptible(&cb->sem);
				// }

				#ifdef DEBUG_RDMA_CLIENT
				printk(KERN_ERR "Got IB_WC_RECV \n");
				#endif

				 break;
			case IB_WC_SEND:
				// ret = client_send(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "send wc error: %d\n", ret);
				// 	goto error;
				// }

				#ifdef DEBUG_RDMA_CLIENT
				printk(KERN_ERR "Got IB_WC_SEND \n");
				#endif

				 break;
			case IB_WC_RDMA_READ:
				// ret = client_read_done(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "read wc error: %d, cb->state=%d\n", ret, cb->state);
				// 	goto error;
				// }

				#ifdef DEBUG_RDMA_CLIENT
				printk(KERN_ERR "Got IB_WC_RDMA_READ \n");
				#endif

				break;
			case IB_WC_RDMA_WRITE:
				// ret = client_write_done(cb, &wc);
				// if (ret) {
				// 	printk(KERN_ERR PFX "write wc error: %d, cb->state=%d\n", ret, cb->state);
				// 	goto error;
				// }

				#ifdef DEBUG_RDMA_CLIENT
				printk(KERN_ERR "Got IB_WC_RDMA_WRITE \n");
				#endif

				break;
			default:
				printk(KERN_ERR "%s:%d Unexpected opcode %d, Shutting down\n", __func__, __LINE__, wc.opcode);
				goto error;
		} // switch

		if(stop_waiting_on_cq){
			printk("%s, Stop waiting on CQ \n", __func__);
			break;
		}
		//else{
		//	printk("rdma_cq_event_handler: Waiting on ib_poll_cq(). ");
		//}


	} // While   

	if (ret){
		printk(KERN_ERR "%s, poll error %d\n", __func__, ret);
		goto error;
	}
	return;
error:
	rdma_session->state = ERROR;
}




/**
 * Send a RDMA message to remote server.
 */
static int send_messaget_to_remote(struct rdma_session_context *rdma_session, int messge_type  , int size_gb)
{
	int ret = 0;
	struct ib_send_wr * bad_wr;
	rdma_session->send_buf->type = messge_type;
	rdma_session->send_buf->size_gb = size_gb; 

	#ifdef DEBUG_RDMA_CLIENT
	printk("Send a Message to Remote memory server. cb->send_buf->type : %d \n", messge_type);
	#endif

	ret = ib_post_send(rdma_session->qp, &rdma_session->sq_wr, &bad_wr);
	if (ret) {
		printk(KERN_ERR "%s, BIND_SINGLE MSG send error %d\n", __func__, ret);
		return ret;
	}
	return ret;	
}







//int main(int argc, char* argv[]){


/**
 * Build the RDMA connection to remote memory server.
 * 
 */
static int octopus_RDMA_connect(struct rdma_session_context * rdma_session){

  	//struct rdma_cm_id  *cm_id;   // device descripor, [?] 
	int ret;


 	//1) init rdma_session_context
	rdma_session = (struct rdma_session_context *)kzalloc(sizeof(struct rdma_session_context), GFP_KERNEL);
	
	// Register the IB device,
	// Parameters
	// device 				 : init_net is an external symbols of kernel, get it from module.sysvers 
	// rdma_cm_event_handler : Register a CM event handler function. Used for RDMA connection.
	// IB driver_data   	 : rdma_session_context
	// RDMA connect type 	 : IB_QPT_RC, reliable communication.
  	rdma_session->cm_id = rdma_create_id(&init_net, octopus_rdma_cm_event_handler, rdma_session, RDMA_PS_TCP, IB_QPT_RC);  // TCP, RC, reliable IB connection 
  	
	// Used for a async   
	rdma_session->state = IDLE;

  	// The number of  on-the-fly wr ?? every cores handle one ?? 
	// This depth is used for CQ entry ? send/receive queue depth ?
  	rdma_session->txdepth = RDMA_READ_WRITE_QUEUE_DEPTH * num_online_cpus() + 1; //[?] What's this used for ? What's meaning of the value ?
  

  	// Setup socket information
	// Debug : for test, write the ip:port as 10.0.0.2:9400
  	rdma_session->port = htons((uint16_t)9400);  // After transffer to big endian, the decimal value is 47140
	char ip[] = "10.0.0.2";
  	ret= in4_pton(ip, strlen(ip), rdma_session->addr, -1, NULL);   // char* to ipv4 address ?
  	if(ret == 0){  		// kernel 4.11.0 , success 1; failed 0.
		printk("Assign ip %s to  rdma_session->addr : %s failed.\n",ip, rdma_session->addr );
	}
	rdma_session->addr_type = AF_INET;  //ipv4

	// Debug
//	#ifdef DEBUG_RDMA_CLIENT
//	printk("kernel_cb->port(network big endian): %d , kernel_cb->addr : %s \n",rdma_session->port, rdma_session->addr);
//	#endif


  	// Initialize the queue.
   	init_waitqueue_head(&rdma_session->sem);
	#ifdef DEBUG_RDMA_CLIENT
  	printk("%s, Initialized rdma_session_context fields successfully.\n", __func__);
	#endif

  	//2) Resolve address(ip:port) and route to destination IB. 
   	ret = rdma_resolve_ip_to_ib_device(rdma_session);
	if (ret){
		printk (KERN_ERR "%s, bind socket error (addr or route resolve error)\n", __func__);
     	return ret;
   	}
	#ifdef DEBUG_RDMA_CLIENT
	else{
     	printk("%s,Binded to remote server successfully.\n", __func__);
   	}
	#endif

  	// 3) Create the QP,CQ, PD
	//  Before we connect to remote memory server, we have to setup the rdma queues, CQ, QP.
	//	We also need to register the DMA buffer for two-sided communication and configure the Protect Domain, PD.
	//
	// Build the rdma queues.
  	ret = octopus_create_rdma_queues(rdma_session, rdma_session->cm_id);
  	if(ret){
		  printk(KERN_ERR "%s, Create rdma queues failed. \n", __func__);
  	}

	// 4) Register some message passing used DMA buffer.
	// 
	//rdma_session->mem = DMA;
	// !! Do nothing for now.
  	ret = octopus_setup_buffers(rdma_session);
	if(ret){
		printk(KERN_ERR "%s, Bind DMA buffer error\n", __func__);
	}
	#ifdef DEBUG_RDMA_CLIENT
	else{
		printk("%s, Allocate and Bind DMA buffer successfully \n", __func__);
	}
	#endif


	//5) Build the connection to Remote
	ret = octopus_connect_remote_memory_server(rdma_session);
	if(ret){
		printk(KERN_ERR "%s,Connect to remote server error \n", __func__);
	}
	#ifdef DEBUG_RDMA_CLIENT
	else{
		printk("%s, Connect to remote server successfully \n", __func__);
	}
	#endif
	//6) Get free memory information from Remote Mmeory Server

	// Post the WR to CQ to wait for WC.
	struct ib_recv_wr *bad_wr;
	ret = ib_post_recv(rdma_session->qp, &rdma_session->rq_wr, &bad_wr); 

	//7) Post a message to Remote memory server.
	//wait_event_interruptible(rdma_session->sem, rdma_session->state == SEND_MESSAGE);
	//printk("Receive message down, wake up to send message.\n");
	//send_messaget_to_remote(rdma_session, 9, 2);  // 9 : bind_single
	//send_messaget_to_remote(cb, 6, 2);  // 6 : activity


	printk("%s,Exit the main() function.\n", __func__);

	return ret;
}







// invoked by insmod 
static int __init octopus_rdma_client_init_module(void)
{
	printk("Init module, octopus - kernel level rdma client.. \n");

	//printk("Do nothing for now. \n");

	//main(0,NULL);

	octopus_RDMA_connect(rdma_session_global);

	return 0;
}

// invoked by rmmod 
static void __exit octopus_rdma_client_cleanup_module(void)
{
  	printk(" Exit Kernel Space IB test module .\n");
	printk(" Destroy the built InfiniBand resource \n");


	// 
	//[?] Should send a disconnect event to remote memory server?
	//		Invoke free_qp directly will cause kernel crashed. 
	//

	//octopus_free_qp(rdma_session_global);
	//IS_free_buffers(cb);  //!! DO Not invoke this.
	//if(cb != NULL && cb->cm_id != NULL){
	//	rdma_disconnect(cb->cm_id);
	//}

}

module_init(octopus_rdma_client_init_module);
module_exit(octopus_rdma_client_cleanup_module);