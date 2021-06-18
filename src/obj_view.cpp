//
// Created by madhawa on 2020-02-01.
//
#include <WingedEdge/Edge.h>
#include <WingedEdge/Vertex.h>
#include <WingedEdge/OBJMesh.h>

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
#include <OBJViewerCanvas.h>

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
    OBJViewApplication() : nanogui::Screen(Eigen::Vector2i(900, 600), "Winged Edge Obj Viewer with Subdivision", false) {
        using namespace nanogui;

        //First, we need to create a window context in which we will render both the interface and OpenGL canvas
        Window *window = new Window(this, "OBJ Viewer");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());


        //OpenGL canvas initialization, we can control the background color and also its size
        mCanvas = new OBJViewer::OBJViewerCanvas(window);
        mCanvas->setBackgroundColor({100, 100, 100, 255});
        mCanvas->setSize({400, 400});
        new Label(window, "Right Click: Yaw and Pitch. Left Click: Pan. Mouse Wheel: Zoom", "sans-bold");
        //This is how we add widgets, in this case, they are connected to the same window as the OpenGL canvas
        Widget *tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 5));

        //then we start adding elements one by one as shown below
        Button *b0 = new Button(tools, "Random Color");
        b0->setCallback([this]() { mCanvas->setBackgroundColor(Vector4i(rand() % 256, rand() % 256, rand() % 256, 255)); });

        Button *b1 = new Button(tools, "Random Rotation");
        b1->setCallback([this]() { mCanvas->setRotation(nanogui::Vector3f((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f)); });

        //widgets demonstration
        nanogui::GLShader mShader;

        //Then, we can create another window and insert other widgets into it
        Window *anotherWindow = new Window(this, "Controls");
        anotherWindow->setPosition(Vector2i(500, 15));
        anotherWindow->setLayout(new GroupLayout());

        // Button to load default cube
        Button *button = new Button(anotherWindow, "New Cube");
        button->setCallback([&] {
            OBJMesh objMesh;
            objMesh.setCube();
            mCanvas->loadObjMesh(&objMesh);
        });

        // Exit button
        button = new Button(anotherWindow, "Exit");
        button->setCallback([&] {
            exit(0);
        });

        // Roll slider
        new Label(anotherWindow, "Rotation on roll axis", "sans-bold");
        Widget *panelRoll = new Widget(anotherWindow);
        panelRoll->setLayout(new BoxLayout(Orientation::Horizontal,
                                          Alignment::Middle, 0, 0));

        //Roll the mesh using slider.
        Slider *rollSlider = new Slider(panelRoll);
        rollSlider->setValue(0);
        rollSlider->setFixedWidth(150);
        rollSlider->setCallback([&](float value) {
            // the middle point should be 0 rad
            // then we need to multiply by 2 to make it go from -1. to 1.
            // then we make it go from -2*M_PI to 2*M_PI
            float radians = (value - 0.5f)*2*2*M_PI;
            //then use this to rotate on just one axis
            mCanvas->setRotation(nanogui::Vector3f(mCanvas->getRotation().x(), radians, mCanvas->getRotation().z()));
            //when you implement the other sliders and/or the Arcball, you need to keep track
            //of the other rotations used for the second and third axis... It will not stay as 0.0f
        });


        // Saving and loading obj meshes
        new Label(anotherWindow, "Load and save", "sans-bold");
        tools = new Widget(anotherWindow);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        Button *b = new Button(tools, "Open");
        b->setCallback([&] {

            string dialogResult = file_dialog(
                    { {"obj", "OBJ File"}}, false);

            if(!dialogResult.empty()){
                mCanvas->loadObj(dialogResult); // Load the mesh
            }
        });
        b = new Button(tools, "Save");
        b->setCallback([&] {
            string dialogResult = file_dialog(
                    { {"obj", "OBJ File"}}, true);

            if(!dialogResult.empty()){
                mCanvas->saveObj(dialogResult); // Save the mesh
            }
        });

        //Select shading mode
        new Label(anotherWindow, "Shading Mode", "sans-bold");
        ComboBox *combo = new ComboBox(anotherWindow, { "Flat shaded", "Smooth shaded", "Wireframe", "Shaded with mesh edges"} );
        combo->setSelectedIndex(3);
        combo->setWidth(200);
        combo->setCallback([&](int value) {
            if(value == 0){
                mCanvas->setFlatShading();
            }
            else if(value == 1){
                mCanvas->setSmoothShading();
            }
            else if (value==2){
                mCanvas->setWireframeMode();
            }
            else if (value==3){
                mCanvas->setShadedWithMeshEdges();
            }
//            cout << "Combo box selected: " << value << endl;
        });

        //Then, we can create another window and insert other widgets into it
        Window *subdivisionWindow = new Window(this, "Subdivision");
        subdivisionWindow->setPosition(Vector2i(500, 340));
        subdivisionWindow->setLayout(new GroupLayout());

        button = new Button(subdivisionWindow, "Reset");
        button->setCallback([&] {
            mCanvas->resetToOriginalMesh();
        });


        subdLabel = new Label(subdivisionWindow, "Sub-division count: 1", "sans-bold");
        Slider *subdivSlider = new Slider(subdivisionWindow);
        subdivSlider->setValue(1);
        subdivSlider->setRange(std::pair<float ,float>(1,4));
        subdivSlider->setFixedWidth(150);
        subdivSlider->setValue(1);
        subdivSlider->setCallback([&](float value) {
            subdLabel->setCaption("Sub-division count: " + std::to_string((int)value));
            subdivisionAmount = value;
        });

        new Label(subdivisionWindow, "Loop Subdivision", "sans-bold");
        button = new Button(subdivisionWindow, "Loop");
        button->setCallback([&] {
            mCanvas->resetToOriginalMesh();
            for(int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopSubdivision();
        });


        button = new Button(subdivisionWindow, "Tessellate");
        button->setCallback([&] {
            mCanvas->resetToOriginalMesh();
            for(int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopTessellation();
        });



        //Method to assemble the interface defined before it is called
        performLayout();
    }


    // Handles camera movements using mouse
    virtual bool mouseMotionEvent(const Eigen::Vector2i &p, const Vector2i &rel, int button, int modifiers) override {
        if (button == GLFW_MOUSE_BUTTON_3 ) {
            //Get right click drag mouse event. Rotate about x and z axis.
            mCanvas->setRotation(nanogui::Vector3f(mCanvas->getRotation().x()+rel.y()/100.0f, mCanvas->getRotation().y(), mCanvas->getRotation().z()-rel.x()/100.0f));
            return true;
        }
        else if (button == GLFW_MOUSE_BUTTON_2 ){
            // Do movements relative to camera rotation
            Eigen::Matrix4f camRotation;
            camRotation.setIdentity();
            camRotation.topLeftCorner<3,3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mCanvas->getRotation()[1], Eigen::Vector3f::UnitY()) *
                                                               Eigen::AngleAxisf(mCanvas->getRotation()[2],  Eigen::Vector3f::UnitZ()) *
                                                               Eigen::AngleAxisf(mCanvas->getRotation()[0], Eigen::Vector3f::UnitX()));


            Eigen::Vector4f camOffset = camRotation * Eigen::Vector4f(rel.x()/100.0f,0,rel.y()/100.0f, 1) ;
            mCanvas->setTarget(nanogui::Vector3f(mCanvas->getTarget().x()+camOffset.x(), mCanvas->getTarget().y()+camOffset.y(), mCanvas->getTarget().z() + camOffset.z()));
            return true;
        }
        return false;
    }

    virtual bool scrollEvent(const Vector2i &p, const Vector2f &rel) override {
        // Zooming
        mCanvas->setZoom( mCanvas->getZoom() + rel.y()/10.0f);
        if (mCanvas->getZoom() < 0.1){
            mCanvas->setZoom(0.1);
        }
        return true ;
    }

    virtual void drawContents() override {
        // Update canvas MVP
        mCanvas->updateMVP();
    }

    virtual void draw(NVGcontext *ctx) {
        /* Draw the user interface */
        Screen::draw(ctx);
    }


private:
    OBJViewer::OBJViewerCanvas *mCanvas;
    float subdivisionAmount = 1;
    Label* subdLabel = nullptr;
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
