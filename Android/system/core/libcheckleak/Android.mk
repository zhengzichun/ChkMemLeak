LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= libcheckleak
LOCAL_SRC_FILES := checkleak.cpp

LOCAL_SHARED_LIBRARIES := libbacktrace

include $(BUILD_SHARED_LIBRARY)
