//
// Created by madhawa on 2020-02-01.
//

#ifndef NANOGUI_TEST_FACE_H
namespace WingedEdge {
#define NANOGUI_TEST_FACE_H

    class Edge;

    /**
     * WingedEdge Face definition according to the Wikipedia article https://en.wikipedia.org/wiki/Winged_edge
     */
    class Face {
    private:
        Edge* mEdge; //First edge of face
    public:
        Face();
        /**
         * Get an edge adjacent to face
         * @return
         */
        Edge* getEdge();

        /**
         * Set an edge adjacent to face
         * @param nEdge
         */
        void setEdge(Edge* nEdge);
    };
}

#include "Edge.h"


#endif //NANOGUI_TEST_FACE_H
