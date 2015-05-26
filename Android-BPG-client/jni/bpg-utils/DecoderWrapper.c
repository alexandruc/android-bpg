#include "DecoderWrapper.h"

#include "decode.h"
#include <stdlib.h>
#include <android/log.h>

extern int bpg_get_buffer_size_from_bpg(uint8_t *bpgBuffer, int bpgBufferSize);
extern void decode_buffer(uint8_t* bufIn, unsigned int bufInLen, uint8_t** bufOut, unsigned int* bufOutLen, enum DecodeTo format);

JNIEXPORT jint JNICALL Java_com_example_android_1bpg_1client_DecoderWrapper_fetchDecodedBufferSize
(JNIEnv *env, jclass class, jbyteArray encBuffer, jint encBufferSize)
{
	jboolean isCopy;
	int capacity = 0;
	jbyte* cEncArray = (*env)->GetByteArrayElements(env, encBuffer, &isCopy);
	if(NULL == cEncArray){
		return -1;
	}
	else{
		capacity = bpg_get_buffer_size_from_bpg(cEncArray, encBufferSize);

		(*env)->ReleaseByteArrayElements(env, encBuffer, cEncArray, JNI_ABORT);
	}
	return capacity;
}

JNIEXPORT jbyteArray JNICALL Java_com_example_android_1bpg_1client_DecoderWrapper_decodeBuffer
	(JNIEnv *env, jclass class, jbyteArray encBuffer, jint encBufferSize)
{
	jboolean isCopy;
	jbyteArray decBuffer;
	//get c-style array
	jbyte* cEncArray = (*env)->GetByteArrayElements(env, encBuffer, &isCopy);
	if(NULL == cEncArray){
		__android_log_print(ANDROID_LOG_INFO, "decodeBufferV2", "FAILED to allocate cEncArray");
		return NULL;
	}
	else{
		uint8_t* outBuf;
		unsigned int outBufSize = 0;
		decode_buffer(cEncArray, encBufferSize, &outBuf, &outBufSize, BMP);

		//convert back to java-style array
		decBuffer = (*env)->NewByteArray(env, outBufSize);
		if(NULL == decBuffer){
			(*env)->ReleaseByteArrayElements(env, encBuffer, cEncArray, JNI_ABORT);
			__android_log_print(ANDROID_LOG_INFO, "decodeBuffer", "FAILED to allocate decBuffer : outBufferSize=%d", outBufSize);
			return NULL;
		}
		else{
			(*env)->SetByteArrayRegion(env, decBuffer, 0, outBufSize, outBuf);
		}
		(*env)->ReleaseByteArrayElements(env, encBuffer, cEncArray, JNI_ABORT);
	}
	return decBuffer;
}

static JNINativeMethod method_table[] = {
		{"fetchDecodedBufferSize", "([BI)I", (void *) Java_com_example_android_1bpg_1client_DecoderWrapper_fetchDecodedBufferSize},
		{"decodeBuffer", "([BI)[B", (void *) Java_com_example_android_1bpg_1client_DecoderWrapper_decodeBuffer},
};

static int method_table_size = sizeof(method_table) / sizeof(method_table[0]);
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env;
	if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
		return JNI_ERR;
	} else {
		jclass clazz = (*env)->FindClass(env, "com/example/android_bpg_client/DecoderWrapper");
		if (clazz) {
			jint ret = (*env)->RegisterNatives(env, clazz, method_table, method_table_size);
			(*env)->DeleteLocalRef(env, clazz);
			return ret == 0 ? JNI_VERSION_1_6 : JNI_ERR;
		} else {
			return JNI_ERR;
		}
	}
}
