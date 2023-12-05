LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/$(SDL_PATH)/../include
LOCAL_SRC_FILES := $(wildcard $(LOCAL_PATH)/$(SDL_PATH)/../../../../../src/*.cpp)
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -lOpenSLES -lEGL -llog -landroid
LOCAL_CPP_FEATURES += exceptions rtti

include $(BUILD_SHARED_LIBRARY)
