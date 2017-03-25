 LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := bpg
LOCAL_SRC_FILES := libbpg.c \
					libavcodec/hevc_cabac.c \
					libavcodec/hevc_filter.c \
					libavcodec/hevc.c \
					libavcodec/hevcpred.c \
					libavcodec/hevc_refs.c \
					libavcodec/hevcdsp.c \
					libavcodec/hevc_mvs.c \
					libavcodec/hevc_ps.c \
					libavcodec/hevc_sei.c \
					libavcodec/utils.c \
					libavcodec/cabac.c \
					libavcodec/golomb.c \
					libavcodec/videodsp.c \
					libavutil/buffer.c \
					libavutil/frame.c \
					libavutil/log2_tab.c \
					libavutil/md5.c \
					libavutil/mem.c \
					libavutil/pixdesc.c

LOCAL_CFLAGS += -Os \
				-Wall \
				-MMD \
				-fno-asynchronous-unwind-tables \
				-fdata-sections \
				-ffunction-sections \
				-fno-math-errno \
				-fno-signed-zeros \
				-fno-tree-vectorize \
				-fomit-frame-pointer \
				-D_FILE_OFFSET_BITS=64 \
				-D_LARGEFILE_SOURCE \
				-D_REENTRANT \
				-g \
				-D_ISOC99_SOURCE \
				-D_POSIX_C_SOURCE=200112 \
				-D_XOPEN_SOURCE=600 \
				-DHAVE_AV_CONFIG_H \
				-std=c99 \
				-D_GNU_SOURCE=1 \
				-DUSE_VAR_BIT_DEPTH \
				-DUSE_PRED						
					
LOCAL_C_INCLUDES := $(LOCAL_PATH)/ \
					$(LOCAL_PATH)/libavutil \
					$(LOCAL_PATH)/libavcodec

include $(BUILD_STATIC_LIBRARY)