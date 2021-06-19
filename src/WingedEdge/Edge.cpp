//
// Created by madhawa on 2020-02-01.
//

#include <WingedEdge/Edge.h>


WingedEdge::Edge::Edge() {
    mVertDest = nullptr;
    mVertOrigin = nullptr;
    mEdgeLeftCCW = nullptr;
    mEdgeLeftCW = nullptr;
    mEdgeRightCCW = nullptr;
    mEdgeRightCW = nullptr;
    mLeftFace = nullptr;
    mRightFace = nullptr;
}

WingedEdge::Edge::Edge(Vertex *origin, Vertex *destination) : Edge(){
    mVertOrigin = origin;
    mVertDest = destination;
}
