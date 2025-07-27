#https://github.com/meemknight/cmakeSetup
#Version 1.1.0

cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)
#delete the out folder after changing if visual studio doesn recognize the change!

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(IMGUI_INCLUDE_DIR "./thirdparty/imgui")
set(IMGUI_BACKENDS_INCLUDE_DIR "./thirdparty/imgui/backends")
set(SDL3_INCLUDE_DIR "./thirdparty/SDL/include")
set(GLAD_INCLUDE_DIR "./thirdparty/glad/include")
set(GLM_INCLUDE_DIR "./thirdparty/glm/glm")

#may give problems on linux
if(PRODUCTION_BUILD)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
else()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

if(MSVC) 
add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

project(mygame)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
 
if(BUILD_SHARED_LIBS)
    message(STATUS "${PROJECT_NAME}: Build engine shared library")
    set(BUILD_LIB_FLAG "SHARED")
else()
    message(STATUS "${PROJECT_NAME}: Build engine static library")
    set(BUILD_LIB_FLAG "STATIC")
endif()
add_subdirectory(thirdparty/glad)				
add_subdirectory(thirdparty/SDL EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/glm EXCLUDE_FROM_ALL)

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCE_FILES imgui_impl_opengl3.cpp imgui_impl_SDL3.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp)
file(GLOB_RECURSE IMGUI_HEADER_FILES imgui_impl_opengl3.h imgui_impl_SDL3.h imgui.h imconfig.h imgui_internal.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h)

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 20)

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1) 

if(PRODUCTION_BUILD)
	# setup the ASSETS_PATH macro to be in the root folder of your exe
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/") 

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=0) 

else()
	# This is useful to get an ASSETS_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0) 
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPLEMT_BUILD=1) 

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	#set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
	
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

endif()

#force remove unicode
if (WIN32)
	target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE /UUNICODE /U_UNICODE)
endif()

message(STATUS "${PROJECT_NAME}: Creating imgui lib")
add_library(imgui ${BUILD_LIB_FLAG} ${IMGUI_SOURCE_FILES} ${IMGUI_HEADER_FILES})

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
target_include_directories(imgui PUBLIC ${IMGUI_BACKENDS_INCLUDE_DIR})
target_include_directories(imgui PUBLIC ${SDL3_INCLUDE_DIR})
target_include_directories(imgui PUBLIC ${GLAD_INCLUDE_DIR})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ${IMGUI_INCLUDE_DIR})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ${IMGUI_BACKENDS_INCLUDE_DIR})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ${GLAD_INCLUDE_DIR})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC ${GLM_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

#ADD enet version! also don't forget to uncomment enet from above
#target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw 
#	glad stb_image stb_truetype gl2d raudio imgui safeSave profilerLib enet glui)


if(TARGET SDL3::SDL3main)
    target_link_libraries(mygame PUBLIC SDL3::SDL3main)
endif()
target_link_libraries(mygame PUBLIC SDL3::SDL3 glad imgui glm::glm)
