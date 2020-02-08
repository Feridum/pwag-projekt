#version 430 core
out vec4 FragColor;

in vec2 TexGeoCoord_FS_in;
in float Heigth_FS_in;

uniform float wallH;
uniform float Zvalue;

// texture sampler
uniform sampler2D soil;
uniform sampler2D grass;
uniform sampler2D snow;
uniform float hill_height;



void main()
{
    vec4 texel0 = texture(soil, TexGeoCoord_FS_in);
    vec4 texel1 = texture(grass, TexGeoCoord_FS_in);
	vec4 texel2 = texture(snow, TexGeoCoord_FS_in);

	float ratio = Heigth_FS_in/hill_height;

	if(Heigth_FS_in < 0.4 * hill_height){
		FragColor = mix(texel0, texel1, ratio + 0.3);
	}
	else if(Heigth_FS_in < 0.7 * hill_height){
		FragColor = mix(texel0, texel1, 0.7 - ratio);
	}
	else if(Heigth_FS_in < 0.9 * hill_height){
		FragColor = mix(texel0, texel2, ratio);
	}
	else {
		FragColor = texel2;
	}
    
}