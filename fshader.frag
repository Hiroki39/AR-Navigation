/*****************************
 * File: fshader.glsl
 *       A simple fragment shader
 *****************************/

#version 150  // Comment/un-comment this line to resolve compilation errors
              // due to different settings of the default GLSL version

in vec4 color;
out vec4 fColor;

void main() { fColor = color; }
