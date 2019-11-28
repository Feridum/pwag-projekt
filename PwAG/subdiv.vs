#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;

out vec2 TexGeoCoord;

void main()
{
  gl_Position = vec4(vPosition, 1.0);

  TexGeoCoord = texPosition;
}

