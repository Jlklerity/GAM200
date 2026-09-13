#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 avertexTexCoord;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 v_color;
out vec2 vTexCoord;

void main(){
    v_color = color;
    vTexCoord = avertexTexCoord;
    
    gl_Position = u_Projection * u_View * vec4(position, 1.0);
}