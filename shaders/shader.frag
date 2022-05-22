#version 450

layout (location = 0) in vec3 inFragColor;
layout (location = 1) in vec3 inFragPosWorld;
layout (location = 2) in vec3 inFragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor; // w is intensity
  vec3 lightPosition;
  vec4 lightColor;
  vec4 cameraPosition;
} ubo;

layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

float lightAttenuation(vec3 dirToLight, in float intensity)
{
  float attenuationValue = 1.0;
  float distanceToLight = dot(dirToLight, dirToLight);
  return intensity / (1.0 + attenuationValue * distanceToLight);
}

float phongTerm(vec3 directionFromLight, vec3 surfaceNormal, float cosAngIncidence)
{
  float shininessFactor = 1.0;
  vec3 viewDirection = normalize(ubo.cameraPosition.xyz - inFragPosWorld); // direction to camera
  vec3 reflectDir = reflect(directionFromLight, surfaceNormal);
  float phongTerm = dot(viewDirection, reflectDir);
  phongTerm = clamp(phongTerm, 0, 1);
  phongTerm = cosAngIncidence != 0.0 ? phongTerm : 0.0;
  phongTerm = pow(phongTerm, shininessFactor);

  return phongTerm;
}

float blinnTerm(vec3 directionToLight, vec3 surfaceNormal, float cosAngIncidence)
{
  float shininessFactor = 23.0;
  vec3 viewDirection = normalize(ubo.cameraPosition.xyz - inFragPosWorld); // direction to camera
  vec3 halfAngle = normalize(directionToLight + viewDirection);
  float blinnTerm = dot(surfaceNormal, halfAngle);
  blinnTerm = clamp(blinnTerm, 0, 1);
  blinnTerm = cosAngIncidence != 0.0 ? blinnTerm : 0.0;
  blinnTerm = pow(blinnTerm, shininessFactor);

  return blinnTerm;
}

void main()
{
  vec3 dirToLight = ubo.lightPosition - inFragPosWorld;
  vec3 surfaceNormal = normalize(inFragNormalWorld);
  float cosAngIncidence = max(dot(inFragNormalWorld, normalize(dirToLight)), 0);

  float attenIntensity = lightAttenuation(dirToLight, ubo.lightColor.w);
  float phongTerm = phongTerm(-dirToLight, surfaceNormal, cosAngIncidence);
  float blinnTerm = blinnTerm(dirToLight, surfaceNormal, cosAngIncidence);

  vec3 lightColor = vec3(ubo.lightColor);
  vec3 diffuseLight = lightColor * attenIntensity * cosAngIncidence;
  vec3 ambientLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; // apply intensity

  vec3 diffuseColor = inFragColor * diffuseLight;
  vec3 specularColor = vec3(1.00, 1.00, 1.00) * attenIntensity * blinnTerm;
  vec3 ambientColor = inFragColor * ambientLight;

  if(gl_FrontFacing)
    outColor = vec4((diffuseColor + specularColor + ambientColor), 1.0);
  else
    outColor = vec4((ambientColor), 1.0);
}
