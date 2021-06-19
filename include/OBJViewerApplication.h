//
// Created by madhawa on 2021-06-18.
//

#ifndef OBJ_VIEW_OBJVIEWERAPPLICATION_H
#define OBJ_VIEW_OBJVIEWERAPPLICATION_H

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
class OBJViewerApplication : public nanogui::Screen {
public:
    /**
     * Initialize Application
     */
    OBJViewerApplication();

    /**
     * Callback to handle mouse movement events.
     */
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override;

    /**
     * Mousewheel scroll event
     */
    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override;

    /**
     * Callback to update layout.
     */
    virtual void drawContents() override;

    /**
     * Render callback
     */
    virtual void draw(NVGcontext *ctx) override;

    /**
     * Destructor
     */
    virtual ~OBJViewerApplication() {
        // Note: nanogui automatically destroys controls and widgets. We don't have to manually delete them.
    }


private:
    /**
     * Create the widget window that has basic controls
     */
    void createControlsWindow();

    /**
     * Create the widget window that has subdivision features.
     */
    void createSubdivisionWindow();

    /**
     * Create widget window that host the OBJ viewer
     */
    void createOBJViewerWindow();

    OBJViewer::OBJViewerCanvas *mCanvas; // Canvas used for rendering
    int subdivisionAmount; // Selected subdivision amount
    Label *lblSubdivisionCount; // Label displaying the selected subdivision setting

    // Random number generator
    std::random_device randomDevice;
    std::default_random_engine randomEngine;
};


#endif //OBJ_VIEW_OBJVIEWERAPPLICATION_H
