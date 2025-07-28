cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

#if debug - turn this off
#if release - turn this on
set(PRODUCTION_BUILD OFF CACHE BOOL "MAKE THIS A PRODUCTION BUILD" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SDL_SHARED OFF CACHE BOOL "Static SDL3" FORCE)
set(SDL_STATIC ON CACHE BOOL "Static SDL3" FORCE)

set(IMGUI_INCLUDE_DIR "./thirdparty/imgui")
set(IMGUI_BACKENDS_INCLUDE_DIR "./thirdparty/imgui/backends")
set(SDL3_INCLUDE_DIR "./thirdparty/SDL3/include")
set(SDL3_GFX_INCLUDE_DIR "./thirdparty/SDL3_gfx")
set(SDL3_IMAGE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/SDL3_image")
set(SDL3_IMAGE_INCLUDE_DIR "${SDL3_IMAGE_DIR}/include")

set(SDL3_LIB SDL3::SDL3)  

add_subdirectory("thirdparty/SDL3" EXCLUDE_FROM_ALL)
add_subdirectory("thirdparty/SDL3_gfx")

# might give you problems on Linux
if(PRODUCTION_BUILD)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

project(2DShadow)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
if(BUILD_SHARED_LIBS)
	message(STATUS "${PROJECT_NAME}: THE PROJECT IN SHARED MODE!")
	set(BUILD_LIB_FLAG SHARED)
else()
	message(STATUS "${PROJECT_NAME}: THE PROJECT IN STATIC MODE!")
	set(BUILD_LIB_FLAG STATIC)
endif()

file(GLOB_RECURSE MY_SOURCES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE IMGUI_SOURCE_FILES 
     imgui_impl_opengl3.cpp imgui_impl_SDL3.cpp imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp)
file(GLOB_RECURSE IMGUI_HEADER_FILES
     imgui_impl_opengl3.h imgui_impl_SDL3.h imgui.h imconfig.h imgui_internal.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h)
		
add_executable("${PROJECT_NAME}")
		
set_property(TARGET "${PROJECT_NAME}" PROPERTY CMAKE_CXX_STANDARD 20)

if(PRODUCTION_BUILD)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCE_PATH="./resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=0)
else()
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=1)
endif()

target_sources("${PROJECT_NAME}" PRIVATE "${MY_SOURCES}")

#when a file doesnt have cmakeLists.txt, but does have cmake folder
message(STATUS "${PROJECT_NAME}: creating ImGui")
add_library(imgui "${BUILD_LIB_FLAG}" "${IMGUI_SOURCE_FILES}" "${IMGUI_HEADER_FILES}")

message(STATUS "${PROJECT_NAME}: creating SDL3_image")
add_library(SDL3_image SHARED IMPORTED)
set_target_properties(SDL3_image PROPERTIES 
	IMPORTED_LOCATION "${SDL3_IMAGE_DIR}/lib/SDL3_image.lib"
	IMPORTED_IMPLIB "${SDL3_IMAGE_DIR}/lib/SDL3_image.lib"
	IMPORTED_LOCATION_DEBUG "${SDL3_IMAGE_DIR}/lib/SDL3_image.dll"
	INTERFACE_INCLUDE_DIRECTORIES "${SDL3_IMAGE_INCLUDE_DIR}")

target_include_directories("${PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/"
													"${IMGUI_INCLUDE_DIR}" 
													"${IMGUI_BACKENDS_INCLUDE_DIR}" 
													"${SDL3_GFX_INCLUDE_DIR}" 
													"${SDL3_IMAGE_INCLUDE_DIR}")
target_include_directories(imgui PUBLIC "${IMGUI_INCLUDE_DIR}" 
										"${IMGUI_BACKENDS_INCLUDE_DIR}"
										"${SDL3_INCLUDE_DIR}")
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DLL/SDL3_image.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/DLL/SDL3.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMENT "Copying DLLs to executable directory"
    )
endif()
if(TARGET SDL3::SDL3main)
	target_link_libraries("${PROJECT_NAME}" PUBLIC SDL3::SDL3main)
endif()
target_link_libraries("${PROJECT_NAME}" PUBLIC 
    SDL3::SDL3 
    SDL3_image 
    "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL3_gfx/build/SDL3_gfx_Static/Release/SDL3_gfx_static.lib" 
    imgui)