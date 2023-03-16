cmake_minimum_required (VERSION 3.8)

string(TIMESTAMP TODAY_YEAR "%Y")
string(TIMESTAMP TODAY_MONTH "%m")
string(TIMESTAMP TODAY_DAY "%d")

set(MY_PRODUCT_NUMBER 1)
set(MY_PRODUCT_VERSION "${TODAY_YEAR}")
set(MY_BUILD_NUMBER "${TODAY_MONTH}")
set(MY_SUB_NUMBER "${TODAY_DAY}")

set(CMAKE_CXX_STANDARD 17)

PROJECT(Abrechnungsprogramm)

#target_compile_features(Abrechnungsprogramm PRIVATE cxx_std_17)

#QT
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR})
#find_package(OpenGL)
find_package(Qt6Core REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6OpenGL REQUIRED)
find_package(Qt6Xml REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6PrintSupport REQUIRED)

set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

add_executable(Abrechnungsprogramm ${SOURCES})

target_include_directories(Abrechnungsprogramm PRIVATE ${FORMS_DIR})
target_include_directories(Abrechnungsprogramm PRIVATE ${INCLUDE_DIR})
target_include_directories(Abrechnungsprogramm PRIVATE ${SOURCE_DIR})

target_link_libraries(Abrechnungsprogramm Qt6::OpenGL)
target_link_libraries(Abrechnungsprogramm Qt6::Xml)
target_link_libraries(Abrechnungsprogramm Qt6::Widgets)
target_link_libraries(Abrechnungsprogramm Qt6::PrintSupport)
target_link_libraries(Abrechnungsprogramm ${QT_LIBRARIES} ${OPENGL_LIBRARIES})



set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Abrechnungsprogramm)