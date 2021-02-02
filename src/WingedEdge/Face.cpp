//
// Created by madhawa on 2020-02-01.
//

#include <WingedEdge/Face.h>

Edge *Face::getEdge() {
    return mEdge;
}

void Face::setEdge(Edge *nEdge) {
    mEdge = nEdge;
}

Face::Face() {
    mEdge = nullptr;
}
