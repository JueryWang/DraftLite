cmake_minimum_required(VERSION 3.10)

project(DraftLite)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:NOREF")

set(Qt6_ROOT "C:/Users/Admin/Documents/lib/Qt/6.7.3/msvc2022_64")

set(QML_IMPORT_PATH "${Qt6_ROOT}/plugins/qml" CACHE STRING "QML module path")
# 传递给目标程序

set(CMAKE_PREFIX_PATH ${Qt6_ROOT} ${CMAKE_PREFIX_PATH})
list(APPEND CMAKE_PREFIX_PATH  "C:/Users/Admin/Documents/lib/open62541/lib/cmake/open62541")
list(APPEND CMAKE_PREFIX_PATH "C:/Users/Admin/Documents/lib/QXslx/Debug/lib/cmake/QXlsxQt6")
list(APPEND CMAKE_PREFIX_PATH "C:/Users/Admin/Documents/lib/libcurl/lib/cmake/CURL")
list(APPEND CMAKE_PREFIX_PATH "C:/Users/Admin/Documents/lib/Eigen3/cmake")
list(APPEND CMAKE_PREFIX_PATH "C:/Users/Admin/Documents/lib/hiredis/share/hiredis")

set(BOOST_ROOT "C:/Users/Admin/Documents/lib/boost_1_82_0")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib64-msvc-14.3")
#set(KDL_ROOT "C:/Users/Admin/Documents/lib/orocos-kdl/share/orocos_kdl/cmake")
#set(KDL_LIBRARYDIR "C:/Users/Admin/Documents/lib/orocos-kdl/lib")
#set(EIGEN3_ROOT "C:/Users/Admin/Documents/lib/Eigen3/cmake")
set(QT_QPA_PLATFORM_PLUGIN_PATH "C:/Users/Admin/Documents/lib/Qt/6.7.3/msvc2022_64/platforms")

find_package(open62541 REQUIRED)
#find_package(orocos_kdl REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(CURL REQUIRED)
find_package(QT NAMES Qt5 Qt6 COMPONENTS OpenGL Core Gui Widgets QuickWidgets OpenGL Network Qml Quick WebEngineQuick QuickControls2 QuickLayouts Xml Sql REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets QuickWidgets OpenGL Network Qml Quick WebEngineQuick QuickControls2 QuickLayouts Xml Sql)
find_package(OpenMP REQUIRED)
find_package(Hiredis CONFIG REQUIRED)

include_directories(include)
include_directories(${Qt${QT_VERSION_MAJOR}_INCLUDE_DIRS})
include_directories(C:/Users/Admin/Documents/lib/dxflib)
include_directories(C:/Users/Admin/Documents/lib/Clipper2/include)
include_directories(C:/Users/Admin/Documents/lib/QXslx/include)
include_directories(C:/Users/Admin/Documents/lib/OGL)
include_directories(C:/Users/Admin/Documents/lib/freetype)
include_directories(C:/Users/Admin/Documents/lib/QSci/include)
include_directories(C:/Users/Admin/Documents/lib/spdlog/include)
include_directories(${Boost_INCLUDE_DIRS})

add_compile_options(${Qt${QT_VERSION_MAJOR}_EXECUTABLE_COMPILE_FLAGS})
set(AUTH_LIB
      ${CMAKE_CURRENT_SOURCE_DIR}/bin/WgAuthorization.lib  
)
set(OPENGL_LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/bin/glfw3.lib
    ${CMAKE_CURRENT_SOURCE_DIR}/bin/glad.lib
)
set(DXF_LIB
     $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/bin/dxflibd.lib>
     $<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/bin/dxflib.lib>
)
set(Clipper_LIB
     $<$<CONFIG:Debug>:C:/Users/Admin/Documents/lib/Clipper2/Debug/lib/Clipper2.lib>
     $<$<CONFIG:Release>:C:/Users/Admin/Documents/lib/Clipper2/Release/lib/Clipper2.lib>
)
set (QXslx_LIB
    $<$<CONFIG:Debug>:C:/Users/Admin/Documents/lib/QXslx/Debug/lib/QXlsxQt6.lib>
    $<$<CONFIG:Release>:C:/Users/Admin/Documents/lib/QXslx/Release/lib/QXlsxQt6.lib>
)
set(FREETYPE_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/bin/freetype.lib
)
set(Qsci_LIB
    $<$<CONFIG:Debug>:C:/Users/Admin/Documents/lib/QSci/Debug/qscintilla2_qt6d.lib>
    $<$<CONFIG:Release>:C:/Users/Admin/Documents/lib/QSci/Release/qscintilla2_qt6.lib>
)

set(SpdLog_LIB
    $<$<CONFIG:Debug>:C:/Users/Admin/Documents/lib/spdlog/lib/Debug/spdlogd.lib>
    $<$<CONFIG:Release>:C:/Users/Admin/Documents/lib/spdlog/lib/Release/spdlog.lib>
)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)
file(GLOB SHADERS Shader/*)
file(GLOB ModalHeaders include/ModalEvent/*.h)
file(GLOB ModalSources src/ModalEvent/*.cpp)
file(GLOB CommonHeaders include/Common/*.h)
file(GLOB CommonSources src/Common/*.cpp)
file(GLOB IOHeaders include/IO/*.h)
file(GLOB IOSources src/IO/*.cpp)
file(GLOB GraphicsHeaders include/Graphics/*.h)
file(GLOB GraphicsSources src/Graphics/*.cpp)
file(GLOB ControlsHeaders include/Controls/*.h)
file(GLOB ControlsSources src/Controls/*.cpp)
file(GLOB UIHeaders include/UI/*.h)
file(GLOB UISources src/UI/*.cpp)
file(GLOB UIComponentHeaders include/UI/Components/*.h)
file(GLOB UIComponentSources src/UI/Components/*.cpp)
file(GLOB UIConfigerHeaders include/UI/Configer/*.h)
file(GLOB UIConfigerSources src/UI/Configer/*.cpp)
file(GLOB NetHeaders include/NetWork/*.h)
file(GLOB NetSources src/NetWork/*.cpp)
file(GLOB PathHeaders include/Path/*.h)
file(GLOB PathSources src/Path/*.cpp)
file(GLOB AlgoHeaders include/Algorithm/*.h)
file(GLOB AlgoSources src/Algorithm/*.cpp)
file(GLOB QmlExternal qml/*.qml)
file(GLOB_RECURSE AuthHeaders include/Auth/*.h include/Auth/*.cpp)
file(GLOB AuthSources src/Auth/*.cpp)

qt6_add_resources(RCC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")

qt_add_executable(${PROJECT_NAME}
        WIN32
        ${HEADERS}
        ${SOURCES}
        ${SHADERS}
        ${IOHeaders}
        ${CommonHeaders}
        ${CommonSources}
        ${IOHeaders}
        ${IOSources}
        ${GraphicsHeaders}
        ${GraphicsSources}
        ${ControlsHeaders}
        ${ControlsSources}
        ${ModalHeaders}
        ${ModalSources}
        ${UIHeaders}
        ${UISources}
        ${NetHeaders}
        ${NetSources}
        ${UIComponentHeaders}
        ${UIComponentSources}
        ${UIConfigerHeaders}
        ${UIConfigerSources}
        ${PathHeaders}
        ${PathSources}
        ${AlgoHeaders}
        ${AlgoSources}
        ${AuthHeaders}
        ${AuthSources}
        ${RCC_SOURCES}
        app.rc
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "Source Files" FILES ${SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Shader" PREFIX "Shader Files" FILES ${SHADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/qml" PREFIX "QML Files" FILES ${QmlExternal})

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/ModalEvent" 
             PREFIX "Header Files/ModalEvent" 
             FILES ${ModalHeaders})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/ModalEvent" 
             PREFIX "Source Files/ModalEvent" 
             FILES ${ModalSources})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Common"
             PREFIX "Header Files/Common"
             FILES ${CommonHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Common"
             PREFIX "Source Files/Common"
             FILES ${CommonSources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/IO"
             PREFIX "Header Files/IO"
             FILES ${IOHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/IO"
             PREFIX "Source Files/IO"
             FILES ${IOSources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Controls"
             PREFIX "Header Files/Controls"
             FILES ${ControlsHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Controls"
             PREFIX "Source Files/Controls"
             FILES ${ControlsSources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Graphics"
             PREFIX "Header Files/Graphics"
             FILES ${GraphicsHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics"
             PREFIX "Source Files/Graphics"
             FILES ${GraphicsSources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/UI"
             PREFIX "Header Files/UI"
             FILES ${UIHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/UI"
             PREFIX "Source Files/UI"
             FILES ${UISources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/UI/Components"
            PREFIX "Header Files/UI/Components"
            FILES ${UIComponentHeaders}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Components"
            PREFIX "Source Files/UI/Components"
            FILES ${UIComponentSources}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/UI/Configer"
            PREFIX "Header Files/UI/Configer"
            FILES ${UIConfigerHeaders}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/UI/Configer"
            PREFIX "Source Files/UI/Configer"
            FILES ${UIConfigerSources}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/NetWork"
             PREFIX "Header Files/NetWork"
            FILES ${NetHeaders})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/NetWork"
             PREFIX "Source Files/NetWork"
             FILES ${NetSources}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Path"
             PREFIX "Header Files/Path"
            FILES ${PathHeaders}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Path"
            PREFIX "Source Files/Path"
            FILES ${PathSources}
            )
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Algorithm"
            PREFIX "Header Files/Algorithm"
            FILES ${AlgoHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Algorithm"
            PREFIX "Source Files/Algorithm"
            FILES ${AlgoSources}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/qml"
            PREFIX "Source Files/Qml"
            FILES ${QmlExternal}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/Auth"
            PREFIX "Header Files/Auth"
            FILES ${AuthHeaders}
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src/Auth"
            PREFIX "Source Files/Auth"
            FILES ${AuthSources}
)


file(GLOB DLL_DEPENDS "${CMAKE_SOURCE_DIR}/bin/*.dll")
file(GLOB EXE_DEPENDS "${CMAKE_SOURCE_DIR}/bin/*.exe")

set(Copy_Externals
    "${CMAKE_SOURCE_DIR}/bin/dwg2dxf.exe"
    "${CMAKE_SOURCE_DIR}/bin/libredwg-0.dll"
)

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>
)

set(RESOURCE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Shader
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources
    ${DLL_DEPENDS}
    ${EXE_DEPENDS}
)

add_custom_command(TARGET ${PROJECT_NAME}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Shader $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>/Shader
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/Resources  $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>/Resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/plugins  $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>/plugins
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/qml  $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>/qml
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${Copy_Externals}  $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/config.ini ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/config.ini
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_DEPENDS} $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${EXE_DEPENDS} $<1:${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    QML_IMPORT_PATH="${QML_IMPORT_PATH}"
    QT_QPA_PLATFORM_PLUGIN_PATH="${QT_QPA_PLATFORM_PLUGIN_PATH}"
)

if(WIN32)
  #多线程编译
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  set(QT_PLUGIN_PATH "${Qt6_ROOT}/plugins")
  add_definitions(-DQT_QPA_PLATFORM_PLUGIN_PATH="${QT_PLUGIN_PATH}")

  # 复制平台插件到输出目录（可选，用于部署）
  file(COPY "${QT_PLUGIN_PATH}/platforms/" DESTINATION "${CMAKE_BINARY_DIR}/platforms/")
  if(MSVC)
      if(MSVC_VERSION GREATER_EQUAL 1920)
    add_compile_options("/utf-8")
    else()
    add_compile_options("/source-charset:utf-8" "/execution-charset:utf-8")
    endif()
    target_link_options(${PROJECT_NAME} PRIVATE "/ignore:4099")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
   endif()
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::WebEngineQuick
    Qt${QT_VERSION_MAJOR}::QuickControls2 
    Qt${QT_VERSION_MAJOR}::QuickLayouts
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Sql
    ${AUTH_LIB}
    ${OPENGL_LIBS}
    ${DXF_LIB}
    ${FREETYPE_LIB}
    ${Clipper_LIB}
    ${QXslx_LIB}
    ${Qsci_LIB}
    ${SpdLog_LIB}
    $<$<CONFIG:Debug>:${BOOST_LIBRARYDIR}/libboost_serialization-vc143-mt-gd-x64-1_82.lib>
    $<$<CONFIG:Release>:${BOOST_LIBRARYDIR}/libboost_serialization-vc143-mt-x64-1_82.lib>
    open62541::open62541
    hiredis::hiredis
    CURL::libcurl
    Eigen3::Eigen
    OpenMP::OpenMP_CXX
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/
    DESTINATION Application
    FILES_MATCHING
    PATTERN "*"
)