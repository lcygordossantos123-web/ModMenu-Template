extern "C" void StartHack();
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Macros.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Menu/Setup.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <jni.h>
#include <list>
#include <pthread.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define targetLibName OBFUSCATE("libil2cpp.so")

void *hack_thread(void *) {
  LOGD(OBFUSCATE("pthread created"));

  do {
    sleep(1);
  } while (!isLibraryLoaded(targetLibName));

  LOGD(OBFUSCATE("%s has been loaded"), (const char *)targetLibName);

#if defined(__aarch64__)
  StartHack();                 // <-- aqui, depois do while
#else
  LOGD(OBFUSCATE("Done"));
#endif

  return nullptr;
}

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
  jobjectArray ret;

  const char *features[] = {
      // coloque seus toggles/sliders aqui depois
  };

  int Total_Feature = (sizeof features / sizeof features[0]);
  ret = (jobjectArray)env->NewObjectArray(
      Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
      env->NewStringUTF(""));

  for (int i = 0; i < Total_Feature; ++i)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

  return (ret);
}

void Changes(JNIEnv *env, jclass clazz, jobject obj, jint featNum,
             jstring featName, jint value, jboolean boolean, jstring str) {

  const char *name = env->GetStringUTFChars(featName, nullptr);
  const char *text = str != nullptr ? env->GetStringUTFChars(str, nullptr) : "";

  LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d | Text: = %s"),
       featNum, name, static_cast<int>(value), boolean, text);

  switch (featNum) {
      break;
  }
}

__attribute__((constructor)) void lib_main() {
  pthread_t ptid;
  pthread_create(&ptid, nullptr, hack_thread, nullptr);
}

int RegisterMenu(JNIEnv *env) {
  JNINativeMethod methods[] = {
      {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"),
       reinterpret_cast<void *>(Icon)},
      {OBFUSCATE("IconWebViewData"), OBFUSCATE("()Ljava/lang/String;"),
       reinterpret_cast<void *>(IconWebViewData)},
      {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE("()Z"),
       reinterpret_cast<void *>(isGameLibLoaded)},
      {OBFUSCATE("Init"),
       OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/"
                 "widget/TextView;)V"),
       reinterpret_cast<void *>(Init)},
      {OBFUSCATE("SettingsList"), OBFUSCATE("()[Ljava/lang/String;"),
       reinterpret_cast<void *>(SettingsList)},
      {OBFUSCATE("GetFeatureList"), OBFUSCATE("()[Ljava/lang/String;"),
       reinterpret_cast<void *>(GetFeatureList)},
  };

  jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
  if (!clazz)
    return JNI_ERR;
  if (env->RegisterNatives(clazz, methods,
                           sizeof(methods) / sizeof(methods[0])) != 0)
    return JNI_ERR;
  return JNI_OK;
}

int RegisterPreferences(JNIEnv *env) {
  JNINativeMethod methods[] = {
      {OBFUSCATE("Changes"),
       OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/"
                 "String;)V"),
       reinterpret_cast<void *>(Changes)},
  };
  jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
  if (!clazz)
    return JNI_ERR;
  if (env->RegisterNatives(clazz, methods,
                           sizeof(methods) / sizeof(methods[0])) != 0)
    return JNI_ERR;
  return JNI_OK;
}

int RegisterMain(JNIEnv *env) {
  JNINativeMethod methods[] = {
      {OBFUSCATE("CheckOverlayPermission"),
       OBFUSCATE("(Landroid/content/Context;)V"),
       reinterpret_cast<void *>(CheckOverlayPermission)},
  };
  jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
  if (!clazz)
    return JNI_ERR;
  if (env->RegisterNatives(clazz, methods,
                           sizeof(methods) / sizeof(methods[0])) != 0)
    return JNI_ERR;

  return JNI_OK;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
    return JNI_ERR;

  if (RegisterMenu(env) != 0)
    return JNI_ERR;
  if (RegisterPreferences(env) != 0)
    return JNI_ERR;
  if (RegisterMain(env) != 0)
    return JNI_ERR;
  return JNI_VERSION_1_6;
}
