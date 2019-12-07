#version 430 core
out vec4 FragColor;

in vec2 TexGeoCoord_FS_in;

// texture sampler
uniform sampler2D texture1;

void main()
{
	FragColor = texture(texture1, TexGeoCoord_FS_in);
	//FragColor = vec4(1.0,0.0,0.0,1.0);
}