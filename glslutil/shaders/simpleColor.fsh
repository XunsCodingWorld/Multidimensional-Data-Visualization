#version 420 core

// simpleColor.fsh

// Input from vertex shader programs:
in vec3 colorToFS;

// and output:
out vec4 fragColor;

void main (void) 
{
	fragColor = vec4(colorToFS, 1.0);
}
