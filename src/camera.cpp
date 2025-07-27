#include "camera.hpp"

namespace vke
{
  void Camera::setOrthographicProjection(float left_x, float right_x, float top_y, float bottom_y, float near_z, float far_z)
  {
    projectionMatrix = {
      {
        2.f / (right_x - left_x),
        0.f,
        0.f,
        0.f,
      },
      {
        0.f,
        2.f / (bottom_y - top_y), // negative y points up
        0.f,
        0.f,
      },
      {
        0.f,
        0.f,
        1.f / (far_z - near_z),
        0.f,
      },
      {
        -(right_x + left_x) / (right_x - left_x),
        -(bottom_y + top_y) / (bottom_y - top_y), // negative y points up
        -near_z / (far_z - near_z),
        1.f,
      },
    };
  }

  void Camera::setOrthographicProjection(float wh_aspectRatio, float near_z, float far_z)
  {
    // for  left_x  == -wh_aspectRatio // (w/h)
    // and  right_x == wh_aspectRatio
    //  top_y should be -1 and
    //  bottom_y should be 1
    //
    // (right_x - left_x) / (bottom_y - top_y) == (width) / (height)
    // (right_x - left_x) == wh_ar * (bottom_y - top_y) // -> w = wh_ar * h
    // (wh_ar -(-aspectRatio)) == aspectRatio * (1 -(-1))
    // (wh_ar + aspectRatio) == aspectRatio * (1 + 1)
    // 2 * wh_ar == aspectRatio * 2
    //
    // a similar ideia can be applied to the height
    // (bottom_y - top_y) / hw_ar * (right_x - left_x) // -> h = hw_ar * w

    float left_x{-wh_aspectRatio};
    float right_x{wh_aspectRatio};
    float top_y{-1.f};
    float bottom_y{1.f};

    if(wh_aspectRatio < 1.f) {
      right_x = 1.f;
      left_x = -1.f;
      top_y = (1 / -wh_aspectRatio);   // convert to hw_ar
      bottom_y = (1 / wh_aspectRatio); // convert to hw_ar
    }

    setOrthographicProjection(left_x, right_x, top_y, bottom_y, near_z, far_z);
  }

  void Camera::setPerspectiveProjection(float fovY, float wh_aspectRatio, float near_z, float far_z)
  {
    assert(glm::abs(wh_aspectRatio - std::numeric_limits<float>::epsilon() > 0.f));

    float hw_aspectRatio{1.f}; // hw -> height/width
    if(wh_aspectRatio - std::numeric_limits<float>::epsilon() < 1.0f) {
      hw_aspectRatio = 1 / wh_aspectRatio; // convert to hw_ar
      wh_aspectRatio = 1.0f;
    }

    const float tanHalfFovY{std::tan(fovY / 2)};
    projectionMatrix = {
      {
        1.f / (wh_aspectRatio * tanHalfFovY),
        0.f,
        0.f,
        0.f,
      },
      {
        0.f,
        1.f / (hw_aspectRatio * tanHalfFovY),
        0.f,
        0.f,
      },
      {
        0.f,
        0.f,
        far_z / (far_z - near_z),
        1.f,
      },
      {
        0.f,
        0.f,
        -(far_z * near_z) / (far_z - near_z),
        0.f,
      },
    };
  }

  void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
  {
    assert("Null direction" && direction != glm::vec3{0.f});

    const glm::vec3 f{glm::normalize(direction)};         // z (f-orward)
    const glm::vec3 r{glm::normalize(glm::cross(f, up))}; // x (r-ight)
    const glm::vec3 u{glm::cross(f, r)};                  // y (u-p)

    viewMatrix = {
      {
        r.x,
        u.x,
        f.x,
        0.f,
      },
      {
        r.y,
        u.y,
        f.y,
        0.f,
      },
      {
        r.z,
        u.z,
        f.z,
        0.f,
      },
      {
        glm::dot(r, -position), // (r.x * -position.x) + (r.y * -position.y) + (r.z * -position.z),
        glm::dot(u, -position), // (u.x * -position.x) + (u.y * -position.y) + (u.z * -position.z),
        glm::dot(f, -position), // (f.x * -position.x) + (f.y * -position.y) + (f.z * -position.z),
        1.f,
      },
    };
  }

  void Camera::setViewRotationXYZ(glm::vec3 position, glm::vec3 rotation)
  {
    const float s1{glm::sin(rotation.x)};
    const float c1{glm::cos(rotation.x)};
    const float s2{glm::sin(rotation.y)};
    const float c2{glm::cos(rotation.y)};
    const float s3{glm::sin(rotation.z)};
    const float c3{glm::cos(rotation.z)};

    const glm::vec3 r{
      (c2 * c3),
      (c1 * s3 + c3 * s1 * s2),
      (s1 * s3 - c1 * c3 * s2),
    };

    const glm::vec3 u{
      (-c2 * s3),
      (c1 * c3 - s1 * s2 * s3),
      (c3 * s1 + c1 * s2 * s3),
    };

    const glm::vec3 f{
      (s2),
      (-c2 * s1),
      (c1 * c2),
    };

    viewMatrix = {
      {
        r.x,
        u.x,
        f.x,
        0.f,
      },
      {
        r.y,
        u.y,
        f.y,
        0.0f,
      },
      {
        r.z,
        u.z,
        f.z,
        0.0f,
      },
      {
        -glm::dot(r, position), //-(l.x * position.x) - (l.y * position.y) - (l.z * position.z),
        -glm::dot(u, position), //-(u.x * position.x) - (u.y * position.y) - (u.z * position.z),
        -glm::dot(f, position), //-(f.x * position.x) - (f.y * position.y) - (f.z * position.z),
        1.f,
      },
    };
  }

  void Camera::setViewRotationZYX(glm::vec3 position, glm::vec3 rotation)
  {
    const float s1{glm::sin(rotation.z)};
    const float c1{glm::cos(rotation.z)};
    const float s2{glm::sin(rotation.y)};
    const float c2{glm::cos(rotation.y)};
    const float s3{glm::sin(rotation.x)};
    const float c3{glm::cos(rotation.x)};

    const glm::vec3 r{
      (c1 * c2),
      (c2 * s1),
      -(s2),
    };

    const glm::vec3 u{
      (c1 * s2 * s3) - (c3 * s1),
      (c1 * c3) + (s1 * s2 * s3),
      (c2 * s3),
    };

    const glm::vec3 f{
      (s1 * s3) + (c1 * c3 * s2),
      (c3 * s1 * s2) - (c1 * s3),
      (c2 * c3),
    };

    viewMatrix = {
      {
        r.x,
        u.x,
        f.x,
        0.f,
      },
      {
        r.y,
        u.y,
        f.y,
        0.0f,
      },
      {
        r.z,
        u.z,
        f.z,
        0.0f,
      },
      {
        -glm::dot(r, position), //-(l.x * position.x) - (l.y * position.y) - (l.z * position.z),
        -glm::dot(u, position), //-(u.x * position.x) - (u.y * position.y) - (u.z * position.z),
        -glm::dot(f, position), //-(f.x * position.x) - (f.y * position.y) - (f.z * position.z),
        1.f,
      },
    };
  }

  void Camera::setViewRotationYXZ(glm::vec3 position, glm::vec3 rotation)
  {
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);

    const glm::vec3 r{
      (c1 * c3 + s1 * s2 * s3),
      (c2 * s3),
      (c1 * s2 * s3 - c3 * s1)};

    const glm::vec3 u{
      (c3 * s1 * s2 - c1 * s3),
      (c2 * c3),
      (c1 * c3 * s2 + s1 * s3)};

    const glm::vec3 f{
      (c2 * s1),
      (-s2),
      (c1 * c2)};

    viewMatrix = {
      {
        r.x,
        u.x,
        f.x,
        0.f,
      },
      {
        r.y,
        u.y,
        f.y,
        0.f,
      },
      {
        r.z,
        u.z,
        f.z,
        0.f,
      },
      {
        -glm::dot(r, position),
        -glm::dot(u, position),
        -glm::dot(f, position),
        1.f,
      },
    };
  }
} // namespace vke
