cmake_minimum_required(VERSION 3.5)
project(obj_view)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

configure_file(glsl/StandardShading.fragmentshader StandardShading.fragmentshader COPYONLY)
configure_file(glsl/StandardShading.vertexshader StandardShading.vertexshader COPYONLY)

include_directories(lib/embed-resource)
add_subdirectory(lib/embed-resource)

embed_resources(MyResources shader/vert.glsl shader/frag.glsl)

add_executable(subd src/obj_view.cpp ${MyResources} src/WingedEdge/Edge.cpp src/WingedEdge/Vertex.cpp src/WingedEdge/Face.cpp src/WingedEdge/WEMesh.cpp src/WingedEdge/OBJMesh.cpp src/Subdivision/loopsubd.cpp src/OBJViewerCanvas.cpp include/OBJViewerCanvas.h include/OBJViewerConstants.h src/OBJViewerApplication.cpp include/OBJViewerApplication.h)
target_link_libraries(subd nanogui ${NANOGUI_EXTRA_LIBS})
