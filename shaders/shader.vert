#version 450

/*
   layout(binding = 0) uniform UniformBufferObject{
   mat4 model;
   mat4 view;
   mat4 proj;
   } ubo;

   void main()
   {
//gl_Position = vec4(push.modelMatrix * inPosition + push.offset, 0.0, 1.0);
//gl_Position = (ubo.proj * ubo.view * ubo.model) * vec4(push.modelMatrix * inPosition + push.offset, 0.0, 1.0);
gl_Position = vec4(inPosition, 0.0, 1.0);
fragColor = inColor + push.color;
}
 */

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
  mat4 projectionViewMatrix;
  vec3 directionToLight;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix; // mat4 instead of mat3 because of alignment
} push;

const float LIGHT_INTENSITY = 0.25f;
const float AMBIENT_ILLUMINATION = 0.03f;

#if 0
void uniformLambertianDiffuse() // for uniform scaling
{
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(inPosition, 1.0f);

  vec3 normalWorldSpace = normalize(modelMatrix * inNormal);
  float cosAngIncidence = clamp(dot(normalWorldSpace, ubo.directionToLight), 0, 1);

  fragColor = (LIGHT_INTENSITY * inColor * cosAngIncidence) + (inColor * AMBIENT_ILLUMINATION);
}

void nonUniformLambertianDiffuse() // for non-uniform scaling
{
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(inPosition, 1.0f);

  mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
  vec3 normalWorldSpace = normalize(normalMatrix * inNormal);
  float cosAngIncidence = clamp(dot(normalWorldSpace, ubo.directionToLight), 0, 1);

  fragColor = (LIGHT_INTENSITY * inColor * cosAngIncidence) + (inColor * AMBIENT_ILLUMINATION);
}

// incomplete
void pointLight()
{
  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(inPosition, 1.0f);
  mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
  vec3 normalWorldSpace = normalize(normalMatrix * inNormal);
  vec3 dirToLight = normalize(inLightPos - normalWorldSpace);
  float cosAngIncidence = clamp(dot(normalWorldSpace, dirToLight), 0, 1);

  fragColor = (LIGHT_INTENSITY * inColor * cosAngIncidence) + (inColor * AMBIENT_ILLUMINATION);
}
#endif

void normalMatrixlambertian()
{
  //gl_Position = push.transform * vec4(inPosition, 1.0f);

  //vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * inNormal);
  //float cosAngIncidence = clamp(dot(normalWorldSpace, DIRECTION_TO_LIGHT), 0, 1);

  //fragColor = (LIGHT_INTENSITY * inColor * cosAngIncidence) + (inColor * AMBIENT_ILLUMINATION);

  gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(inPosition, 1.0f);

  vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * inNormal);
  float cosAngIncidence = clamp(dot(normalWorldSpace, ubo.directionToLight), 0, 1);

  fragColor = (LIGHT_INTENSITY * inColor * cosAngIncidence) + (inColor * AMBIENT_ILLUMINATION);
}

void main()
{
  normalMatrixlambertian();
}
