#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "include/shader.h"
#include "include/camera.h"
#include "include/model.h"

#pragma region OldFunctions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void updateLight();
static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
float calColor(float base, float range, float curVal);
#pragma endregion

// void RenderCylinder();

void showCylinder();

#pragma region OldVars
// settings 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera 摄像机位置
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0, 10.0, 0.0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float radius = 10.0f;                  //Initial radius for camera rotation
const float nearest_distance = 5.0f;   //Nearest distance for camera rotation
const float furthest_distance = 25.0f; //Furthest distance for camera rotation
const float move_velocity = 3.0f;      // speed of camera movement

// timing 记录帧与帧之间的时间差
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Lighting Position
float const light_radius = 8.0f;        // radius of light_object on the y = 5.0f coplanar
float const light_velocity = 2.0f;      // velocity of light_object
float light_angle = 0.0f;               // angle of light and axis
glm::vec3 lightPos(1.0f, 1.0f, 1.0f);   // initial position(r,5,0)
glm::vec3 lightColor(0.0f, 0.0f, 0.0f); // Record the RGB of the light

#pragma endregion

#define STEP 6
#define SAMPLE_CNT (360 / STEP)

#define INITIAL_RADIUS 0.2f
#define INITIAL_LENGTH 0.8f

map<GLfloat, GLfloat> cylinder_base_line;
typedef map<GLfloat, GLfloat>::iterator map_iterator;
typedef map<GLfloat, GLfloat>::reverse_iterator rmap_iterator;

int main()
{

#pragma region Initializaion

    glfwInit(); // Initialize GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Final Project Naomi", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // 当鼠标移动时调用这个函数
    // glfwSetCursorPosCallback(window, mouse_callback);
    // 当鼠标Scroll的时候调用这个函数
    // glfwSetScrollCallback(window, scroll_callback);
    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    //stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

#pragma endregion

#pragma region Shaders and Models

    // build and compile shaders
    Shader ourShader("../shaders/vs.shader", "../shaders/fs.shader");
    Shader lightCubeShader("../shaders/light_cube.vs", "../shaders/light_cube.fs");

    // load models
    Model ourModel("../resources/knife.obj");

#pragma endregion

#pragma region Old Cubes

    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

#pragma endregion

#pragma region Draw Cylinder
    // Insert the first 2 points on the base line
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(-0.2f, 0.1f));
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(0.2f, 0.1f));
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(0.0f, 0.05f));

    map_iterator iter = cylinder_base_line.begin();
    map_iterator iter2 = cylinder_base_line.begin();
    iter2++;

    // STEP = 每几度取一个点， 360/STEP = 一圈要几个点 ， 360/STEP*cylinder_base_line size = 整个圆柱体要取的点。
    GLfloat vertex_list[2 * (360 / STEP)][3];
    GLfloat face_vertex[SAMPLE_CNT + 1][3];
    GLfloat end_vertex[SAMPLE_CNT + 1][3];
    // GLfloat color_list[2 * 360 / STEP][3];
    GLuint index_list[360 / STEP * 3];

    GLuint face_index[SAMPLE_CNT * 3 / 2];
    GLuint end_index[SAMPLE_CNT * 3 / 2];

    for (int i = 0, j = 0; i < SAMPLE_CNT; i += 2, j++)
    {
        float p = i * STEP * 3.14 / 180;
        face_vertex[j][0] = cos(p) * iter->second;
        face_vertex[j][1] = sin(p) * iter->second;
        face_vertex[j][2] = iter->first;
        printf("face_vertex[%i][0] = %f \t face_vertex[%i][1] = %f \t face_vertex[%i][2] = %f \n",
               j, cos(p) * iter->second,
               j, sin(p) * iter->second,
               j, iter->first);
    }
    face_vertex[SAMPLE_CNT][0] = 0.0f;
    face_vertex[SAMPLE_CNT][1] = 0.0f;
    face_vertex[SAMPLE_CNT][2] = iter->first;
    for (int i = 0, j = 0; i < SAMPLE_CNT * 3 / 2; i += 3, j++)
    {
        face_index[i] = j;
        face_index[i + 1] = j + 1;
        face_index[i + 2] = SAMPLE_CNT;

        if (j == SAMPLE_CNT / 2 - 1)
        {
            face_index[i] = 0;
            face_index[i + 1] = SAMPLE_CNT;
            face_index[i + 2] = j;
        }

        printf("face_index[%i] = %i \t face_index[%i] = %i \t face_index[%i] = %i \n",
               i, face_index[i], i + 1, face_index[i + 1], i + 2, face_index[i + 2]);
    }

    unsigned int FVAO, FVBO, FEBO;
    glGenVertexArrays(1, &FVAO);
    glGenBuffers(1, &FVBO);
    glGenBuffers(1, &FEBO);

    glBindVertexArray(FVAO);

    glBindBuffer(GL_ARRAY_BUFFER, FVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(face_vertex), face_vertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_index), face_index, GL_STATIC_DRAW);

    /*glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_list), vertex_list);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_list), sizeof(color_list), color_list);*/

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertex_list)));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    rmap_iterator rend = cylinder_base_line.rbegin();

    for (int i = 0, j = 0; i < SAMPLE_CNT; i += 2, j++)
    {
        float p = i * STEP * 3.14 / 180;
        end_vertex[j][0] = cos(p) * rend->second;
        end_vertex[j][1] = sin(p) * rend->second;
        end_vertex[j][2] = rend->first;
        printf("end_vertex[%i][0] = %f \t end_vertex[%i][1] = %f \t end_vertex[%i][2] = %f \n",
               j, cos(p) * rend->second,
               j, sin(p) * rend->second,
               j, rend->first);
    }
    end_vertex[SAMPLE_CNT][0] = 0.0f;
    end_vertex[SAMPLE_CNT][1] = 0.0f;
    end_vertex[SAMPLE_CNT][2] = rend->first;
    for (int i = 0, j = 0; i < SAMPLE_CNT * 3 / 2; i += 3, j++)
    {
        end_index[i] = j;
        end_index[i + 1] = j + 1;
        end_index[i + 2] = SAMPLE_CNT;

        if (j == SAMPLE_CNT / 2 - 1)
        {
            end_index[i] = 0;
            end_index[i + 1] = SAMPLE_CNT;
            end_index[i + 2] = j;
        }

        printf("end_index[%i] = %i \t end_index[%i] = %i \t end_index[%i] = %i \n",
               i, end_index[i], i + 1, end_index[i + 1], i + 2, end_index[i + 2]);
    }

    unsigned int EVAO, EVBO, EEBO;
    glGenVertexArrays(1, &EVAO);
    glGenBuffers(1, &EVBO);
    glGenBuffers(1, &EEBO);

    glBindVertexArray(EVAO);

    glBindBuffer(GL_ARRAY_BUFFER, EVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(end_vertex), end_vertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(end_index), end_index, GL_STATIC_DRAW);

    /*glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_list), vertex_list);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_list), sizeof(color_list), color_list);*/

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertex_list)));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /*unsigned int ebo;
        glEnableVertexAttribArray(1);
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_list), index_list, GL_STATIC_DRAW);*/

    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * SAMPLE_CNT * (cylinder_base_line.size() - 1));
    /* 画圆柱顶面和底面,底面使用索引偏移量的方式绘制*/

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#pragma endregion

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame; // 两个帧之间的时间差
        lastFrame = currentFrame;
        // printf("deltaFrame = %f \n", deltaTime);

        processInput(window);
        // updateLight();

        // render
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 让camera旋转

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // Set Object Color
        // GLfloat red_value = 150.0f / 255.0f + cos(currentFrame) / 5;
        // GLfloat blue_value = 150.0f / 255.0f + sin(currentFrame) / 5;

        // printf("red_val = %f \t blue_val = %f\n", red_value, blue_value);

        ourShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrixR(glm::vec3(0.0, 2.0, 0.0)); // Set Center of Model is 2.0 instead of 0.0
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        // model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // it's a bit too big for our scene, so scale it down

        map_iterator iter = cylinder_base_line.begin();
        map_iterator iter2 = cylinder_base_line.begin();
        iter2++;
        for (; iter2 != cylinder_base_line.end(); iter2++, iter++)
        {

            for (int i = 0; i < SAMPLE_CNT; i += 2)
            {
                float p = i * STEP * 3.14 / 180;           //一步多少弧度
                vertex_list[i][0] = cos(p) * iter->second; //半径乘以角度
                vertex_list[i][1] = sin(p) * iter->second;
                vertex_list[i][2] = iter->first; //x轴上的位置

                vertex_list[i + 1][0] = cos(p) * iter2->second;
                vertex_list[i + 1][1] = sin(p) * iter2->second;
                vertex_list[i + 1][2] = iter2->first;

                // printf("vertex_list[%i][0] = %f \t vertex_list[%i][1] = %f \t vertex_list[%i][2] = %f \n", i, cos(p) * iter->second,
                //        i, sin(p) * iter->second,
                //        i, iter->first);

                // printf("vertex_list[%i][0] = %f \t vertex_list[%i][1] = %f \t vertex_list[%i][2] = %f \n", i + 1, cos(p) * iter2->second,
                //        i + 1, sin(p) * iter2->second,
                //        i + 1, iter2->first);
            }

            // for (int i = 0; i < SAMPLE_CNT * 2 * (cylinder_base_line.size() - 1); i++)
            // {
            //     color_list[i][0] = color_list[i][1] = color_list[i][2] = 0.8f;
            // }

            for (int i = 0, j = 0; i < (360 / STEP) * 3; i += 3, j++)
            {

                index_list[i] = j;
                index_list[i + 1] = j + 1;
                index_list[i + 2] = j + 2;

                // For the last step
                if (j == SAMPLE_CNT - 2)
                {
                    index_list[i] = j;
                    index_list[i + 1] = j + 1;
                    index_list[i + 2] = 0;
                }
                else if (j == SAMPLE_CNT - 1)
                {
                    index_list[i] = 0;
                    index_list[i + 1] = 1;
                    index_list[i + 2] = j;
                }

                // printf("index_list[%i] = %i \t index_list[%i] = %i \t index_list[%i] = %i \n",
                //        i, index_list[i], i + 1, index_list[i + 1], i + 2, index_list[i + 2]);
            }

            // first, configure the cylinder's VAO (and VBO)
            unsigned int CVAO, CVBO, CEBO;
            glGenVertexArrays(1, &CVAO);
            glGenBuffers(1, &CVBO);
            glGenBuffers(1, &CEBO);

            glBindVertexArray(CVAO);

            glBindBuffer(GL_ARRAY_BUFFER, CVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_list), index_list, GL_STATIC_DRAW);

            /*glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_list), vertex_list);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex_list), sizeof(color_list), color_list);*/

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertex_list)));
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            ourShader.setMat4("model", model);
            glBindVertexArray(CVAO);
            glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));
            glBindVertexArray(FVAO);
            glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));
            glBindVertexArray(EVAO);
            glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));
        }
        // ourModel.Draw(ourShader);
        // RenderCylinder();
        // draw the lamp object
        //glDrawElementsBaseVertex(GL_TRIANGLE_FAN, SAMPLE_CNT, GL_UNSIGNED_INT, (GLvoid*)(0), 1);

        // draw the lamp object
        lightCubeShader.use();
        // glBindVertexArray(CVAO);
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setVec3("lightCubeColor", cos(lightColor.x), cos(lightColor.y), 185.0f / 255.0f + cos(lightColor.z) / 3.63f);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); //
        lightCubeShader.setMat4("model", model);

        // glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));

        // glBindVertexArray(FVAO);
        // glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));

        // glBindVertexArray(EVAO);
        // glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        // glfwSwapBuffers函数会交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），
        // 它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。
        glfwSwapBuffers(window);
        // glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）
        // 更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
        glfwPollEvents();
    }

    //glDeleteVertexArrays(1, &lightCubeVAO);
    //glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
// 检测特定的键是否被按下，并在每一帧做出处理。
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    //TODO
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        lightColor.z -= deltaTime;
    // if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    //     lightColor.y += deltaTime;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        lightColor.z += deltaTime;
}

void updateLight()
{
    lightPos.x = light_radius * sin(light_angle);
    lightPos.z = light_radius * cos(light_angle);
}

float calColor(float base, float range, float curVal)
{
    float baseVal = base / 255.0f;
    // range / 255.0f = 1.0f / x => x = 1 / range * 255.0f
    float rangeDiv = 1.0f / range * 255.0f;
    // rangeVal = cos(curVal)/rangeDiv
    return cos(curVal) / rangeDiv;
}

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
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void RenderCylinder()
{
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // printf("Location of x = %d  y = %d\n",)
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    printf("Location of mouse x = %f y = %f\n", xpos, ypos);
}
