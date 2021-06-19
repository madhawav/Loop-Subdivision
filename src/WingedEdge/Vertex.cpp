//
// Created by madhawa on 2020-02-01.
//

#include <WingedEdge/Vertex.h>
#include <WingedEdge/Edge.h>

void WingedEdge::Vertex::setEdge(WingedEdge::Edge *nEdge) {
    mEdge = nEdge;
}

WingedEdge::Edge *WingedEdge::Vertex::getEdge() {
    return mEdge;
}

WingedEdge::Vertex::Vertex() {
    mEdge = nullptr;
    mPosition = nanogui::Vector3f(0,0,0);
}

float WingedEdge::Vertex::getX() {
    return mPosition.x();
}

float WingedEdge::Vertex::getY() {
    return mPosition.y();
}

float WingedEdge::Vertex::getZ() {
    return mPosition.z();
}

void WingedEdge::Vertex::setPosition(float x, float y, float z) {
    mPosition = nanogui::Vector3f(x,y,z);
}

void WingedEdge::Vertex::setX(float x) {
    mPosition[0] = x;
}

void WingedEdge::Vertex::setY(float y) {
    mPosition[1] = y;
}

void WingedEdge::Vertex::setZ(float z) {
    mPosition[2] = z;
}

WingedEdge::Vertex::Vertex(float x, float y, float z) : WingedEdge::Vertex() {
    setPosition(x,y,z);
}

nanogui::Vector3f WingedEdge::Vertex::getPosition() {
    return mPosition;
}

void WingedEdge::Vertex::setPosition(nanogui::Vector3f pos) {
    this->mPosition = pos;
}


