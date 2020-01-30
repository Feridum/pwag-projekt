#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;

uniform float houseNum;
uniform vec1 houseDataVec;

out vec2 TexGeoCoord_CS_in;

void main()
{
  gl_Position = vec4(vPosition.x, vPosition.z, vPosition.y, 1.0);

  if (houseNum != 0)
  {
    gl_Position.x += houseDataVec[0];
  }

  TexGeoCoord_CS_in = texPosition * (houseNum + houseDataVEc[0]);
}

