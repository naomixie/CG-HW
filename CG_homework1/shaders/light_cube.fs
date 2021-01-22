#version 330 core

uniform vec3 lightCubeColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(lightCubeColor,0.8); // set all 4 vector values to 1.0
}