#version 430 core
out vec4 FragColor;

in vec2 TexGeoCoord_FS_in;
in vec4 HousePosition_FS_in;
uniform float wallH;
uniform float Zvalue;



// texture sampler
uniform sampler2D texture3;
uniform sampler2D texture4;

void main()
{
	vec4 texel2 = texture(texture3, TexGeoCoord_FS_in);
	vec4 texel3 = texture(texture4, TexGeoCoord_FS_in);
	
	if (Zvalue+wallH > HousePosition_FS_in.y)
	{
		FragColor = texel3;	//soil -> wall
	}
	else
	{
		FragColor = texel2;	//grass -> roof
	}
	//FragColor = mix(texel0, texel1, 0.3);


}