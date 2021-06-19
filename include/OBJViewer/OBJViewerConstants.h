//
// Created by madhawa on 2021-06-18.
//

#ifndef OBJ_VIEW_OBJVIEWERCONSTANTS_H
#define OBJ_VIEW_OBJVIEWERCONSTANTS_H

// Look and feel
#define APP_FONT "sans-bold"
#define CANVAS_BACKGROUND_COLOR {100, 100, 100, 255}
#define CANVAS_WIDTH 400
#define CANVAS_HEIGHT 400
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define OBJ_VIEWER_OFFSET_X 15
#define OBJ_VIEWER_OFFSET_Y 15
#define OBJ_VIEWER_WIDGET_SPACING 5
#define OBJ_VIEWER_WIDGET_MARGIN 0
#define SUBDIV_WINDOW_X 500
#define SUBDIV_WINDOW_Y 340
#define SUBDIV_SLIDER_WIDTH 150
#define WINDOW_SUBDIV_X 500
#define WINDOW_SUBDIV_Y 15
#define COMBO_SHADING_WIDTH 200

// Subdivision configuration
#define SUBDIV_DEFAULT_COUNT 1
#define SUBDIV_MIN_COUNT 1
#define SUBDIV_MAX_COUNT 4

// File type
#define FILE_TYPE_EXT "obj"

// Shading configuration
#define SHADING_MODES {TXT_COMBO_FLAT_SHADED, TXT_COMBO_SMOOTH_SHADED, TXT_COMBO_WIREFRAME, TXT_COMBO_SHADED_WITH_WIREFRAME}
#define FLAT_SHADED_INDEX 0
#define SMOOTH_SHADED_INDEX 1
#define WIREFRAME_SHADED_INDEX 2
#define SHADED_WITH_WIREFRAME_INDEX 3
#define DEFAULT_SHADING_INDEX 3

// Viewport Manipulation
#define MOUSE_MOVEMENT_MULTIPLIER 0.01f

// Strings
#define TXT_APP_TITLE "Winged Edge Obj Viewer with Subdivision"
#define TXT_OBJ_VIEWER_TEXT "Left Click: Orbit Camera. Right Click: Pan. Mouse Wheel: Zoom"
#define TXT_WINDOW_SUBDIV_TITLE "Subdivision"
#define TXT_BTN_RESET "Reset"
#define TXT_LBL_SUBDIV_COUNT "Sub-division count: "
#define TXT_LBL_LOOP_SUBDIV "Loop Subdivision"
#define TXT_BTN_LOOP_SUBDIV "Loop"
#define TXT_BTN_LOOP_TESSELLATE "Tessellate"
#define TXT_WINDOW_CONTROLS_TITLE "Controls"
#define TXT_BTN_NEW_CUBE "New Cube"
#define TXT_BTN_EXIT "Exit"
#define TXT_LBL_LOAD_AND_SAVE "Load and save"
#define TXT_BTN_OPEN "Open"
#define TXT_BTN_SAVE "Save"
#define TXT_LBL_SHADING_MODE "Shading Mode"
#define TXT_COMBO_FLAT_SHADED "Flat shaded"
#define TXT_COMBO_SMOOTH_SHADED "Smooth shaded"
#define TXT_COMBO_WIREFRAME "Wireframe"
#define TXT_COMBO_SHADED_WITH_WIREFRAME "Shaded with mesh edges"
#define TXT_WINDOW_OBJ_VIEWER_TITLE "OBJ Viewer"
#define TXT_BTN_RANDOM_COLOR "Random Color"
#define TXT_BTN_RANDOM_ROTATION "Random Rotation"
#define TXT_FILE_TYPE_NAME "OBJ File"
// Error messages
#define TXT_ERR_CANT_OPEN_FILE "Unable to open file"

// 3D Canvas
#define WIREFRAME_OVERLAY_EPS 0.01
#define WIREFRAME_OVERLAY_EPS_WIDTH_FACTOR 0.005f
#define MIN_ZOOM 0.1
#define CAMERA_FAR_CLIP_EXTRA 100

// Shader constants
#define SHADER_NAME "a_smooth_shader"
#define VERTEX_SHADER_FILE_NAME "StandardShading.vertexshader"
#define FRAGMENT_SHADER_FILE_NAME "StandardShading.fragmentshader"
#define SHADER_ATTR_VERTEX_POSITION "vertexPosition_modelspace"
#define SHADER_ATTR_COLOR "color"
#define SHADER_ATTR_VERTEX_NORMAL "vertexNormal_modelspace"
#define SHADER_ATTR_LIGHT_POS "LightPosition_worldspace"
#define SHADER_ATTR_MATRIX_VIEW "V"
#define SHADER_ATTR_MATRIX_MODEL "M"
#define SHADER_ATTR_MATRIX_MVP "MVP"
#define SHADER_ATTR_DIFFUSE_CORRECTION "diffuseCorrection"

#endif //OBJ_VIEW_OBJVIEWERCONSTANTS_H
