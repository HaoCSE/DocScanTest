# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_SYSTEM_NAME Android)
#set(OPENCV_VCSVERSION "3.4.0")
#set(CPACK_PACKAGE_VERSION "${OPENCV_VCSVERSION}")


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#target_link_libraries( # Specifies the target library.
 #                       scanner
                       # Links the target library to the log library
                       # included in the NDK.
  #                     ${log-lib} )



include_directories(src/main/jniLibs/include)
include_directories(src/main/cpp/common/3rdparty/jsoncpp/json)
include_directories(src/main/cpp/common/3rdparty/jsoncpp)
include_directories(src/main/cpp/common/3rdparty/ximageproc)
include_directories(src/main/cpp/common/features)
include_directories(src/main/cpp/common/homography)
include_directories(src/main/cpp/common/parser)

link_directories(
    src/main/jniLibs/${ANDROID_ABI}
)

add_library( # Sets the name of the library.
    scanner

    # Sets the library as a shared library.
    SHARED

    src/main/cpp/common/3rdparty/jsoncpp/jsoncpp.cpp
    src/main/cpp/common/3rdparty/ximageproc/niblack_thresholding.cpp
    src/main/cpp/common/3rdparty/ximageproc/structured_edge_detection.cpp
    src/main/cpp/common/features/Contour.cpp
    src/main/cpp/common/features/Element.cpp
    src/main/cpp/common/features/Group.cpp
    src/main/cpp/common/features/Line.cpp
    src/main/cpp/common/features/Page.cpp
    src/main/cpp/common/features/Para.cpp
    src/main/cpp/common/homography/LWHomographyExtractor.cpp
    src/main/cpp/common/parser/Receipt.cpp
)

add_library( # Sets the name of the library.
    DocScanner

    # Sets the library as a shared library.
    SHARED

    # Provides a relative path to your source file(s).
    src/main/cpp/DocScanner.cpp
)

find_library ( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

target_link_libraries (
    scanner
    opencv_java3

    ${log-lib}
)

target_link_libraries (
    DocScanner
    scanner
    opencv_java3

    ${log-lib}
)

#target_link_libraries (
#    DocScanner
#    ${log-lib}
#)
