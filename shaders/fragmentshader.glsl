#version 300 es

precision mediump float; 

uniform sampler2D uTex2d;
uniform vec4 u_Color;
uniform int u_useTexture;

in vec2 vTexCoord;

out vec4 color;

void main(){
    if (u_useTexture == 1) {
        vec4 texColor = texture(uTex2d, vTexCoord);
        color = vec4(texColor.rgb * u_Color.rgb, texColor.a * u_Color.a);
    } else {
        color = u_Color;
    }
}