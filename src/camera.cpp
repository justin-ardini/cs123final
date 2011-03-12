#include "camera.h"
#include <qgl.h>

Camera::Camera()
{
    eye_ = Vector4(0, 0, -2, 1);
    look_ = Vector4(-eye_.x, -eye_.y, -eye_.z, 0).getNormalized();
    up_ = Vector4(0, 1, 0, 0);
    near_ = 0.1f;
    far_ = 100.f;
    fovy_ = 60.f;
    focalDistance_ = DEFAULT_DISTANCE;
    focalRange_ = DEFAULT_RANGE;
}

Camera::~Camera()
{
}

void Camera::multMatrix()
{
    gluLookAt(eye_.x, eye_.y, eye_.z,
              eye_.x + look_.x, eye_.y + look_.y, eye_.z + look_.z,
              up_.x, up_.y, up_.z);
}

void Camera::lookAt(const Vector4 &eye, const Vector4 &look, const Vector4 &up)
{
    eye_ = eye;
    look_ = look;
    up_ = up;
}

void Camera::mouseMove(const Vector2 &delta, const Qt::MouseButtons &buttons)
{
    if (buttons == Qt::RightButton)
    {
        lookVectorRotate(delta);
    }
    else if (buttons == Qt::LeftButton)
    {
        filmPlaneTranslate(delta);
    }
}

void Camera::mouseWheel(float delta)
{
    lookVectorTranslate(delta);
}

void Camera::filmPlaneTranslate(const Vector2 &delta)
{
    eye_ += (getU() * delta.x - getV() * delta.y) * 0.01;
}

void Camera::lookVectorRotate(const Vector2 &delta)
{
    Vector4 w = getW();
    float angleX = asinf(-w.y) - delta.y * 0.0025;
    float angleY = atan2f(-w.z, -w.x) + delta.x * 0.0025;
    angleX = qMax(-M_PI / 2 + 0.001, qMin(M_PI / 2 - 0.001, (double)angleX));
    look_ = Vector4(cosf(angleY) * cosf(angleX), sinf(angleX), sinf(angleY) * cosf(angleX), 0);
}

void Camera::lookVectorTranslate(float delta)
{
    eye_ += getW() * (delta * -0.005);
}
