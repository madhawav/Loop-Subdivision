//
// Created by madhawa on 2020-02-09.
//

#ifndef OBJ_VIEW_SUBD_H
#define OBJ_VIEW_SUBD_H
#include <WingedEdge/WEMesh.h>
#include <WingedEdge/OBJMesh.h>

/**
 * Performs loop subdivision on sourceMesh and populate tMesh using the result. Source mesh is unchanged.
 * @param tMesh OBJ mesh filled with results of method
 * @param sourceMesh Source mesh to be tesselated. This mesh is unchanged.
 * @param applyEdgeRule if true, edge vertex position is calculated using Edge Geometric Rule of Loop Subdivision. If false, midpoint of edge is chosen.
 * @param applyVertexRule if true, Vertex Geometric rule is applied. Otherwise, vertices are simply copied forward.
 */
void loopSubd(WingedEdge::OBJMesh* tMesh, WingedEdge::WEMesh* sourceMesh, bool applyVertexRule, bool applyEdgeRule);
#endif //OBJ_VIEW_SUBD_H
