//
// Created by madhawa on 2020-02-01.
//

#ifndef NANOGUI_TEST_VERTEX_H
#define NANOGUI_TEST_VERTEX_H

#include <nanogui/common.h>

class Edge;

/**
 * WingedEdge Vertex definition according to the Wikipedia article https://en.wikipedia.org/wiki/Winged_edge
 */
class Vertex {
private:
    nanogui::Vector3f mPosition;
    Edge* mEdge;

public:
    /**
     * Construct a vertex with specified coordinates
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    Vertex(float x, float y, float z);

    /**
     * Construct a vertex with coordinates (0,0,0)
     */
    Vertex();

    /**
     * Returns X coordinate
     * @return
     */
    float getX();

    /**
     * Returns Y coordinate
     * @return
     */
    float getY();

    /**
     * Returns Z coordinate
     * @return
     */
    float getZ();

    /**
     * Sets X coordinate
     * @param x
     */
    void setX(float x);

    /**
     * Sets Y coordinate
     * @param y
     */
    void setY(float y);

    /**
     * Sets Z coordinate
     * @param z
     */
    void setZ(float z);

    /**
     * Retrieve position
     * @return
     */
    nanogui::Vector3f getPosition();

    /**
     * Set position
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     */
    void setPosition(float x,float y, float z);

    void setPosition(nanogui::Vector3f pos);

    /**
     * Retrieve an adjacent edge
     * @return
     */
    Edge* getEdge();

    /**
     * Set an adjacent edge
     * @param nEdge
     */
    void setEdge(Edge* nEdge);
};


#endif //NANOGUI_TEST_VERTEX_H
