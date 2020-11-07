
#include "jvmti.h"


//
// Functions 
//

jint Agent_OnLoad(JavaVM *jvm, char *options, void *reserved);
void Agent_OnUnload(JavaVM *vm);

void MethodEntry(jvmtiEnv *jvmti_env, JNIEnv* jni_env, jthread thread,jmethodID method);
