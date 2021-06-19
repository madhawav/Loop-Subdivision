//
// Created by madhawa on 2020-02-01.
//

#ifndef NANOGUI_TEST_WEMESH_H
#include <WingedEdge/Vertex.h>
#include <WingedEdge/Face.h>
#include <nanogui/common.h>
#include <WingedEdge/OBJMesh.h>
#include <WingedEdge/Edge.h>

namespace WingedEdge {
#define NANOGUI_TEST_WEMESH_H

    /**
     *  Class used to hold WingedEdge representation of a Mesh.
     */
    class WEMesh {
    public:
        WEMesh();
        ~WEMesh();

        /**
         * Populates WingedEdge faces, edges and vertices with appropriate inter-links using provided vertices and faces (vertex indices)
         * @param vertices 3xn array of vertices
         * @param faces 3xt array of vertex indices of each triangular face
         * @return true if success
         */
        bool loadModel(nanogui::MatrixXf vertices, nanogui::MatrixXu faces);

        /**
         * Populates VertexMatrix, NormalMatrix and FaceMatrix for Smooth Shading Purpose.
         * @return True if success
         */
        bool populateSmoothShadingMatrices();

        /**
         * Populates VertexMatrix, NormalMatrix and FaceMatrix for Flat Shading Purpose.
         * @return True if success
         */
        bool populateFlatShadingMatrices();

        /**
         * Populates ExpandedVertexMatrix, that holds vertices used to draw wireFrame on top of shaded mesh.
         * @param epsilon Expansion length
         */
        void populateExpandedVertexMatrix(float epsilon);

        /**
         * Fill vertices and faces matrix of provided objMesh using the information available in WingedEdge structure.
         * @param objMesh
         */
        void fillOBJMesh(WingedEdge::OBJMesh* objMesh) const;

        /**
         * Check whether a model is loaded.
         * @return True if model is loaded. Otherwise false.
         */
        bool isModelLoaded() const;

        /**
         * Returns minimum coordinate in all 3 axis. Used to set camera parameters when a new model is loaded.
         * @return
         */
        nanogui::Vector3f getMinPoint() const;

        /**
         * Returns maximum coordinate in all 3 axis. Used to set camera parameters when a new model is loaded.
         * @return
         */
        nanogui::Vector3f getMaxPoint() const;

        /**
         * Returns matrix of normals used for rendering purpose. Populated by populateFlatShadingMatrices or populateSmoothShadingMatrices
         * @return
         */
        nanogui::MatrixXf getNormalMatrix() const;

        /**
         * Returns matrix of vertices used for rendering purpose. Populated by populateFlatShadingMatrices or populateSmoothShadingMatrices
         * @return
         */
        nanogui::MatrixXf getVertexMatrix() const;

        /**
         * Returns matrix of faces used for rendering purpose. Populated by populateFlatShadingMatrices or populateSmoothShadingMatrices
         * @return
         */
        nanogui::MatrixXu getFaceMatrix() const;

        /**
         * Returns matrix of vertices used for rendering wireframes on top of solid mesh. Populated by populateExpandedVertexMatrix.
         * @return
         */
        nanogui::MatrixXf getExpandedVertexMatrix() const;

        Vertex* getVertices() const;
        Edge* getEdges() const;
        Face* getFaces() const;

        int getVertexCount() const;
        int getEdgeCount() const;
        int getFaceCount() const;

        void allocateVertices(int v);
        void allocateEdges(int e);
        void allocateFaces(int f);

    private:
        // WingedEdge elements live in following arrays
        Vertex* mVertices;
        Edge* mEdges;
        Face* mFaces;

        int mVertexCount;
        int mFaceCount;
        int mEdgeCount;

        nanogui::Vector3f mMinPoint;
        nanogui::Vector3f mMaxPoint;

        // Populated by populateFlatShadingMatrix, populateSmoothShadingMatrix methods
        nanogui::MatrixXf mNormalMatrix;
        nanogui::MatrixXf mVertexMatrix;
        nanogui::MatrixXu mFaceMatrix;

        // Populated by populateExpandedVertexMatrix method
        nanogui::MatrixXf mExpandedVertexMatrix;

        // Used to avoid re-calculations
        bool mSmoothedShaded;
        bool mFlatShaded;

        bool mModelLoaded;

        /**
         * Unload mesh.
         */
        void unloadModel();
    };
}


#endif //NANOGUI_TEST_WEMESH_H
