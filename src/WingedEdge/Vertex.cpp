//
// Created by madhawa on 2020-02-01.
//

#include <WingedEdge/Vertex.h>

void Vertex::setEdge(Edge *nEdge) {
    mEdge = nEdge;
}

Edge *Vertex::getEdge() {
    return mEdge;
}

Vertex::Vertex() {
    mEdge = nullptr;
    mPosition = nanogui::Vector3f(0,0,0);
}

float Vertex::getX() {
    return mPosition.x();
}

float Vertex::getY() {
    return mPosition.y();
}

float Vertex::getZ() {
    return mPosition.z();
}

void Vertex::setPosition(float x, float y, float z) {
    mPosition = nanogui::Vector3f(x,y,z);
}

void Vertex::setX(float x) {
    mPosition[0] = x;
}

void Vertex::setY(float y) {
    mPosition[1] = y;
}

void Vertex::setZ(float z) {
    mPosition[2] = z;
}

Vertex::Vertex(float x, float y, float z) : Vertex() {
    setPosition(x,y,z);
}

nanogui::Vector3f Vertex::getPosition() {
    return mPosition;
}

void Vertex::setPosition(nanogui::Vector3f pos) {
    this->mPosition = pos;
}


