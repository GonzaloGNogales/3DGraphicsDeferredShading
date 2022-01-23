#version 330 core

in vec2 texCoord;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
out vec4 outColor;

const float focalDistance = -25.0;
const float maxDistanceFactor = 1.0/5.0;

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

	//Si no se están usando filtros el cálculo de color se computa de esta forma
	color += texture(colorTex, texCoord + ts * dof, 0.0);  // DOF without Filters
	
	outColor = color;
}



