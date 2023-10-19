#version 330 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

uniform mat4 scaleMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 rotateMatrix;
uniform mat4 rotateTigMatrix;
uniform mat4 translateMatrix;
uniform float dirBrightness;

out vec3 theColor;
out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;
out float brightOffset;

void main()
{
    // position
    vec4 v = vec4(position,1.0);
    vec4 out_position = projMatrix * viewMatrix * scaleMatrix * rotateMatrix * translateMatrix * rotateTigMatrix * v;
    gl_Position = out_position;
    UV = uv;
    
    // transformations
    vec4 normal_temp = viewMatrix *  scaleMatrix  * vec4(normal, 0);
    normalWorld = normal_temp.xyz;
    vec4 tempvertexPositionWorld = viewMatrix *  scaleMatrix * rotateMatrix * translateMatrix * rotateTigMatrix * v;
    vertexPositionWorld = tempvertexPositionWorld.xyz;
    brightOffset = dirBrightness * 0.1;
}
