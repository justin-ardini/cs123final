#ifndef DRAWENGINE_H
#define DRAWENGINE_H

#include <QHash>
#include <QString>
#define GL_GLEXT_LEGACY // no glext.h, we have our own
#include <qgl.h>
#include "glm.h"
#include "common.h"
#include "terrain.h"
#include "camera.h"
#include <Qt>

class QGLContext;
class QGLShaderProgram;
class QFile;
class QGLFramebufferObject;
class QKeyEvent;

struct Model {
    GLMmodel *model;
    GLuint idx;
};


static const QString TERRAIN_TEX0 = "textures/terrain/dirt.jpg";
static const QString TERRAIN_TEX1 = "textures/terrain/grass.jpg";
static const QString TERRAIN_TEX2 = "textures/terrain/rock.jpg";
static const QString TERRAIN_TEX3 = "textures/terrain/snow.jpg";


class DrawEngine {
public:

    //ctor and dtor
    DrawEngine(const QGLContext *context, int w, int h);
    ~DrawEngine();

    //methods
    Camera * getCamera() { return &camera_; }
    float getBlurSize() const { return 1.0f / blurFactor_; }
    void draw_frame(float time, int w, int h);
    void resize_frame(int w, int h);
    void mouse_wheel_event(int dx);
    void mouse_drag_event(float2 p0, float2 p1, const Qt::MouseButtons &buttons);
    void key_press_event(QKeyEvent *event);
    //getters and setters
    float fps() const { return fps_; }

protected:

    //methods
    void perspective_camera(int w, int h);
    void orthogonal_camera(int w, int h);
    void textured_quad(int w, int h, bool flip);
    void render_scene(float time, int w, int h);
    void realloc_framebuffers(int w, int h);
    void load_models();
    void load_textures();
    GLuint load_texture(const QFile &file);
    void load_shaders();
    GLuint load_cube_map(QList<QFile *> files);
    void create_fbos(int w, int h);
    void render_water();
    void render_reflections();
    void render_refraction();

    //member variables
    QHash<QString, QGLShaderProgram *> shader_programs_; // hash map of all shader programs
    QHash<QString, QGLFramebufferObject *> framebuffer_objects_; // hash map of all framebuffer objects
    QHash<QString, Model> models_; // hashmap of all models
    QHash<QString, GLuint> textures_; // hashmap of all textures
    const QGLContext *context_; // the current OpenGL context to render to
    float previous_time_, fps_; // the previous time and the fps counter
    Camera camera_; // a simple camera struct
    Terrain *terrain_;
    bool dofEnabled_; // Depth of field?
    bool depthmapEnabled_; // Show depth map?
    float offsetX_, offsetY_;
    GLuint bumpMap_;
    float blurFactor_;
};

#endif // DRAWENGINE_H
