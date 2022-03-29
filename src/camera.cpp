#include "camera.hpp"

namespace vke
{
  void Camera::setOrthographicProjection(float left_x, float right_x, float top_y, float bottom_y, float near_z, float far_z)
  {
    // It is expected that
    //
    //   left_x   == -width/height
    //   right_x  ==  width/height
    //   top_y    == -height/width
    //   bottom_y ==  height/width
    //

    if(right_x < 1.f)
    {
      right_x = 1.f;
      left_x = -1.f;
    }
    else
    {
      bottom_y = 1.f;
      top_y = -1.f;
    }

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

  void Camera::setPerspectiveProjection(float x_aspectRatio, float y_aspectRatio, float fov, float near_z, float far_z)
  {
    assert(glm::abs(x_aspectRatio - std::numeric_limits<float>::epsilon() > 0.f));
    assert(glm::abs(y_aspectRatio - std::numeric_limits<float>::epsilon() > 0.f));

    if(x_aspectRatio < 1.f)
    {
      x_aspectRatio = 1.f;
    }
    else
    {
      y_aspectRatio = 1.f;
    }

    // originally was fov_y
    const float tanHalf_fov{std::tan(fov / 2)};
    projectionMatrix = {
      {
        1.f / (x_aspectRatio * tanHalf_fov),
        0.f,
        0.f,
        0.f,
      },
      {
        0.f,
        1.f / (y_aspectRatio * tanHalf_fov),
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

    // f = position - direction?
    // https://songho.ca/opengl/gl_camera.html
    const glm::vec3 f{glm::normalize(direction)};         // z (f-orward)
    const glm::vec3 l{glm::normalize(glm::cross(f, up))}; // x (l-eft)
    const glm::vec3 u{glm::cross(f, l)};                  // y (u-p)

    viewMatrix = {
      {
        l.x,
        u.x,
        f.x,
        0.f, // 1.f
      },
      {
        l.y,
        u.y,
        f.y,
        0.f, // 1.f
      },
      {
        l.z,
        u.z,
        f.z,
        0.f, // 1.f
      },
      {
        -glm::dot(l, position), //  -(l.x * position.x) - (l.y * position.y) - (l.z * position.z),
        -glm::dot(u, position), //  -(u.x * position.x) - (u.y * position.y) - (u.z * position.z),
        -glm::dot(f, position), //  -(f.x * position.x) - (f.y * position.y) - (f.z * position.z),
        1.f,
      },
    };
  }

  void Camera::broken_setViewXYZ(glm::vec3 position, glm::vec3 rotation)
  {
    const float s1{glm::sin(rotation.x)};
    const float c1{glm::cos(rotation.x)};
    const float s2{glm::sin(rotation.y)};
    const float c2{glm::cos(rotation.y)};
    const float s3{glm::sin(rotation.z)};
    const float c3{glm::cos(rotation.z)};

    const glm::vec3 l{
      (c2 * c3),
      (c1 * s3 + c3 * s1 * s2),
      (s1 * s3 - (c1 * c3 * s2)),
    };

    const glm::vec3 u{
      (-c2 * s3),
      (c1 * c3 - (s1 * s2 * s3)),
      (c3 * s1 + c1 * s2 * s3),
    };

    const glm::vec3 f{
      (s2),
      ((-c2) * s1),
      (c1 * c2),
    };

    viewMatrix = {
      {
        l.x,
        u.x,
        f.x,
        1.f, // 0.f
      },
      {
        l.y,
        u.y,
        f.y,
        1.f, // 0.f
      },
      {
        l.z,
        u.z,
        f.z,
        1.f, // 0.f
      },
      {
        -glm::dot(l, position), //-(l.x * position.x) - (l.y * position.y) - (l.z * position.z),
        -glm::dot(u, position), //-(u.x * position.x) - (u.y * position.y) - (u.z * position.z),
        -glm::dot(f, position), //-(f.x * position.x) - (f.y * position.y) - (f.z * position.z),
        1.f,
      },
    };
  }

  // XYZ is broken --- https://www.youtube.com/watch?v=wFV9zPU_Cjg
  void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation)
  {
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);

    const glm::vec3 l{
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
        l.x,
        u.x,
        f.x,
        1.f,
      },
      {
        l.y,
        u.y,
        f.y,
        1.f,
      },
      {
        l.z,
        u.z,
        f.z,
        1.f,
      },
      {
        -glm::dot(l, position),
        -glm::dot(u, position),
        -glm::dot(f, position),
        1.f,
      },
    };
  }
} // namespace vke
