//
// Created by madhawa on 2020-02-01.
//

#include <iostream>
#include <nanogui/common.h>
#include <Eigen/Geometry>
#include <WingedEdge/Edge.h>
#include <WingedEdge/Vertex.h>
#include <WingedEdge/Face.h>
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/OBJMesh.h>

WEMesh::WEMesh() {
    mVertices = nullptr;
    mFaces = nullptr;
    mEdges = nullptr;
    mMinPoint = nanogui::Vector3f(0,0,0);
    mMaxPoint = nanogui::Vector3f(0,0,0);
    mVertexMatrix = nanogui::MatrixXf(3,0);
    mFaceMatrix = nanogui::MatrixXu(3,0);
    mNormalMatrix = nanogui::MatrixXf(3, 0);
    mExpandedVertexMatrix = nanogui::MatrixXf(3,0);
    mVertexCount = 0;
    mFaceCount = 0;
    mEdgeCount = 0;
}

/**
 * Populates WingedEdge faces, edges and vertices with appropriate inter-links using provided vertices and faces (vertex indices)
 * @param vertices 3xn array of vertices
 * @param faces 3xt array of vertex indices of each triangular face
 * @return true if success
 */
bool WEMesh::loadModel(nanogui::MatrixXf vertices, nanogui::MatrixXu faces){
    std::cout << std::endl << "Populating Winged Edge" <<std::endl;

    // Reset state variables since a new mesh is loaded
    mMinPoint = nanogui::Vector3f(0,0,0);
    mMaxPoint = nanogui::Vector3f(0,0,0);
    mVertexMatrix = nanogui::MatrixXf(3,0);
    mFaceMatrix = nanogui::MatrixXu(3,0);
    mNormalMatrix = nanogui::MatrixXf(3, 0);
    mExpandedVertexMatrix = nanogui::MatrixXf(3,0);

    isSmoothedShaded = false;
    isFlatShaded = false;

    delete[] mVertices;
    delete[] mEdges;
    delete[] mFaces;

    mVertexCount = vertices.cols();
    mFaceCount = faces.cols();
    mEdgeCount = mFaceCount*3/2;

    mVertices = new Vertex[mVertexCount];
    mEdges = new Edge[mFaceCount*3/2];
    mFaces = new Face[mFaceCount];

    // Used to locate an edge using a vertex pair. Think of this as a hash map
    Edge*** vertexPairEdgeArray = new Edge**[mVertexCount];
    for(int i = 0; i < mVertexCount; i++){
        vertexPairEdgeArray[i] = new Edge*[mVertexCount];
        for (int j = 0; j < mVertexCount; j++){
            vertexPairEdgeArray[i][j] = nullptr;
        }
    }

    // load vertices
    for(int i = 0; i < mVertexCount; i++){
        mVertices[i].setPosition(vertices(0,i),vertices(1,i),vertices(2,i));

        // update dimensions
        for(int j = 0; j < 3; j++)
        {
            if(mVertices[i].getPosition()[j] < mMinPoint[j]){
                mMinPoint[j] = mVertices[i].getPosition()[j];
            }
            if(mVertices[i].getPosition()[j] > mMaxPoint[j]){
                mMaxPoint[j] = mVertices[i].getPosition()[j];
            }
        }
    }

    int edgeCount = 0;
    // Loop through faces and populate edges. Fill LeftFace and RightFace.
    for (int f = 0; f < mFaceCount; f++)
    {
        Edge* edgeBank[] = {nullptr, nullptr, nullptr}; //keep track of 3 edges belonging to this face
        for(int k = 0; k < 3; k++)
        {
            int v1_index = faces(k,f);
            int v2_index = faces((k+1)%3, f);
            // v1 => v2 is counter clockwise direction
            // v2 => v1 is clockwise direction
            // v2 => v1 is my edge in clockwise direction. But need to check whether v1 => v2 is already acquired by another face in clockwise direction
            if(vertexPairEdgeArray[v1_index][v2_index] == nullptr){
                // OK. the edge v1 => v2 does not exist. I can create edge v2 => v1.
                vertexPairEdgeArray[v2_index][v1_index] = &(mEdges[edgeCount]);
                mEdges[edgeCount].mVertOrigin = &mVertices[v2_index];
                mEdges[edgeCount].mVertDest = &mVertices[v1_index];
                // I am the right face.
                mEdges[edgeCount].mRightFace = &mFaces[f];
                if(k == 0)
                {
                    mFaces[f].setEdge(&(mEdges[edgeCount]));
                }
                edgeBank[k] = &(mEdges[edgeCount]);

                // Check whether two end nodes has an edge. If not, set this edge
                if(mVertices[v1_index].getEdge() == nullptr)
                    mVertices[v1_index].setEdge(&(mEdges[edgeCount]));

                if(mVertices[v2_index].getEdge() == nullptr)
                    mVertices[v2_index].setEdge(&(mEdges[edgeCount]));

                edgeCount++;
            }
            else{
                // Edge v1 => v2 already exists. I cant create v2=>v1. Instead, I should be the other face
                vertexPairEdgeArray[v1_index][v2_index]->mLeftFace = &mFaces[f];
                if(k == 0)
                {
                    mFaces[f].setEdge(vertexPairEdgeArray[v1_index][v2_index]);
                }
                edgeBank[k] = vertexPairEdgeArray[v1_index][v2_index];
            }
        }

        assert(edgeBank[0] != nullptr);
        assert(edgeBank[1] != nullptr);
        assert(edgeBank[2] != nullptr);

        //Now to fill Adjacent edges
        for(int k = 0; k < 3; k++)
        {
            int v1_index = faces(k,f);
            int v2_index = faces((k+1)%3, f);
            // v2 => v1 is clockwise direction
            if(vertexPairEdgeArray[v2_index][v1_index] != nullptr)
            {
                assert(vertexPairEdgeArray[v1_index][v2_index] == nullptr);
                // I have set v2=>v1. I am the right face. k is anticlockwise.
                int c = k - 1;
                if(c < 0) c = 2;
                vertexPairEdgeArray[v2_index][v1_index]->mEdgeRightCW = edgeBank[c];
                vertexPairEdgeArray[v2_index][v1_index]->mEdgeRightCCW = edgeBank[(k+1)%3];
            }
            else if(vertexPairEdgeArray[v1_index][v2_index] != nullptr){
                assert(vertexPairEdgeArray[v2_index][v1_index]== nullptr);
                // I have joined to v1 => v2 face already in existence. So, I am the left face. k is anticlockwise.
                vertexPairEdgeArray[v1_index][v2_index]->mEdgeLeftCCW = edgeBank[(k+1)%3];

                int c = k - 1;
                if(c < 0) c = 2;
                vertexPairEdgeArray[v1_index][v2_index]->mEdgeLeftCW = edgeBank[c];
            }
        }


    }
    std::cout << std::endl << "Winged Edge Populated" <<std::endl;
    std::cout << "Winged Edge: Actual Edge Count: "<< edgeCount << std::endl;
    std::cout << "Winged Edge: Expected Edge Count (Face Count * 3 / 2): "<< mFaceCount*3/2 << std::endl;

    std::cout << "Min Point: "<< mMinPoint[0] << ", " << mMinPoint[1] << ", " << mMinPoint[2] << std::endl;
    std::cout << "Max Point: "<< mMaxPoint[0] << ", " << mMaxPoint[1] << ", " << mMaxPoint[2] << std::endl;

    for(int i = 0; i < mVertexCount; i++){
        delete[] vertexPairEdgeArray[i];
    }
    delete[] vertexPairEdgeArray;
    return true;
}

nanogui::Vector3f WEMesh::getMinPoint() {
    return mMinPoint;
}

nanogui::Vector3f WEMesh::getMaxPoint() {
    return mMaxPoint;
}

nanogui::MatrixXu WEMesh::getFaceMatrix() {
    return mFaceMatrix;
}

nanogui::MatrixXf WEMesh::getVertexMatrix() {
    return mVertexMatrix;
}

nanogui::MatrixXf WEMesh::getNormalMatrix() {
    return mNormalMatrix;
}

/**
 * Populates VertexMatrix, NormalMatrix and FaceMatrix for Smooth Shading Purpose.
 * @return True if success
 */
bool WEMesh::populateSmoothShadingMatrices() {
    if(isSmoothedShaded)
        return true;

    // Initialize matrices
    mVertexMatrix = nanogui::MatrixXf(3, mVertexCount);
    mNormalMatrix = nanogui::MatrixXf(3,  mVertexCount);
    mFaceMatrix = nanogui::MatrixXu(3, mFaceCount);

    // Used to normalize the vertex normal after summing up normals from each adjacent face
    int* adjacentFaceCount = new int[mVertexCount];

    for(int i = 0; i < mVertexCount; i++)
    {
        mVertexMatrix.col(i) << mVertices[i].getPosition();
        mNormalMatrix.col(i) << nanogui::Vector3f(0,0,0); // Accumulate normals from adjacent faces here
        adjacentFaceCount[i] = 0; // None accumulated so far
    }

    // Loop through faces
    int vertexIndex = 0;
    for(int f = 0; f < mFaceCount; f++)
    {
        // Identify v1, v2 and v3 in order
        Vertex* v1 ;
        Vertex* v2 ;
        Vertex* v3 ;
        if( mFaces[f].getEdge()->mRightFace == &(mFaces[f])){
            // I am the right face
            v2 = mFaces[f].getEdge()->mVertOrigin;
            v1 = mFaces[f].getEdge()->mVertDest;
            v3 = mFaces[f].getEdge()->mEdgeRightCW->mVertOrigin; // Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeRightCW->mVertDest;
            }
        }
        else if (mFaces[f].getEdge()->mLeftFace == &(mFaces[f])){
            // I am the left face
            v1 = mFaces[f].getEdge()->mVertOrigin;
            v2 = mFaces[f].getEdge()->mVertDest;
            v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertOrigin; //Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertDest;
            }
        }
        else{
            assert(false); // Something wrong with WingedEdge structure
        }

        // Do we have missing faces? Is the mesh a manifold?
        assert(v1 != nullptr);
        assert(v2 != nullptr);
        assert(v3 != nullptr);

        // Calculate Face Normal
        nanogui::Vector3f normal = -((v3->getPosition() - v1->getPosition()).cross( (v2->getPosition() - v1->getPosition())));
        normal.normalize();

        // Distribute Face normal among vertices
        adjacentFaceCount[v1-mVertices] += 1;
        adjacentFaceCount[v2-mVertices] += 1;
        adjacentFaceCount[v3-mVertices] += 1;
        mNormalMatrix.col(v1-mVertices)+= normal;
        mNormalMatrix.col(v2-mVertices)+= normal;
        mNormalMatrix.col(v3-mVertices)+= normal;


        // Pointer arithmatic to figure out index of element in array
        mFaceMatrix.col(f) << v1-mVertices, v2-mVertices, v3-mVertices;
        vertexIndex+=3;

    }

    // Normalize accumulated normals
    for(int i = 0; i < mVertexCount; i++)
    {
        mNormalMatrix.col(i) << (mNormalMatrix.col(i) / adjacentFaceCount[i]).normalized();
    }
    isSmoothedShaded = true;
    isFlatShaded = false;

    delete[] adjacentFaceCount;
    return true;
}

/**
 * Populates VertexMatrix, NormalMatrix and FaceMatrix for Flat Shading Purpose.
 * @return True if success
 */
bool WEMesh::populateFlatShadingMatrices() {
    if(isFlatShaded)
        return true;

    // Initialize matrices
    mVertexMatrix = nanogui::MatrixXf(3, mFaceCount*3); // Each face has its own vertices
    mNormalMatrix = nanogui::MatrixXf(3, mFaceCount*3);
    mFaceMatrix = nanogui::MatrixXu(3, mFaceCount);

    int vertexIndex = 0;
    // Loop through faces
    for(int f = 0; f < mFaceCount; f++)
    {
        // Identify v1, v2 and v3 in order
        Vertex* v1 ;
        Vertex* v2 ;
        Vertex* v3;
        if( mFaces[f].getEdge()->mRightFace == &(mFaces[f])){
            // I am the right face
            v2 = mFaces[f].getEdge()->mVertOrigin;
            v1 = mFaces[f].getEdge()->mVertDest;
            v3 =  mFaces[f].getEdge()->mEdgeRightCW->mVertOrigin; // Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeRightCW->mVertDest;
            }
        }
        else if (mFaces[f].getEdge()->mLeftFace == &(mFaces[f])){
            // I am the left face
            v1 = mFaces[f].getEdge()->mVertOrigin;
            v2 = mFaces[f].getEdge()->mVertDest;
            v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertOrigin; // Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertDest;
            }
        }
        else{
            assert(false); // Something wrong with WingedEdge structure
        }

        // Do we have missing faces? Is the mesh a manifold?
        assert(v1 != nullptr);
        assert(v2 != nullptr);
        assert(v3 != nullptr);

        // Calculate face normals using cross product
        nanogui::Vector3f normal = -((v3->getPosition() - v1->getPosition()).cross( (v2->getPosition() - v1->getPosition())));
        normal.normalize();

        // Add newly calculated vertices for the face
        mVertexMatrix.col(vertexIndex) << v1->getPosition();
        mNormalMatrix.col(vertexIndex) << normal;

        mVertexMatrix.col(vertexIndex+1) << v2->getPosition();
        mNormalMatrix.col(vertexIndex+1) << normal;

        mVertexMatrix.col(vertexIndex+2) << v3->getPosition();
        mNormalMatrix.col(vertexIndex+2) << normal;

        // Add new face
        mFaceMatrix.col(f) << vertexIndex, vertexIndex+1, vertexIndex+2;
        vertexIndex+=3;

    }
    isSmoothedShaded = false;
    isFlatShaded = true;
    return true;
}

WEMesh::~WEMesh() {
    delete[] mVertices;
    delete[] mEdges;
    delete[] mFaces;
}

/**
 * Fill vertices and faces matrix of provided objMesh using the information available in WingedEdge structure.
 * @param objMesh
 */
void WEMesh::fillOBJMesh(OBJMesh * objMesh) {
    nanogui::MatrixXf vertexMatrix =  nanogui::MatrixXf(3, mVertexCount);
    nanogui::MatrixXu faceMatrix = nanogui::MatrixXu(3, mFaceCount);

    for(int i = 0; i < mVertexCount; i++)
    {
        vertexMatrix.col(i) << mVertices[i].getPosition();
    }

    int vertexIndex = 0;
    for(int f = 0; f < mFaceCount; f++)
    {
        // Identify vertices in order
        Vertex* v1 ;
        Vertex* v2 ;
        Vertex* v3;
        if( mFaces[f].getEdge()->mRightFace == &(mFaces[f])){
            v2 = mFaces[f].getEdge()->mVertOrigin;
            v1 = mFaces[f].getEdge()->mVertDest;
            v3 =  mFaces[f].getEdge()->mEdgeRightCW->mVertOrigin; // Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeRightCW->mVertDest;
            }
        }
        else if (mFaces[f].getEdge()->mLeftFace == &(mFaces[f])){
            v1 = mFaces[f].getEdge()->mVertOrigin;
            v2 = mFaces[f].getEdge()->mVertDest;
            v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertOrigin; // Can be origin or dest
            if(v3 == v1 || v3 == v2){
                v3 =  mFaces[f].getEdge()->mEdgeLeftCW->mVertDest;
            }
        }
        else{
            // Something wrong with winged edge structure
            assert(false);
        }

        // Is it a manifold?
        assert(v1 != nullptr);
        assert(v2 != nullptr);
        assert(v3 != nullptr);

        // Pointer arithmatic to figure out index of element in array
        faceMatrix.col(f) << v1-mVertices, v2-mVertices, v3-mVertices;
        vertexIndex+=3;

    }

    objMesh->setMatrices(vertexMatrix, faceMatrix);
}

nanogui::MatrixXf WEMesh::getExpandedVertexMatrix() {
    return mExpandedVertexMatrix;
}

/**
 * Populates ExpandedVertexMatrix, that holds vertices used to draw wireFrame on top of shaded mesh.
 * @param epsilon Expansion length
 */
void WEMesh::populateExpandedVertexMatrix(float epsilon) {
    mExpandedVertexMatrix = nanogui::MatrixXf(3, mVertexCount);
    for(int i = 0; i < mVertexMatrix.cols(); i++){
        mExpandedVertexMatrix.col(i) << mVertexMatrix.col(i) + mNormalMatrix.col(i).normalized() * epsilon;
    }
}

Vertex *WEMesh::getVertices() {
    return mVertices;
}

Edge *WEMesh::getEdges() {
    return mEdges;
}

Face *WEMesh::getFaces() {
    return mFaces;
}

int WEMesh::getVertexCount() {
    return mVertexCount;
}

int WEMesh::getEdgeCount() {
    return mEdgeCount;
}

int WEMesh::getFaceCount() {
    return mFaceCount;
}

void WEMesh::allocateVertices(int v){
    delete[] mVertices;
    mVertexCount = v;
    mVertices = new Vertex[mVertexCount];
}
void WEMesh::allocateEdges(int e){
    delete[] mEdges;
    mEdgeCount = e;
    mEdges = new Edge[mEdgeCount];
}
void WEMesh::allocateFaces(int f){
    delete[] mFaces;
    mFaceCount = f;
    mFaces = new Face[mFaceCount];
}
