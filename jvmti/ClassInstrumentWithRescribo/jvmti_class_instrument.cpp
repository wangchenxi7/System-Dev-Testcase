

#include "jvmti_class_instrument.hpp"

JNIEXPORT jint JNICALL
Agent_OnLoad(JavaVM *jvm, char *options, void *reserved)
{
	jvmtiEnv *jvmti;
	jvmtiError error;
	jint res;
	jvmtiCapabilities capa;				 //
	jvmtiEventCallbacks callbacks; // callback handler

	// debug - entry
	printf("%s, Register JVMTI Agent. \n", __func__);

	// Get the JVMTI environment
	res = jvm->GetEnv((void **)&jvmti, JVMTI_VERSION_11);
	if (res != JNI_OK || jvmti == NULL)
	{
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
	if (error != JVMTI_ERROR_NONE)
	{
		return JNI_ERR;
	}

	// register the class file modification event trigger
	error = jvmti->SetEventNotificationMode(JVMTI_ENABLE,
																					JVMTI_EVENT_CLASS_FILE_LOAD_HOOK,
																					NULL);
	if (error != JVMTI_ERROR_NONE)
	{
		return JNI_ERR;
	}

	jvmtiEventCallbacks event_callbacks = {};
	event_callbacks.ClassFileLoadHook = ClassFileLoadHook; // modify the bytecode of class file
	error = jvmti->SetEventCallbacks(&event_callbacks,
																	 sizeof(event_callbacks));
	if (error != JVMTI_ERROR_NONE)
	{
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
void JNICALL ClassFileLoadHook(jvmtiEnv *jvmti,
															 JNIEnv *jni_env,
															 jclass class_being_redefined,
															 jobject loader,
															 const char *name,
															 jobject protection_domain,
															 jint class_data_len,
															 const unsigned char *class_data,
															 jint *new_class_data_len,
															 unsigned char **new_class_data)
{

	const uint8_t *buffer = class_data;  // the bytecode data
	bool print_class = false;

	//debug - print before instrumentation
	#ifdef DEBUG_INSTRUMENT_BRIEF
	//if(strcmp(name, "objItem") == 0){
	if (strcmp(name, "java/util/zip/ZipFile") == 0) { 
		printf("Find the %s, print the instrumented calss to debug-before.class\n", name);

	  std::ofstream modified_class;
  	modified_class.open("debug-before.class");
		modified_class.write( (const char* )class_data, class_data_len);
		modified_class.close();
	}
	#endif


	int i,j;
	ClassFile class_file(&buffer);
	assert(((buffer - class_data) == class_data_len) && "Incomplete class file read");

	ConstantPool *constant_pool = class_file.get_constant_pool();
	uint16_t prefetch_method_index = constant_pool->get_or_create_methodref_index(
			"java/lang/System",
			"testWithParameter",
			"(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;I)V");

	Methods *methods = class_file.get_methods();
	for (auto &method : methods->get())
	{

		// For some method, java.lang.Object can't be initialized ?
		// e.g., Type uninitializedThis (current frame, stack[0]) is not assignable to 'java/lang/Object'
		if (function_filtered(method))
		{
			continue;
		}

		// #1 Skip some useless functions
		// method descriptor is the function signatures
		// e.g., (Ljava/lang/Object;)V
		// ( paramters, separated by ; ), V : Void , the return value type
		ConstantPoolUtf8 *descriptor = method->get_descriptor_utf8();
		if (descriptor->get_data()[1] == ')')
		{
			// skip non-parameter functions
			// e.g., ()V
			continue;
		}

		// #3, prepare to instrument parameters
		Code *code = method->get_code();
		if (!code)
		{
			continue;
		}
		code->set_max_stack(code->get_max_stack() + 6); // make enough space for 6 method paramters.
		auto inserter = code->create_front_inserter();

		// #3 Parse the method signature
		// Find all the object parameters, and push them into prefetch queue.
		// descriptors for local viriable
		// Primitives (Sinlge charactor)
		//	short -> S;
		//	int -> I;
		//	long -> J;
		//	double -> D;
		//	float -> F;
		//	boolean	-> Z;
		//  byte	-> B;
		//  char  -> C;
		//
		//
		// Object instance
		// 	start with L, end with ; e.g., L***;
		//
		// Object array
		//	start with [, e.g., [L***;
		size_t descriptor_size = descriptor->get_length();
		uint8_t param_count = 0; // the local variable index
		uint8_t non_static_local_val_offset = 0;
		bool obj_ref = false;
		bool obj_array = false;
		uint16_t inserted_obj_ref_count = 0;
	

		// The first local vairable of non-static method is *this*
		// The first local variable of static method is parameter 0.
		if (method->is_static() == false){
				non_static_local_val_offset = 1;
		}


		// parse the signature
		// first character is the '(', skip it.
		for (i = 1; i < descriptor_size  && inserted_obj_ref_count < PARAM_NUM ; i++)
		{

			//1) Find an array. Could be primitive array or object array.
			if (descriptor->get_data()[i] == '[')
			{
				obj_array = parse_object_array(descriptor, i);
				if(obj_array){
					// Push an local variable into the stack
					//inserter.insert_aload(param_count + non_static_local_val_offset); // aload index; load the [index] local variables into stack.
					insert_reference_load_instruction(param_count, non_static_local_val_offset, inserter);

					// debug insert null
					//inserter.insert_aconst_null();
					
					#ifdef DEBUG_INSTRUMENT_BRIEF
						printf("Find an object ref for paramter[%u]. obj_array ? %d , non-static-offset %u \n", 
																												param_count, 
																												obj_array, 
																												non_static_local_val_offset);
					#endif
				

					obj_array = false;
					inserted_obj_ref_count++; // after instrumenting any parameters, instrument the function.				
				}

				// update local variable index
				// Waring : for a primitve type, [D and [L, they are object ref and consume 1 local variable slot.
				param_count++;
				continue; // consume the ;
			}
			else if (descriptor->get_data()[i] == 'L' && descriptor->get_data()[i-1] != '[' )
			{
				// 2) object instance	
				obj_ref = parse_object_instance(descriptor, i);
				if(obj_ref){
					// Push an local variable into the stack
					//inserter.insert_aload(param_count + non_static_local_val_offset); // aload index; load the [index] local variables into stack.
					insert_reference_load_instruction(param_count, non_static_local_val_offset, inserter);

					//debug
					//inserter.insert_aconst_null();
					
					
					#ifdef DEBUG_INSTRUMENT_BRIEF
						// parameter count from 0.
						printf("Find an object ref for paramter[%u]. obj_ref ? %d , non-static-offset %u \n", 
																												param_count, 
																												obj_ref, 
																												non_static_local_val_offset);
					#endif

					obj_ref = false;
					inserted_obj_ref_count++; // after instrumenting any parameters, instrument the function.		
				}

				// update local variable index
				param_count++;
				continue;  // consume the ;
			}
			// else if (descriptor->get_data()[i] == ';')
			// {
			// 	// end of an object/object array descriptor

			// 	// Find a object refer, push into prefetch queu
			// 	// L****; or [L**;
			// 	// Debug skipped object array for now
			// 	if(obj_ref || obj_array )
			// 	{
			// 		//inserter.insert_nop();
			// 		//inserter.insert_aload_0();
			// 		inserter.insert_aload(param_count + non_static_local_val_offset); // aload index; load the [index] local variables into stack.
			// 		#ifdef DEBUG_INSTRUMENT_BRIEF
			// 			printf("Find an object ref for paramter[%u]. obj_ref ? %d , non-static-offset %u \n", 
			// 																									param_count + non_static_local_val_offset, 
			// 																									obj_ref, 
			// 																									non_static_local_val_offset);
			// 		#endif

			// 		obj_ref = false; // end of instrumenting an parameter
			// 		obj_array = false;
			// 		inserted_obj_ref_count++; // after instrumenting any parameters, instrument the function.
			// 	}

			// 	param_count++;
			// 	//printf("Find an ;, ++, to %u\n", param_count);
			// }

			// Parse the next Signature symbols
			// [XX] according to current design, both obj_ref and obj_array are false, if reach here.
			// Go to parse the primives
			if ((obj_ref || obj_array) == false)
			{

				if (descriptor->get_data()[i] == 'D')
				{
					// Double, consume 2 local variable slots
					param_count += 2;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an D, +2, from %u to %u\n", param_count - 2, param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'J')
				{
					// long, consume 2 local variable slots
					param_count += 2;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an J, +2, from %u to %u\n", param_count - 2, param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'I')
				{
					// Int, consume 1 local variable slots
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an I, ++, from %u to %u\n", param_count - 1, param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'S')
				{
					// Short, consume 1 local variable slot
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an S, ++, from %u to %u\n", param_count - 1,param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'F')
				{
					// Float, consume 1 local variable slot
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an F, ++, from %u to %u\n", param_count - 1,param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'Z')
				{
					// Boolean, consume 1 local variable slot
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an Z, ++, from %u to %u\n",param_count - 1, param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'C')
				{
					// Char, consume 1 local variable slot
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an C, ++, from %u to %u\n",param_count - 1, param_count);
					#endif
				}
				else if (descriptor->get_data()[i] == 'B')
				{
					// Byte, consume 1 local variable slot
					param_count++;
					#ifdef DEBUG_INSTRUMENT_DETAIL
					printf("Find an B, ++, from %u to %u\n",param_count - 1, param_count);
					#endif
				}

			} // end of parsing primitives

			// end of parsing signature
			if (descriptor->get_data()[i] == ')')
			{
				// end of signature scanning
				// no need to scan the return values
				break;
			}

			//nothing to do.

		} // end of for, parsing the signature

		// # 4, if didn't push any variables into stack, skip
		if (inserted_obj_ref_count == 0)
			continue;

		// #5, Update the modified function
		//		 The profiling functions assume less or equal than 5 object references are passed in.
		//		 The 6h parameter is the number of pushed object references.

		for (i = inserted_obj_ref_count; i < PARAM_NUM; i++)
		{
			// Push NULL pointers into stack to be consumed by the function.
			inserter.insert_aconst_null();
		}

		// #5.2 push the number of valid prameters
		// e.g., bipush 10 /sipush
		inserter.insert_sipush(inserted_obj_ref_count);

		inserter.insert_invokestatic(prefetch_method_index);

		#ifdef DEBUG_INSTRUMENT_BRIEF
			printf("Instrumented method: ");
			method->print();
			printf("descriptor:");
			descriptor->print();
			printf("<---END\n");
		#endif

		code->sync();
		while (code->fix_offsets())
		{
			code->sync();
		}

	} // end of for : method

	uint32_t byte_size = class_file.get_byte_size();
	assert((byte_size <= INT32_MAX) && "Class len must fit in an int");

	*new_class_data_len = byte_size;
	jvmti->Allocate(byte_size, new_class_data);
	class_file.write_buffer(new_class_data);

	// debug - print after instrumentation
	#ifdef DEBUG_INSTRUMENT_BRIEF
	// write the modified class to file

	// if (strcmp(name, "java/util/jar/JarFile") == 0) 

	// Debug the instrumented class
	// e.g.,
	// java/util/jar/JarFile, ::getEntry
	// java/util/zip/ZipFile, getEntry
	print_to_file("java/util/zip/ZipFile", name, (const char* )(*new_class_data),  *new_class_data_len);
	// if(print_class)
	// 	print_to_file(NULL, name, (const char* )(*new_class_data),  *new_class_data_len);
	// // reset to false
	// print_class = false;
	#endif

}

/**
 * The failed corner cases.
 * Skip them.
 * 
 */
bool function_filtered(std::unique_ptr<Method> &method)
{

	bool skip = false;

	if (method->is_name("<clinit>") || method->is_name("<init>") || method->is_name("main"))
	{
		skip = true;
	}
	else if (method->is_name("linkToTargetMethod"))
	{
		skip = true;
	}

ret:
	return skip;
}


/**
 *  Recognize the objct array from the method signature.
 * 	Object array is treated as normal object instance.
 * 
 * e.g., 
 * 	Lcom/ctc/wstx/io/WstxInputSource;Ljava/lang/String;[CIILjavax/xml/stream/Location;Ljava/net/URL;
 * 	
 * 	object instance:
 * 		Lcom/ctc/wstx/io/WstxInputSource;
 * 		Ljava/lang/String;
 * 		Ljavax/xml/stream/Location;
 * 		Ljava/net/URL;
 * 
 * 	Primivies :
 * 		[C	// primitive array
 * 		I		// single primitive
 * 		I
 * 		L
 * 
 *	Consume the string within the singnature descriptor,
 *	Update the idnex of descriptor string, i
 *	Update the current parameter counter
 * 	 
 * 	End with a another kind of parameter.
 * 	e.g.,
 * 		primitive types,
 * 		;
 * 
 *
 * 	Return :
 * 		True or False : if found an object array parameter
 * 
 * 	Warning : 
 * 		[ is a symbol for array. NOT necessarily an object array.
 *			It may also be a primitive array. .e.g, char[], which is [C
 * 
 */
bool parse_object_array(ConstantPoolUtf8 *descriptor, int &descriptor_index ){

	bool find_obj_array = false;

	// Entrance 
	// 	Found a character, [
	#ifdef DEBUG_INSTRUMENT_BRIEF
	 if(descriptor->get_data()[descriptor_index] != '[' ){
		 printf("ERROR in %s\n",__func__);
		 return false;
	 }
	#endif


	
	// 1) consume the [
  // Points to the first [ now.
	// one dimension [${primitive_type}
	// 2 and more [...[${primitive_type}
	do{

		descriptor_index++; 
	}while(descriptor->get_data()[descriptor_index] == '[');


	// 2) Recognize the primitive array
	// if the symbol following the last [ is a primitive type, then this is a primitive array.
	// OR it has to be L.
	// To handle the primitve array.
	// e.g., [B
	// we are pointing the B now.
	if( is_primive_type(descriptor, descriptor_index) == true  ){
		// this is a primitive array,
		// pointing the primitive symbol, B. 
		// This symbol will be consumed by the for loop.
		// return false
		goto ret;
	}

	// 3) recognize the object array
	#ifdef DEBUG_INSTRUMENT_BRIEF
		if(descriptor->get_data()[descriptor_index] != 'L'){
			printf("\n%s, parameter[%u] is not [.  parsing object array failed !! \n\n", descriptor_index );
			goto ret;
		}
	#endif

	do{
		// Warning : For an object array, the chracters betwee [L and ; may contain some characters same as primitive type
		// e.g., [Lio/netty/util/concurrent/GenericFutureListener;

		descriptor_index ++; // consume one more symbol
	}while(descriptor->get_data()[descriptor_index] != ';' );
	
	// Leave the ; to be consumed by for loop


	// if reach here, find an object array
	// return true
	find_obj_array = true;

ret:
	return find_obj_array;
}


/**
 * Parse an object instance.
 * e.g., Ljava/lang/String;
 * 
 * The object instances all start with a L.
 * Exclude the object array here, which starts with [L
 * 
 * 
 * Return Value
 * 		true : reconize an object instance successfully
 * 		false : exception happens
 * 
 */
bool parse_object_instance(ConstantPoolUtf8 *descriptor, int &descriptor_index ){
	// Entrance 
	// Found a character, L
	descriptor_index++; // consume the L

	do{

		descriptor_index ++; // consume one more symbol
	}while(descriptor->get_data()[descriptor_index] != ';' );

	// Leave the ; to be consumed by for loop

	return true;
}


/**
 * To judge if current Signature character is a primitve type
 * 
 * Primitives (Sinlge charactor)
 *	 	short -> S;
 *		int -> I;
 *		long -> J;
 *		double -> D;
 *		float -> F;
 *		boolean	-> Z;
 *		byte	-> B;
 *		char  -> C;
 *
 * More Explanation
 *
 */
bool is_primive_type(ConstantPoolUtf8 *descriptor, int descriptor_index ){

	switch(descriptor->get_data()[descriptor_index]) {

		case 'S':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type S, Short\n");
				return true;
			#endif
		case 'I':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type I, Int\n");
				return true;
			#endif
		case 'F':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type F, Float\n");
				return true;
			#endif
		case 'Z':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type Z, boolean\n");
				return true;
			#endif
		case 'B':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type B, Byte\n");
				return true;
			#endif
		case 'C':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type C, Char\n");
				return true;
			#endif
		case 'J':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type J, Long\n");
				return true;
			#endif
		case 'D':
			#ifdef DEBUG_INSTRUMENT_DETAIL
				printf("Find primitive type D, Double\n");
			#endif

			return true;
			break;

		default:
			return false;

	}// end of switch

	
	return false;
}


/**
 * Insert the alod_# and aload index instruction.
 *  
 */
void insert_reference_load_instruction(uint8_t param_count, uint8_t non_static_local_val_offset, Code::InstructionInserter &inserter){

	uint8_t local_val_index = param_count + non_static_local_val_offset;

	switch (local_val_index)
	{
	case (uint8_t)0:
		inserter.insert_aload_0();
		break;

	case (uint8_t)1:
		inserter.insert_aload_1();
		break;
	
	case (uint8_t)2:
		inserter.insert_aload_2();
		break;

	case (uint8_t)3:
		inserter.insert_aload_3();
		break;
	
	default:
		inserter.insert_aload(local_val_index);  // index is larger than 3

		break;
	}


}






//
// Debug functions
//


/**
 * Print the class file of the code to a specific file
 *  
 */
void print_to_file(const char *class_to_be_printed, const char* cur_class_name , const char* class_buffer, int byte_length ){
	
	bool print = false;

	if(class_to_be_printed == NULL){
		// not specific any class, print all the class
		print = true;
	}else if( strcmp(class_to_be_printed, cur_class_name) == 0 ){
		// print specific class
		print = true;
	}


	if(print){
		
		// The class name contains /, can be a problem here.
		//char debug_suffix[13] = "-debug.class";
		//char *file_name = (char *)malloc(strlen(cur_class_name) + strlen(debug_suffix) +1 );

		//strcpy(file_name, cur_class_name);
		//strcat(file_name, debug_suffix);

		//debug
		char file_name[18] = "debug-after.class";

		#ifdef DEBUG_INSTRUMENT_BRIEF
			printf("Find the %s, print the instrumented calss to %s\n", cur_class_name, file_name);
		#endif

	  std::ofstream modified_class;
  	modified_class.open(file_name);
		modified_class.write( class_buffer, byte_length);
		modified_class.close();
	}


	return;
}