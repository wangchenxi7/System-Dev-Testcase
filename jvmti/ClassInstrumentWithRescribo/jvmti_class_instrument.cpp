

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
	size_t i;
	ClassFile class_file(&buffer);
	assert(((buffer - class_data) == class_data_len)
	       && "Incomplete class file read");

	ConstantPool* constant_pool = class_file.get_constant_pool();
	uint16_t prefetch_method_index = constant_pool->get_or_create_methodref_index(
		"java/lang/System",
		"testWithParameter",
		"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;I)V"
	);

	Methods* methods = class_file.get_methods();
	for (auto& method : methods->get()) {

		// For some method, java.lang.Object can't be initialized ?
		// e.g., Type uninitializedThis (current frame, stack[0]) is not assignable to 'java/lang/Object'
		if (method->is_name("<clinit>") || method->is_name("<init>") || method->is_name("main") ) {
			continue;
		}



		// #1 Skip some useless functions
		// method descriptor is the function signatures
		// e.g., (Ljava/lang/Object;)V
		// ( paramters, separated by ; ), V : Void , the return value type
		ConstantPoolUtf8* descriptor = method->get_descriptor_utf8();
		if (descriptor->get_data()[1] == ')') {
			// skip non-parameter functions
			// e.g., ()V
			continue;
		}

		
		// #3, prepare to instrument parameters
		Code* code = method->get_code();
		if (!code) {
			continue;
		}
		code->set_max_stack(code->get_max_stack() + 6);
		auto inserter = code->create_front_inserter();

		
		// #3 Find all the object parameters, and push them into prefetch queue.
		size_t descriptor_size = descriptor->get_length();
		uint8_t param_count = 0;  // the local variable index
		bool obj_ref = false;
		uint16_t inserted_obj_ref_count= 0;


		for(i=0; i< descriptor_size; i++){
			if(descriptor->get_data()[i] == 'L' ){
				// find an object instance
				obj_ref = true;
			}else if(descriptor->get_data()[i] == ';'){
				// end of a parameter
				
				// Find a object refer, push into prefetch queu
				// L****;
				if(obj_ref){

					//inserter.insert_nop();
					//inserter.insert_aload_0();
					inserter.insert_aload(param_count); // aload index; load the [index] local variables into stack.

					printf("Find a object ref, paramter[%lu] \n", param_count);
					obj_ref = false; // end of instrumenting an parameter
					inserted_obj_ref_count++ ; // after instrumenting any parameters, instrument the function.
				}
				
				// Warning : Even if there are any constant paramters before the L##; the count of local variable should be correct.
				// Double check this assumption.
				param_count++;
			}else if(descriptor->get_data()[i] == ')'){
				// end of signature scanning
				// no need to scan the return values
				break;
			}

			// nothing to do.
		}

		// # 4, if didn't push any variables into stack, skip
		if(inserted_obj_ref_count == 0)
			continue;


		// #5, Update the modified function
		//		 The profiling functions assume less or equal than 5 object references are passed in.
		//		 The 6h parameter is the number of pushed object references.

		for( i = inserted_obj_ref_count; i< 5; i++){
			// Push NULL pointers into stack to be consumed by the function.
			inserter.insert_aconst_null();
		}

		// #5.2 push the number of valid prameters
		// e.g., bipush 10 /sipush
		inserter.insert_sipush(inserted_obj_ref_count);

		inserter.insert_invokestatic(prefetch_method_index);

		//debug
		printf("Instrumented method: ");
		method->print();
		printf("descriptor:");
		descriptor->print();


		code->sync();
		while (code->fix_offsets()) {
			code->sync();
		}
	} // end of for : method

	uint32_t byte_size = class_file.get_byte_size();
	assert((byte_size <= INT32_MAX) && "Class len must fit in an int");

	*new_class_data_len = byte_size;
	jvmti->Allocate(byte_size, new_class_data);
	class_file.write_buffer(new_class_data);



}

