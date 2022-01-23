#version 330 core

in vec2 texCoord;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 outColor;

const float focalDistance = -25.0;
const float maxDistanceFactor = 1.0/5.0;

#define MASK_SIZE_5x5 25u

const vec2 texIdx5x5[MASK_SIZE_5x5] = vec2[](
vec2(-2.0,2.0), vec2(-1.0,2.0), vec2(0.0,2.0), vec2(1.0,2.0), vec2(2.0,2.0),
vec2(-2.0,1.0), vec2(-1.0,1.0), vec2(0.0,1.0), vec2(1.0,1.0), vec2(2.0,1.0),
vec2(-2.0,0.0), vec2(-1.0,0.0), vec2(0.0,0.0), vec2(1.0,0.0), vec2(2.0,0.0),
vec2(-2.0,-1.0), vec2(-1.0,-1.0), vec2(0.0,-1.0), vec2(1.0,-1.0), vec2(2.0,-1.0),
vec2(-2.0,-2.0), vec2(-1.0,-2.0), vec2(0.0,-2.0), vec2(1.0,-2.0), vec2(2.0,-2.0));

uniform float mask5x5[MASK_SIZE_5x5];

void main()
{
	//Sería más rápido utilizar una variable uniform el tamaño de la textura.
	vec2 ts = vec2(1.0) / vec2(textureSize(colorTex, 0));

	float vbz = texture(depthTex, texCoord).x;
	float vcz = - (1.0 * 50.0) / (50.0 + vbz * (1.0 - 50.0));

	float dof = abs(vcz - focalDistance) * maxDistanceFactor;
	dof = clamp(dof, 0.0, 1.0);
	dof *= dof;

	vec4 color = vec4(0.0);

	for (uint i = 0u; i < MASK_SIZE_5x5; i++) {
		vec2 iidx = texCoord + ts * texIdx5x5[i] * dof;
		color += texture(colorTex, iidx, 0.0) * mask5x5[i];
	}

	outColor = color;
}

