//
// Created by madhawa on 2021-06-18.
//
#include <OBJViewer/OBJViewerCanvas.h>
#include <nanogui/common.h>
#include <random>
#include <nanogui/screen.h>

#ifndef OBJ_VIEW_OBJVIEWERAPPLICATION_H
namespace OBJViewer {
#define OBJ_VIEW_OBJVIEWERAPPLICATION_H

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
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const nanogui::Vector2i &rel, int button, int modifiers) override;

    /**
     * Mousewheel scroll event
     */
    virtual bool scrollEvent(const nanogui::Vector2i &p, const nanogui::Vector2f &rel) override;

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

    OBJViewerCanvas *mCanvas; // Canvas used for rendering
    int subdivisionAmount; // Selected subdivision amount
    nanogui::Label *lblSubdivisionCount; // Label displaying the selected subdivision setting

    // Random number generator
    std::random_device randomDevice;
    std::default_random_engine randomEngine;
};
}

#endif //OBJ_VIEW_OBJVIEWERAPPLICATION_H
