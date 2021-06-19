//
// Created by madhawa on 2020-02-09.
//

#include <iostream>
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/Edge.h>
#include <WingedEdge/OBJMesh.h>
#include <Subdivision/Subdivision.h>

#define _USE_MATH_DEFINES
#include <math.h>

// Uncomment to do extra logging
//#define LOG_EXTRA

/**
 * Each edge of source edge is represented by two edges in the resultant mesh of loop subdivision.
 * The extended edge data structure is used to maintain relationship between each edge in source mesh and two newly added
 * edges in target mesh
 */
class ExtendedEdge {
public:
    WingedEdge::Edge *originalEdge;
    WingedEdge::Edge *newEdgeS;
    WingedEdge::Edge *newEdgeE;
    bool added;

    ExtendedEdge() {
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
class ExtendedFace {
public:
    WingedEdge::Face *originalFace;
    ExtendedEdge *extendedEdge1;
    ExtendedEdge *extendedEdge2;
    ExtendedEdge *extendedEdge3;

    WingedEdge::Edge *internalEdge12; // From midpoint of external edge 1 to midpoint of external edge 2
    WingedEdge::Edge *internalEdge23;
    WingedEdge::Edge *internalEdge31;

    bool faceAdded;

    ExtendedFace() {
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
WingedEdge::Vertex *getCommonVertex(const WingedEdge::Edge *e1, const WingedEdge::Edge *e2) {
    if (e1->mVertOrigin == e2->mVertOrigin)
        return e1->mVertOrigin;
    else if (e1->mVertOrigin == e2->mVertDest)
        return e1->mVertOrigin;
    else if (e1->mVertDest == e2->mVertOrigin)
        return e1->mVertDest;
    else if (e1->mVertDest == e2->mVertDest)
        return e1->mVertDest;
    assert(false);
}

/**
 * Apply vertex rule of loop subdivision to the specified vertex
 * @param sourceMesh Mesh used to read neighbouring vertices
 * @param targetMesh Mesh that gets updated with the new vertex rule
 * @param vertexIndex Index of vertex to update
 */
void applyVertexRule(const WingedEdge::WEMesh *sourceMesh, WingedEdge::WEMesh *targetMesh,
                     int vertexIndex) {
    //Apply Vertex rule

    // Accumulate coordinates of neighbouring vertices
    float neighbourX = 0;
    float neighbourY = 0;
    float neighbourZ = 0;

    WingedEdge::Vertex *currentVertex = &(sourceMesh->getVertices()[vertexIndex]);
    WingedEdge::Edge *startEdge = currentVertex->getEdge();
    // Walk around clockwise and count edges and sum up neighbours coordinates
    WingedEdge::Edge *currentEdge = nullptr;
    int adjacentEdgeCount = 0;
    if (startEdge->mVertOrigin == currentVertex) {
        if (startEdge->mEdgeRightCW->mVertOrigin == currentVertex ||
            startEdge->mEdgeRightCW->mVertDest == currentVertex) {
            currentEdge = (startEdge->mEdgeRightCW);
            if (startEdge->mEdgeRightCW->mVertOrigin == currentVertex) {
                neighbourX += startEdge->mEdgeRightCW->mVertDest->getX();
                neighbourY += startEdge->mEdgeRightCW->mVertDest->getY();
                neighbourZ += startEdge->mEdgeRightCW->mVertDest->getZ();
            } else {
                neighbourX += startEdge->mEdgeRightCW->mVertOrigin->getX();
                neighbourY += startEdge->mEdgeRightCW->mVertOrigin->getY();
                neighbourZ += startEdge->mEdgeRightCW->mVertOrigin->getZ();
            }
        } else if (startEdge->mEdgeRightCCW->mVertOrigin == currentVertex ||
                   startEdge->mEdgeRightCCW->mVertDest == currentVertex) {
            currentEdge = (startEdge->mEdgeRightCCW);
            if (startEdge->mEdgeRightCCW->mVertOrigin == currentVertex) {
                neighbourX += startEdge->mEdgeRightCCW->mVertDest->getX();
                neighbourY += startEdge->mEdgeRightCCW->mVertDest->getY();
                neighbourZ += startEdge->mEdgeRightCCW->mVertDest->getZ();
            } else {
                neighbourX += startEdge->mEdgeRightCCW->mVertOrigin->getX();
                neighbourY += startEdge->mEdgeRightCCW->mVertOrigin->getY();
                neighbourZ += startEdge->mEdgeRightCCW->mVertOrigin->getZ();
            }
        } else {
            assert(false);
        }
    } else if (startEdge->mVertDest == currentVertex) {
        if (startEdge->mEdgeLeftCW->mVertOrigin == currentVertex ||
            startEdge->mEdgeLeftCW->mVertDest == currentVertex) {
            currentEdge = (startEdge->mEdgeLeftCW);
            if (startEdge->mEdgeLeftCW->mVertOrigin == currentVertex) {
                neighbourX += startEdge->mEdgeLeftCW->mVertDest->getX();
                neighbourY += startEdge->mEdgeLeftCW->mVertDest->getY();
                neighbourZ += startEdge->mEdgeLeftCW->mVertDest->getZ();
            } else {
                neighbourX += startEdge->mEdgeLeftCW->mVertOrigin->getX();
                neighbourY += startEdge->mEdgeLeftCW->mVertOrigin->getY();
                neighbourZ += startEdge->mEdgeLeftCW->mVertOrigin->getZ();
            }
        } else if (startEdge->mEdgeLeftCCW->mVertOrigin == currentVertex ||
                   startEdge->mEdgeLeftCCW->mVertDest == currentVertex) {
            currentEdge = (startEdge->mEdgeLeftCCW);
            if (startEdge->mEdgeLeftCCW->mVertOrigin == currentVertex) {
                neighbourX += startEdge->mEdgeLeftCCW->mVertDest->getX();
                neighbourY += startEdge->mEdgeLeftCCW->mVertDest->getY();
                neighbourZ += startEdge->mEdgeLeftCCW->mVertDest->getZ();
            } else {
                neighbourX += startEdge->mEdgeLeftCCW->mVertOrigin->getX();
                neighbourY += startEdge->mEdgeLeftCCW->mVertOrigin->getY();
                neighbourZ += startEdge->mEdgeLeftCCW->mVertOrigin->getZ();
            }
        } else {
            assert(false);
        }
    } else {
        assert(false);
    }
    assert(currentEdge != startEdge);
    adjacentEdgeCount += 1;
    while (currentEdge != startEdge) { // Keep walking until we end up at the edge we started
        adjacentEdgeCount += 1;
        if (currentEdge->mVertOrigin == currentVertex) {
            if (currentEdge->mEdgeRightCW->mVertOrigin == currentVertex ||
                currentEdge->mEdgeRightCW->mVertDest == currentVertex) {
                currentEdge = (currentEdge->mEdgeRightCW);
                if (currentEdge->mEdgeRightCW->mVertOrigin == currentVertex) {
                    neighbourX += currentEdge->mEdgeRightCW->mVertDest->getX();
                    neighbourY += currentEdge->mEdgeRightCW->mVertDest->getY();
                    neighbourZ += currentEdge->mEdgeRightCW->mVertDest->getZ();
                } else {
                    neighbourX += currentEdge->mEdgeRightCW->mVertOrigin->getX();
                    neighbourY += currentEdge->mEdgeRightCW->mVertOrigin->getY();
                    neighbourZ += currentEdge->mEdgeRightCW->mVertOrigin->getZ();
                }
            } else if (currentEdge->mEdgeRightCCW->mVertOrigin == currentVertex ||
                       currentEdge->mEdgeRightCCW->mVertDest == currentVertex) {
                currentEdge = (currentEdge->mEdgeRightCCW);
                if (currentEdge->mEdgeRightCCW->mVertOrigin == currentVertex) {
                    neighbourX += currentEdge->mEdgeRightCCW->mVertDest->getX();
                    neighbourY += currentEdge->mEdgeRightCCW->mVertDest->getY();
                    neighbourZ += currentEdge->mEdgeRightCCW->mVertDest->getZ();
                } else {
                    neighbourX += currentEdge->mEdgeRightCCW->mVertOrigin->getX();
                    neighbourY += currentEdge->mEdgeRightCCW->mVertOrigin->getY();
                    neighbourZ += currentEdge->mEdgeRightCCW->mVertOrigin->getZ();
                }
            } else {
                assert(false);
            }
        } else if (currentEdge->mVertDest == currentVertex) {
            if (currentEdge->mEdgeLeftCW->mVertOrigin == currentVertex ||
                currentEdge->mEdgeLeftCW->mVertDest == currentVertex) {
                currentEdge = (currentEdge->mEdgeLeftCW);
                if (currentEdge->mEdgeLeftCW->mVertOrigin == currentVertex) {
                    neighbourX += currentEdge->mEdgeLeftCW->mVertDest->getX();
                    neighbourY += currentEdge->mEdgeLeftCW->mVertDest->getY();
                    neighbourZ += currentEdge->mEdgeLeftCW->mVertDest->getZ();
                } else {
                    neighbourX += currentEdge->mEdgeLeftCW->mVertOrigin->getX();
                    neighbourY += currentEdge->mEdgeLeftCW->mVertOrigin->getY();
                    neighbourZ += currentEdge->mEdgeLeftCW->mVertOrigin->getZ();
                }
            } else if (currentEdge->mEdgeLeftCCW->mVertOrigin == currentVertex ||
                       currentEdge->mEdgeLeftCCW->mVertDest == currentVertex) {
                currentEdge = (currentEdge->mEdgeLeftCCW);
                if (currentEdge->mEdgeLeftCCW->mVertOrigin == currentVertex) {
                    neighbourX += currentEdge->mEdgeLeftCCW->mVertDest->getX();
                    neighbourY += currentEdge->mEdgeLeftCCW->mVertDest->getY();
                    neighbourZ += currentEdge->mEdgeLeftCCW->mVertDest->getZ();
                } else {
                    neighbourX += currentEdge->mEdgeLeftCCW->mVertOrigin->getX();
                    neighbourY += currentEdge->mEdgeLeftCCW->mVertOrigin->getY();
                    neighbourZ += currentEdge->mEdgeLeftCCW->mVertOrigin->getZ();
                }
            } else {
                assert(false);
            }
        } else {
            assert(false);
        }
    }
#ifdef LOG_EXTRA
    std::cout << "Adjacent Edge Count " << adjacentEdgeCount << std::endl;
    std::cout << "Neighbour " << neighbourX << " " << neighbourY << " " << neighbourZ << std::endl;
#endif
    float k = adjacentEdgeCount;

    // Beta calculation
    float beta = 1.0 / k * (5.0 / 8.0 - ((3.0 / 8.0 + 1.0 / 4.0 * cos(2 * M_PI / k)) *
                                         (3.0 / 8.0 + 1.0 / 4.0 * cos(2 * M_PI / k))));
    //        beta = 0.05;
    float myX = sourceMesh->getVertices()[vertexIndex].getX();
    float myY = sourceMesh->getVertices()[vertexIndex].getY();
    float myZ = sourceMesh->getVertices()[vertexIndex].getZ();

    float newX = neighbourX * beta + myX * (1 - k * beta);
    float newY = neighbourY * beta + myY * (1 - k * beta);
    float newZ = neighbourZ * beta + myZ * (1 - k * beta);

    targetMesh->getVertices()[vertexIndex].setPosition(newX, newY, newZ);
    //End of Vertex Rule
}

/**
 * Apply edge rule of loop subdivision to a specified edge.
 * @param targetMesh Mesh being updated.
 * @param addedVertexCount Number of vertices added to the target mesh so far.
 * @param edgesCW Edges in clockwise order.
 * @param e Index of specified edge in edgesCW
 */
void applyEdgeRule(const WingedEdge::WEMesh *targetMesh, int addedVertexCount, WingedEdge::Edge *const *edgesCW,
                   int e) {// Apply Edge Rule
    double newX = edgesCW[e]->mVertOrigin->getX() * 3.0 / 8.0 + edgesCW[e]->mVertDest->getX() * 3.0 / 8.0;
    double newY = edgesCW[e]->mVertOrigin->getY() * 3.0 / 8.0 + edgesCW[e]->mVertDest->getY() * 3.0 / 8.0;
    double newZ = edgesCW[e]->mVertOrigin->getZ() * 3.0 / 8.0 + edgesCW[e]->mVertDest->getZ() * 3.0 / 8.0;

    if (edgesCW[e]->mEdgeRightCW->mVertOrigin == edgesCW[e]->mVertOrigin ||
        edgesCW[e]->mEdgeRightCW->mVertOrigin == edgesCW[e]->mVertDest) {
        newX += edgesCW[e]->mEdgeRightCW->mVertDest->getX() * 1.0 / 8.0;
        newY += edgesCW[e]->mEdgeRightCW->mVertDest->getY() * 1.0 / 8.0;
        newZ += edgesCW[e]->mEdgeRightCW->mVertDest->getZ() * 1.0 / 8.0;
    } else if (edgesCW[e]->mEdgeRightCW->mVertDest == edgesCW[e]->mVertOrigin ||
               edgesCW[e]->mEdgeRightCW->mVertDest == edgesCW[e]->mVertDest) {
        newX += edgesCW[e]->mEdgeRightCW->mVertOrigin->getX() * 1.0 / 8.0;
        newY += edgesCW[e]->mEdgeRightCW->mVertOrigin->getY() * 1.0 / 8.0;
        newZ += edgesCW[e]->mEdgeRightCW->mVertOrigin->getZ() * 1.0 / 8.0;
    } else {
        assert(false);
    }

    if (edgesCW[e]->mEdgeLeftCW->mVertOrigin == edgesCW[e]->mVertOrigin ||
        edgesCW[e]->mEdgeLeftCW->mVertOrigin == edgesCW[e]->mVertDest) {
        newX += edgesCW[e]->mEdgeLeftCW->mVertDest->getX() * 1.0 / 8.0;
        newY += edgesCW[e]->mEdgeLeftCW->mVertDest->getY() * 1.0 / 8.0;
        newZ += edgesCW[e]->mEdgeLeftCW->mVertDest->getZ() * 1.0 / 8.0;
    } else if (edgesCW[e]->mEdgeLeftCW->mVertDest == edgesCW[e]->mVertOrigin ||
               edgesCW[e]->mEdgeLeftCW->mVertDest == edgesCW[e]->mVertDest) {
        newX += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getX() * 1.0 / 8.0;
        newY += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getY() * 1.0 / 8.0;
        newZ += edgesCW[e]->mEdgeLeftCW->mVertOrigin->getZ() * 1.0 / 8.0;
    } else {
        assert(false);
    }

    targetMesh->getVertices()[addedVertexCount].setPosition(newX, newY, newZ);
    // End of Edge Rule
}

/**
 * Update vertices copied from the source mesh
 * @param sourceMesh Source mesh
 * @param useVertexRule Specify true to apply vertex rule.
 * @param targetMesh Target mesh the method updates.
 */
void updateVertices(const WingedEdge::WEMesh *sourceMesh, bool useVertexRule,
                    WingedEdge::WEMesh *targetMesh) {// Apply vertex rule
    for (int i = 0; i < sourceMesh->getVertexCount(); i++) {
        if (useVertexRule) {
            applyVertexRule(sourceMesh, targetMesh, i);
        } else {
            // Just copy forward vertices of vertex rule is not applied
            targetMesh->getVertices()[i].setPosition(sourceMesh->getVertices()[i].getPosition());
        }
    }
}

/**
 * Subdivide edges copied from the source mesh
 * @param sourceMesh Source mesh
 * @param useEdgeRule Specify true to apply edge rule
 * @param targetMesh Target mesh being updated
 * @param extendedEdges The array of extended edges is populated with the relationship between an original edge and a new edge pair.
 * @param addedVertexCount The vertex count is updated.
 * @param addedEdgeCount The edge count is updated.
 */
void subdivideEdges(const WingedEdge::WEMesh *sourceMesh, bool useEdgeRule,
                    WingedEdge::WEMesh *targetMesh, ExtendedEdge *extendedEdges,
                    int &addedVertexCount,
                    int &addedEdgeCount) {
    for (int f = 0; f < sourceMesh->getFaceCount(); f++) {
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
        // Create edge vertices
        // Identify three edges in CW orientation
        WingedEdge::Edge *edgesCW[] = {e1, e2, e3};
        for (int e = 0; e < 3; e++) {
            if (!(extendedEdges[edgesCW[e] - sourceMesh->getEdges()].added)) {
                // The edge has not divided yet. Lets divide it and add it

                if (useEdgeRule) {
                    applyEdgeRule(targetMesh, addedVertexCount, edgesCW, e);
                } else {
                    // Just take average of two vertices of edge rule is not applied
                    targetMesh->getVertices()[addedVertexCount].setPosition(
                            edgesCW[e]->mVertOrigin->getPosition() * 0.5 + edgesCW[e]->mVertDest->getPosition() * 0.5);
                }

                targetMesh->getEdges()[addedEdgeCount].mVertOrigin = &(targetMesh->getVertices()[
                        edgesCW[e]->mVertOrigin - sourceMesh->getVertices()]);
                targetMesh->getEdges()[addedEdgeCount].mVertDest = &(targetMesh->getVertices()[addedVertexCount]);
                targetMesh->getVertices()[addedVertexCount].setEdge(&targetMesh->getEdges()[addedEdgeCount]);
                if (targetMesh->getEdges()[addedEdgeCount].mVertOrigin->getEdge() == nullptr) {
                    targetMesh->getEdges()[addedEdgeCount].mVertOrigin->setEdge(
                            &targetMesh->getEdges()[addedEdgeCount]);
                }

                targetMesh->getEdges()[addedEdgeCount + 1].mVertOrigin = &(targetMesh->getVertices()[addedVertexCount]);
                targetMesh->getEdges()[addedEdgeCount + 1].mVertDest = &(targetMesh->getVertices()[
                        edgesCW[e]->mVertDest - sourceMesh->getVertices()]);
                if (targetMesh->getEdges()[addedEdgeCount + 1].mVertDest->getEdge() == nullptr) {
                    targetMesh->getEdges()[addedEdgeCount + 1].mVertDest->setEdge(
                            &targetMesh->getEdges()[addedEdgeCount + 1]);
                }

                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].added = true;
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].originalEdge = edgesCW[e];
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS = &(targetMesh->getEdges()[addedEdgeCount]);
                extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE = &(targetMesh->getEdges()[addedEdgeCount +
                                                                                                       1]);

                addedVertexCount++;
                addedEdgeCount += 2;
#ifdef LOG_EXTRA
                std::cout << "Divided edge [" << edgesCW[e]->mVertOrigin->getX() << " "
                          << edgesCW[e]->mVertOrigin->getY() << " " << edgesCW[e]->mVertOrigin->getZ() <<
                          "<< - >>" << edgesCW[e]->mVertDest->getX() << " " << edgesCW[e]->mVertDest->getY() << " "
                          << edgesCW[e]->mVertDest->getZ() <<
                          "] as [" << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getX()
                          << ", " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getY() <<
                          ", " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertOrigin->getZ()
                          << " << >> " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getX()
                          << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getY() << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeS->mVertDest->getZ() << "] and ["
                          <<
                          extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getX() << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getY() <<
                          ", " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertOrigin->getZ()
                          << " << >> " << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getX()
                          << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getY() << ", "
                          << extendedEdges[edgesCW[e] - sourceMesh->getEdges()].newEdgeE->mVertDest->getZ()
                          << std::endl;
#endif
            }
        }

    }
    for (int _e = 0; _e < sourceMesh->getEdgeCount(); _e++) {
        assert(extendedEdges[_e].added == true);
    }
    for (int _v = 0; _v < addedVertexCount; _v++) {
        assert(targetMesh->getVertices()[_v].getEdge() != nullptr);
    }

}

/**
 * Subdivide faces, adding the internal edges.
 * @param sourceMesh Source mesh
 * @param targetMesh Target mesh being updated
 * @param extendedFaces The array of extended faces is populated with the relationship between an original face and the 4 triangles in the new subdivided face.
 * @param extendedEdges The array of extended edges is populated with the relationship between an original edge and a new edge pair.
 * @param addedEdgeCount Added edges count is updated.
 */
void subdivideFaces(const WingedEdge::WEMesh *sourceMesh, WingedEdge::WEMesh *targetMesh,
                    ExtendedFace *extendedFaces, ExtendedEdge *extendedEdges,
                    int &addedEdgeCount) {
    //Now all the vertices are in place. Now lets subdivide faces, adding the internal edges.
    for (int f = 0; f < sourceMesh->getFaceCount(); f++) {
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
#ifdef LOG_EXTRA
        std::cout << "Edges added " << addedEdgeCount << std::endl;
#endif
    }
}

/**
 * Compute 3xFaceCount matrix identifying vertex triplets in each face of target mesh.
 * @param originalMesh Original mesh fed into the subdivision process
 * @param subdividedMesh Subdivided mesh we generate with updated vertices and edges
 * @param expectedFaceCount Count of faces expected in the final subdivided mesh
 * @param extendedFaces Array of extended faces linking each original face to 4 subdivided faces
 * @return 3 x expectedFaceCount array containing vertex indices of each face.
 */
nanogui::MatrixXu computeFaceIndices(const WingedEdge::WEMesh *originalMesh, const WingedEdge::WEMesh *subdividedMesh,
                                     int expectedFaceCount, const ExtendedFace *extendedFaces) {
    // Identify triangles in subdivided mesh, so we can generate face indices of a triangular mesh
    int addedFaceCount = 0;
    nanogui::MatrixXu faceIndices(3, expectedFaceCount);

    for (int f = 0; f < originalMesh->getFaceCount(); f++) {
        const ExtendedFace *currentExtendedFace = &(extendedFaces[f]);
        ExtendedEdge *party1[3] = {currentExtendedFace->extendedEdge1, currentExtendedFace->extendedEdge2,
                                   currentExtendedFace->extendedEdge3};
        ExtendedEdge *party2[3] = {currentExtendedFace->extendedEdge2, currentExtendedFace->extendedEdge3,
                                   currentExtendedFace->extendedEdge1};
        WingedEdge::Edge *diagonalParty[3] = {currentExtendedFace->internalEdge12, currentExtendedFace->internalEdge23,
                                              currentExtendedFace->internalEdge31};
        //        Vertex* commonPoint = nullptr;
        WingedEdge::Edge *party1Part[3];
        WingedEdge::Edge *party2Part[3];

        for (int k = 0; k < 3; k++) {
            if (party1[k]->newEdgeS->mVertOrigin == party2[k]->newEdgeS->mVertOrigin) {
                party1Part[k] = party1[k]->newEdgeS;
                party2Part[k] = party2[k]->newEdgeS;
            } else if (party1[k]->newEdgeS->mVertOrigin == party2[k]->newEdgeE->mVertDest) {
                party1Part[k] = party1[k]->newEdgeS;
                party2Part[k] = party2[k]->newEdgeE;
            } else if (party1[k]->newEdgeE->mVertDest == party2[k]->newEdgeE->mVertDest) {
                party1Part[k] = party1[k]->newEdgeE;
                party2Part[k] = party2[k]->newEdgeE;
            } else if (party1[k]->newEdgeE->mVertDest == party2[k]->newEdgeS->mVertOrigin) {
                party1Part[k] = party1[k]->newEdgeE;
                party2Part[k] = party2[k]->newEdgeS;
            }
        }

        // Edges in CW order: party1: party2: diagonalParty
        for (int k = 0; k < 3; k++) {
            WingedEdge::Vertex *c1 = getCommonVertex(party1Part[k], party2Part[k]);
            WingedEdge::Vertex *c2 = getCommonVertex(party2Part[k], diagonalParty[k]);
            WingedEdge::Vertex *c3 = getCommonVertex(diagonalParty[k], party1Part[k]);
#ifdef LOG_EXTRA
            std::cout << "Face: " << c1 - subdividedMesh->getVertices() << ", " << c2 - subdividedMesh->getVertices()
                      << ", " << c3 - subdividedMesh->getVertices() << std::endl;
#endif
            faceIndices.col(addedFaceCount++) << c3 - subdividedMesh->getVertices(), c2 - subdividedMesh->getVertices(),
                    c1 -
                    subdividedMesh->getVertices();
        }
        WingedEdge::Vertex *d1 = getCommonVertex(diagonalParty[0], diagonalParty[1]);
        WingedEdge::Vertex *d2 = getCommonVertex(diagonalParty[1], diagonalParty[2]);
        WingedEdge::Vertex *d3 = getCommonVertex(diagonalParty[2], diagonalParty[0]);

        faceIndices.col(addedFaceCount++) << d3 - subdividedMesh->getVertices(), d2 - subdividedMesh->getVertices(),
                d1 -
                subdividedMesh->getVertices();
    }
    return faceIndices;
}

/**
 * Extract vertices from the subdivided mesh
 * @param subdividedMesh Subdivided mesh
 * @return
 */
nanogui::MatrixXf computeVertexIndices(const WingedEdge::WEMesh *subdividedMesh) {
    // Identify vertices of triangular mesh
    nanogui::MatrixXf positions(3, subdividedMesh->getVertexCount());
    for (int v = 0; v < subdividedMesh->getVertexCount(); v++) {
        positions.col(v) << subdividedMesh->getVertices()[v].getPosition();
    }
    return positions;
}

/**
 * Performs loop subdivision on sourceMesh and populate tMesh using the result. Source mesh is unchanged.
 * @param tMesh OBJ mesh filled with results of method
 * @param sourceMesh Source mesh to be tesselated. This mesh is unchanged.
 * @param applyEdgeRule if true, edge vertex position is calculated using Edge Geometric Rule of Loop Subdivision. If false, midpoint of edge is chosen.
 * @param applyVertexRule if true, Vertex Geometric rule is applied. Otherwise, vertices are simply copied forward.
 */
void Subdivision::loopSubdivision(WingedEdge::OBJMesh *tMesh, const WingedEdge::WEMesh *sourceMesh, bool useVertexRule,
                                  bool useEdgeRule) {
    WingedEdge::WEMesh _targetMesh;
    WingedEdge::WEMesh *targetMesh = &_targetMesh;

    // Each edge gets partitioned by a vertex
    int newVertexCount = sourceMesh->getVertexCount() + sourceMesh->getEdgeCount();
    // Each edge gets divided into two and each face gets 3 more edges
    int newEdgeCount = sourceMesh->getEdgeCount() * 2 + sourceMesh->getFaceCount() * 3;
    //Each triangle face gets divided into 4
    int newFaceCount = sourceMesh->getFaceCount() * 4;

    ExtendedFace* extendedFaces = new ExtendedFace[sourceMesh->getFaceCount()];
    ExtendedEdge* extendedEdges = new ExtendedEdge[sourceMesh->getEdgeCount()];

    // Allocate resources to hold subdivided mesh
    targetMesh->allocateVertices(newVertexCount);
    targetMesh->allocateEdges(newEdgeCount);
    targetMesh->allocateFaces(newFaceCount);

    // Copy forward and update vertices using the vertex rule
    updateVertices(sourceMesh, useVertexRule, targetMesh);

    int addedVertexCount = sourceMesh->getVertexCount();
    int addedEdgeCount = 0;

    // Subdivide edges
    subdivideEdges(sourceMesh, useEdgeRule, targetMesh, extendedEdges, addedVertexCount, addedEdgeCount);
    assert(addedVertexCount == newVertexCount);
#ifdef LOG_EXTRA
    std::cout << "Added vertices " << addedVertexCount << " Total should be " << newVertexCount << std::endl;
#endif

    // Subdivide faces
    subdivideFaces(sourceMesh, targetMesh, extendedFaces, extendedEdges, addedEdgeCount);
    assert(addedEdgeCount == newEdgeCount);
#ifdef LOG_EXTRA
    std::cout << "Edges added " << addedEdgeCount << "/" << newEdgeCount << std::endl;
#endif

    // Create the subdivided OBJ mesh
    auto faceIndices = computeFaceIndices(sourceMesh, targetMesh, newFaceCount, extendedFaces);
    auto positions = computeVertexIndices(targetMesh);

    // Assign vertices and face indices to triangular mesh
    tMesh->setMatrices(positions, faceIndices);

	delete[] extendedFaces;
	delete[] extendedEdges;
}






