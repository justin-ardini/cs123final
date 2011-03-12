#define GL_GLEXT_LEGACY // no glext.h, we have our own
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include "glext.h"
#include <GL/glu.h>
#include "glm.h"

#include "drawengine.h"
#include <QKeyEvent>
#include <QGLContext>
#include <QHash>
#include <QGLShaderProgram>
#include <QQuaternion>
#include <QVector3D>
#include <QString>
#include <iostream>
#include <QFile>
#include <QGLFramebufferObject>

using std::cout;
using std::endl;

enum SKYBOX_TYPE {
    SKYBOX_ALPINE,
    SKYBOX_ISLAND,
    SKYBOX_HOURGLASS
};

// Changes the skybox texture
#define SKYBOX SKYBOX_ISLAND
// Changes the water level
#define SEA_LEVEL 7.3f
// Changes the water quad size
#define WATER_QUAD_SIZE 10.0f


/**
  DrawEngine ctor.  Expects a Valid OpenGL context and the viewport's current
  width and height.  Initializes the draw engine.  Loads models,textures,shaders,
  and allocates framebuffers.  Also sets up OpenGL to begin drawing.

  @param context The current OpenGL context this drawing engine is associated with.
  Probably should be the context from the QGLWidget.
  @param w The viewport width used to allocate the correct framebuffer size.
  @param h The viewport heigh used to alloacte the correct framebuffer size.

**/
DrawEngine::DrawEngine(const QGLContext *context, int w, int h) : context_(context),
        dofEnabled_(true), depthmapEnabled_(false), offsetX_(0.0f), offsetY_(0.0f), bumpMap_(-1),
        blurFactor_(1.6f) {
    // Initialize OGL settings
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_POLYGON_SMOOTH); //Enable smoothing

    glShadeModel(GL_SMOOTH); //Smooth or flat shading model
    glPolygonMode(GL_FRONT, GL_FILL); //Shaded mode
    glPolygonMode(GL_BACK, GL_FILL);

    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glFrontFace(GL_CCW);
    glDisable(GL_DITHER);
    glDisable(GL_LIGHTING);
    glClearColor(0.0f,0.0f,0.0f,0.0f);

    // Initialize member variables
    previous_time_ = 0.0;

    //Initialize resources
    cout << "Using OpenGL Version " << glGetString(GL_VERSION) << endl << endl;
    // Ideally we would now check to make sure all the OGL functions we use are supported
    // by the video card.  but that's a pain to do so we're not going to.

    cout << "Loading Resources" << endl;
    load_models();
    load_shaders();

    terrain_ = new Terrain();
    srand(2);
    float3 tl(-10, 10, 2);
    float3 tr(10, 10, 4);
    float3 bl(-10, -10, 8);
    float3 br(10, -10, 6);
    terrain_->populateTerrain(tl, tr, bl, br);
    terrain_->populateNormals();

    load_textures();
    create_fbos(w,h);

    cout << "Rendering..." << endl;
}

DrawEngine::~DrawEngine() {
    delete terrain_;
    foreach(QGLShaderProgram *sp,shader_programs_)
        delete sp;
    foreach(QGLFramebufferObject *fbo,framebuffer_objects_)
        delete fbo;
    foreach(GLuint id,textures_)
        ((QGLContext *)(context_))->deleteTexture(id);
    foreach(Model m,models_)
        glmDelete(m.model);
}

/**
  Loads models used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_models() {
    cout << "Loading models..." << endl;
    models_["skybox"].idx = glGenLists(1);
    glNewList(models_["skybox"].idx,GL_COMPILE);
    //Be glad we wrote this for you...ugh.
    glBegin(GL_QUADS);
    float fExtent = 50.f;
    glTexCoord3f(1.0f,-1.0f,-1.0f); glVertex3f(fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f);glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f); glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f);glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,-1.0f); glVertex3f(fExtent,-fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f);  glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f); glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);  glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f); glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f);  glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f); glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f); glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f); glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,-1.0f);glVertex3f(-fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,1.0f,1.0f);glVertex3f(-fExtent,fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,1.0f);glVertex3f(fExtent,fExtent,fExtent);
    glTexCoord3f(1.0f,1.0f,-1.0f);glVertex3f(fExtent,fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,-1.0f);glVertex3f(-fExtent,-fExtent,-fExtent);
    glTexCoord3f(-1.0f,-1.0f,1.0f);glVertex3f(-fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,1.0f); glVertex3f(fExtent,-fExtent,fExtent);
    glTexCoord3f(1.0f,-1.0f,-1.0f);glVertex3f(fExtent,-fExtent,-fExtent);
    glEnd();
    glEndList();
    cout << "\t  skybox compiled " << endl;
}
/**
  Loads shaders used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_shaders() {
    shader_programs_["terrain"] = new QGLShaderProgram(context_);
    shader_programs_["terrain"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                         "shaders/terrain.vert");
    shader_programs_["terrain"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                         "shaders/terrain.frag");
    shader_programs_["terrain"]->link();
    cout << "\t  shaders/terrain " << endl;

    shader_programs_["water"] = new QGLShaderProgram(context_);
    shader_programs_["water"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                         "shaders/water.vert");
    shader_programs_["water"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                         "shaders/water.frag");
    shader_programs_["water"]->link();
    cout << "\t  shaders/water " << endl;

    shader_programs_["blur_x"] = new QGLShaderProgram(context_);
    shader_programs_["blur_x"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/blurx.vert");
    shader_programs_["blur_x"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/blurx.frag");
    shader_programs_["blur_x"]->link();
    cout << "\t  shaders/blurx " << endl;

    shader_programs_["blur_y"] = new QGLShaderProgram(context_);
    shader_programs_["blur_y"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/blury.vert");
    shader_programs_["blur_y"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/blury.frag");
    shader_programs_["blur_y"]->link();
    cout << "\t  shaders/blury " << endl;

    shader_programs_["lerp"] = new QGLShaderProgram(context_);
    shader_programs_["lerp"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/lerp.vert");
    shader_programs_["lerp"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/lerp.frag");
    shader_programs_["lerp"]->link();
    cout << "\t  shaders/lerp " << endl;

    shader_programs_["depthmap"] = new QGLShaderProgram(context_);
    shader_programs_["depthmap"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/depthmap.vert");
    shader_programs_["depthmap"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/depthmap.frag");
    shader_programs_["depthmap"]->link();
    cout << "\t  shaders/depthmap " << endl;
}

/**
  Loads textures used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_textures() {
    cout << "Loading textures..." << endl;
    QList<QFile *> fileList;

    switch (SKYBOX) {
    case SKYBOX_ALPINE:
        fileList.append(new QFile("textures/alpine/alpine_west.bmp"));
        fileList.append(new QFile("textures/alpine/alpine_east.bmp"));
        fileList.append(new QFile("textures/alpine/alpine_up.bmp"));
        fileList.append(new QFile("textures/alpine/alpine_down.bmp"));
        fileList.append(new QFile("textures/alpine/alpine_south.bmp"));
        fileList.append(new QFile("textures/alpine/alpine_north.bmp"));
        break;
    case SKYBOX_HOURGLASS:
        fileList.append(new QFile("textures/hourglass/hourglass_west.bmp"));
        fileList.append(new QFile("textures/hourglass/hourglass_east.bmp"));
        fileList.append(new QFile("textures/hourglass/hourglass_up.bmp"));
        fileList.append(new QFile("textures/hourglass/hourglass_down.bmp"));
        fileList.append(new QFile("textures/hourglass/hourglass_south.bmp"));
        fileList.append(new QFile("textures/hourglass/hourglass_north.bmp"));
        break;
    case SKYBOX_ISLAND:
    default:
        fileList.append(new QFile("textures/islands/islands_west.bmp"));
        fileList.append(new QFile("textures/islands/islands_east.bmp"));
        fileList.append(new QFile("textures/islands/islands_up.bmp"));
        fileList.append(new QFile("textures/islands/islands_down.bmp"));
        fileList.append(new QFile("textures/islands/islands_south.bmp"));
        fileList.append(new QFile("textures/islands/islands_north.bmp"));
    }

    GLuint terrainTextures[4];
    terrainTextures[0] = load_texture(TERRAIN_TEX0);
    terrainTextures[1] = load_texture(TERRAIN_TEX1);
    terrainTextures[2] = load_texture(TERRAIN_TEX2);
    terrainTextures[3] = load_texture(TERRAIN_TEX3);
    bumpMap_ = load_texture(QString("textures/water01_bumpmap.jpg"));
    textures_["cube_map_1"] = load_cube_map(fileList);

    terrain_->setTextures(terrainTextures);
}

/**
  Load a single texture.
  **/
GLuint DrawEngine::load_texture(const QFile &file) {
    QImage image, texture;
    GLuint toReturn = -1;
    if(!file.exists()){
        std::cout << "texture load fail" <<std::endl;
        return -1;
    }
    else{
        std::cout << "texture load success" <<std::endl;
    }

    image.load(file.fileName());
    image = image.mirrored(false, true);
    texture = QGLWidget::convertToGLFormat(image);

    glGenTextures(1, &toReturn);
    glActiveTexture(GL_TEXTURE0 + toReturn);
    glBindTexture(GL_TEXTURE_2D, toReturn);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    return toReturn;
}


/**
  Creates the intial framebuffers for drawing.  Called by the ctor once
  upon initialization.

  @todo Finish filling this in.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::create_fbos(int w, int h) {
    //Allocate the main framebuffer object for rendering the scene to
    //This needs a depth attachment.
    framebuffer_objects_["fbo_0"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
    framebuffer_objects_["fbo_0"]->format().setSamples(16);
    //Allocate the secondary framebuffer obejcts for rendering textures to (post process effects)
    //These do not require depth attachments.
    framebuffer_objects_["fbo_1"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
    framebuffer_objects_["fbo_2"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
    framebuffer_objects_["reflection"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
    framebuffer_objects_["refraction"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::Depth,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
}

/**
  Reallocates all the framebuffers.  Called when the viewport is
  resized.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::realloc_framebuffers(int w, int h) {
    foreach(QGLFramebufferObject *fbo,framebuffer_objects_)  {
        const QString &key = framebuffer_objects_.key(fbo);
        QGLFramebufferObjectFormat format = fbo->format();
        delete fbo;
        framebuffer_objects_[key] = new QGLFramebufferObject(w,h,format);
    }
}


/**
  Should render one frame at the given elapsed time in the program.
  Assumes that the GL context is valid when this method is called.

  @param time: the current program time in milliseconds
  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::draw_frame(float time, int w, int h) {
    fps_ = 1000.f / (time - previous_time_), previous_time_ = time;

    // Render just the reflected scene about sea level to a framebuffer
    framebuffer_objects_["reflection"]->bind();
    perspective_camera(w, h);
    glActiveTexture(GL_TEXTURE0);
    render_reflections();
    framebuffer_objects_["reflection"]->release();

    // Render just the scene below sea level to a framebuffer
    framebuffer_objects_["refraction"]->bind();
    perspective_camera(w, h);
    glActiveTexture(GL_TEXTURE0);
    render_refraction();
    framebuffer_objects_["refraction"]->release();

    // Refraction testing
    if (false) {
        orthogonal_camera(w, h);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["refraction"]->texture());
        textured_quad(w, h, true);
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    // Render the scene to framebuffer 0, tracking how much we need to blur later
    framebuffer_objects_["fbo_0"]->bind();
    perspective_camera(w, h);
    // Ensure that GL_TEXTURE0 is active before rendering the scene!
    glActiveTexture(GL_TEXTURE0);
    render_scene(w, h);
    framebuffer_objects_["fbo_0"]->release();

    orthogonal_camera(w, h);

    if (depthmapEnabled_) {
        // If depth map enabled, just render the alpha (blend) values
        shader_programs_["depthmap"]->bind();
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        textured_quad(w, h, true);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader_programs_["depthmap"]->release();
    } else if (dofEnabled_) {
        glViewport(0, 0, w, h);

        // Third pass: Gaussian filtering along the X axis
        framebuffer_objects_["fbo_1"]->bind();
        shader_programs_["blur_x"]->bind();
        shader_programs_["blur_x"]->setUniformValue("Width", w * blurFactor_);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        textured_quad(w, h, true);
        shader_programs_["blur_x"]->release();
        framebuffer_objects_["fbo_1"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);

        // Fourth pass: Gaussian filtering along the Y axis
        framebuffer_objects_["fbo_2"]->bind();
        shader_programs_["blur_y"]->bind();
        shader_programs_["blur_y"]->setUniformValue("Height", h * blurFactor_);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_1"]->texture());
        textured_quad(w, h, true);
        shader_programs_["blur_y"]->release();
        framebuffer_objects_["fbo_2"]->release();
        glBindTexture(GL_TEXTURE_2D, 0);

        // Fifth pass: final compositing
        glDrawBuffer(GL_BACK);
        shader_programs_["lerp"]->bind();
        shader_programs_["lerp"]->setUniformValue("Tex0", 0);
        shader_programs_["lerp"]->setUniformValue("Tex1", 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_2"]->texture());

        // Multitextured quad
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 1.0f);
        glVertex2f(0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 1.0f);
        glVertex2f(w, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 1.0f, 0.0f);
        glVertex2f(w, h);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
        glVertex2f(0.0f, h);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader_programs_["lerp"]->release();

    } else {
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        textured_quad(w, h, true);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Make sure texture0 is active for text rendering afterwards
    glActiveTexture(GL_TEXTURE0);
}


/**
  Renders the reflections of the scene about the water level
**/
void DrawEngine::render_reflections() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);

    glPushMatrix();
    // 2.38 is a magic number connected to transformations to the terrain
    glTranslatef(0.0f, -2.38f, 0.0f);
    glScalef(1.0f, -1.0f, 1.0f);
    glTranslatef(0.0f, 2.38f, 0.0f);

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // First, render the terrain with the terrain shader
    shader_programs_["terrain"]->bind();
    glActiveTexture(GL_TEXTURE0);
    terrain_->updateTerrainShaderParameters(shader_programs_["terrain"]);
    shader_programs_["terrain"]->setUniformValue("seaLevel", SEA_LEVEL);
    shader_programs_["terrain"]->setUniformValue("isReflection", 1.0f);
    shader_programs_["terrain"]->setUniformValue("focalDistance", camera_.getFocalDistance());
    shader_programs_["terrain"]->setUniformValue("focalRange", camera_.getFocalRange());

    glPushMatrix();
    glTranslatef(0.0f, -28.0f, 0.0f);
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    glScalef(3.5f, 3.5f, 3.5f);
    terrain_->render();
    glPopMatrix();
    shader_programs_["terrain"]->release();

    glPopMatrix();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
}

/**
  Render the refraction to a framebuffer
  **/
void DrawEngine::render_refraction() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // First, render the terrain with the terrain shader
    shader_programs_["terrain"]->bind();
    glActiveTexture(GL_TEXTURE0);
    terrain_->updateTerrainShaderParameters(shader_programs_["terrain"]);
    shader_programs_["terrain"]->setUniformValue("seaLevel", SEA_LEVEL);
    shader_programs_["terrain"]->setUniformValue("isReflection", 2.0f);
    shader_programs_["terrain"]->setUniformValue("focalDistance", camera_.getFocalDistance());
    shader_programs_["terrain"]->setUniformValue("focalRange", camera_.getFocalRange());
    glPushMatrix();
    glTranslatef(0, -28.f, 0.f);
    glRotatef(270, 1, 0, 0);
    glScalef(3.5, 3.5, 3.5);
    terrain_->render();
    glPopMatrix();
    shader_programs_["terrain"]->release();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
}


/**
  Renders the actual scene.  May be called multiple times by
  DrawEngine::draw_frame(float time,int w,int h) if necessary.

  @param time: the current time
  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::render_scene(int w, int h) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glPushMatrix();

    // First, render the terrain with the terrain shader
    shader_programs_["terrain"]->bind();
    glActiveTexture(GL_TEXTURE0);
    terrain_->updateTerrainShaderParameters(shader_programs_["terrain"]);
    shader_programs_["terrain"]->setUniformValue("focalDistance", camera_.getFocalDistance());
    shader_programs_["terrain"]->setUniformValue("focalRange", camera_.getFocalRange());
    shader_programs_["terrain"]->setUniformValue("isReflection", 0.0f);

    glTranslatef(0, -28.f, 0.f);
    glRotatef(270, 1, 0, 0);
    glScalef(3.5, 3.5, 3.5);
    terrain_->render();
    shader_programs_["terrain"]->release();

    // Then render the water with the water shader
    shader_programs_["water"]->bind();
    shader_programs_["water"]->setUniformValue("reflection", 0);
    shader_programs_["water"]->setUniformValue("bumpMap", 7);
    shader_programs_["water"]->setUniformValue("refraction", 8);
    shader_programs_["water"]->setUniformValue("focalDistance", camera_.getFocalDistance());
    shader_programs_["water"]->setUniformValue("focalRange", camera_.getFocalRange());
    // These casts to float are necessary, c'mon GLSL
    shader_programs_["water"]->setUniformValue("screenWidth", (float) w);
    shader_programs_["water"]->setUniformValue("screenHeight", (float) h);
    shader_programs_["water"]->setUniformValue("offsetX", offsetX_);
    shader_programs_["water"]->setUniformValue("offsetY", offsetY_);

    render_water();
    shader_programs_["water"]->release();

    glPopMatrix();

    // Update the water animation offset
    offsetX_ = offsetX_ + 0.001f;
    offsetY_ = offsetY_ + 0.001f;

    if(offsetX_ >= 1.0f){
        offsetX_ = 0.0f;
        offsetY_ = 0.0f;
    }

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);
}

/**
  Renders the water as a large quad.
  **/
void DrawEngine::render_water() {
    // Bind the reflection to id 0
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["reflection"]->texture());

    // Bind the bump map to id 1
    glActiveTexture(GL_TEXTURE7);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, bumpMap_);

    // Bind the refraction to id 2
    glActiveTexture(GL_TEXTURE8);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["refraction"]->texture());

    // Draw the water quad
    glBegin(GL_QUADS);
        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE7, 0.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE8, 0.0f, 0.0f);
        glNormal3f(0, 0, 1);
        glVertex3f(-WATER_QUAD_SIZE, -WATER_QUAD_SIZE, SEA_LEVEL);

        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE7, 1.0f, 0.0f);
        glMultiTexCoord2f(GL_TEXTURE8, 1.0f, 0.0f);
        glNormal3f(0, 0, 1);
        glVertex3f(WATER_QUAD_SIZE, -WATER_QUAD_SIZE, SEA_LEVEL);

        glMultiTexCoord2f(GL_TEXTURE0, 1.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE7, 1.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE8, 1.0f, 1.0f);
        glNormal3f(0, 0, 1);
        glVertex3f(WATER_QUAD_SIZE, WATER_QUAD_SIZE, SEA_LEVEL);

        glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE7, 0.0f, 1.0f);
        glMultiTexCoord2f(GL_TEXTURE8, 0.0f, 1.0f);
        glNormal3f(0, 0, 1);
        glVertex3f(-WATER_QUAD_SIZE, WATER_QUAD_SIZE, SEA_LEVEL);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
  Draws a textured quad. The texture most be bound and unbound
  before and after calling this method - this method assumes that the texture
  has been bound before hand.

  @param w: the width of the quad to draw
  @param h: the height of the quad to draw
  @param flip: flip the texture vertically

**/
void DrawEngine::textured_quad(int w, int h, bool flip) {
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, flip ? 1.0f : 0.0f);
    glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, flip ? 1.0f : 0.0f);
    glVertex2f(w, 0.0f);
    glTexCoord2f(1.0f, flip ? 0.0f : 1.0f);
    glVertex2f(w, h);
    glTexCoord2f(0.0f, flip ? 0.0f : 1.0f);
    glVertex2f(0.0f, h);
    glEnd();
}

/**
  Called to switch to the perspective OpenGL camera.
  Used to render the scene regularly with the current camera parameters.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::perspective_camera(int w, int h) {
    float ratio = w / static_cast<float>(h);

    // Set up projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy_, ratio, camera_.near_, camera_.far_);

    // Set up modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera_.multMatrix();

}

/**
  Called to switch to an orthogonal OpenGL camera.
  Useful for rending a textured quad across the whole screen.

  @param w the viewport width
  @param h the viewport height

**/
void DrawEngine::orthogonal_camera(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,static_cast<float>(w),static_cast<float>(h),0.f,-1.f,1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  Called when the viewport has been resized. Needs to
  resize the camera perspective and reallocate the framebuffer
  sizes.

  @param w the viewport width
  @param h the viewport height

**/
void DrawEngine::resize_frame(int w, int h) {
    glViewport(0,0,w,h);
    realloc_framebuffers(w,h);
}

/**
  Called by GLWidget when the mouse is dragged.  Rotates the camera
  based on mouse movement.

  @param p0 the old mouse position
  @param p1 the new mouse position
**/
void DrawEngine::mouse_drag_event(float2 p0, float2 p1, const Qt::MouseButtons &buttons) {
    camera_.mouseMove(Vector2(p1.x - p0.x, p1.y - p0.y), buttons);
}

/**
  Called by GLWidget when the mouse wheel is turned. Zooms the camera in
  and out.

  @param dx The delta value of the mouse wheel movement.
**/
void DrawEngine::mouse_wheel_event(int dx) {
    camera_.mouseWheel(dx);
}

/**
  Loads the cube map into video memory.

  @param files a list of files containing the cube map images (should be length
  six) in order.
  @return The assigned OpenGL id to the cube map.
**/
GLuint DrawEngine::load_cube_map(QList<QFile *> files) {
    GLuint id;
    glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_CUBE_MAP,id);
    for(unsigned i = 0; i < 6; ++i) {
        QImage image,texture;
        image.load(files[i]->fileName());
        image = image.mirrored(false,true);
        texture = QGLWidget::convertToGLFormat(image);
        texture = texture.scaledToWidth(2048,Qt::SmoothTransformation);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,3,3,texture.width(),texture.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture.bits());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
        cout << "\t  " << files[i]->fileName().toStdString() << " " << endl;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    return id;
}


/**
  Called when a key has been pressed in the GLWidget.

  @param event The key press event associated with the current key press.
  **/
void DrawEngine::key_press_event(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Up:
        camera_.focalDistance_ += 5.0f;
        break;
    case Qt::Key_Down:
        camera_.focalDistance_ -= 5.0f;
        break;
    case Qt::Key_Left:
        if (camera_.focalRange_ >= 5.0f) {
           camera_.focalRange_ -= 5.0f;
        }
        break;
    case Qt::Key_Right:
        if (camera_.focalRange_ <= 195.0f) {
           camera_.focalRange_ += 5.0f;
        }
        break;
    case Qt::Key_D:
        dofEnabled_ = !dofEnabled_;
        break;
    case Qt::Key_M:
        depthmapEnabled_ = !depthmapEnabled_;
        break;
    case Qt::Key_O:
        if (blurFactor_ <= 10) {
            blurFactor_ += 0.5f;
        }
        break;
    case Qt::Key_P:
        if (blurFactor_ >= 1.0f) {
          blurFactor_ -= 0.5f;
        }
        break;
    }
}
