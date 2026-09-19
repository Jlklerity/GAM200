#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 avertexTexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform mat4 u_Model;

out vec2 vTexCoord;

void main(){
    vTexCoord = avertexTexCoord;
    gl_Position = u_Projection * u_View * u_Model * vec4(position, 1.0);
}