#version 300 es

precision mediump float; 

uniform sampler2D uTex2d;
uniform bool uIsModulating;

in vec3 v_color;
in vec2 vTexCoord;

out vec4 color;

void main(){
    vec4 texColor = texture(uTex2d, vTexCoord);
    
    if (uIsModulating) {
        texColor = vec4(texColor.rgb * v_color, texColor.a);
    }
    color = texColor;
}