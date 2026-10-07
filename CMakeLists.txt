cmake_minimum_required(VERSION 3.26)
project(neuron-viewer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Add DouterGraphics
if(NOT TARGET douter-graphics)
    add_subdirectory(vendor/DG)
endif()

file(GLOB_RECURSE NV_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
add_executable(neuron-viewer ${NV_SOURCES})

if(EMSCRIPTEN)
    set_target_properties(neuron-viewer PROPERTIES SUFFIX ".html")
	target_link_options(neuron-viewer PRIVATE 
		"--preload-file" "${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
		"-sMAX_WEBGL_VERSION=2"
		"--shell-file" "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
	)

	set_property(TARGET neuron-viewer APPEND PROPERTY 
        OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
endif()

target_link_libraries(neuron-viewer PRIVATE douter-graphics)

#Custom Targets
add_custom_target(CopyAssets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:neuron-viewer>/assets"
)
if(NOT EMSCRIPTEN)
add_dependencies(neuron-viewer CopyAssets)
endif()
