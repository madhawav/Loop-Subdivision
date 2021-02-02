//
// Created by madhawa on 2020-02-01.
//
#include <WingedEdge/Edge.h>
#include <WingedEdge/Vertex.h>
#include <WingedEdge/Face.h>
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/OBJMesh.h>
#include <Subdivision/subd.h>

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/glcanvas.h>
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


class MyGLCanvas : public nanogui::GLCanvas {
public:
    MyGLCanvas(Widget *parent) : nanogui::GLCanvas(parent) {
        using namespace nanogui;
        drawWireframe = true;
        drawSolid = true;
        flatShading = false;
        isSubdivided = false;
        M.setIdentity();
        V.setIdentity();
        P.setIdentity();
        MVP.setIdentity();

        // I have slightly modified the StandardShading.fragmentshader file to make wireframe colour different from mesh colour.
        mShader.initFromFiles("a_smooth_shader", "StandardShading.vertexshader", "StandardShading.fragmentshader");

        // Setup a cube
        OBJMesh objMesh;
        objMesh.setCube();
        loadObjMesh(&objMesh);

    }

    bool saveObj(string path){
        OBJMesh objMesh;
        weMesh.fillOBJMesh(&objMesh);
        std::ofstream outFile;
        outFile.open(path);
        if (!outFile) {
            cout << "Unable to open file";
            return false;
        }
        return objMesh.saveFile(&outFile);
    }

    /**
     * Loads an OBJ file from disk. Converts it to a WingedEdgeMesh and setup shading.
     * @param path Path to obj file
     * @return
     */
    bool loadObj(string path){
        OBJMesh objMesh;
        std::ifstream inFile;
        inFile.open(path);
        if (!inFile) {
            cout << "Unable to open file";
            return false;
        }

        objMesh.parseFile(&inFile);
        inFile.close();

        return loadObjMesh(&objMesh);
    }

    /**
     * Perform loop subdivision once
     */
    void performLoopSubdivision(){
        if(!isSubdivided){
            weMesh.fillOBJMesh(&originalMesh);
        }
        OBJMesh subResult;
        loopSubd(&subResult, &weMesh, true, true);
        loadObjMesh(&subResult);
        isSubdivided = true;
    }

    /**
     * Perform only tesselation used by loop subdivision
     */
    void performLoopTesselate(){
        if(!isSubdivided){
            weMesh.fillOBJMesh(&originalMesh);
        }
        OBJMesh subResult;
        loopSubd(&subResult, &weMesh, false, false);
        loadObjMesh(&subResult);
        isSubdivided = true;
    }

    /**
     * Re-load unsubdivided mesh
     */
    void resetOriginalMesh(){
        if(!isSubdivided)
            return;
        loadObjMesh(&originalMesh);
        originalMesh.setCube(); // Clears up memory
        isSubdivided = false;
    }


    /**
     * Loads an OBJ Mesh and converts it to a WingedEdgeMesh and setup shading.
     * @param objMesh
     * @return
     */
    bool loadObjMesh(OBJMesh* objMesh){
        // Convert to WingedEdge
        if (!weMesh.loadModel(objMesh->getVertices(), objMesh->getFaces()))
            return false;

        // Populate matrices used for shading
        if (!(flatShading ? weMesh.populateFlatShadingMatrices() : weMesh.populateSmoothShadingMatrices()))
            return false;

        // halfWidth is used to determine project matrix so that model is visible
        halfWidth = weMesh.getMaxPoint()[0];
        for(int i = 0; i < 3; i++){
            if(weMesh.getMaxPoint()[i] > halfWidth)
                halfWidth = weMesh.getMaxPoint()[i];
            if(-weMesh.getMinPoint()[i] > halfWidth)
                halfWidth = -weMesh.getMinPoint()[i];
        }

        // Epsilon used to offset wireframe rendered on top of mesh
        wireframeOverlayEpsilon = 0.01;
        if(halfWidth / 200 < wireframeOverlayEpsilon)
            wireframeOverlayEpsilon = halfWidth / 200;

        if(drawWireframe && drawSolid)
            weMesh.populateExpandedVertexMatrix(wireframeOverlayEpsilon);

        // Reset camera view
        mTarget = Eigen::Vector3f(0,0,0);
        mZoom = 1;
        mRotation = Eigen::Vector3f(0,0,0);

        // Push new matrices to renderer
        refreshGeometry();
        return true;
    }

    /**
     * Refresh geometry loaded into shader using the geometry available in loaded WingedEdge mesh.
     */
    void refreshGeometry(){
        // Fetch matrices from WingedEdge mesh
        positions = weMesh.getVertexMatrix();
        indices = weMesh.getFaceMatrix();
        normals = weMesh.getNormalMatrix();

        colors = MatrixXf(3, weMesh.getVertexMatrix().cols());
        for (int i = 0; i < weMesh.getVertexMatrix().cols(); i++){
            colors.col( i) << 0.5 + fmod(i*0.1f,0.5), 0, 0;
            colors.col( i) << 1,0,0;
        }

        // Push matrices
        mShader.bind();
        mShader.uploadIndices(indices);
        mShader.uploadAttrib("vertexPosition_modelspace", positions);
        mShader.uploadAttrib("color", colors);
        mShader.uploadAttrib("vertexNormal_modelspace", normals);

        // This the light origin position in your environment, which is totally arbitrary
        // however it is better if it is behind the observer
        // I am not going to do this since then effect of flat shading/ smooth shading will not be that prominent.
        mShader.setUniform("LightPosition_worldspace", Eigen::Vector3f(-2 * halfWidth-2,6* halfWidth+6,-4 * halfWidth-4));
    }

    //flush data on call
    ~MyGLCanvas() {
        mShader.free();
    }

    /**
     * Set zoom level. Minimum zoom is 0.1. Default is 1.
     * @param zoom
     */
    void setZoom(double zoom){
        if(zoom < 0.1)
            return;
        mZoom = zoom;
    }

    /**
     * Retrieve zoom level
     * @return
     */
    double getZoom(){
        return mZoom;
    }

    /**
     * Retrieve rotation vector
     * @return
     */
    nanogui::Vector3f getRotation(){
        return mRotation;
    }

    /**
     * Set rotation vector
     * @param vRotation
     */
    void setRotation(nanogui::Vector3f vRotation) {
        mRotation = vRotation;
    }

    /**
     * Retrieve camera look at target. Used to calculate camera translate direction when panned using mouse.
     * @return
     */
    nanogui::Vector3f getTarget(){
        return mTarget;
    }

    /**
     * Set camera look at target.
     * @param vTarget
     */
    void setTarget(nanogui::Vector3f vTarget) {
        mTarget = vTarget;
    }

    /**
     * Switch to Smooth Shaded with Wireframes mode
     */
    void setShadedWithMeshEdges(){
        weMesh.populateSmoothShadingMatrices();
        weMesh.populateExpandedVertexMatrix(wireframeOverlayEpsilon);
        refreshGeometry();
        drawWireframe = true;
        drawSolid = true;
        flatShading = false;
    }

    /**
     * Switch to Wireframe mode
     */
    void setWireframeMode(){
        weMesh.populateSmoothShadingMatrices();
        refreshGeometry();
        drawWireframe = true;
        drawSolid = false;
        flatShading = false;
    }


    /**
     * Switch to smooth shading mode
     */
    void setSmoothShading(){
        weMesh.populateSmoothShadingMatrices();
        refreshGeometry();
        drawWireframe = false;
        drawSolid = true;
        flatShading = false;
    }

    /**
     * Switch to flat shading mode
     */
    void setFlatShading(){
        weMesh.populateFlatShadingMatrices();
        refreshGeometry();
        drawWireframe = false;
        drawSolid = true;
        flatShading = true;
    }

    /**
     * Update model, view and projection matrices
     */
    void updateMVP(){
        // Calculate view matrix.
        // Camera looks at target position, from a point at a distance (maximum distance between any two points in the mesh) from target point,
        // from an orietation defined by rotation vector.
        nanogui::Matrix4f camRotation;
        camRotation.setIdentity();
        camRotation.topLeftCorner<3,3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[1], nanogui::Vector3f::UnitY()) *
                                                           Eigen::AngleAxisf(mRotation[2],  nanogui::Vector3f::UnitZ()) *
                                                           Eigen::AngleAxisf(mRotation[0], nanogui::Vector3f::UnitX()));

        nanogui::Vector4f camPosition = camRotation * nanogui::Vector4f(0,halfWidth*2,0, 1) ;
        nanogui::Vector4f camUp = camRotation * nanogui::Vector4f(0,0,1,1);
        V = nanogui::lookAt(camPosition.topRows<3>() + mTarget, mTarget, camUp.topRows<3>());


        //We do rotations using view.
        M.setIdentity();

        // Projection matrix is set based on zoom level and maximum distance between any two vertices in mesh
        P = nanogui::ortho(-halfWidth*2/mZoom,halfWidth*2/mZoom,-halfWidth*2/mZoom,halfWidth*2/mZoom,0,halfWidth*4+100);
        MVP = P * V * M;
    }

    //OpenGL calls this method constantly to update the screen.
    virtual void drawGL() override {
        using namespace nanogui;

        //refer to the previous explanation of mShader.bind();
        mShader.bind();

        //this simple command updates the positions matrix. You need to do the same for color and indices matrices too
        mShader.uploadAttrib("vertexPosition_modelspace", positions);

        // Pass in MVP matrices
        mShader.setUniform("V", V);
        mShader.setUniform("M", M);
        mShader.setUniform("MVP", MVP);

        // If enabled, does depth comparisons and update the depth buffer.
        // Avoid changing if you are unsure of what this means.
        glEnable(GL_DEPTH_TEST);

        // Cull back faces so wireframes look similar to the preview given in assignment pdf
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Draw solid
        if(drawSolid){
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            mShader.drawIndexed(GL_TRIANGLES, 0, weMesh.getFaceMatrix().cols());
        }

        // Draw wireframe
        if(drawWireframe){
            if(drawSolid)
            {
                // Load expanded vertices
                mShader.uploadAttrib("vertexPosition_modelspace", weMesh.getExpandedVertexMatrix());
            }

            // Correct diffuse so wireframe is coloured differently from shaded mesh
            mShader.setUniform("diffuseCorrection", Eigen::Vector3f(1,1,1));

            // Draw as lines
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            mShader.drawIndexed(GL_TRIANGLES, 0, weMesh.getFaceMatrix().cols());

            mShader.setUniform("diffuseCorrection", Eigen::Vector3f(0,0,0));
            if(drawSolid)
            {
                mShader.uploadAttrib("vertexPosition_modelspace", positions);
            }
        }
        // Reset to render filled solids
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glDisable(GL_DEPTH_TEST);
    }

//Instantiation of the variables that can be acessed outside of this class to interact with the interface
//Need to be updated if a interface element is interacting with something that is inside the scope of MyGLCanvas
private:
    bool drawSolid = true; // Should the the mesh be drawn shaded/filled?
    bool drawWireframe = false;  // Should the mesh be drawn as wireframe?
    bool flatShading = true; // Is flat shading being used?

    float halfWidth = 2; // Half the maximum distance between any 2 points on mesh

    WEMesh weMesh; // The loaded WingedEdge mesh that is being rendered

    // Matrices used for rendering
    MatrixXf normals = MatrixXf(3, 8);
    MatrixXu indices = MatrixXu(3, 12);
    MatrixXf positions = MatrixXf(3, 8);
    MatrixXf colors = MatrixXf(3, 12);

    // Shader
    nanogui::GLShader mShader;

    // Camera rotation
    Eigen::Vector3f mRotation = Eigen::Vector3f(0,0,0);
    Eigen::Vector3f mTarget = Eigen::Vector3f(0,0,0); // Camera look at position
    double mZoom = 1; // Zoom level

    float wireframeOverlayEpsilon = 0.01; // Gap between wireframe and solid rendering

    // MVP matrices
    nanogui::Matrix4f M;
    nanogui::Matrix4f V;
    nanogui::Matrix4f P;
    nanogui::Matrix4f MVP;

    bool isSubdivided = false;
    OBJMesh originalMesh; // Mesh prior to subdivision

};

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
        mCanvas = new MyGLCanvas(window);
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
            mCanvas->resetOriginalMesh();
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
            mCanvas->resetOriginalMesh();
            for(int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopSubdivision();
        });


        button = new Button(subdivisionWindow, "Tessellate");
        button->setCallback([&] {
            mCanvas->resetOriginalMesh();
            for(int i = 0; i < subdivisionAmount; i++)
                mCanvas->performLoopTesselate();
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
    MyGLCanvas *mCanvas;
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
