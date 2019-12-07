#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;

out vec2 TexGeoCoord_CS_in;

void main()
{
  gl_Position = vec4(vPosition.x, vPosition.z, vPosition.y, 1.0);

  TexGeoCoord_CS_in = texPosition;
}

