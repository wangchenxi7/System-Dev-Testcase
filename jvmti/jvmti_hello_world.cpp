/**
 *  Execute the function defined in the agent, OnLoad phase.
 * 
 * 
 * Current problems:
 *      MethodEntry doesn't work ?
 */



#include "jvmti_hello_world.h"




/** 
 * JNIEXPORT 
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
  jvmtiCapabilities capa = {};  //init to \0 ; 
  jvmtiEventCallbacks callbacks = {};  // callback handler

  // debug - entry
  printf("Hello JVMTI. \n");


  // Get the JVMTI environment
  res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);
  if (res != JNI_OK || jvmti == NULL) {
    printf("Unable to get access to JVMTI version 1.0");
  }

  // set the jvmtiCapabilities
  capa.can_get_bytecodes = 1;
  capa.can_get_constant_pool = 1; // get the method 

  capa.can_generate_method_entry_events = 1;  // when execute a method
  capa.can_generate_method_exit_events  = 1;  // when exit a method
  capa.can_access_local_variables = 1;    // method parameters are within local variables

  error = (jvmti)->AddCapabilities(&capa); // apply the capabilities

  // register the callback function
  error = (jvmti)->SetEventCallbacks(&callbacks, (jint)sizeof(callbacks));
  if(error != JNI_OK){
    return JNI_ERR;
  }

  // invoke the registered MethodEntry, when the event JVMTI_EVENT_METHOD_ENTRY is triggered.
  // JVMTI doesn't recommend using the MethodEntry. It's too slow.
  callbacks.MethodEntry = &MethodEntry;
  //callbacks.MethodExit = &MethodExit;

  error = (jvmti)->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_ENTRY, (jthread)NULL);  
  if(error != JNI_OK){
    return JNI_ERR;
  }    
  error = (jvmti)->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_METHOD_EXIT, (jthread)NULL);  
  if(error != JNI_OK){
    return JNI_ERR;
  }  


  return JNI_OK;
}


/**
 * Optional exit point.
 *  
 */
void Agent_OnUnload(JavaVM *vm){
    printf("Agent Unloaded\n");
}


/**
 * Callback function,
 *  print the executed method's information.
 *  
 */
void MethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread, jmethodID method){
  jvmtiError error;
  char* name_p;
  char* signature_p;
  char* generic_p;

  error = (jvmti_env)->GetMethodName(method, &name_p, &signature_p, &generic_p);

  printf("Method Entry NAME:%s SIG:%s GEN:%s entered ==>\n", name_p, signature_p, generic_p);
}