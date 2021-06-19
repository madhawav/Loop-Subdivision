//
// Created by madhawa on 2020-02-09.
//

#include <iostream>
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/Edge.h>
#include <WingedEdge/OBJMesh.h>
#include <Subdivision/Subdivision.h>


/**
 * Each edge of source edge is represented by two edges in the resultant mesh of loop subdivision.
 * The extended edge data structure is used to maintain relationship between each edge in source mesh and two newly added
 * edges in target mesh
 */
class ExtendedEdge{
public:
    WingedEdge::Edge* originalEdge;
    WingedEdge::Edge* newEdgeS;
    WingedEdge::Edge* newEdgeE;
    bool added;
    ExtendedEdge(){
        originalEdge = nullptr;
        newEdgeE = nullptr;
        newEdgeS = nullptr;
        added = false;
    }
};

/**
 * Each triangle of original mesh is represented by 4 triangles in the subdivided mesh. Extended face is used to link
 * each face of original mesh with 4 faces in resultant mesh.
 */
class ExtendedFace{
public:
    WingedEdge::Face* originalFace;
    ExtendedEdge* extendedEdge1;
    ExtendedEdge* extendedEdge2;
    ExtendedEdge* extendedEdge3;

    WingedEdge::Edge* internalEdge12; // From midpoint of external edge 1 to midpoint of external edge 2
    WingedEdge::Edge* internalEdge23;
    WingedEdge::Edge* internalEdge31;

    bool faceAdded;

    ExtendedFace(){
        originalFace = nullptr;
        extendedEdge1 = extendedEdge2 = extendedEdge3 = nullptr;
        internalEdge12 = internalEdge23 = internalEdge31 = nullptr;
        faceAdded = false;
    }
};

/**
 * Rerrieve vertex shared by two edges e1 and e2
 * @param e1
 * @param e2
 * @return
 */
WingedEdge::Vertex* getCommonVertex(WingedEdge::Edge* e1, WingedEdge::Edge* e2)
{
    if(e1->mVertOrigin == e2->mVertOrigin)
        return e1->mVertOrigin;
    else if(e1->mVertOrigin == e2->mVertDest)
        return e1->mVertOrigin;
    else if(e1->mVertDest == e2->mVertOrigin)
        return e1->mVertDest;
    else if(e1->mVertDest == e2->mVertDest)
        return e1->mVertDest;
    assert(false);
}

void Subdivision::loopSubdivision(WingedEdge::OBJMesh *tMesh, WingedEdge::WEMesh *sourceMesh, bool applyVertexRule, bool applyEdgeRule) {
    WingedEdge::WEMesh _targetMesh;
    WingedEdge::WEMesh* targetMesh = &_targetMesh;

    // Each edge gets partioned by a vertex
    int newVertexCount = sourceMesh->getVertexCount() + sourceMesh->getEdgeCount();
    // Each edge gets divided into two and each face gets 3 more edges
    int newEdgeCount = sourceMesh->getEdgeCount() * 2 + sourceMesh->getFaceCount() * 3;
    //Each triangle face gets divided into 4
    int newFaceCount = sourceMesh->getFaceCount() * 4;

    ExtendedFace extendedFaces[sourceMesh->getFaceCount()];
    ExtendedEdge extendedEdges[sourceMesh->getEdgeCount()];

    // Allocate resources to hold subdivided mesh
    targetMesh->allocateVertices(newVertexCount);
    targetMesh->allocateEdges(newEdgeCount);
    targetMesh->allocateFaces(newFaceCount);

    for(int i = 0; i < sourceMesh->getVertexCount(); i++) {
        if(applyVertexRule)
        {
            //Apply Vertex rule

            // Accumulate coordinates of neighbouring vertices
            float neighbourX = 0;
            float neighbourY = 0;
            float neighbourZ = 0;

            WingedEdge::Vertex* currentVertex = &(sourceMesh->getVertices()[i]);
            WingedEdge::Edge* startEdge = currentVertex->getEdge();
            // Walk around clockwise and count edges and sum up neighbours coordinates
            WingedEdge::Edge* currentEdge = nullptr;
            int adjacentEdgeCount = 0;
            if(startEdge->mVertOrigin == currentVertex){
                if(startEdge->mEdgeRightCW->mVertOrigin == currentVertex || startEdge->mEdgeRightCW->mVertDest == currentVertex){
                    currentEdge = (startEdge->mEdgeRightCW);
                    if(startEdge->mEdgeRightCW->mVertOrigin == currentVertex){
                        neighbourX += startEdge->mEdgeRightCW->mVertDest->getX();
                        neighbourY += startEdge->mEdgeRightCW->mVertDest->getY();
                        neighbourZ += startEdge->mEdgeRightCW->mVertDest->getZ();
                    }
                    else{
                        neighbourX += startEdge->mEdgeRightCW->mVertOrigin->getX();
                        neighbourY += startEdge->mEdgeRightCW->mVertOrigin->getY();
                        neighbourZ += startEdge->mEdgeRightCW->mVertOrigin->getZ();
                    }
                }
                else if(startEdge->mEdgeRightCCW->mVertOrigin == currentVertex || startEdge->mEdgeRightCCW->mVertDest == currentVertex){
                    currentEdge = (startEdge->mEdgeRightCCW);
                    if(startEdge->mEdgeRightCCW->mVertOrigin == currentVertex){
                        neighbourX += startEdge->mEdgeRightCCW->mVertDest->getX();
                        neighbourY += startEdge->mEdgeRightCCW->mVertDest->getY();
                        neighbourZ += startEdge->mEdgeRightCCW->mVertDest->getZ();
                    }
                    else{
                        neighbourX += startEdge->mEdgeRightCCW->mVertOrigin->getX();
                        neighbourY += startEdge->mEdgeRightCCW->mVertOrigin->getY();
                        neighbourZ += startEdge->mEdgeRightCCW->mVertOrigin->getZ();
                    }
                } else{
                    assert(false);
                }
            } else if(startEdge->mVertDest == currentVertex){
                if(startEdge->mEdgeLeftCW->mVertOrigin == currentVertex || startEdge->mEdgeLeftCW->mVertDest == currentVertex){
                    currentEdge = (startEdge->mEdgeLeftCW);
                    if(startEdge->mEdgeLeftCW->mVertOrigin == currentVertex){
                        neighbourX += startEdge->mEdgeLeftCW->mVertDest->getX();
                        neighbourY += startEdge->mEdgeLeftCW->mVertDest->getY();
                        neighbourZ += startEdge->mEdgeLeftCW->mVertDest->getZ();
                    }
                    else{
                        neighbourX += startEdge->mEdgeLeftCW->mVertOrigin->getX();
                        neighbourY += startEdge->mEdgeLeftCW->mVertOrigin->getY();
                        neighbourZ += startEdge->mEdgeLeftCW->mVertOrigin->getZ();
                    }
                }
                else if(startEdge->mEdgeLeftCCW->mVertOrigin == currentVertex || startEdge->mEdgeLeftCCW->mVertDest == currentVertex){
                    currentEdge = (startEdge->mEdgeLeftCCW);
                    if(startEdge->mEdgeLeftCCW->mVertOrigin == currentVertex){
                        neighbourX += startEdge->mEdgeLeftCCW->mVertDest->getX();
                        neighbourY += startEdge->mEdgeLeftCCW->mVertDest->getY();
                        neighbourZ += startEdge->mEdgeLeftCCW->mVertDest->getZ();
                    }
                    else{
                        neighbourX += startEdge->mEdgeLeftCCW->mVertOrigin->getX();
                        neighbourY += startEdge->mEdgeLeftCCW->mVertOrigin->getY();
                        neighbourZ += startEdge->mEdgeLeftCCW->mVertOrigin->getZ();
                    }
                } else{
                    assert(false);
                }
            }
            else{
                assert(false);
            }
            assert(currentEdge != startEdge);
            adjacentEdgeCount += 1;
            while (currentEdge != startEdge){ // Keep walking until we end up at the edge we started
                adjacentEdgeCount += 1;
                if(currentEdge->mVertOrigin == currentVertex){
                    if(currentEdge->mEdgeRightCW->mVertOrigin == currentVertex || currentEdge->mEdgeRightCW->mVertDest == currentVertex){
                        currentEdge = (currentEdge->mEdgeRightCW);
                        if(currentEdge->mEdgeRightCW->mVertOrigin == currentVertex){
                            neighbourX += currentEdge->mEdgeRightCW->mVertDest->getX();
                            neighbourY += currentEdge->mEdgeRightCW->mVertDest->getY();
                            neighbourZ += currentEdge->mEdgeRightCW->mVertDest->getZ();
                        }
                        else{
                            neighbourX += currentEdge->mEdgeRightCW->mVertOrigin->getX();
                            neighbourY += currentEdge->mEdgeRightCW->mVertOrigin->getY();
                            neighbourZ += currentEdge->mEdgeRightCW->mVertOrigin->getZ();
                        }
                    }
                    else if(currentEdge->mEdgeRightCCW->mVertOrigin == currentVertex || currentEdge->mEdgeRightCCW->mVertDest == currentVertex){
                        currentEdge = (currentEdge->mEdgeRightCCW);
                        if(currentEdge->mEdgeRightCCW->mVertOrigin == currentVertex){
                            neighbourX += currentEdge->mEdgeRightCCW->mVertDest->getX();
                            neighbourY += currentEdge->mEdgeRightCCW->mVertDest->getY();
                            neighbourZ += currentEdge->mEdgeRightCCW->mVertDest->getZ();
                        }
                        else{
                            neighbourX += currentEdge->mEdgeRightCCW->mVertOrigin->getX();
                            neighbourY += currentEdge->mEdgeRightCCW->mVertOrigin->getY();
                            neighbourZ += currentEdge->mEdgeRightCCW->mVertOrigin->getZ();
                        }
                    } else{
                        assert(false);
                    }
                } else if(currentEdge->mVertDest == currentVertex){
                    if(currentEdge->mEdgeLeftCW->mVertOrigin == currentVertex || currentEdge->mEdgeLeftCW->mVertDest == currentVertex){
                        currentEdge = (currentEdge->mEdgeLeftCW);
                        if(currentEdge->mEdgeLeftCW->mVertOrigin == currentVertex){
                            neighbourX += currentEdge->mEdgeLeftCW->mVertDest->getX();
                            neighbourY += currentEdge->mEdgeLeftCW->mVertDest->getY();
                            neighbourZ += currentEdge->mEdgeLeftCW->mVertDest->getZ();
                        }
                        else{
                            neighbourX += currentEdge->mEdgeLeftCW->mVertOrigin->getX();
                            neighbourY += currentEdge->mEdgeLeftCW->mVertOrigin->getY();
                            neighbourZ += currentEdge->mEdgeLeftCW->mVertOrigin->getZ();
                        }
                    }
                    else if(currentEdge->mEdgeLeftCCW->mVertOrigin == currentVertex || currentEdge->mEdgeLeftCCW->mVertDest == currentVertex){
                        currentEdge = (currentEdge->mEdgeLeftCCW);
                        if(currentEdge->mEdgeLeftCCW->mVertOrigin == currentVertex){
                            neighbourX += currentEdge->mEdgeLeftCCW->mVertDest->getX();
                            neighbourY += currentEdge->mEdgeLeftCCW->mVertDest->getY();
                            neighbourZ += currentEdge->mEdgeLeftCCW->mVertDest->getZ();
                        }
                        else{
                            neighbourX += currentEdge->mEdgeLeftCCW->mVertOrigin->getX();
                            neighbourY += currentEdge->mEdgeLeftCCW->mVertOrigin->getY();
                            neighbourZ += currentEdge->mEdgeLeftCCW->mVertOrigin->getZ();
                        }
                    } else{
                        assert(false);
                    }
                }
                else{
                    assert(false);
                }
            }
            std::cout << "Adjacent Edge Count " << adjacentEdgeCount << std::endl;
            std::cout << "Neighbour " << neighbourX << " "<< neighbourY << " " << neighbourZ << std::endl;
            float k = adjacentEdgeCount;

            // Beta calculation
            float beta = 1.0/k * (5.0/8.0 -((3.0/8.0+1.0/4.0*cos(2*M_PI/k))*(3.0/8.0+1.0/4.0*cos(2*M_PI/k))));
            //        beta = 0.05;
            float myX = sourceMesh->getVertices()[i].getX();
            float myY = sourceMesh->getVertices()[i].getY();
            float myZ = sourceMesh->getVertices()[i].getZ();

            float newX = neighbourX * beta + myX * (1-k*beta);
            float newY = neighbourY * beta + myY * (1-k*beta);
            float newZ = neighbourZ * beta + myZ * (1-k*beta);

            targetMesh->getVertices()[i].setPosition(newX, newY, newZ);
            //End of Vertex Rule
        }
        else{
            // Just copy forward vertices of vertex rule is not applied
            targetMesh->getVertices()[i].setPosition(sourceMesh->getVertices()[i].getPosition());
        }

    }
    int addedVertexCount = sourceMesh->getVertexCount();

    int addedEdgeCount = 0;
    int addedFaceCount = 0;

    for(int f = 0; f < sourceMesh->getFaceCount(); f++){
        // identify 3 edges of the face in CW order
        WingedEdge::Edge* e1 = sourceMesh->getFaces()[f].getEdge();
        WingedEdge::Edge* e2 = nullptr;
        WingedEdge::Edge* e3 = nullptr;
        if(&(sourceMesh->getFaces()[f]) == e1->mRightFace){
            e2 = e1->mEdgeRightCW;
            e3 = e1->mEdgeRightCCW;
        }
        else if(&(sourceMesh->getFaces()[f]) == e1->mLeftFace){
            e2 = e1->mEdgeLeftCW;
            e3 = e1->mEdgeLeftCCW;
        } else{
            assert(false);
        }
        // Create edge vertices
        // Identify three edges in CW orientation
        WingedEdge::Edge* edgesCW[] = {e1, e2, e3};
        for(int e = 0; e < 3; e++)
        {
            //            std::cout << e << std::endl;
            if(!(extendedEdges[edgesCW[e] - sourceMesh->getEdges()].added)){
                // The edge has not divided yet. Lets divide it and add it

                if(applyEdgeRule)
                {
                    // Apply Edge Rule
                    double newX = edgesCW[e]->mVertOrigin->getX() * 3.0/8.0 + edgesCW[e]->mVertDest->getX() * 3.0/8.0;
                    double newY = edgesCW[e]->mVertOrigin->getY() * 3.0/8.0 + edgesCW[e]->mVertDest->getY() * 3.0/8.0;
                    double newZ = edgesCW[e]->mVertOrigin->getZ() * 3.0/8.0 + edgesCW[e]->mVertDest->getZ() * 3.0/8.0;

                    if(edgesCW[e]->mEdgeRightCW->mVertOrigin == edgesCW[e]->mVertOrigin || edgesCW[e]->mEdgeRightCW->mVertOrigin == edgesCW[e]->mVertDest){
                        newX += edgesCW[e]->mEdgeRightCW->mVertDest->getX() * 1.0/8.0;
                        newY += edgesCW[e]->mEdgeRightCW->mVertDest->getY() * 1.0/8.0;
                        newZ += edgesCW[e]->mEdgeRightCW->mVertDest->getZ() * 1.0/8.0;
                    }
                    else if(edgesCW[e]->mEdgeRightCW->mVertDest == edgesCW[e]->mVertOrigin || edgesCW[e]->mEdgeRightCW->mVertDest == edgesCW[e]->mVertDest){
                        newX += edgesCW[e]->mEdgeRightCW->mVertOrigin->getX() * 1.0/8.0;
                        newY += edgesCW[e]->mEdgeRightCW->mVertOrigin->getY() * 1.0/8.0;
                        newZ += edgesCW[e]->mEdgeRightCW->mVertOrigin->getZ() * 1.0/8.0;
                    }
                    else{
                        assert(false);
                    }

                    if(edgesCW[e]->mEdgeLeftCW->mVertOrigin == edgesCW[e]->mVertOrigin || edgesCW[e]->mEdgeLeftCW->mVertOrigin == edgesCW[e]->mVertDest){
                        newX += edgesCW[e]->mEdgeLeftCW->mVertDest->getX() * 1.0/8.0;
                        newY += edgesCW[e]->mEdgeLeftCW->mVertDest->getY() * 1.0/8.0;
                        newZ += edgesCW[e]->mEdgeLeftCW->mVertDest->getZ() * 1.0/8.0;
                    }
                    else if(edgesCW[e]->mEdgeLeftCW->mVertDest == edgesCW[e]->mVertOrigin || edgesCW[e]->mEdgeLeftCW->mVertDest == edgesCW[e]->mVertDest){
                        newX += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getX() * 1.0/8.0;
                        newY += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getY() * 1.0/8.0;
                        newZ += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getZ() * 1.0/8.0;
                    }
                    else{
                        assert(false);
                    }

                    targetMesh->getVertices()[addedVertexCount].setPosition(newX, newY, newZ);
                    // End of Edge Rule
                }
                else{
                    // Just take average of two vertices of edge rule is not applied
                    targetMesh->getVertices()[addedVertexCount].setPosition(edgesCW[e]->mVertOrigin->getPosition() * 0.5 + edgesCW[e]->mVertDest->getPosition() * 0.5);
                }

                targetMesh->getEdges()[addedEdgeCount].mVertOrigin = &(targetMesh->getVertices()[edgesCW[e]->mVertOrigin - sourceMesh->getVertices()]);
                targetMesh->getEdges()[addedEdgeCount].mVertDest = &(targetMesh->getVertices()[addedVertexCount]);
                targetMesh->getVertices()[addedVertexCount].setEdge(&targetMesh->getEdges()[addedEdgeCount]);
                if(targetMesh->getEdges()[addedEdgeCount].mVertOrigin->getEdge() == nullptr){
                    targetMesh->getEdges()[addedEdgeCount].mVertOrigin->setEdge(&targetMesh->getEdges()[addedEdgeCount]);
                }

                targetMesh->getEdges()[addedEdgeCount+1].mVertOrigin = &(targetMesh->getVertices()[addedVertexCount]);
                targetMesh->getEdges()[addedEdgeCount+1].mVertDest = &(targetMesh->getVertices()[edgesCW[e]->mVertDest - sourceMesh->getVertices()]);
                if(targetMesh->getEdges()[addedEdgeCount+1].mVertDest->getEdge() == nullptr){
                    targetMesh->getEdges()[addedEdgeCount+1].mVertDest->setEdge(&targetMesh->getEdges()[addedEdgeCount+1]);
                }

                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].added = true;
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].originalEdge = edgesCW[e];
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS = &(targetMesh->getEdges()[addedEdgeCount]);
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE = &(targetMesh->getEdges()[addedEdgeCount+1]);

                addedVertexCount++;
                addedEdgeCount += 2;
                std::cout << "Divided edge [" << edgesCW[e]->mVertOrigin->getX() << " " << edgesCW[e]->mVertOrigin->getY()  << " " << edgesCW[e]->mVertOrigin->getZ() <<
                          "<< - >>" << edgesCW[e]->mVertDest->getX() << " " << edgesCW[e]->mVertDest->getY()  << " " << edgesCW[e]->mVertDest->getZ() <<
                          "] as [" << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getX() << ", " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getY() <<
                          ", " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getZ() << " << >> " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getX() << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getY() <<", " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getZ() << "] and [" <<
                          extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getX() << ", " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getY() <<
                          ", " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getZ() << " << >> " <<  extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getX() << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getY() <<", " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getZ()<< std::endl;
            }
        }

    }
    assert(addedVertexCount == newVertexCount);
    for(int _e = 0; _e < sourceMesh->getEdgeCount(); _e++){
        assert(extendedEdges[_e].added == true);
    }
    for(int _v = 0; _v < newVertexCount; _v++){
        assert(targetMesh->getVertices()[_v].getEdge() != nullptr);
    }
    std::cout << "Added vertices " << addedVertexCount << " Total should be " << newVertexCount << std::endl;

    //Now all the vertices are in place. Now lets subdivide faces, adding the internal edges.
    for(int f = 0; f < sourceMesh->getFaceCount(); f++) {
        // identify 3 edges of the face in CW order
        WingedEdge::Edge *e1 = sourceMesh->getFaces()[f].getEdge();
        WingedEdge::Edge *e2 = nullptr;
        WingedEdge::Edge *e3 = nullptr;
        if (&(sourceMesh->getFaces()[f]) == e1->mRightFace) {
            e2 = e1->mEdgeRightCW;
            e3 = e1->mEdgeRightCCW;
        } else if (&(sourceMesh->getFaces()[f]) == e1->mLeftFace) {
            e2 = e1->mEdgeLeftCW;
            e3 = e1->mEdgeLeftCCW;
        } else {
            assert(false);
        }
        WingedEdge::Edge *edgesCW[] = {e1, e2, e3};
        ExtendedFace *currentExtendedFace = &(extendedFaces[f]);
        currentExtendedFace->extendedEdge1 = &(extendedEdges[e1 - sourceMesh->getEdges()]);
        currentExtendedFace->extendedEdge2 = &(extendedEdges[e2 - sourceMesh->getEdges()]);
        currentExtendedFace->extendedEdge3 = &(extendedEdges[e3 - sourceMesh->getEdges()]);

        // Add 3 internal edges on the face of triangle
        //  e1_mid -> e2_mid
        targetMesh->getEdges()[addedEdgeCount].mVertOrigin = currentExtendedFace->extendedEdge1->newEdgeS->mVertDest;
        targetMesh->getEdges()[addedEdgeCount].mVertDest = currentExtendedFace->extendedEdge2->newEdgeS->mVertDest;
        currentExtendedFace->internalEdge12 = &(targetMesh->getEdges()[addedEdgeCount]);
        addedEdgeCount++;
        //  e2_mid -> e3_mid
        targetMesh->getEdges()[addedEdgeCount].mVertOrigin = currentExtendedFace->extendedEdge2->newEdgeS->mVertDest;
        targetMesh->getEdges()[addedEdgeCount].mVertDest = currentExtendedFace->extendedEdge3->newEdgeS->mVertDest;
        currentExtendedFace->internalEdge23 = &(targetMesh->getEdges()[addedEdgeCount]);
        addedEdgeCount++;
        //  e3_mid -> e1_mid
        targetMesh->getEdges()[addedEdgeCount].mVertOrigin = currentExtendedFace->extendedEdge3->newEdgeS->mVertDest;
        targetMesh->getEdges()[addedEdgeCount].mVertDest = currentExtendedFace->extendedEdge1->newEdgeS->mVertDest;
        currentExtendedFace->internalEdge31 = &(targetMesh->getEdges()[addedEdgeCount]);
        addedEdgeCount++;
        //        std::cout << "Edges added " << addedEdgeCount << " " << newEdgeCount << std::endl;
    }

    assert(addedEdgeCount == newEdgeCount);
    std::cout << "Edges added " << addedEdgeCount << "/" << newEdgeCount << std::endl;

    // Identify triangles in subdivided mesh, so we can generate face indices of a triangular mesh
    nanogui::MatrixXu faceIndices(3, newFaceCount);

    for(int f = 0; f < sourceMesh->getFaceCount(); f++){
        ExtendedFace *currentExtendedFace = &(extendedFaces[f]);
        ExtendedEdge* party1[3] = {currentExtendedFace->extendedEdge1, currentExtendedFace->extendedEdge2, currentExtendedFace->extendedEdge3};
        ExtendedEdge* party2[3] = {currentExtendedFace->extendedEdge2, currentExtendedFace->extendedEdge3, currentExtendedFace->extendedEdge1};
        WingedEdge::Edge* diagonalParty[3] = {currentExtendedFace->internalEdge12, currentExtendedFace->internalEdge23, currentExtendedFace->internalEdge31};
        //        Vertex* commonPoint = nullptr;
        WingedEdge::Edge* party1Part[3];
        WingedEdge::Edge* party2Part[3];

        for(int k = 0; k < 3; k++)
        {
            if(party1[k]->newEdgeS->mVertOrigin == party2[k]->newEdgeS->mVertOrigin){
                party1Part[k] = party1[k]->newEdgeS;
                party2Part[k] = party2[k]->newEdgeS;
            }
            else if(party1[k]->newEdgeS->mVertOrigin == party2[k]->newEdgeE->mVertDest){
                party1Part[k] = party1[k]->newEdgeS;
                party2Part[k] = party2[k]->newEdgeE;
            }
            else if(party1[k]->newEdgeE->mVertDest == party2[k]->newEdgeE->mVertDest){
                party1Part[k] = party1[k]->newEdgeE;
                party2Part[k] = party2[k]->newEdgeE;
            }
            else if(party1[k]->newEdgeE->mVertDest == party2[k]->newEdgeS->mVertOrigin){
                party1Part[k] = party1[k]->newEdgeE;
                party2Part[k] = party2[k]->newEdgeS;
            }
        }

        // Edges in CW order: party1: party2: diagonalParty
        for(int k = 0; k < 3;k++){
            WingedEdge::Vertex* c1 = getCommonVertex(party1Part[k], party2Part[k]);
            WingedEdge::Vertex* c2 = getCommonVertex(party2Part[k], diagonalParty[k]);
            WingedEdge::Vertex* c3 = getCommonVertex(diagonalParty[k], party1Part[k]);
            std::cout << "Face: " << c1 - targetMesh->getVertices() << ", " << c2 - targetMesh->getVertices() << ", " << c3-targetMesh->getVertices() << std::endl;
            faceIndices.col(addedFaceCount++) << c3- targetMesh->getVertices(), c2- targetMesh->getVertices(), c1- targetMesh->getVertices();
        }
        WingedEdge::Vertex* d1 = getCommonVertex(diagonalParty[0], diagonalParty[1]);
        WingedEdge::Vertex* d2 = getCommonVertex(diagonalParty[1], diagonalParty[2]);
        WingedEdge::Vertex* d3 = getCommonVertex(diagonalParty[2], diagonalParty[0]);

        faceIndices.col(addedFaceCount++) << d3- targetMesh->getVertices(), d2- targetMesh->getVertices(), d1- targetMesh->getVertices();
    }

    // Identify vertices of triangular mesh
    nanogui::MatrixXf positions(3,newVertexCount);
    for(int v = 0; v < newVertexCount; v++){
        positions.col(v) << targetMesh->getVertices()[v].getPosition();
    }

    // Assign vertices and face indices to triangular mesh
    tMesh->setMatrices(positions, faceIndices);

}

