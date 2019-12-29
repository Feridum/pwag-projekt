#version 430 core
out vec4 FragColor;

in vec2 TexGeoCoord_FS_in;

// texture sampler
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	vec4 texel0 = texture(texture1, TexGeoCoord_FS_in);
    vec4 texel1 = texture(texture2, TexGeoCoord_FS_in);
	FragColor = mix(texel0, texel1, 0.3);
}