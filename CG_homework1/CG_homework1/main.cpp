#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//#include <fstream>
#include <iostream>
#include <stdio.h>

#include "include/shader.h"
#include "include/camera.h"
#include "include/model.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void updateLight();
float calColor(float base, float range, float curVal);

const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                   "}\n\0";

// settings 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit(); // Initialize GLFW
    // glfWindowHint 配置GFLFW，这里说明了使用OpenGL3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // For MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Homework2 Naomi", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 通知GLFW将我们窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    // 注册这个函数，告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers 初始化GLAD
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    // glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("../shaders/vs.shader", "../shaders/fs.shader");
    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    //int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    //glCompileShader(vertexShader);
    //// check for shader compile errors
    //int success;
    //char infoLog[512];
    //glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}
    //// fragment shader
    //int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    //glCompileShader(fragmentShader);
    //// check for shader compile errors
    //glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}
    //// link shaders
    //int shaderProgram = glCreateProgram();
    //glAttachShader(shaderProgram, vertexShader);
    //glAttachShader(shaderProgram, fragmentShader);
    //glLinkProgram(shaderProgram);
    //// check for linking errors
    //glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    //if (!success) {
    //    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    //}
    //glDeleteShader(vertexShader);
    //glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // Reading from txt------------------------------------------------------------------------------------
    const char *filePath = "../resources/overlapping.txt";
    float triangle_one[18], triangle_two[18], triangle_three[18], triangle_four[18];
    //float color_one[9], color_two[9], color_three[9], color_four[9];
    FILE *mFile = fopen(filePath, "r");

    if (mFile == NULL)
        perror("Error opening file");
    else
    {
        int count = 0;
        while (!feof(mFile))
        {
            //fscanf(mFile, "%f %f %f ", &vertices[count], &vertices[count + 1], &vertices[count + 2]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f ", &vertices[count + 6], &vertices[count + 7], &vertices[count + 8]);   //Vertex 2 for triangle
            //fscanf(mFile, "%f %f %f ", &vertices[count + 12], &vertices[count + 13], &vertices[count + 14]);   //Vertex 3 for triangle
            //fscanf(mFile, "%f %f %f ", &vertices[count + 3], &vertices[count + 4], &vertices[count + 5]);   // Color for vertex 1
            //fscanf(mFile, "%f %f %f ", &vertices[count + 9], &vertices[count + 10], &vertices[count + 11]);   //Color for vertex 2
            //fscanf(mFile, "%f %f %f\n", &vertices[count + 15], &vertices[count + 16], &vertices[count + 17]);   //Color for vertex 3
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_one[count], &triangle_one[count + 1], &triangle_one[count + 2], &triangle_one[count + 6], &triangle_one[count + 7], &triangle_one[count + 8], &triangle_one[count + 12], &triangle_one[count + 13], &triangle_one[count + 14]);                          //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_one[count + 3], &triangle_one[count + 4], &triangle_one[count + 5], &triangle_one[count + 9], &triangle_one[count + 10], &triangle_one[count + 11], &triangle_one[count + 15], &triangle_one[count + 16], &triangle_one[count + 17]);                   //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_two[count], &triangle_two[count + 1], &triangle_two[count + 2], &triangle_two[count + 6], &triangle_two[count + 7], &triangle_two[count + 8], &triangle_two[count + 12], &triangle_two[count + 13], &triangle_two[count + 14]);                          //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_two[count + 3], &triangle_two[count + 4], &triangle_two[count + 5], &triangle_two[count + 9], &triangle_two[count + 10], &triangle_two[count + 11], &triangle_two[count + 15], &triangle_two[count + 16], &triangle_two[count + 17]);                   //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_three[count], &triangle_three[count + 1], &triangle_three[count + 2], &triangle_three[count + 6], &triangle_three[count + 7], &triangle_three[count + 8], &triangle_three[count + 12], &triangle_three[count + 13], &triangle_three[count + 14]);        //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_three[count + 3], &triangle_three[count + 4], &triangle_three[count + 5], &triangle_three[count + 9], &triangle_three[count + 10], &triangle_three[count + 11], &triangle_three[count + 15], &triangle_three[count + 16], &triangle_three[count + 17]); //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_four[count], &triangle_four[count + 1], &triangle_four[count + 2], &triangle_four[count + 6], &triangle_four[count + 7], &triangle_four[count + 8], &triangle_four[count + 12], &triangle_four[count + 13], &triangle_four[count + 14]);                 //Vertex 1 for triangle
            fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_four[count + 3], &triangle_four[count + 4], &triangle_four[count + 5], &triangle_four[count + 9], &triangle_four[count + 10], &triangle_four[count + 11], &triangle_four[count + 15], &triangle_four[count + 16], &triangle_four[count + 17]);          //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_two[count], &triangle_two[count + 1], &triangle_two[count + 2], &triangle_two[count + 3], &triangle_two[count + 4], &triangle_two[count + 5], &triangle_two[count + 6], &triangle_two[count + 7], &triangle_two[count + 8]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_two[count], &color_two[count + 1], &color_two[count + 2], &color_two[count + 3], &color_two[count + 4], &color_two[count + 5], &color_two[count + 6], &color_two[count + 7], &color_two[count + 8]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_three[count], &triangle_three[count + 1], &triangle_three[count + 2], &triangle_three[count + 3], &triangle_three[count + 4], &triangle_three[count + 5], &triangle_three[count + 6], &triangle_three[count + 7], &triangle_three[count + 8]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_three[count], &color_three[count + 1], &color_three[count + 2], &color_three[count + 3], &color_three[count + 4], &color_three[count + 5], &color_three[count + 6], &color_three[count + 7], &color_three[count + 8]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_four[count], &triangle_four[count + 1], &triangle_four[count + 2], &triangle_four[count + 3], &triangle_four[count + 4], &triangle_four[count + 5], &triangle_four[count + 6], &triangle_four[count + 7], &triangle_four[count + 8]);   //Vertex 1 for triangle
            //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_four[count], &color_four[count + 1], &color_four[count + 2], &color_four[count + 3], &color_four[count + 4], &color_four[count + 5], &color_four[count + 6], &color_four[count + 7], &color_four[count + 8]);   //Vertex 1 for triangle
            count += 9; // finished 1 triangle
        }
        fclose(mFile);
        for (int i = 0; i < 18; i++)
        {
            //cout << vertices[i] << "\t";
            //vertices[i] /= 100;
            if (i == 0 || i == 1 || i == 2 || i == 6 || i == 7 || i == 8 || i == 12 || i == 13 || i == 14)
            {
                triangle_one[i] = triangle_one[i] / 100.0f;
                triangle_two[i] = triangle_two[i] / 100.0f;
                triangle_three[i] = triangle_three[i] / 100.0f;
                triangle_four[i] = triangle_four[i] / 100.0f;
            }
            //cout << triangle_one[i] / 100.0f << "\t";/*
            //cout << triangle_two[i] / 100.0f << "\t";
            //cout << triangle_three[i] / 100.0f << "\t";
            //cout << triangle_four[i] / 100.0f << "\t";
            //cout << endl;*/
            cout << triangle_one[i] << "\t";
            cout << triangle_two[i] << "\t";
            cout << triangle_three[i] << "\t";
            cout << triangle_four[i] << "\t";
            cout << endl;
        }
    }

    float firstTriangle[] = {
        1.0f, 0.0f, 0.0f, // left
        0.0f, 1.0f, 0.0f, // right
        0.0f, 0.0f, 1.0f, // top
    };
    float secondTriangle[] = {
        1.0f, 0.0f, 0.0f, // left
        0.0f, 1.0f, 0.0f, // right
        0.0f, 0.0f, 1.0f, // top
    };
    float thirdTriangle[] = {
        1.0f, 0.0f, 0.0f, // left
        0.0f, 1.0f, 0.0f, // right
        0.0f, 0.0f, 1.0f, // top
    };
    float fourthTriangle[] = {
        1.0f, 0.0f, 0.0f, // left
        0.0f, 1.0f, 0.0f, // right
        0.0f, 0.0f, 1.0f, // top
    };
    unsigned int VBOs[4], VAOs[4], CVBOs[4], CVAOs[4];
    glGenVertexArrays(4, VAOs); // we can also generate multiple VAOs or buffers at the same time
    glGenBuffers(4, VBOs);

    // first triangle setup
    // --------------------
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_one), triangle_one, GL_STATIC_DRAW);
    //GLint colAttrib = glGetAttribLocation(ourShader.ID, "position");
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // Vertex attributes stay the same
    glEnableVertexAttribArray(0);

    //glBindVertexArray(CVAOs[0]);
    //glBindBuffer(GL_ARRAY_BUFFER, CVBOs[0]);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);
    // GLint colAttrib = glGetAttribLocation(ourShader.ID, "color");
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    // // glBindVertexArray(0); // no need to unbind at all as we directly bind a different VAO the next few lines
    // // second triangle setup
    // // ---------------------
    glBindVertexArray(VAOs[1]);             // note that we bind to a different VAO now
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]); // and a different VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_two), triangle_two, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out
    glEnableVertexAttribArray(0);

    // //glBindVertexArray(CVAOs[1]);
    // //glBindBuffer(GL_ARRAY_BUFFER, CVBOs[1]);
    // //glBufferData(GL_ARRAY_BUFFER, sizeof(color_two), color_two, GL_STATIC_DRAW);
    // colAttrib = glGetAttribLocation(ourShader.ID, "color");
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    // // glBindVertexArray(0); // not really necessary as well, but beware of calls that could affect VAOs while this one is bound (like binding element buffer objects, or enabling/disabling vertex attributes)
    // // third triangle setup
    // // --------------------
     glBindVertexArray(VAOs[2]);
     glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_three), triangle_three, GL_STATIC_DRAW);
     glEnableVertexAttribArray(0);
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);	// Vertex attributes stay the same

     glEnableVertexAttribArray(1);
     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // // fourth triangle setup
    // // ---------------------
     glBindVertexArray(VAOs[3]);	// note that we bind to a different VAO now
     glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);	// and a different VBO
     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_four), triangle_four, GL_STATIC_DRAW);
     glEnableVertexAttribArray(0);
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out

     glEnableVertexAttribArray(1);
     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //glUseProgram(shaderProgram);
        ourShader.use();
        // draw first triangle using the data from the first VAO
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        // then we draw the second triangle using the data from the second VAO
         glBindVertexArray(VAOs[1]);
         glDrawArrays(GL_TRIANGLES, 0, 3);
        // draw third triangle using the data from the first VAO
         glBindVertexArray(VAOs[2]);
         glDrawArrays(GL_TRIANGLES, 0, 3);
        // draw the fourth triangle using the data from the second VAO
         glBindVertexArray(VAOs[3]);
         glDrawArrays(GL_TRIANGLES, 0, 3);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    //glDeleteProgram(ourShader);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
// 检测特定的键是否被按下，并在每一帧做出处理。
//void processInput(GLFWwindow *window)
//{
//    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//        glfwSetWindowShouldClose(window, true);
//    //TODO
//    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//    // camera.ProcessKeyboard(FORWARD, deltaTime);
//    {
//        radius = (radius <= nearest_distance) ? nearest_distance : radius;
//        radius -= deltaTime * move_velocity;
//    }
//    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//    // camera.ProcessKeyboard(BACKWARD, deltaTime);
//    {
//        radius = (radius >= furthest_distance) ? furthest_distance : radius;
//        radius += deltaTime * move_velocity;
//    }
//
//    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//        light_angle -= deltaTime * light_velocity;
//    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//        light_angle += deltaTime * light_velocity;
//
//    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
//        lightColor.z -= deltaTime;
//    // if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
//    //     lightColor.y += deltaTime;
//    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
//        lightColor.z += deltaTime;
//}
//
//void updateLight()
//{
//    lightPos.x = light_radius * sin(light_angle);
//    lightPos.z = light_radius * cos(light_angle);
//}

//float calColor(float base, float range, float curVal)
//{
//    float baseVal = base / 255.0f;
//    // range / 255.0f = 1.0f / x => x = 1 / range * 255.0f
//    float rangeDiv = 1.0f / range * 255.0f;
//    // rangeVal = cos(curVal)/rangeDiv
//    return cos(curVal) / rangeDiv;
//}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// 当用户改变窗口的大小的时候，视口也应该被调整
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    // 通过调用glViewport函数来设置窗口的维度(Dimension)
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
//void mouse_callback(GLFWwindow *window, double xpos, double ypos)
//{
//    if (firstMouse)
//    {
//        lastX = xpos;
//        lastY = ypos;
//        firstMouse = false;
//    }
//
//    float xoffset = xpos - lastX;
//    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
//
//    lastX = xpos;
//    lastY = ypos;
//
//    camera.ProcessMouseMovement(xoffset, yoffset);
//}
//
//// glfw: whenever the mouse scroll wheel scrolls, this callback is called
//// ----------------------------------------------------------------------
//void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
//{
//    camera.ProcessMouseScroll(yoffset);
//}