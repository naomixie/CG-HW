#version 330 core
out vec4 FragColor;

in vec4 vPos;

uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = vPos;
}