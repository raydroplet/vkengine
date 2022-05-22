#include "components.hpp"

namespace vke::cmp
{
  glm::mat2 Transform2D::mat2() const
  {
    const float sin = glm::sin(rotation);
    const float cos = glm::cos(rotation);

    glm::mat2 rotationMat{{cos, sin}, {-sin, cos}};
    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};

    return rotationMat * scaleMat;
  }

  // use the Transform3D values to build a 4x4 affine transformation matrix.
  // translation * Rx * Ry * Rz * scale <--- in that order (right-left)
  glm::mat4 Transform3D::mat4() const
  {
    auto transform{glm::translate(glm::mat4{1.f}, translation)}; //creates a 4x4 translation matrix
    transform = glm::rotate(transform, rotation.x, {1.f, 0.f, 0.f});
    transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
    transform = glm::rotate(transform, rotation.z, {0.f, 0.f, 1.f});
    transform = glm::scale(transform, scale);

    return transform;
  }

  glm::mat4 Transform3D::optimized_mat4() const
  {
    // extrinsic rotation
    const float s1{glm::sin(rotation.x)};
    const float c1{glm::cos(rotation.x)};
    const float s2{glm::sin(rotation.y)};
    const float c2{glm::cos(rotation.y)};
    const float s3{glm::sin(rotation.z)};
    const float c3{glm::cos(rotation.z)};

    return glm::mat4{
      {
        scale.x * (c2 * c3),
        scale.x * (c1 * s3 + c3 * s1 * s2),
        scale.x * (s1 * s3 - c1 * c3 * s2),
        0.0f,
      },
      {
        scale.y * (-c2 * s3),
        scale.y * (c1 * c3 - s1 * s2 * s3),
        scale.y * (c3 * s1 + c1 * s2 * s3),
        0.0f,
      },
      {
        scale.z * (s2),
        scale.z * (-c2 * s1),
        scale.z * (c1 * c2),
        0.0f,
      },
      {
        translation.x,
        translation.y,
        translation.z,
        1.0f,
      },
    };
  }

  glm::mat3 Transform3D::normalMatrix()
  {
    // normal matrix (keep rotations, invert scales)
    const float s1{glm::sin(rotation.x)};
    const float c1{glm::cos(rotation.x)};
    const float s2{glm::sin(rotation.y)};
    const float c2{glm::cos(rotation.y)};
    const float s3{glm::sin(rotation.z)};
    const float c3{glm::cos(rotation.z)};
    const glm::vec3 invScale{1.f / scale};

    return glm::mat3{
      {
        invScale.x * (c2 * c3),
        invScale.x * (c1 * s3 + c3 * s1 * s2),
        invScale.x * (s1 * s3 - c1 * c3 * s2),
      },
      {
        invScale.y * (-c2 * s3),
        invScale.y * (c1 * c3 - s1 * s2 * s3),
        invScale.y * (c3 * s1 + c1 * s2 * s3),
      },
      {
        invScale.z * (s2),
        invScale.z * (-c2 * s1),
        invScale.z * (c1 * c2),
      },
    };
  }

  Color::operator glm::vec3()
  {
    return color;
  }
} // namespace vke::component
