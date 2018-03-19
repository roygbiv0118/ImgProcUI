###IPUI
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/Bin")

include_directories(include)
include_directories(${PROJECT_SOURCE_DIR}/ImageProcess/include)
include_directories(${PROJECT_SOURCE_DIR}/RgbProcess/include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

###############################################
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${OPENCV_DIR})

set(OpenCV_STATIC OFF)
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
message("<<<OpenCV_FOUND>>>")
include_directories(${OpenCV_INCLUDE_DIRS})
endif()

###############################################
set(CMAKE_AUTOMOC ON)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})

find_package(Qt5 COMPONENTS Widgets AxContainer)
if(Qt5Widgets_FOUND)
message("<<<Qt5Widgets_FOUND>>>")
message("<<<QT_DEFINITIONS: ${QT_DEFINITIONS}>>>")
add_definitions(${QT_DEFINITIONS})
endif()


###############################################
FILE(GLOB HEADERS "include/*.h")
FILE(GLOB SOURCES "src/*.cpp")
FILE(GLOB FORMS "form/*.ui")
FILE(GLOB RES "*.rc")

qt5_wrap_cpp(MOC_HEADERS ${HEADERS})
qt5_wrap_ui(FORMS_HEADERS ${FORMS})

###No ${MOC_HEADERS}
add_executable(IPUI WIN32 ${HEADERS} ${SOURCES} ${RES} ${FORMS} ${FORMS_HEADERS})


###############################################
target_link_libraries(IPUI Qt5::Widgets Qt5::AxContainer)

target_link_libraries(IPUI ${PROJECT_SOURCE_DIR}/Bin/ImageProcess.lib ${PROJECT_SOURCE_DIR}/Bin/RgbProcess.lib)

###############################################
FILE(GLOB QT_RUNTIME ${QT_DIR}/bin/*.dll)
#INSTALL(FILES ${QT_RUNTIME} DESTINATION "${PROJECT_SOURCE_DIR}/Bin")
#INSTALL(DIRECTORY ${QT_DIR}/plugins/ DESTINATION .)
FILE(COPY ${QT_RUNTIME} DESTINATION "${PROJECT_SOURCE_DIR}/Bin")
FILE(COPY ${QT_DIR}/plugins/ DESTINATION "${PROJECT_SOURCE_DIR}/Bin")
