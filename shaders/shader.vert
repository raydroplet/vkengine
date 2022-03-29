#version 450

/*
layout(binding = 0) uniform UniformBufferObject{
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

void main()
{
  //gl_Position = vec4(push.transform * inPosition + push.offset, 0.0, 1.0);
  //gl_Position = (ubo.proj * ubo.view * ubo.model) * vec4(push.transform * inPosition + push.offset, 0.0, 1.0);
  gl_Position = vec4(inPosition, 0.0, 1.0);
  fragColor = inColor + push.color;
}
*/

layout(push_constant) uniform Push {
  mat4 transform;
//  vec2 offset;
  vec3 color;
} push;

//layout(location = 0) in vec2 inPosition;
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
  //gl_Position = vec4(push.transform * inPosition + push.offset, 0.0, 1.0);
  //gl_Position = push.transform * vec4(inPosition, 0.0f, 1.0f);

  gl_Position = push.transform * vec4(inPosition, 1.0f);
  fragColor = inColor;
}
