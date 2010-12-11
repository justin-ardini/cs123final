#ifndef CAMERA_H
#define CAMERA_H

#include "CS123Algebra.h"
#include <QMouseEvent>
#include <QWheelEvent>

#define DEFAULT_DISTANCE 0.0f
#define DEFAULT_RANGE 40.0f

class Camera
{
public:
    Camera();
    ~Camera();

    float getFocalDistance() const { return m_focalDistance; }
    float getFocalRange() const { return m_focalRange; }
    Vector4 getEye() const { return m_eye; }
    Vector4 getLook() const { return m_look; }
    Vector4 getUp() const { return m_up; }

    Vector4 getU() const { return m_look.cross(m_up).getNormalized(); }
    Vector4 getV() const { return getU().cross(m_look).getNormalized(); }
    Vector4 getW() const { return -m_look.getNormalized(); }

    void multMatrix();
    void lookAt(const Vector4 &eye, const Vector4 &look, const Vector4 &up);

    void mouseMove(const Vector2 &delta, const Qt::MouseButtons &buttons);
    void mouseWheel(float delta);

    Vector4 m_eye, m_look, m_up;
    float m_near, m_far;
    float m_fovy;
    float m_focalDistance, m_focalRange;

private:
    void filmPlaneTranslate(const Vector2 &delta);
    void lookVectorRotate(const Vector2 &delta);
    void lookVectorTranslate(float delta);

};

#endif // CAMERA_H
