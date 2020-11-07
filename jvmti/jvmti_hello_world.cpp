/**
 *  Execute the function defined in the agent, OnLoad phase.
 * 
 */

#include "jvmti.h"






/** 
 * JNIEXPORT ? 
 *  Tell the JVM to export a function, defined here.
 * 
 *  Parameters:
 *    JavaVM : the JVM is attached to
 *    void* : The parameters passed to the JVM.
 * 
 *  Return:
 *    0 : success,
 *   non-zero : error num.
 */
JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved){
  jvmtiEnv *jvmti;
  jvmtiError error;
  jint res;
  jvmtiCapabilities capa;  // ? 
  jrawMonitorID rawMonitorID;

  // Get the JVMTI environment
  res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);

  if (res != JNI_OK || jvmti == NULL) {
    printf("Unable to get access to JVMTI version 1.0");
  }

  // debug
  printf("Hello JVMTI. \n");



  return JNI_OK;
}


