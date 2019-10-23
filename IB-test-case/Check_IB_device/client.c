#include <stdio.h>
#include <infiniband/verbs.h>


int  main(int argc, char* argv[]){

  struct ibv_device **device_list;
  struct ibv_context *ctx;
  struct ibv_device_attr *device_attr;
  int ret = 0;


  device_list = ibv_get_device_list(NULL);
  if(!device_list){
    printf("ibv_get_device_list failed. \n");
    return -1;
  }

  ctx = ibv_open_device(device_list[0]);
  if (!ctx) {
    fprintf(stderr, "Error, failed to open the device '%s'\n", ibv_get_device_name(device_list[0]));
    return -1; 
  }else{
    printf("The device '%s' was opened\n", ibv_get_device_name(ctx->device));
  }

  device_attr = (struct ibv_device_attr *)malloc(sizeof(struct ibv_device_attr));
  memset(device_attr, '\0', sizeof(struct ibv_device_attr));

  ret = ibv_query_device(ctx, device_attr);
  if(ret != 0){
    printf("ibv_query_device failed. \n");
    return -1;
  }else{
      //print the attributs of the infiniband device.
      printf("max_mr_size: 0x%llx \n", device_attr->max_mr_size );
      printf("max_qp: 0x%x \n", device_attr->max_qp );
      printf("max_qp_wr: 0x%x \n", device_attr->max_qp_wr );
      printf("max_sge: 0x%x \n", device_attr->max_sge );
  }


  // Close the device
  ret = ibv_close_device(ctx);
  if (ret) {
    fprintf(stderr, "Error, failed to close the device '%s'\n",  ibv_get_device_name(ctx->device));
    return ret;
  }

  return ret;
}