#include "camera.h"
#include <qgl.h>

Camera::Camera()
{
    m_eye = Vector4(0, 0, -2, 1);
    m_look = Vector4(-m_eye.x, -m_eye.y, -m_eye.z, 0).getNormalized();
    m_up = Vector4(0, 1, 0, 0);
    m_near = 0.1f;
    m_far = 100.f;
    m_fovy = 60.f;
    m_focalDistance = DEFAULT_DISTANCE;
    m_focalRange = DEFAULT_RANGE;
}

Camera::~Camera()
{
}

void Camera::multMatrix()
{
    gluLookAt(m_eye.x, m_eye.y, m_eye.z,
              m_eye.x + m_look.x, m_eye.y + m_look.y, m_eye.z + m_look.z,
              m_up.x, m_up.y, m_up.z);
}

void Camera::lookAt(const Vector4 &eye, const Vector4 &look, const Vector4 &up)
{
    m_eye = eye;
    m_look = look;
    m_up = up;
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
    m_eye += (getU() * delta.x - getV() * delta.y) * 0.01;
}

void Camera::lookVectorRotate(const Vector2 &delta)
{
    Vector4 w = getW();
    float angleX = asinf(-w.y) - delta.y * 0.0025;
    float angleY = atan2f(-w.z, -w.x) + delta.x * 0.0025;
    angleX = qMax(-M_PI / 2 + 0.001, qMin(M_PI / 2 - 0.001, (double)angleX));
    m_look = Vector4(cosf(angleY) * cosf(angleX), sinf(angleX), sinf(angleY) * cosf(angleX), 0);
}

void Camera::lookVectorTranslate(float delta)
{
    m_eye += getW() * (delta * -0.005);
}
