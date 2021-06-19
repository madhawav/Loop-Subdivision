cmake_minimum_required(VERSION 3.5)
project(obj_view)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
include_directories(include)

# Nano GUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
set(NANOGUI_INSTALL       OFF CACHE BOOL " " FORCE)

# Link nanogui as a static library
set(NANOGUI_BUILD_SHARED OFF CACHE BOOL " " FORCE)

add_subdirectory(lib/nanogui)
include_directories(lib/nanogui/include)
include_directories(${NANOGUI_EXTRA_INCS})
add_definitions(${NANOGUI_EXTRA_DEFS})

# Shading resources
configure_file(glsl/StandardShading.fragmentshader StandardShading.fragmentshader COPYONLY)
configure_file(glsl/StandardShading.vertexshader StandardShading.vertexshader COPYONLY)

# Embed Resource is used to read shading resources
add_subdirectory(lib/embed-resource)
include_directories(lib/embed-resource)

embed_resources(MyResources shader/vert.glsl shader/frag.glsl)

add_executable(subd src/obj_view.cpp ${MyResources} src/WingedEdge/Edge.cpp src/WingedEdge/Vertex.cpp src/WingedEdge/Face.cpp src/WingedEdge/WEMesh.cpp src/WingedEdge/OBJMesh.cpp src/Subdivision/LoopSubdivision.cpp src/OBJViewer/OBJViewerCanvas.cpp include/OBJViewer/OBJViewerCanvas.h include/OBJViewer/OBJViewerConstants.h src/OBJViewer/OBJViewerApplication.cpp include/OBJViewer/OBJViewerApplication.h)

target_link_libraries(subd nanogui ${NANOGUI_EXTRA_LIBS})
