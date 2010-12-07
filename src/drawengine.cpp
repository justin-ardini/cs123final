/**
  A simple OpenGL drawing engine.

  @author psastras
**/

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


/**
  @paragraph DrawEngine ctor.  Expects a Valid OpenGL context and the viewport's current
  width and height.  Initializes the draw engine.  Loads models,textures,shaders,
  and allocates framebuffers.  Also sets up OpenGL to begin drawing.

  @param context The current OpenGL context this drawing engine is associated with.
  Probably should be the context from the QGLWidget.

  @param w The viewport width used to allocate the correct framebuffer size.
  @param h The viewport heigh used to alloacte the correct framebuffer size.

**/
DrawEngine::DrawEngine(const QGLContext *context,int w,int h) : context_(context) {
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

    /*
    // Setup Global Lighting
    GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f }; //OpenGL defaults to 0.2, 0.2, 0.2, 1.0
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);

    // Setup Local Lighting

    GLfloat ambientLight[] = {0.1f, 0.1f, 0.1f, 1.0f};
    GLfloat diffuseLight[] = { 1.0f, 1.0f, 1.0, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat position[] = { 0, 10, 5, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glClearColor(0, 0, 0, 0); //Set the color to clear buffers to

    glEnable(GL_LIGHTING); //Enable lighting
    glEnable(GL_LIGHT0);
    */

    glFrontFace(GL_CCW);
    glDisable(GL_DITHER);
    glDisable(GL_LIGHTING);
    glClearColor(0.0f,0.0f,0.0f,0.0f);

    //init member variables
    previous_time_ = 0.0f;
    camera_.center.x = 0.f,camera_.center.y = 0.f,camera_.center.z = 0.f;
    camera_.eye.x = 0.f,camera_.eye.y = 0.0f,camera_.eye.z = -2.f;
    camera_.up.x = 0.f,camera_.up.y = 1.f,camera_.up.z = 0.f;
    camera_.near = 0.1f,camera_.far = 100.f;
    camera_.fovy = 60.f;
    camera_.focalDistance = DEFAULT_DISTANCE;
    camera_.focalRange = DEFAULT_RANGE;

    //init resources - so i heard you like colored text?
    cout << "Using OpenGL Version " << glGetString(GL_VERSION) << endl << endl;
    //ideally we would now check to make sure all the OGL functions we use are supported
    //by the video card.  but that's a pain to do so we're not going to.

    cout << "\033Loading Resources\033" << endl;
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

    cout << "\033Rendering...\033" << endl;
}

/**
  @paragraph Dtor
**/
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
  @paragraph Loads models used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_models() {
    cout << "\033Loading models...\033" << endl;
    /* models_["dragon"].model = glmReadOBJ("/home/jardini/course/cs123_labs/lab09/src/models/xyzrgb_dragon.obj");
    glmUnitize(models_["dragon"].model);
    models_["dragon"].idx = glmList(models_["dragon"].model,GLM_SMOOTH);
    cout << "\t \033[32m/course/cs123/data/mesh/xyzrgb_dragon_old.obj\033[0m" << endl;
    //Create grid
    models_["grid"].idx = glGenLists(1);
    glNewList(models_["grid"].idx,GL_COMPILE);
    float r = 1.f,dim = 10,delta = r * 2 / dim;
    for(int y = 0; y < dim; ++y) {
        glBegin(GL_QUAD_STRIP);
        for(int x = 0; x <= dim; ++x) {
            glVertex2f(x * delta - r,y * delta - r);
            glVertex2f(x * delta - r,(y + 1) * delta - r);
        }
        glEnd();
    }
    glEndList();
    cout << "\t \033grid compiled\033" << endl;
    */
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
    cout << "\t \033skybox compiled\033" << endl;
}
/**
  @paragraph Loads shaders used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_shaders() {
    cout << "\033Loading shaders...\033" << endl;
    shader_programs_["reflect"] = new QGLShaderProgram(context_);
    shader_programs_["reflect"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                       "shaders/reflect.vert");
    shader_programs_["reflect"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "shaders/reflect.frag");
    shader_programs_["reflect"]->link();
    cout << "\t \033shaders/reflect\033" << endl;
    shader_programs_["refract"] = new QGLShaderProgram(context_);
    shader_programs_["refract"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                       "shaders/refract.vert");
    shader_programs_["refract"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                       "shaders/refract.frag");
    shader_programs_["refract"]->link();
    cout << "\t \033shaders/refract\033" << endl;

    shader_programs_["terrain"] = new QGLShaderProgram(context_);
    shader_programs_["terrain"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                         "shaders/testshader1.vert");
    shader_programs_["terrain"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                         "shaders/testshader1.frag");
    shader_programs_["terrain"]->link();
    cout << "\t \033shaders/terrain\033" << endl;

    shader_programs_["downsample"] = new QGLShaderProgram(context_);
    shader_programs_["downsample"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/downsample.vert");
    shader_programs_["downsample"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/downsample.frag");
    shader_programs_["downsample"]->link();
    cout << "\t \033shaders/downsample\033" << endl;

    shader_programs_["blur_x"] = new QGLShaderProgram(context_);
    shader_programs_["blur_x"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/blurx.vert");
    shader_programs_["blur_x"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/blurx.frag");
    shader_programs_["blur_x"]->link();
    cout << "\t \033shaders/blurx\033" << endl;

    shader_programs_["blur_y"] = new QGLShaderProgram(context_);
    shader_programs_["blur_y"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/blury.vert");
    shader_programs_["blur_y"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/blury.frag");
    shader_programs_["blur_y"]->link();
    cout << "\t \033shaders/blury\033" << endl;

    shader_programs_["lerp"] = new QGLShaderProgram(context_);
    shader_programs_["lerp"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/lerp.vert");
    shader_programs_["lerp"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/lerp.frag");
    shader_programs_["lerp"]->link();
    cout << "\t \033shaders/lerp\033" << endl;

    shader_programs_["renderblur"] = new QGLShaderProgram(context_);
    shader_programs_["renderblur"]->addShaderFromSourceFile(QGLShader::Vertex,
                                                            "shaders/downsample.vert");
    shader_programs_["renderblur"]->addShaderFromSourceFile(QGLShader::Fragment,
                                                            "shaders/renderblur.frag");
    shader_programs_["renderblur"]->link();
    cout << "\t \033shaders/renderblur\033" << endl;
}

/**
  @paragraph Loads textures used by the program.  Caleed by the ctor once upon
  initialization.
**/
void DrawEngine::load_textures() {
    cout << "\033Loading textures...\033" << endl;
    QList<QFile *> fileList;

    // Alpine

    fileList.append(new QFile("../src/textures/alpine/alpine_west.bmp"));
    fileList.append(new QFile("../src/textures/alpine/alpine_east.bmp"));
    fileList.append(new QFile("../src/textures/alpine/alpine_up.bmp"));
    fileList.append(new QFile("../src/textures/alpine/alpine_down.bmp"));
    fileList.append(new QFile("../src/textures/alpine/alpine_south.bmp"));
    fileList.append(new QFile("../src/textures/alpine/alpine_north.bmp"));
    /*
    // Lagoon
    fileList.append(new QFile("../src/textures/lagoon/lagoon_west.bmp"));
    fileList.append(new QFile("../src/textures/lagoon/lagoon_east.bmp"));
    fileList.append(new QFile("../src/textures/lagoon/lagoon_up.bmp"));
    fileList.append(new QFile("../src/textures/lagoon/lagoon_down.bmp"));
    fileList.append(new QFile("../src/textures/lagoon/lagoon_south.bmp"));
    fileList.append(new QFile("../src/textures/lagoon/lagoon_north.bmp")); */
    /*
    // Hourglass
    fileList.append(new QFile("../src/textures/hourglass/hourglass_west.bmp"));
    fileList.append(new QFile("../src/textures/hourglass/hourglass_east.bmp"));
    fileList.append(new QFile("../src/textures/hourglass/hourglass_up.bmp"));
    fileList.append(new QFile("../src/textures/hourglass/hourglass_down.bmp"));
    fileList.append(new QFile("../src/textures/hourglass/hourglass_south.bmp"));
    fileList.append(new QFile("../src/textures/hourglass/hourglass_north.bmp")); */
    // Island
    /*
    fileList.append(new QFile("../src/textures/islands/islands_west.bmp"));
    fileList.append(new QFile("../src/textures/islands/islands_east.bmp"));
    fileList.append(new QFile("../src/textures/islands/islands_up.bmp"));
    fileList.append(new QFile("../src/textures/islands/islands_down.bmp"));
    fileList.append(new QFile("../src/textures/islands/islands_south.bmp"));
    fileList.append(new QFile("../src/textures/islands/islands_north.bmp"));*/

    GLuint terrainTextures[4];
    terrainTextures[0] = load_texture(TERRAIN_TEX0);
    terrainTextures[1] = load_texture(TERRAIN_TEX1);
    terrainTextures[2] = load_texture(TERRAIN_TEX2);
    terrainTextures[3] = load_texture(TERRAIN_TEX3);
    textures_["cube_map_1"] = load_cube_map(fileList);

    terrain_->setTextures(terrainTextures);

    GLuint tempInt = load_texture(QString("../water01_bumpmap.jpg"));
    terrain_->bumpmap = tempInt;
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
    glActiveTexture(GL_TEXTURE0+toReturn );
    glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0+toReturn);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    return toReturn;
}


/**
  @paragraph Creates the intial framebuffers for drawing.  Called by the ctor once
  upon initialization.

  @todo Finish filling this in.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::create_fbos(int w,int h) {

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
    framebuffer_objects_["fbo_3"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
    framebuffer_objects_["fbo_4"] = new QGLFramebufferObject(w,h,QGLFramebufferObject::NoAttachment,
                                                             GL_TEXTURE_2D,GL_RGBA16F_ARB);
}

/**
  @paragraph Reallocates all the framebuffers.  Called when the viewport is
  resized.

  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::realloc_framebuffers(int w,int h) {
    foreach(QGLFramebufferObject *fbo,framebuffer_objects_)  {
        const QString &key = framebuffer_objects_.key(fbo);
        QGLFramebufferObjectFormat format = fbo->format();
        delete fbo;
        framebuffer_objects_[key] = new QGLFramebufferObject(w,h,format);
    }
}


void draw_quad() {
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2d(-1, -1);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2d(1, -1);
    glTexCoord2f( 1.0f, 1.0f);
    glVertex2d(1, 1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2d(-1, 1);
    glEnd();
}


/**
  @paragraph Should render one frame at the given elapsed time in the program.
  Assumes that the GL context is valid when this method is called.

  @todo Finish filling this in

  @param time: the current program time in milliseconds
  @param w:    the viewport width
  @param h:    the viewport height
**/
void DrawEngine::draw_frame(float time,int w,int h) {
    fps_ = 1000.f / (time - previous_time_), previous_time_ = time;
    //Render the scene to framebuffer 0, tracking how much we need to blur later
    framebuffer_objects_["fbo_0"]->bind();

    perspective_camera(w, h);
    render_scene(time, w, h);
    framebuffer_objects_["fbo_0"]->release();

    // Copy the rendered scene into framebuffer 1
    //framebuffer_objects_["fbo_0"]->blitFramebuffer(framebuffer_objects_["fbo_1"],
    //                                               QRect(0,0,w,h),framebuffer_objects_["fbo_0"],
    //                                               QRect(0,0,w,h),GL_COLOR_BUFFER_BIT,GL_NEAREST);

    orthogonal_camera(w, h);

    if (true) {
        // Second Pass: Downsampling
        //framebuffer_objects_["fbo_1"]->bind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //shader_programs_["downsample"]->bind();
        //glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        //glViewport(0, 0, w / 2, h / 2);
        //textured_quad(w, h, true);
        //shader_programs_["downsample"]->release();
        //framebuffer_objects_["fbo_1"]->release();

        glViewport(0, 0, w, h);
        // Third pass: Gaussian filtering along the X axis
        framebuffer_objects_["fbo_2"]->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_programs_["blur_x"]->bind();
        shader_programs_["blur_x"]->setUniformValue("Width", w * 2);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        textured_quad(w, h, true);
        shader_programs_["blur_x"]->release();
        framebuffer_objects_["fbo_2"]->release();

        // Fourth pass: Gaussian filtering along the Y axis
        framebuffer_objects_["fbo_3"]->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader_programs_["blur_y"]->bind();
        shader_programs_["blur_y"]->setUniformValue("Height", h * 2);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_2"]->texture());
        textured_quad(w, h, true);
        shader_programs_["blur_y"]->release();
        framebuffer_objects_["fbo_3"]->release();

        // Fifth pass: final compositing
        //framebuffer_objects_["fbo_4"]->bind();
        glDrawBuffer(GL_BACK);
        shader_programs_["lerp"]->bind();
        shader_programs_["lerp"]->setUniformValue("Tex0", 0);
        shader_programs_["lerp"]->setUniformValue("Tex1", 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_3"]->texture());

        //glViewport(0, 0, w, h);

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

        shader_programs_["lerp"]->release();
        //framebuffer_objects_["fbo_4"]->release();

        //glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_4"]->texture());
        //textured_quad(w, h, true);
        //glBindTexture(GL_TEXTURE_2D, 0);
    }
    /*else {
        glBindTexture(GL_TEXTURE_2D, framebuffer_objects_["fbo_0"]->texture());
        textured_quad(w, h, true);
        glBindTexture(GL_TEXTURE_2D, 0);
    } */

}



/**
  @paragraph Renders the actual scene.  May be called multiple times by
  DrawEngine::draw_frame(float time,int w,int h) if necessary.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::render_scene(float time,int w,int h) {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textures_["cube_map_1"]);
    glCallList(models_["skybox"].idx);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // set the uniform values for the terrain shader
    shader_programs_["terrain"]->bind();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrain_->bumpmap);
    terrain_->updateTerrainShaderParameters(shader_programs_["terrain"]);
    shader_programs_["terrain"]->setUniformValue("focalDistance", camera_.focalDistance);
    shader_programs_["terrain"]->setUniformValue("focalRange", camera_.focalRange);
    glPushMatrix();
    glTranslatef(0, -15.f, 0.f);
    glRotatef(270, 1, 0, 0);
    glScalef(2.5, 2.5, 2.5);
    terrain_->render();
    glPopMatrix();
    shader_programs_["terrain"]->release();

    /*
    glActiveTexture(GL_TEXTURE0);

    shader_programs_["refract"]->bind();
    shader_programs_["refract"]->setUniformValue("CubeMap", GL_TEXTURE0);
    shader_programs_["refract"]->release();

    shader_programs_["reflect"]->bind();
    shader_programs_["reflect"]->setUniformValue("CubeMap",GL_TEXTURE0);
    shader_programs_["reflect"]->release(); */

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    glDisable(GL_TEXTURE_CUBE_MAP);
}


/**
  @paragraph Draws a textured quad. The texture most be bound and unbound
  before and after calling this method - this method assumes that the texture
  has been bound before hand.

  @param w: the width of the quad to draw
  @param h: the height of the quad to draw
  @param flip: flip the texture vertically

**/
void DrawEngine::textured_quad(int w,int h,bool flip) {
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f,flip ? 1.0f : 0.0f);
    glVertex2f(0.0f,0.0f);
    glTexCoord2f(1.0f,flip ? 1.0f : 0.0f);
    glVertex2f(w,0.0f);
    glTexCoord2f(1.0f,flip ? 0.0f : 1.0f);
    glVertex2f(w,h);
    glTexCoord2f(0.0f,flip ? 0.0f : 1.0f);
    glVertex2f(0.0f,h);
    glEnd();
}

/**
  @paragraph Called to switch to the perspective OpenGL camera.
  Used to render the scene regularly with the current camera parameters.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::perspective_camera(int w,int h) {
    float ratio = w / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camera_.fovy,ratio,camera_.near,camera_.far);
    gluLookAt(camera_.eye.x,camera_.eye.y,camera_.eye.z,
              camera_.center.x,camera_.center.y,camera_.center.z,
              camera_.up.x,camera_.up.y,camera_.up.z);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  @paragraph Called to switch to an orthogonal OpenGL camera.
  Useful for rending a textured quad across the whole screen.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::orthogonal_camera(int w,int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,static_cast<float>(w),static_cast<float>(h),0.f,-1.f,1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
  @paragraph Called when the viewport has been resized. Needs to
  resize the camera perspective and reallocate the framebuffer
  sizes.

  @param w: the viewport width
  @param h: the viewport height

**/
void DrawEngine::resize_frame(int w,int h) {
    glViewport(0,0,w,h);
    realloc_framebuffers(w,h);
}

/**
  @paragraph Called by GLWidget when the mouse is dragged.  Rotates the camera
  based on mouse movement.

  @param p0: the old mouse position
  @param p1: the new mouse position
**/
void DrawEngine::mouse_drag_event(float2 p0,float2 p1) {
    int dx = p1.x - p0.x,dy = p1.y - p0.y;
    QQuaternion qq = QQuaternion::fromAxisAndAngle(0, 1, 0, -dx / 5.0);
    QVector3D qv3 = qq.rotatedVector(QVector3D(camera_.eye.x, camera_.eye.y,
                                               camera_.eye.z));
    qq = QQuaternion::fromAxisAndAngle(qq.rotatedVector(QVector3D(1, 0, 0)), dy / 5.0);
    qv3 = qq.rotatedVector(qv3);
    camera_.eye.x = qv3.x(), camera_.eye.y = qv3.y(), camera_.eye.z = qv3.z();
}

/**
  @paragraph Called by GLWidget when the mouse wheel is turned. Zooms the camera in
  and out.

  @param dx: The delta value of the mouse wheel movement.
**/
void DrawEngine::mouse_wheel_event(int dx) {
    if((camera_.center - camera_.eye).getMagnitude() > .5 || dx < 0)
        camera_.eye += (camera_.center - camera_.eye).getNormalized() * dx * .005;
}

/**
  @paragraph Loads the cube map into video memory.

  @param files: a list of files containing the cube map images (should be length
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
        texture = texture.scaledToWidth(1024,Qt::SmoothTransformation);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,3,3,texture.width(),texture.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,texture.bits());
        gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X +i, 3, texture.width(), texture.height(), GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
        cout << "\t \033" << files[i]->fileName().toStdString() << "\033" << endl;
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glBindTexture(GL_TEXTURE_CUBE_MAP,0);
    return id;
}


/**
  @paragraph Called when a key has been pressed in the GLWidget.

  @param event: The key press event associated with the current key press.
  **/
void DrawEngine::key_press_event(QKeyEvent *event) {
    switch(event->key()) {
    case Qt::Key_Up:
        camera_.focalDistance += 1.0f;
        break;
    case Qt::Key_Down:
        camera_.focalDistance -= 1.0f;
        break;
    case Qt::Key_Left:
        camera_.focalRange -= 1.0f;
        break;
    case Qt::Key_Right:
        camera_.focalRange += 1.0f;
        break;
    }
}
