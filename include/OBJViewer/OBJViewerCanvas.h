//
// Created by madhawa on 2021-06-18.
//

#ifndef OBJ_VIEW_OBJVIEWERCANVAS_H

#include <nanogui/common.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/glcanvas.h>
#include <nanogui/widget.h>
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/OBJMesh.h>


namespace OBJViewer {
#define OBJ_VIEW_OBJVIEWERCANVAS_H

/**
 * Canvas used for rendering of a mesh.
 */
    class OBJViewerCanvas : public nanogui::GLCanvas {
    public:
        /**
         * Initialize OBJViewerCanvas.
         * @param parent Parent widget containing the canvas.
         */
        OBJViewerCanvas(Widget *parent);

        /**
         * Save loaded mesh as a .obj file.
         * @param path Path to save.
         * @return True if success. False otherwise.
         */
        bool saveObj(const std::string &path) const;

        /**
         * Loads an OBJ file from disk. Converts it to a WingedEdgeMesh and setup shading.
         * @param path Path to obj file
         * @return True if success. False otherwise.
         */
        bool loadObj(const std::string &path);

        /**
         * Perform loop subdivision.
         */
        void performLoopSubdivision();

        /**
         * Perform tessellation used by loop subdivision
         */
        void performLoopTessellation();

        /**
         * Re-load unsubdivided mesh.
         */
        void resetToOriginalMesh();


        /**
         * Loads an OBJ Mesh and converts it to a WingedEdgeMesh and setup shading.
         * @param objMesh
         * @return
         */
        bool loadObjMesh(WingedEdge::OBJMesh *objMesh);

        /**
         * Refresh geometry loaded into shader using the geometry available in loaded WingedEdge mesh.
         */
        void refreshGeometry();

        /**
         * Destructor.
         */
        virtual ~OBJViewerCanvas();

        /**
         * Set zoom level. Minimum zoom is 0.1. Default is 1.
         * @param zoom
         */
        void setZoom(float zoom);

        /**
         * Retrieve zoom level
         * @return
         */
        double getZoom() const;

        /**
         * Retrieve rotation vector
         * @return
         */
        nanogui::Vector3f getRotation() const;

        /**
         * Set rotation vector
         * @param vRotation
         */
        void setRotation(nanogui::Vector3f vRotation);

        /**
         * Retrieve camera look at target. Used to calculate camera translate direction when panned using mouse.
         * @return
         */
        nanogui::Vector3f getTarget() const;

        /**
         * Set camera look at target.
         * @param vTarget
         */
        void setTarget(nanogui::Vector3f vTarget);

        /**
         * Switch to Smooth Shaded with Wireframes mode
         */
        void setShadedWithMeshEdges();

        /**
         * Switch to Wireframe mode
         */
        void setWireframeMode();


        /**
         * Switch to smooth shading mode
         */
        void setSmoothShading();

        /**
         * Switch to flat shading mode
         */
        void setFlatShading();

        /**
         * Update model, view and projection matrices
         */
        void updateMVP();

        //OpenGL calls this method constantly to update the screen.
        virtual void drawGL() override;

    private:
        bool drawSolid; // Should the the mesh be drawn shaded/filled?
        bool drawWireframe;  // Should the mesh be drawn as wireframe?
        bool flatShading; // Is flat shading being used?
        float halfWidth{}; // Half the size of object.
        float wireframeOverlayEpsilon{}; // Tiny gap between wireframe and the solid.

        WingedEdge::WEMesh weMesh; // The loaded WingedEdge mesh that is being rendered

        // Matrices used for rendering
        nanogui::MatrixXf normals;
        nanogui::MatrixXu indices;
        nanogui::MatrixXf positions;
        nanogui::MatrixXf colors;

        // Shader
        nanogui::GLShader mShader;

        // Camera rotation
        Eigen::Vector3f mRotation;
        Eigen::Vector3f mTarget; // Camera look at position
        float mZoom; // Zoom level

        // MVP matrices
        nanogui::Matrix4f M;
        nanogui::Matrix4f V;
        nanogui::Matrix4f P;
        nanogui::Matrix4f MVP;

        bool isSubdivided;
        WingedEdge::OBJMesh originalMesh; // Mesh prior to subdivision
    };
}


#endif //OBJ_VIEW_OBJVIEWERCANVAS_H
