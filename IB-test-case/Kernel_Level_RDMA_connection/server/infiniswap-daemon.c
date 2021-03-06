/*
 * Infiniswap, remote memory paging over RDMA
 * Copyright 2017 University of Michigan, Ann Arbor
 * GPLv2 License
 */

#include "rdma-common.h"

static int on_connect_request(struct rdma_cm_id *id);
static int rdma_connected(struct rdma_cm_id *id);
static int on_disconnect(struct rdma_cm_id *id);
static int on_cm_event(struct rdma_cm_event *event);
static void usage(const char *argv0);
long page_size;
int running;


int main(int argc, char **argv)
{
  struct sockaddr_in6 addr;
  struct rdma_cm_event *event = NULL;
  struct rdma_cm_id *listener = NULL;
  struct rdma_event_channel *ec = NULL;   // [?] Event channel ?
  uint16_t port = 0;
  pthread_t free_mem_thread;

  if (argc != 3)
    usage(argv[0]);
  page_size = sysconf(_SC_PAGE_SIZE);

  memset(&addr, 0, sizeof(addr));
  addr.sin6_family = AF_INET6;                    //[?] Ipv6 ???
  inet_pton(AF_INET6, argv[1], &addr.sin6_addr);
  addr.sin6_port = htons(atoi(argv[2]));

  TEST_Z(ec = rdma_create_event_channel());
  TEST_NZ(rdma_create_id(ec, &listener, NULL, RDMA_PS_TCP));
  TEST_NZ(rdma_bind_addr(listener, (struct sockaddr *)&addr));
  TEST_NZ(rdma_listen(listener, 10)); /* backlog=10 is arbitrary */

  port = ntohs(rdma_get_src_port(listener));

  printf("listening on port %d.\n", port);

  //free
  running = 1;
  TEST_NZ(pthread_create(&free_mem_thread, NULL, (void *)free_mem, NULL));   // [?] Deamon thread

  //
  // [?] Not like kernel level client running in a notify mode, here has to poll the cm event manually ?
  //
  while (rdma_get_cm_event(ec, &event) == 0) {
    struct rdma_cm_event event_copy;

    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);    // [?] like TCP, need to response a acknowledgement to the RDMA package sender ?

    if (on_cm_event(&event_copy))   // RDMA communication event handler. 
      break;
  }

  rdma_destroy_id(listener);
  rdma_destroy_event_channel(ec);

  return 0;
}

/**
 * Cient server send a reques to build a RDMA connection.   
 * ACCEPT the RDMA conenction.
 * 
 * rdma_cm_id : is listening on the Ip of the IB.
 * 
 */
int on_connect_request(struct rdma_cm_id *id)
{
  struct rdma_conn_param cm_params;

  printf("received connection request.\n");
  build_connection(id);
  build_params(&cm_params);
  TEST_NZ(rdma_accept(id, &cm_params));  // ACCEPT the request to build RDMA connection.

  return 0;
}

/**
 *  After accept the RDMA connection from client.
 *  Send the free size of this server immediately. 
 *  !! Warning !! The client has to post a receive WR waiting for this already.
 * 
 */
int rdma_connected(struct rdma_cm_id *id)
{
  on_connect(id->context);

  printf("connection build\n");
  /* J: only server send mr, client doesn't */
  send_free_mem_size(id->context);

  return 0;
}


int on_disconnect(struct rdma_cm_id *id)
{
  printf("peer disconnected.\n");

  destroy_connection(id->context);
  return 0;
}


/**
 * Handle the communication(CM) event.
 *    a. Accept the RDMA connection request from client.
 *    b. Sent memory free size to client. 
 *    DONE.
 * 
 * More explanation
 *    Self defined behavior, for these RDMA CM event, send some RDMA WR back to caller.
 *    The caller has to post a receive WR to receive these WR ?
 * 
 */
int on_cm_event(struct rdma_cm_event *event)
{
  int r = 0;

  if (event->event == RDMA_CM_EVENT_CONNECT_REQUEST){    // 1) ACCEPT the RDMA conenct request from client.

    #ifdef DEBUG_RDMA_CLIENT
    printf("Get RDMA_CM_EVENT_CONNECT_REQUEST \n");
    #endif

    r = on_connect_request(event->id);                  //    event->id : rdma_cm_id 
  }else if (event->event == RDMA_CM_EVENT_ESTABLISHED){   // 2) After ACCEPT the connect request, server will get a RDMA_CM_EVENT_ESTABLISHED ack?
    
    #ifdef DEBUG_RDMA_CLIENT
    printf("Get RDMA_CM_EVENT_ESTABLISHED \n");
    #endif
    
    r = rdma_connected(event->id);                       //    send the free memory to client size of current server.
  }else if (event->event == RDMA_CM_EVENT_DISCONNECTED){

    #ifdef DEBUG_RDMA_CLIENT
    printf("Get RDMA_CM_EVENT_DISCONNECTED \n");
    #endif

    r = on_disconnect(event->id);
  }else{
    die("on_cm_event: unknown event.");
  }

  return r;
}

void usage(const char *argv0)
{
  fprintf(stderr, "usage: %s ip port\n", argv0);
  exit(1);
}
