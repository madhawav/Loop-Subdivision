//
// Created by madhawa on 2020-02-01.
//
#include <WingedEdge/Edge.h>
#include <WingedEdge/Vertex.h>
#include <WingedEdge/OBJMesh.h>

#include "OBJViewerCanvas.h"
#include "OBJViewerConstants.h"

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/combobox.h>
#include <nanogui/slider.h>
#include <iostream>
#include <string>
#include <random>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>


#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

using nanogui::Screen;
using nanogui::Window;
using nanogui::GroupLayout;
using nanogui::Button;
using nanogui::CheckBox;
using nanogui::Vector2f;
using nanogui::Vector2i;
using nanogui::MatrixXu;
using nanogui::MatrixXf;
using nanogui::Label;
using nanogui::Arcball;


/**
 * The WingedEdge OBJ Viewer
 */
class OBJViewApplication : public nanogui::Screen {
public:
    /**
     * Initialize Application
     */
    OBJViewApplication() : nanogui::Screen(Eigen::Vector2i(WINDOW_WIDTH, WINDOW_HEIGHT), TXT_APP_TITLE,
                                           false), randomDevice(), randomEngine(randomDevice()) {
        using namespace nanogui;
        createOBJViewerWindow();
        createControlsWindow();
        createSubdivisionWindow();

        // Assemble layout.
        performLayout();
    }

    /**
     * Callback to handle mouse movement events.
     */
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
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

    /**
     * Mousewheel scroll event
     */
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override {
        // Zooming
        mCanvas->setZoom(mCanvas->getZoom() + rel.y() / 10.0f);
        if (mCanvas->getZoom() < 0.1) {
            mCanvas->setZoom(0.1);
        }
        return true;
    }

    /**
     * Callback to update layout.
     */
    virtual void drawContents() override {
        // Update canvas MVP
        mCanvas->updateMVP();
    }

    /**
     * Render callback
     */
    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }

    /**
     * Destructor
     */
    virtual ~OBJViewApplication() {
        // Note: nanogui automatically destroys controls and widgets. We don't have to manually delete them.
    }


private:
    void createControlsWindow() {
        using namespace nanogui;

        auto *controlsWindow = new Window(this, TXT_WINDOW_CONTROLS_TITLE);
        controlsWindow->setPosition(Vector2i(WINDOW_SUBDIV_X, WINDOW_SUBDIV_Y));
        controlsWindow->setLayout(new GroupLayout());

        // Button to load default cube
        auto *btnNewCube = new Button(controlsWindow, TXT_BTN_NEW_CUBE);
        btnNewCube->setCallback([&] {
            OBJMesh objMesh;
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
                    mCanvas->setFlatShading();
                    break;
                case SMOOTH_SHADED_INDEX:
                    mCanvas->setSmoothShading();
                    break;
                case WIREFRAME_SHADED_INDEX:
                    mCanvas->setWireframeMode();
                    break;
                case SHADED_WITH_WIREFRAME_INDEX:
                    mCanvas->setShadedWithMeshEdges();
                    break;
            }
        });
    }

    void createSubdivisionWindow() {
        using namespace nanogui;
        auto *subdivisionWindow = new Window(this, TXT_WINDOW_SUBDIV_TITLE);
        subdivisionWindow->setPosition(Vector2i(SUBDIV_WINDOW_X, SUBDIV_WINDOW_Y));
        subdivisionWindow->setLayout(new GroupLayout());

        auto *btnReset = new Button(subdivisionWindow, TXT_BTN_RESET);
        btnReset->setCallback([&] {
            mCanvas->resetToOriginalMesh();
        });


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

        new Label(subdivisionWindow, TXT_LBL_LOOP_SUBDIV, APP_FONT);
        auto *btnLoopSubdiv = new Button(subdivisionWindow, TXT_BTN_LOOP_SUBDIV);
        btnLoopSubdiv->setCallback([&] {
            mCanvas->resetToOriginalMesh();
            for (int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopSubdivision();
        });


        auto *btnTessellate = new Button(subdivisionWindow, TXT_BTN_LOOP_TESSELLATE);
        btnTessellate->setCallback([&] {
            mCanvas->resetToOriginalMesh();
            for (int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopTessellation();
        });

    }

    void createOBJViewerWindow() {
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

    OBJViewer::OBJViewerCanvas *mCanvas;
    int subdivisionAmount = 1;
    Label *lblSubdivisionCount = nullptr;

    // Random number generator
    std::random_device randomDevice;
    std::default_random_engine randomEngine;
};


int main(int /* argc */, char ** /* argv */) {
    try {

        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<OBJViewApplication> app = new OBJViewApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
        MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
        std::cerr << error_msg << endl;
#endif
        return -1;
    }

    return 0;
}
