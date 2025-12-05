cmake_minimum_required(VERSION 4.0)
project(mxvsatv-reflex-ultrawide-fix VERSION 1.0 LANGUAGES CXX)
set(PATCHER_VERSION "1.0.0")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DPATCHER_VERSION=\"${PATCHER_VERSION}\")
add_executable(${PROJECT_NAME}
    src/main.cpp
)
set_target_properties(mxvsatv-reflex-ultrawide-fix PROPERTIES
        OUTPUT_NAME "MXvsATV-Reflex-Patcher-v${PATCHER_VERSION}"
)