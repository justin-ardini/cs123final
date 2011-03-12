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

    float getFocalDistance() const { return focalDistance_; }
    float getFocalRange() const { return focalRange_; }
    Vector4 getEye() const { return eye_; }
    Vector4 getLook() const { return look_; }
    Vector4 getUp() const { return up_; }

    Vector4 getU() const { return look_.cross(up_).getNormalized(); }
    Vector4 getV() const { return getU().cross(look_).getNormalized(); }
    Vector4 getW() const { return -look_.getNormalized(); }

    void multMatrix();
    void lookAt(const Vector4 &eye, const Vector4 &look, const Vector4 &up);

    void mouseMove(const Vector2 &delta, const Qt::MouseButtons &buttons);
    void mouseWheel(float delta);

    Vector4 eye_, look_, up_;
    float near_, far_;
    float fovy_;
    float focalDistance_, focalRange_;

private:
    void filmPlaneTranslate(const Vector2 &delta);
    void lookVectorRotate(const Vector2 &delta);
    void lookVectorTranslate(float delta);

};

#endif // CAMERA_H
