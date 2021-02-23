#version 330 core                               //着色器的开头总是要声明版本
layout (location = 0) in vec3 aPos;             //
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec4 vPos;                                  //输出变量，包含4个float分量的默认向量
out vec3 Normal; //Normal vectors
out vec3 FragPos;   //Actual Fragment's position   

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// main函数，每个着色器的入口点都是main函数，在这个函数中我们处理所有的输入变量，
// 并将结果输出到输出变量中。
void main()
{   
    //为了设置顶点着色器的输出，我们必须把位置数据赋值给预定义的gl_Position变量，
    //它在幕后是vec4类型的。在main函数的最后，我们将gl_Position设置的值会成为该顶点着色器的输出。
    // gl_Position = projection * view * model * vec4(aPos, 1.0);
    // vPos= gl_Position;
    // FragPos = vec3(model * vec4(aPos,1.0));
    // Normal = aNormal;

    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    gl_Position = projection * view * vec4(FragPos, 1.0);
}