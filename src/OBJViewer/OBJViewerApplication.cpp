//
// Created by madhawa on 2021-06-18.
//

#include <nanogui/common.h>
#include <nanogui/screen.h>
#include <nanogui/glutil.h>
#include <nanogui/widget.h>
#include <nanogui/button.h>
#include <nanogui/window.h>
#include <nanogui/combobox.h>
#include <nanogui/label.h>
#include <nanogui/layout.h>
#include <nanogui/slider.h>

#include <OBJViewer/OBJViewerApplication.h>
#include <OBJViewer/OBJViewerCanvas.h>
#include <OBJViewer/OBJViewerConstants.h>
#include <WingedEdge/OBJMesh.h>

using namespace nanogui;
using std::string;

OBJViewer::OBJViewerApplication::OBJViewerApplication() : nanogui::Screen(nanogui::Vector2i(WINDOW_WIDTH, WINDOW_HEIGHT),
                                                                          TXT_APP_TITLE,
                                                                          false),
                                                          randomDevice(), randomEngine(randomDevice()),
                                                          subdivisionAmount(1), lblSubdivisionCount(nullptr) {

    // Initialize UI layout.
    createOBJViewerWindow();
    createControlsWindow();
    createSubdivisionWindow();

    // Assemble layout.
    performLayout();
}

bool OBJViewer::OBJViewerApplication::mouseMotionEvent(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) {
    if (button == GLFW_MOUSE_BUTTON_2) {
        //Left click mouse drag event. Orbit camera.
        mCanvas->setRotation(
                nanogui::Vector3f(mCanvas->getRotation().x() + rel.y() * MOUSE_MOVEMENT_MULTIPLIER,
                                  mCanvas->getRotation().y(),
                                  mCanvas->getRotation().z() - rel.x() * MOUSE_MOVEMENT_MULTIPLIER));
        return true;
    } else if (button == GLFW_MOUSE_BUTTON_3) {
        // Right click mouse drag event.
        // Pan camera relative to current orientation.
        Eigen::Matrix4f camRotation;
        camRotation.setIdentity();
        camRotation.topLeftCorner<3, 3>() = Eigen::Matrix3f(
                Eigen::AngleAxisf(mCanvas->getRotation()[1], Eigen::Vector3f::UnitY()) *
                Eigen::AngleAxisf(mCanvas->getRotation()[2], Eigen::Vector3f::UnitZ()) *
                Eigen::AngleAxisf(mCanvas->getRotation()[0], Eigen::Vector3f::UnitX()));

        Eigen::Vector4f camOffset = camRotation * Eigen::Vector4f(rel.x() * MOUSE_MOVEMENT_MULTIPLIER, 0,
                                                                  rel.y() * MOUSE_MOVEMENT_MULTIPLIER, 1);
        mCanvas->setTarget(nanogui::Vector3f(mCanvas->getTarget().x() + camOffset.x(),
                                             mCanvas->getTarget().y() + camOffset.y(),
                                             mCanvas->getTarget().z() + camOffset.z()));
        return true;
    }
    return false;
}

bool OBJViewer::OBJViewerApplication::scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) {
    // Zooming
    mCanvas->setZoom(mCanvas->getZoom() + rel.y() / 10.0f);
    if (mCanvas->getZoom() < 0.1) {
        mCanvas->setZoom(0.1);
    }
    return true;
}

void OBJViewer::OBJViewerApplication::drawContents() {
    // Update canvas MVP
    mCanvas->updateMVP();
}

void OBJViewer::OBJViewerApplication::draw(NVGcontext *ctx) {
    /* Draw the user interface */
    Screen::draw(ctx);
}

void OBJViewer::OBJViewerApplication::createControlsWindow() {
    using namespace nanogui;

    auto *controlsWindow = new Window(this, TXT_WINDOW_CONTROLS_TITLE);
    controlsWindow->setPosition(Vector2i(WINDOW_SUBDIV_X, WINDOW_SUBDIV_Y));
    controlsWindow->setLayout(new GroupLayout());

    // Button to load default cube
    auto *btnNewCube = new Button(controlsWindow, TXT_BTN_NEW_CUBE);
    btnNewCube->setCallback([&] {
        WingedEdge::OBJMesh objMesh;
        objMesh.setCube();
        mCanvas->loadObjMesh(&objMesh);
    });

    // Exit button
    auto *btnExit = new Button(controlsWindow, TXT_BTN_EXIT);
    btnExit->setCallback([&] {
        exit(0);
    });

    // Saving and loading OBJ meshes
    new Label(controlsWindow, TXT_LBL_LOAD_AND_SAVE, APP_FONT);
    auto *tools = new Widget(controlsWindow);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, OBJ_VIEWER_WIDGET_MARGIN, OBJ_VIEWER_WIDGET_SPACING));

    // Load new mesh
    auto *btnOpen = new Button(tools, TXT_BTN_OPEN);
    btnOpen->setCallback([&] {

        string dialogResult = file_dialog(
                {{FILE_TYPE_EXT, TXT_FILE_TYPE_NAME}}, false);

        if (!dialogResult.empty()) {
            mCanvas->loadObj(dialogResult); // Load the mesh
        }
    });

    // Save mesh
    auto *btnSave = new Button(tools, TXT_BTN_SAVE);
    btnSave->setCallback([&] {
        string dialogResult = file_dialog(
                {{FILE_TYPE_EXT, TXT_FILE_TYPE_NAME}}, true);

        if (!dialogResult.empty()) {
            mCanvas->saveObj(dialogResult); // Save the mesh
        }
    });

    //Select shading mode
    new Label(controlsWindow, TXT_LBL_SHADING_MODE, APP_FONT);
    auto *comboShading = new ComboBox(controlsWindow,
                                      SHADING_MODES);
    comboShading->setSelectedIndex(DEFAULT_SHADING_INDEX);
    comboShading->setWidth(COMBO_SHADING_WIDTH);
    comboShading->setCallback([&](int value) {
        switch (value) {
            case FLAT_SHADED_INDEX:
                mCanvas->switchToFlatShading();
                break;
            case SMOOTH_SHADED_INDEX:
                mCanvas->switchToSmoothShading();
                break;
            case WIREFRAME_SHADED_INDEX:
                mCanvas->switchToWireframeShading();
                break;
            case SHADED_WITH_WIREFRAME_INDEX:
                mCanvas->switchToSmoothWithWireframeShading();
                break;
        }
    });
}

void OBJViewer::OBJViewerApplication::createSubdivisionWindow() {
    using namespace nanogui;
    auto *subdivisionWindow = new Window(this, TXT_WINDOW_SUBDIV_TITLE);
    subdivisionWindow->setPosition(Vector2i(SUBDIV_WINDOW_X, SUBDIV_WINDOW_Y));
    subdivisionWindow->setLayout(new GroupLayout());

    // Reset to original mesh
    auto *btnReset = new Button(subdivisionWindow, TXT_BTN_RESET);
    btnReset->setCallback([&] {
        mCanvas->resetToOriginalMesh();
    });

    // Specify subdivision settings
    lblSubdivisionCount = new Label(subdivisionWindow, TXT_LBL_SUBDIV_COUNT + std::to_string(SUBDIV_DEFAULT_COUNT),
                                    APP_FONT);
    auto *subdivSlider = new Slider(subdivisionWindow);
    subdivSlider->setRange(std::pair<float, float>(SUBDIV_MIN_COUNT, SUBDIV_MAX_COUNT));
    subdivSlider->setFixedWidth(SUBDIV_SLIDER_WIDTH);
    subdivSlider->setValue(SUBDIV_DEFAULT_COUNT);
    subdivSlider->setCallback([&](float value) {
        lblSubdivisionCount->setCaption(TXT_LBL_SUBDIV_COUNT + std::to_string(static_cast<int>(value)));
        subdivisionAmount = static_cast<int>(value);
    });

    // Loop subdivision section
    new Label(subdivisionWindow, TXT_LBL_LOOP_SUBDIV, APP_FONT);
    auto *btnLoopSubdiv = new Button(subdivisionWindow, TXT_BTN_LOOP_SUBDIV);
    btnLoopSubdiv->setCallback([&] {
        mCanvas->resetToOriginalMesh();
        for (int i = 0; i < subdivisionAmount; i++)
            mCanvas->performLoopSubdivision();
    });

    // Tessellation
    auto *btnTessellate = new Button(subdivisionWindow, TXT_BTN_LOOP_TESSELLATE);
    btnTessellate->setCallback([&] {
        mCanvas->resetToOriginalMesh();
        for (int i = 0; i < subdivisionAmount; i++)
            mCanvas->performLoopTessellation();
    });

}

void OBJViewer::OBJViewerApplication::createOBJViewerWindow() {
    using namespace nanogui;
    // Create window
    Window *window = new Window(this, TXT_WINDOW_OBJ_VIEWER_TITLE);
    window->setPosition(Vector2i(OBJ_VIEWER_OFFSET_X, OBJ_VIEWER_OFFSET_Y));
    window->setLayout(new GroupLayout());

    // Create GL Canvas
    mCanvas = new OBJViewer::OBJViewerCanvas(window);
    mCanvas->setBackgroundColor(CANVAS_BACKGROUND_COLOR);
    mCanvas->setSize({CANVAS_WIDTH, CANVAS_HEIGHT});

    // Create UI Elements
    new Label(window, TXT_OBJ_VIEWER_TEXT, APP_FONT);

    // Widget container
    auto *tools = new Widget(window);
    tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                   Alignment::Middle, OBJ_VIEWER_WIDGET_MARGIN, OBJ_VIEWER_WIDGET_SPACING));

    // Random color button
    auto *btnRandomColor = new Button(tools, TXT_BTN_RANDOM_COLOR);
    btnRandomColor->setCallback(
            [this]() {
                std::uniform_int_distribution<> colorDistribution(0, UINT8_MAX);
                mCanvas->setBackgroundColor(
                        Vector4i(colorDistribution(randomEngine), colorDistribution(randomEngine),
                                 colorDistribution(randomEngine), UINT8_MAX));
            });

    // Random rotation button
    Button *btnRandomRotation = new Button(tools, TXT_BTN_RANDOM_ROTATION);
    btnRandomRotation->setCallback([this]() {
        std::uniform_real_distribution<float> rotationDistribution(0.0f, M_PI * 2);
        mCanvas->setRotation(
                nanogui::Vector3f(rotationDistribution(randomEngine),
                                  rotationDistribution(randomEngine),
                                  rotationDistribution(randomEngine)));
    });

}
