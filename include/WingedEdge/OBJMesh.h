//
// Created by madhawa on 2020-02-01.
//

#ifndef NANOGUI_TEST_OBJMESH_H
#include <fstream>
#include <WingedEdge/Vertex.h>
#include <nanogui/common.h>

namespace WingedEdge {
#define NANOGUI_TEST_OBJMESH_H

    /**
     * Mesh loaded from an OBJ file. Contains vertices and faces (Vertex indices)
     */
    class OBJMesh {
    public:
        OBJMesh();

        ~OBJMesh();

        /**
         * Parse an OBJ file and populate matrices Vertices and Faces.
         * @param fs Input stream from obj file
         * @return true if success
         */
        bool parseFile(std::ifstream *fs);

        /**
         * Saves mesh as an obj file.
         * @param fs
         * @return
         */
        bool saveFile(std::ofstream *fs);

        /**
         * Set the matrix of vertices and faces of the mesh. Used by WingedEdge to convert back to an OBJ Mesh for saving purpose.
         * @param vertices Matrix of vertices
         * @param faces Matrix of faces
         */
        void setMatrices(nanogui::MatrixXf vertices, nanogui::MatrixXu faces);

        /**
         * Set the mesh to become a cube of 2 unit length
         */
        void setCube();

        /**
         * Retrieve matrix of vertices
         * @return 3xn Matrix
         */
        nanogui::MatrixXf getVertices();

        /**
         * Retrieve matrix of faces (Vertex indices)
         * @return 3xt Matrix
         */
        nanogui::MatrixXu getFaces();

        /**
         * Retrieve number of vertices
         * @return
         */
        int getVertexCount();

        /**
         * Retrieve number of faces
         * @return
         */
        int getFaceCount();

    private:
        int mVertexCount;
        int mFaceCount;

        nanogui::MatrixXf mVertices;
        nanogui::MatrixXu mFaces;
    };
}


#endif //NANOGUI_TEST_OBJMESH_H
