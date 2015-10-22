// vertex shader

#version 410

in vec4 position;
//parameter passed from openFrameworks
uniform mat4 modelViewProjectionMatrix;

void main()
{   
    // send the vertices to the fragment shader
	gl_Position = modelViewProjectionMatrix * position;
}