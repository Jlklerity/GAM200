#version 300 es

precision mediump float; 

uniform sampler2D uTex2d;
uniform bool uIsModulating;
uniform int u_useTexture;

in vec4 v_color;
in vec2 vTexCoord;

out vec4 color;

void main(){
    if(u_useTexture == 1)  
    {
        vec4 texColor = texture(uTex2d, vTexCoord);
        
        if (uIsModulating) {
            texColor = vec4(texColor.rgb * v_color.rgb, texColor.a);
        }
        color = texColor;
    }
    else 
    {
        color = v_color; 
    }
}