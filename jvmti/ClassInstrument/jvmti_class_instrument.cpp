

#include "jvmti_class_instrument.h"



JNIEXPORT jint JNICALL 
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved){
  jvmtiEnv *jvmti;
  jvmtiError error;
  jint res;
  jvmtiCapabilities capa;  //
  jvmtiEventCallbacks callbacks;  // callback handler

  // debug - entry
  printf("%s, Register JVMTI Agent. \n", __func__);


  // Get the JVMTI environment
  res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_1_0);
  if (res != JNI_OK || jvmti == NULL) {
    printf("%s, Unable to get access to JVMTI version 1.0 \n", __func__);
  }



  // set the jvmtiCapabilities
  memset(&capa, 0, sizeof(capa));

  // sent the Event, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, before the VM is initialized 
  // if can_generate_early_class_hook_events and can_generate_all_class_hook_events are both set, 
  // sent the event in the primordial phase
  capa.can_generate_early_class_hook_events = 0;    
	capa.can_generate_all_class_hook_events = 1;

  capa.can_get_bytecodes = 1;
  capa.can_get_constant_pool = 1;
  capa.can_maintain_original_method_order = 1;

  error = (jvmti)->AddCapabilities(&capa); // apply the capabilities
  if(error != JVMTI_ERROR_NONE){
    return JNI_ERR;
  }

  // register the class file modification event trigger
  error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
	                                      JVMTI_EVENT_CLASS_FILE_LOAD_HOOK,
	                                      NULL);
	if (error != JVMTI_ERROR_NONE) {
		return JNI_ERR;
	}

	jvmtiEventCallbacks event_callbacks = {};
	event_callbacks.ClassFileLoadHook = ClassFileLoadHook;  // modify the bytecode of class file
	error = jvmti->SetEventCallbacks(&event_callbacks,
	                               sizeof(event_callbacks));
	if (error != JVMTI_ERROR_NONE) {
		return JNI_ERR;
	}

  return JNI_OK;
}



/**
 * Callback functions 
 *  
 *  Purpose:
 *  Instrument a function call within each method. 
 *  Push the object parameters into a prefetch queue.
 * 
 * 
 * Paramters:
 * 
 *  class_being_redefined: The class being redefined or retransformed. NULL if sent by class load.
 *  name: name of the class file.
 *  class_data_len: Length of current class file data buffer
 *  class_data: Pointer to the current class file data buffer.
 * 
 *  new_class_data: points to the modified new class file 
 * 
 * 
 *  [?] What's the difference between class_being_redefined and class_data ?
 *      class_being_redefined is the descriptor ?
 *      class_data is the real bytecode data ?
 * 
 * 
 * When this event is triggered, the class pointed by class_being_redefined is not loaded yet.
 * 
 * 
 */
void JNICALL ClassFileLoadHook(jvmtiEnv* jvmti,
                               JNIEnv* jni_env,
                               jclass class_being_redefined,
                               jobject loader,
                               const char* name,
                               jobject protection_domain,
                               jint class_data_len,
                               const unsigned char* class_data,
                               jint* new_class_data_len,
                               unsigned char** new_class_data) {
        
  jvmtiError error;                               
  int i;
  jclass *classes;
	jint constant_pool_count_ptr, constant_pool_byte_count_ptr;
  jint method_count_ptr;
  jmethodID* methods_ptr;
  jint count;
  unsigned char* constant_pool_bytes_ptr;

	error = jvmti->GetLoadedClasses(&count, &classes);
	if (error != JVMTI_ERROR_NONE) {
				printf("ERROR: JVMTI GetLoadedClasses failed, error code %d ! \n", error);
	}
			
  for (i = 0; i < count; i++) {
	  char *sig;
		jvmti->GetClassSignature(classes[i], &sig, NULL);
    if (error != JVMTI_ERROR_NONE) {
	    printf("ERROR: JVMTI GetClassSignature failed, error code %d !\n", error);
	  }
    printf("get class signature : %s \n",sig);
  }


  // how to use the Retransformclasses() function to instrument the byteocde ?


  // Loaded classes may not be prepared.
  //
  // for(i=0; i<count; i++){
  //   error = jvmti->GetClassMethods(classes[i], &method_count_ptr, &methods_ptr );
  //   if (error != JVMTI_ERROR_NONE) {
	//     printf("ERROR: JVMTI GetClassMethods failed, error code %d !\n", error);
	//   }
  // }


  // for(i=0; i<count; i++){
  //   error = jvmti->GetConstantPool(classes[i], &constant_pool_count_ptr, &constant_pool_byte_count_ptr, &constant_pool_bytes_ptr);
  //   if (error != JVMTI_ERROR_NONE) {
	//     printf("ERROR: JVMTI GetConstantPool failed, error code %d !\n", error);
	//   }
  // }


}

