#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 avertexTexCoord;

uniform float u_offsetX;
uniform float u_offsetY;

out vec3 v_color;
out vec2 vTexCoord;

void main(){
    v_color = color;
    vTexCoord = avertexTexCoord;
    
    gl_Position = vec4(position.x + u_offsetX, position.y + u_offsetY, position.z, 1.0);
}