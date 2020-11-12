

#include "jvmti_class_instrument.hpp"



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
  res = jvm->GetEnv((void **) &jvmti, JVMTI_VERSION_11);
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
        
	const uint8_t* buffer = class_data;
	ClassFile class_file(&buffer);
	assert(((buffer - class_data) == class_data_len)
	       && "Incomplete class file read");

	ConstantPool* constant_pool = class_file.get_constant_pool();
	uint16_t o1_methodref_index = constant_pool->get_or_create_methodref_index(
		"java/lang/System",
		"test_with_parameter_obj",
		"(Ljava/lang/Object;)V"
	);

	Methods* methods = class_file.get_methods();
	for (auto& method : methods->get()) {

		// For some method, java.lang.Object can't be initialized ?
		// e.g., Type uninitializedThis (current frame, stack[0]) is not assignable to 'java/lang/Object'
		if (method->is_name("<clinit>") || method->is_name("<init>") ) {
			continue;
		}

		// FIXME: Bad hack to look for an object as the first arugment
		if (method->is_static()) {
			ConstantPoolUtf8* descriptor = method->get_descriptor_utf8();
			if (descriptor->get_data()[1] != 'L') {
				continue;
			}
		}

		Code* code = method->get_code();
		if (!code) {
			continue;
		}
		code->set_max_stack(code->get_max_stack() + 1);
		auto inserter = code->create_front_inserter();
		//inserter.insert_nop();
		inserter.insert_aload_0();
		inserter.insert_invokestatic(o1_methodref_index);
		code->sync();
		while (code->fix_offsets()) {
			code->sync();
		}
	}

	uint32_t byte_size = class_file.get_byte_size();
	assert((byte_size <= INT32_MAX) && "Class len must fit in an int");

	*new_class_data_len = byte_size;
	jvmti->Allocate(byte_size, new_class_data);
	class_file.write_buffer(new_class_data);



}

