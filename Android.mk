ifneq ($(HOST_OS),windows)

LOCAL_PATH:=$(call my-dir)

LOCAL_SRC_FILES := \
    ApiGen.cpp \
    EntryPoint.cpp \
    main.cpp \
    strUtils.cpp \
    TypeFactory.cpp \

LOCAL_CXX_STL := libc++_static
LOCAL_MODULE := emugen
LOCAL_HOST_BUILD := true

include $(BUILD_HOST_EXECUTABLE)

endif
