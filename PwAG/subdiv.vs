#version 430 core
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 texPosition;

uniform int houseNum;
//uniform vec3 houseFirstPart[2];
//uniform vec4 houseSecoundPart[2];
//uniform float params[3];
//uniform vec4 t[2];

out vec2 TexGeoCoord_CS_in;
out vec3 HouseCoord_CS_in_F[10];
out vec4 HouseCoord_CS_in_S[10];

void main()
{
  gl_Position = vec4(vPosition.x, vPosition.z, vPosition.y, 1.0);


  TexGeoCoord_CS_in = texPosition;// + t[1].x);//houseFirstPart[0][0] + houseSecoundPart[0][0]);

  if (houseNum > 0)
  {
    for (int i = 0; i < houseNum; i++)
    {
        HouseCoord_CS_in_F[i] = vec3(0.0, 0.0, 0.0);
        HouseCoord_CS_in_S[i] = vec4(1.0, 2.0, 0.0, 0.5);
    }
  }
}

//{ 0.0f, 0.0f, 0.0f, 5.0f, 10.0f, 0.0f, 5.0f }