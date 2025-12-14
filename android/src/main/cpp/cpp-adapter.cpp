#include <jni.h>
#include "NitroDuckdbOnLoad.hpp"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
  return margelo::nitro::duckdb::initialize(vm);
}
