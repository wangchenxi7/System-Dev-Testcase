/*
 * Infiniswap, remote memory paging over RDMA
 * Copyright 2017 University of Michigan, Ann Arbor
 * GPLv2 License
 */
#ifndef RDMA_COMMON_H
#define RDMA_COMMON_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <rdma/rdma_cma.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/kernel.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define TEST_NZ(x) do { if ( (x)) die("error: " #x " failed (returned non-zero)." ); } while (0)
#define TEST_Z(x)  do { if (!(x)) die("error: " #x " failed (returned zero/null)."); } while (0)

#define CQ_QP_BUSY 1
#define CQ_QP_IDLE 0
#define CQ_QP_DOWN 2


#ifdef USER_MAX_CLIENT
  #define MAX_CLIENT	USER_MAX_CLIENT
#else
  // #define MAX_CLIENT	32

  // debug
  #define MAX_CLIENT	1
#endif

#define EXTRA_CHUNK_NUM 2


#ifdef USER_MAX_REMOTE_MEMORY
  #define MAX_FREE_MEM_GB USER_MAX_REMOTE_MEMORY //for local memory management
  #define MAX_MR_SIZE_GB MAX_FREE_MEM_GB //for msg passing
#else
  #define MAX_FREE_MEM_GB 32    //for local memory management
  #define MAX_MR_SIZE_GB 32     //for msg passing
#endif


#define ONE_MB 1048576      // 1024 x 2014 bytes
#define ONE_GB 1073741824   // 1024 x 1024 x 1024 bytes

#ifdef USER_REMOTE_MEMORY_EVICT
  #define FREE_MEM_EVICT_THRESHOLD USER_REMOTE_MEMORY_EVICT //in GB
#else
  #define FREE_MEM_EVICT_THRESHOLD 8 //in GB
#endif

#ifdef USER_REMOTE_MEMORY_EXPAND
  #define FREE_MEM_EXPAND_THRESHOLD USER_REMOTE_MEMORY_EXPAND //in GB
#else
  #define FREE_MEM_EXPAND_THRESHOLD 16 // in GB
#endif

#ifdef USER_EVICT_HIT_LIMIT
  #define MEM_EVICT_HIT_THRESHOLD USER_EVICT_HIT_LIMIT
#else
  #define MEM_EVICT_HIT_THRESHOLD 1 
#endif

#ifdef USER_EXPAND_HIT_LIMIT
  #define MEM_EXPAND_HIT_THRESHOLD USER_EXPAND_HIT_LIMIT
#else
  #define MEM_EXPAND_HIT_THRESHOLD 20
#endif

#ifdef USER_MEASURED_FREE_MEM_WEIGHT
  #define CURR_FREE_MEM_WEIGHT USER_MEASURED_FREE_MEM_WEIGHT
#else
  #define CURR_FREE_MEM_WEIGHT 0.7
#endif



#define CHUNK_SIZE_GB					1   // Habe to be 1GB at current ! or will cause inconsistence problems. 

// Enable the debug information.
#define DEBUG_RDMA_CLIENT 1


#define ntohll(x) (((uint64_t)(ntohl((int)((x << 32) >> 32))) << 32) | \
        (unsigned int)ntohl(((int)(x >> 32))))

enum mode {
  M_WRITE,
  M_READ
};


/**
 * The self defined structure of the  RDMA package.  
 * 
 */
// struct message {
//   uint64_t buf[MAX_MR_SIZE_GB];
//   uint32_t rkey[MAX_MR_SIZE_GB];   // Used by remote(client)
//   int size_gb;
//   //uint64_t size;
//   enum {
//     DONE = 1, //C   // start from 1
//     MAP_CHUNKS, //S
//     INFO_SINGLE,
//     FREE_SIZE, //S
//     EVICT,          // 5,
//     ACTIVITY,
//     STOP, //S
//     BIND, //C
//     BIND_SINGLE,
//     QUERY //C       // 10,
//   } type;
// };

//
// Use  the new names.
//
struct message {
  	
	// Information of the chunk to be mapped to remote memory server.
	uint64_t buf[MAX_MR_SIZE_GB];		  // Remote addr, usd by clinet for RDMA read/write.
  uint32_t rkey[MAX_MR_SIZE_GB];   	// remote key
  int size_gb;						// Size of the chunk ?

	enum {
		DONE = 1,				      // Start from 1
		SEND_CHUNKS,				  // send the remote_addr/rkey of multiple Chunks
		SEND_SINGLE_CHUNK,		// send the remote_addr/rkey of a single Chunk
		FREE_SIZE,
		EVICT,        			  // 5
		ACTIVITY,				
		
		STOP,					        //7, upper SIGNALs are used by server, below SIGNALs are used by client.

		REQUEST_CHUNKS,
		REQUEST_SINGLE_CHUNK,	// Send a request to ask for a single chunk.
		QUERY         			  // 10
	} type;
};



/**
 * Used for RDMA conection.
 * 
 */
struct context {
  struct ibv_context *ctx;
  struct ibv_pd *pd;
  struct ibv_cq *cq;
  struct ibv_comp_channel *comp_channel;

  pthread_t cq_poller_thread;
};

struct atomic_t{
  int value;
  pthread_mutex_t mutex;
};


struct connection {

  struct rdma_session *sess;
  int conn_index;                         //conn index in sess->conns
  int sess_chunk_map[MAX_MR_SIZE_GB];     // [?] How to use this array ? -1 0r 1 ?
  int mapped_chunk_size;

  sem_t evict_sem;
  sem_t stop_sem;

  struct rdma_cm_id *id;
  struct ibv_qp *qp;

  int connected;

  struct ibv_mr *recv_mr;       // Used for RDMA message passing.
  struct ibv_mr *send_mr;
  struct ibv_mr *rdma_remote_mr;  // [??]

  struct ibv_mr peer_mr;

  struct message *recv_msg;
  struct message *send_msg;

  char *rdma_remote_region;
  //struct rdma_remote_mem rdma_remote;

  struct atomic_t cq_qp_state;

  pthread_t free_mem_thread;
  long free_mem_gb;
  unsigned long rdma_buf_size;

  enum {
    S_WAIT,
    S_BIND,
    S_DONE
  } server_state;

  enum {
    SS_INIT,
    SS_MR_SENT, 
    SS_STOP_SENT,
    SS_DONE_SENT
  } send_state;

  enum {
    RS_INIT,
    RS_STOPPED_RECV,
    RS_DONE_RECV
  } recv_state;
};

#define CHUNK_MALLOCED 1
#define CHUNK_EMPTY	0
struct rdma_remote_mem{
  char*           region_list[MAX_FREE_MEM_GB];       // [?] Start address of the chunk. 
  struct ibv_mr*  mr_list[MAX_FREE_MEM_GB];           // The corresponding Memory Region of region_list. 
  int size_gb; 
  int mapped_size;
  int conn_map[MAX_FREE_MEM_GB];        // chunk is used by which connection, or -1
  int malloc_map[MAX_FREE_MEM_GB];      // Record if this chunk malloced or not.
  int conn_chunk_map[MAX_FREE_MEM_GB];  // session_chunk 
};

enum conn_state{
  CONN_IDLE,
  CONN_CONNECTED,
  CONN_MAPPED,
  CONN_FAILED
};

struct chunk_activity{
  uint64_t activity;
  int chunk_index;
};


struct rdma_session {
	struct  connection*   conns[MAX_CLIENT];        // need to init NULL. One "struct connection" per client.
  enum    conn_state    conns_state[MAX_CLIENT];
	int                   conn_num;	

	struct rdma_remote_mem  rdma_remote;		// Manage all the remote chunk within this session
  struct chunk_activity   *evict_list;

};

void die(const char *reason);

void build_connection(struct rdma_cm_id *id);
void build_params(struct rdma_conn_param *params);
void destroy_connection(void *context);
void * get_serving_mem_region(void *context);
void on_connect(void *context);
void send_single_chunk_to_client(void *context, int client_chunk_index);
void send_chunks_to_client(void *context, int size_gb);
void send_stop(void *context, int n);
void send_evict(void *context, int n);
void send_free_mem_size(void *context);
void* free_mem(void *data);

#endif
