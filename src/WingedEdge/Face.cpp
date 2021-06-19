//
// Created by madhawa on 2020-02-01.
//

#include <WingedEdge/Face.h>
#include <WingedEdge/Edge.h>

WingedEdge::Edge * WingedEdge::Face::getEdge() const {
    return mEdge;
}

void WingedEdge::Face::setEdge(WingedEdge::Edge *nEdge) {
    mEdge = nEdge;
}

WingedEdge::Face::Face() {
    mEdge = nullptr;
}
