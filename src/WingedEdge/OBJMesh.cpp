//
// Created by madhawa on 2020-02-01.
//

#include <iostream>
#include <cstring>
#include <nanogui/common.h>
#include <WingedEdge/OBJMesh.h>

WingedEdge::OBJMesh::OBJMesh() {
    mVertexCount = 0;
    mFaceCount = 0;
    mVertices = nanogui::MatrixXf(3,0);
    mFaces = nanogui::MatrixXu(3,0);
}

/**
 * Parse an OBJ file and populate matrices Vertices and Faces.
 * @param fs Input stream from obj file
 * @return true if success
 */
bool WingedEdge::OBJMesh::parseFile(std::ifstream* fs) {
    char line[100]; // Buffer for reading purpose
    memset(line,0, sizeof(line));

    if(!fs->is_open())
        return false;

    // Wait for the first comment with file size
    while(line[0] != "#"[0]){
        fs->getline(line,100);
    }
    // Parse comment and retrieve metadata
    sscanf(line, "# %d %d", &mVertexCount, &mFaceCount);
    if(mVertexCount == 0 || mFaceCount == 0 )
        return false;

    // Reserve memory
    mVertices = nanogui::MatrixXf(3, mVertexCount);
    mFaces = nanogui::MatrixXu(3, mFaceCount);
#ifdef LOG_EXTRA
    std::cout << "Vertex Count: " << mVertexCount << "\t Edge Count: " << mFaceCount << std::endl;
#endif

    int vertexIndex = 0;
    int faceIndex = 0;

    while(!fs->eof()){
        fs->getline(line,100);
        // Skip over comments
        while(line[0] == "#"[0]){
            fs->getline(line,100);
        }
        if(line[0] == "v"[0])
        {
            // Parse vertex
            float px, py, pz;
            sscanf(line, "v %f %f %f", &px, &py, &pz);
            mVertices.col( vertexIndex++) << nanogui::Vector3f(px, py, pz);
#ifdef LOG_EXTRA
            std::cout << "Loaded vertex: " << px << " " << py << " " << pz << std::endl;
#endif
        }
        else if(line[0] == "f"[0])
        {
            // Parse face
            int v1, v2, v3;
            sscanf(line, "f %d %d %d", &v1, &v2, &v3);
            mFaces.col( faceIndex++ ) << v1-1, v2-1, v3-1;
#ifdef LOG_EXTRA
            std::cout << "Face added: " << v1 << " " << v2 << " " << v3 << std::endl;
#endif
        }
    }

#ifdef LOG_EXTRA
    std::cout << "Vertex Matrix " << mVertices.rows() << " x " << mVertices.cols() << std::endl;
#endif
    return true;

}

WingedEdge::OBJMesh::~OBJMesh() {
}

nanogui::MatrixXf WingedEdge::OBJMesh::getVertices() const {
    return mVertices;
}

nanogui::MatrixXu WingedEdge::OBJMesh::getFaces() const{
    return mFaces;
}

int WingedEdge::OBJMesh::getVertexCount() const{
    return mVertexCount;
}

int WingedEdge::OBJMesh::getFaceCount() const{
    return mFaceCount;
}

void WingedEdge::OBJMesh::setMatrices(nanogui::MatrixXf vertices, nanogui::MatrixXu faces) {
    mVertices = vertices;
    mFaces = faces;
    mVertexCount = mVertices.cols();
    mFaceCount = mFaces.cols();
}

/**
 * Saves mesh as an obj file.
 * @param fs Output file stream
 * @return True if success. Otherwise false.
 */
bool WingedEdge::OBJMesh::saveFile(std::ofstream *fs) const{
    // Save metadata
    *fs << "# " << mVertexCount << " " << mFaceCount << std::endl;

    // Save vertices
    for(int i = 0; i < mVertexCount; i++){
        char line[500];
        sprintf(line, "v %f %f %f",mVertices(0, i) ,mVertices(1, i), mVertices(2, i) );
        *fs << line << std::endl;
    }

    // Save faces
    for(int i = 0; i < mFaceCount; i++){
        *fs << "f " << (mFaces(0, i)+1) << " " << (mFaces(1, i)+1)<< " " << (mFaces(2, i)+1) << std::endl;
    }
}

/**
 * Set the mesh to become a cube of 2 unit length
 */
void WingedEdge::OBJMesh::setCube() {
    mVertexCount = 8;
    mVertices = nanogui::MatrixXf(3,8);

    mVertices.col(0) <<  1,  1, -1; //top right back
    mVertices.col(1) << -1,  1, -1; //bottom right back
    mVertices.col(2) << -1,  1, 1; //bottom right front
    mVertices.col(3) <<  1,  1, 1; //top right front
    mVertices.col(4) <<  1, -1, -1; //top left back
    mVertices.col(5) << -1, -1, -1; //bottom left back
    mVertices.col(6) << -1, -1, 1; //bottom left front
    mVertices.col(7) <<  1, -1, 1; //top left front

    mFaceCount = 12;
    mFaces = nanogui::MatrixXu(3,12);
    mFaces.col( 0) << 0, 1, 2;
    mFaces.col( 1) << 0, 2, 3;
    mFaces.col( 2) << 0, 4, 5;
    mFaces.col( 3) << 0, 5, 1;
    mFaces.col( 4) << 1, 5, 6;
    mFaces.col( 5) << 1, 6, 2;
    mFaces.col( 6) << 2, 6, 7;
    mFaces.col( 7) << 2, 7, 3;
    mFaces.col( 8) << 3, 7, 4;
    mFaces.col( 9) << 3, 4, 0;
    mFaces.col(10) << 4, 7, 6;
    mFaces.col(11) << 4, 6, 5;
}
