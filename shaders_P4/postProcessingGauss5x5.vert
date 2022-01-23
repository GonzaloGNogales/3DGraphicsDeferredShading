#version 330 core

layout (location = 0) in vec3 inPos;	

out vec2 texCoord;

void main()
{
	texCoord = 0.5*inPos.xy+vec2(0.5);
	gl_Position = vec4(inPos,1.0);
}
