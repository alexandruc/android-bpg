 LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := bpg_decoder
LOCAL_SRC_FILES := decoder.c DecoderWrapper.c

#LOCAL_CFLAGS += 

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libbpg
LOCAL_LDLIBS = -L$(SYSROOT)/usr/lib -llog

LOCAL_STATIC_LIBRARIES := bpg

include $(BUILD_SHARED_LIBRARY)