project(msg-generator)


include_directories(
    src/msgconv-generator
)

add_library(${PROJECT_NAME} SHARED
    msg-generator.cpp
)
