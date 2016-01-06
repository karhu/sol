#include "Transform2.hpp"

#include <nanovg.h>

Transform2 Transform2::Identity()
{
    Transform2 t;
    t.m_data[0] = 1;
    t.m_data[1] = 0;
    t.m_data[2] = 0;
    t.m_data[3] = 1;
    t.m_data[4] = 0;
    t.m_data[5] = 0;
    return t;
}

Transform2 Transform2::TSR(vec2f translation, vec2f scale, float rot_d)
{
    Transform2 t = Transform2::Identity();

    float tmp[6];

    nvgTransformRotate(tmp,nvgDegToRad(rot_d));
    nvgTransformPremultiply(t.m_data,tmp);
    nvgTransformScale(tmp,scale.x,scale.y);
    nvgTransformPremultiply(t.m_data,tmp);
    nvgTransformTranslate(tmp,translation.x,translation.y);
    nvgTransformPremultiply(t.m_data,tmp);
    return t;
}

Transform2 Transform2::Translation(vec2f delta)
{
    Transform2 t;
    nvgTransformTranslate(t.data(),delta.x,delta.y);
    return t;
}

Transform2 Transform2::Rotation(float angle_d)
{
    Transform2 t;
    nvgTransformRotate(t.data(),nvgDegToRad(angle_d));
    return t;
}

Transform2 Transform2::Scale(vec2f scale)
{
    Transform2 t;
    nvgTransformScale(t.data(),scale.x,scale.y);
    return t;
}

Transform2 Transform2::Scale(float s)
{
    Transform2 t;
    nvgTransformScale(t.data(),s,s);
    return t;
}

Transform2 Transform2::inverse() const
{
    Transform2 result;
    nvgTransformInverse(result.m_data,m_data);
    return result;
}

Transform2 Transform2::operator*(const Transform2 &next) const
{
    Transform2 result = *this;
    nvgTransformMultiply(result.m_data, next.m_data);
    return result;
}

const float *Transform2::data() const
{
    return m_data;
}

float *Transform2::data()
{
    return m_data;
}


vec2f transform_point(const vec2f &point, const Transform2 &t)
{
    vec2f result;
    nvgTransformPoint(
       &result.x, &result.y,
       t.data(),point.x,point.y);
    return result;
}
