cmake_minimum_required(VERSION 3.10)

project(ds-test-app)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

find_package(CUDA REQUIRED)
find_package(yaml-cpp REQUIRED)


include_directories(
    include
    src
    ${GST_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}

    /opt/nvidia/deepstream/deepstream/sources/includes/
    src/ae/include
)

link_directories(/opt/nvidia/deepstream/deepstream/lib)

add_executable(${PROJECT_NAME}
    src/main.cpp

    src/engine/source/SourceBinFactory.cpp
    src/engine/infer/InferBinFactory.cpp
    src/engine/pipeline/PiplineFactory.cpp
    src/engine/sink/SinkBinFactory.cpp
    src/engine/broker/BrokerBinFactory.cpp
    src/engine/render/RenderBinFactory.cpp
    src/engine/RegisteredFactories.cpp
    
    src/common/yaml-tools.cpp
    src/common/tools.cpp
    src/common/string-tools.cpp
    src/common/gst-tools.cpp

    src/msgconv-generator/msg-generator.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    nvinfer 
    nvinfer_plugin
    ${CUDA_LIBRARIES}
    ${CUDA_cublas_LIBRARY}
    yaml-cpp

    gstreamer-1.0 
    gobject-2.0 
    glib-2.0

    nvdsgst_meta
    nvds_meta

    nvbufsurftransform
    nvdsgst_helper
    nvbufsurface
    nvds_batch_jpegenc
    nvds_yml_parser

    # nvdsgst_helper
    ${JSONCPP_LIBRARIES}
)

add_subdirectory(src/msgconv-generator)

# add_executable(${PROJECT_NAME}-tests
#     src/engine/SourceBinFactory.cpp
#     src/engine/InferBinFactory.cpp
#     src/engine/PiplineFactory.cpp
#     src/engine/SinkBinFactory.cpp
#     src/engine/BrokerBinFactory.cpp
#     src/engine/RenderBinFactory.cpp

#     src/tests/string-tools-test.cpp
#     src/tests/SourceBinFactoryTests.cpp
#     src/tests/InferBinFactoryTests.cpp
#     src/tests/SinkBinFactoryTests.cpp
#     src/tests/ContextFactoryTests.cpp

#     src/common/yaml-tools.cpp
#     src/common/tools.cpp
#     src/common/string-tools.cpp
# )

# target_link_libraries(${PROJECT_NAME}-tests PRIVATE 
#     nvinfer 
#     nvinfer_plugin
#     ${CUDA_LIBRARIES}
#     ${CUDA_cublas_LIBRARY}
#     yaml-cpp

#     gstreamer-1.0 
#     gobject-2.0 
#     glib-2.0

#     nvdsgst_meta
#     ${JSONCPP_LIBRARIES}

#     gtest
#     gtest_main
# )

# enable_testing()
# add_test(NAME ${PROJECT_NAME}-tests COMMAND ${PROJECT_NAME}-tests)