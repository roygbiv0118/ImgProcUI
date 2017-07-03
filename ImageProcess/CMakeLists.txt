###ImageProcess
add_definitions("-DDLL_EXPORTS")

set(LIBRARY_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/Bin")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

###############################################
set(OpenCV_STATIC OFF)
set(CMAKE_PREFIX_PATH ${OPENCV_DIR})

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
message("<<<OpenCV_FOUND>>>")
include_directories(${OpenCV_INCLUDE_DIRS})
message("<<<OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}>>>")
endif()

include_directories(include)

FILE(GLOB SOURCES "src/*.cpp")
FILE(GLOB HEADERS "include/*.h")
add_library(ImageProcess SHARED ${HEADERS} ${SOURCES})

###############################################
###RELWITHDEBINFO to RELEASE
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
target_link_libraries(ImageProcess ${OpenCV_LIBS})

###############################################
FILE(GLOB OPENCV_RUNTIME ${OPENCV_DIR}/x86/vc12/bin/*.dll)
#INSTALL(FILES ${OPENCV_RUNTIME} DESTINATION .)
FILE(COPY ${OPENCV_RUNTIME} DESTINATION "${PROJECT_SOURCE_DIR}/Bin")
