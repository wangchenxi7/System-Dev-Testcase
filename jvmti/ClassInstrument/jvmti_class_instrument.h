#include "jvmti.h"


// C/C++ standard headers
#include <cstring>
#include "stdint.h"



jint Agent_OnLoad(JavaVM *jvm, char *options, void *reserved);
void Agent_OnUnload(JavaVM *vm);


void JNICALL ClassFileLoadHook(jvmtiEnv* env,
                               JNIEnv* jni_env,
                               jclass class_being_redefined,
                               jobject loader,
                               const char* name,
                               jobject protection_domain,
                               jint class_data_len,
                               const unsigned char* class_data,
                               jint* new_class_data_len,
                               unsigned char** new_class_data);
