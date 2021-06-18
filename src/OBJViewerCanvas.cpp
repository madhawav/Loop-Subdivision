//
// Created by madhawa on 2021-06-18.
//

#include "OBJViewerCanvas.h"
#include "WingedEdge/OBJMesh.h"
#include "Subdivision/subd.h"

#include <nanogui/common.h>
#include <nanogui/glcanvas.h>
#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <iostream>

#define WIREFRAME_OVERLAY_EPS 0.01
#define WIREFRAME_OVERLAY_EPS_WIDTH_FACTOR 0.005f
#define MIN_ZOOM 0.1

#define SHADER_NAME "a_smooth_shader"
#define VERTEX_SHADER_FILE_NAME "StandardShading.vertexshader"
#define FRAGMENT_SHADER_FILE_NAME "StandardShading.fragmentshader"
#define SHADER_ATTR_VERTEX_POSITION "vertexPosition_modelspace"
#define SHADER_ATTR_COLOR "color"
#define SHADER_ATTR_VERTEX_NORMAL "vertexNormal_modelspace"
#define SHADER_ATTR_LIGHT_POS "LightPosition_worldspace"
#define SHADER_ATTR_MATRIX_VIEW "V"
#define SHADER_ATTR_MATRIX_MODEL "M"
#define SHADER_ATTR_MATRIX_MVP "MVP"
#define SHADER_ATTR_DIFFUSE_CORRECTION "diffuseCorrection"


OBJViewer::OBJViewerCanvas::OBJViewerCanvas(nanogui::Widget *parent) : nanogui::GLCanvas(parent) {
    mRotation = Eigen::Vector3f(0, 0, 0); // Camera rotation
    mTarget = Eigen::Vector3f(0, 0, 0); // Camera look at position
    mZoom = 1; // Zoom level

    isSubdivided = false;
    drawWireframe = true; // Should the mesh be drawn as wireframe?
    drawSolid = true; // Should the the mesh be drawn shaded/filled?
    flatShading = false;  // Is flat shading being used?

    M.setIdentity();
    V.setIdentity();
    P.setIdentity();
    MVP.setIdentity();

    // Load shader
    mShader.initFromFiles(SHADER_NAME, VERTEX_SHADER_FILE_NAME, FRAGMENT_SHADER_FILE_NAME);

    // Setup a cube
    OBJMesh objMesh;
    objMesh.setCube();
    loadObjMesh(&objMesh);
}

bool OBJViewer::OBJViewerCanvas::saveObj(const string &path) const {
    if (!weMesh.isModelLoaded())
        return false;

    OBJMesh objMesh;
    weMesh.fillOBJMesh(&objMesh);
    std::ofstream outFile;
    outFile.open(path);
    if (!outFile) {
        std::cerr << "Unable to open file";
        return false;
    }
    return objMesh.saveFile(&outFile);
}

bool OBJViewer::OBJViewerCanvas::loadObj(const string &path) {
    OBJMesh objMesh;
    std::ifstream inFile;
    inFile.open(path);
    if (!inFile) {
        std::cerr << "Unable to open file";
        return false;
    }

    objMesh.parseFile(&inFile);
    inFile.close();
    
    isSubdivided = false;
    return loadObjMesh(&objMesh);
}

void OBJViewer::OBJViewerCanvas::performLoopSubdivision() {
    if (!isSubdivided) {
        weMesh.fillOBJMesh(&originalMesh);
    }
    OBJMesh subResult;
    loopSubd(&subResult, &weMesh, true, true);
    loadObjMesh(&subResult);
    isSubdivided = true;
}

void OBJViewer::OBJViewerCanvas::performLoopTessellation() {
    if (!isSubdivided) {
        weMesh.fillOBJMesh(&originalMesh);
    }
    OBJMesh subResult;
    loopSubd(&subResult, &weMesh, false, false);
    loadObjMesh(&subResult);
    isSubdivided = true;
}

void OBJViewer::OBJViewerCanvas::resetToOriginalMesh() {
    if (!isSubdivided)
        return;
    loadObjMesh(&originalMesh);
    originalMesh.setCube(); // Clears up memory
    isSubdivided = false;
}

bool OBJViewer::OBJViewerCanvas::loadObjMesh(OBJMesh *objMesh) {
    // Convert to WingedEdge
    if (!weMesh.loadModel(objMesh->getVertices(), objMesh->getFaces()))
        return false;

    // Populate matrices used for shading
    if (!(flatShading ? weMesh.populateFlatShadingMatrices() : weMesh.populateSmoothShadingMatrices()))
        return false;

    // halfWidth is used to determine project matrix so that model is visible
    halfWidth = weMesh.getMaxPoint()[0];
    for (int i = 0; i < 3; i++) {
        if (weMesh.getMaxPoint()[i] > halfWidth)
            halfWidth = weMesh.getMaxPoint()[i];
        if (-weMesh.getMinPoint()[i] > halfWidth)
            halfWidth = -weMesh.getMinPoint()[i];
    }

    // Epsilon used to offset wireframe rendered on top of mesh
    wireframeOverlayEpsilon = WIREFRAME_OVERLAY_EPS;
    if (halfWidth * WIREFRAME_OVERLAY_EPS_WIDTH_FACTOR < wireframeOverlayEpsilon)
        wireframeOverlayEpsilon = halfWidth * WIREFRAME_OVERLAY_EPS_WIDTH_FACTOR;

    if (drawWireframe && drawSolid)
        weMesh.populateExpandedVertexMatrix(wireframeOverlayEpsilon);

    // Reset camera view
    mTarget = Eigen::Vector3f(0, 0, 0);
    mZoom = 1;
    mRotation = Eigen::Vector3f(0, 0, 0);

    // Push new matrices to renderer
    refreshGeometry();
    return true;
}

void OBJViewer::OBJViewerCanvas::refreshGeometry() {
    // Fetch matrices from WingedEdge mesh
    positions = weMesh.getVertexMatrix();
    indices = weMesh.getFaceMatrix();
    normals = weMesh.getNormalMatrix();

    colors = nanogui::MatrixXf(3, weMesh.getVertexMatrix().cols());
    for (int i = 0; i < weMesh.getVertexMatrix().cols(); i++) {
        colors.col(i) << 1, 0, 0;
    }

    // Push matrices
    mShader.bind();
    mShader.uploadIndices(indices);
    mShader.uploadAttrib(SHADER_ATTR_VERTEX_POSITION, positions);
    mShader.uploadAttrib(SHADER_ATTR_COLOR, colors);
    mShader.uploadAttrib(SHADER_ATTR_VERTEX_NORMAL, normals);
    mShader.setUniform(SHADER_ATTR_LIGHT_POS,
                       Eigen::Vector3f(-2 * halfWidth - 2, 6 * halfWidth + 6, -4 * halfWidth - 4));
}

OBJViewer::OBJViewerCanvas::~OBJViewerCanvas() {
    mShader.free();
}

void OBJViewer::OBJViewerCanvas::setZoom(float zoom) {
    if (zoom < MIN_ZOOM) {
        zoom = MIN_ZOOM;
        return;
    }
    mZoom = zoom;
}

double OBJViewer::OBJViewerCanvas::getZoom() const {
    return mZoom;
}

nanogui::Vector3f OBJViewer::OBJViewerCanvas::getRotation() const {
    return mRotation;
}

void OBJViewer::OBJViewerCanvas::setRotation(nanogui::Vector3f vRotation) {
    mRotation = vRotation;
}

nanogui::Vector3f OBJViewer::OBJViewerCanvas::getTarget() const {
    return mTarget;
}

void OBJViewer::OBJViewerCanvas::setTarget(nanogui::Vector3f vTarget) {
    mTarget = vTarget;
}

void OBJViewer::OBJViewerCanvas::setShadedWithMeshEdges() {
    weMesh.populateSmoothShadingMatrices();
    weMesh.populateExpandedVertexMatrix(wireframeOverlayEpsilon);
    refreshGeometry();
    drawWireframe = true;
    drawSolid = true;
    flatShading = false;
}

void OBJViewer::OBJViewerCanvas::setWireframeMode() {
    weMesh.populateSmoothShadingMatrices();
    refreshGeometry();
    drawWireframe = true;
    drawSolid = false;
    flatShading = false;
}

void OBJViewer::OBJViewerCanvas::setSmoothShading() {
    weMesh.populateSmoothShadingMatrices();
    refreshGeometry();
    drawWireframe = false;
    drawSolid = true;
    flatShading = false;
}

void OBJViewer::OBJViewerCanvas::setFlatShading() {
    weMesh.populateFlatShadingMatrices();
    refreshGeometry();
    drawWireframe = false;
    drawSolid = true;
    flatShading = true;
}

void OBJViewer::OBJViewerCanvas::updateMVP() {
    // Compute view matrix
    nanogui::Matrix4f camRotation;
    camRotation.setIdentity();
    camRotation.topLeftCorner<3, 3>() = Eigen::Matrix3f(Eigen::AngleAxisf(mRotation[1], nanogui::Vector3f::UnitY()) *
                                                        Eigen::AngleAxisf(mRotation[2], nanogui::Vector3f::UnitZ()) *
                                                        Eigen::AngleAxisf(mRotation[0], nanogui::Vector3f::UnitX()));

    nanogui::Vector4f camPosition = camRotation * nanogui::Vector4f(0, halfWidth * 2, 0, 1);
    nanogui::Vector4f camUp = camRotation * nanogui::Vector4f(0, 0, 1, 1);
    V = nanogui::lookAt(camPosition.topRows<3>() + mTarget, mTarget, camUp.topRows<3>());


    // Compute model matrix.
    M.setIdentity();

    // Compute projection matrix.
    P = nanogui::ortho(-halfWidth * 2 / mZoom, halfWidth * 2 / mZoom, -halfWidth * 2 / mZoom, halfWidth * 2 / mZoom, 0,
                       halfWidth * 4 + 100);
    MVP = P * V * M;
}

void OBJViewer::OBJViewerCanvas::drawGL() {
    using namespace nanogui;

    // Bind shader
    mShader.bind();

    // Pass in MVP matrices
    mShader.setUniform(SHADER_ATTR_MATRIX_VIEW, V);
    mShader.setUniform(SHADER_ATTR_MATRIX_MODEL, M);
    mShader.setUniform(SHADER_ATTR_MATRIX_MVP, MVP);

    // Enable z buffer
    glEnable(GL_DEPTH_TEST);

    // Cull back faces so wireframes look similar to the preview given in assignment pdf
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Draw solid
    if (drawSolid) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        mShader.drawIndexed(GL_TRIANGLES, 0, weMesh.getFaceMatrix().cols());
    }

    // Draw wireframe
    if (drawWireframe) {
        if (drawSolid) {
            // Load expanded vertices so the wireframe is drawn on top of solid.
            mShader.uploadAttrib(SHADER_ATTR_VERTEX_POSITION, weMesh.getExpandedVertexMatrix());
        }

        // Correct diffuse so wireframe is coloured differently from shaded mesh
        mShader.setUniform(SHADER_ATTR_DIFFUSE_CORRECTION, Eigen::Vector3f(1, 1, 1));

        // Draw as lines
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mShader.drawIndexed(GL_TRIANGLES, 0, weMesh.getFaceMatrix().cols());

        mShader.setUniform(SHADER_ATTR_DIFFUSE_CORRECTION, Eigen::Vector3f(0, 0, 0));
        if (drawSolid) {
            mShader.uploadAttrib(SHADER_ATTR_VERTEX_POSITION, positions);
        }
    }
    // Reset to render filled solids
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
}
