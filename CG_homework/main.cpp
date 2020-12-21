#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "include/shader.h"
#include "include/camera.h"
#include "include/model.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void updateLight();
float calColor(float base, float range, float curVal);

// settings 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera 摄像机位置
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0, 10.0, 0.0));
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
float const light_radius = 8.0f;                                                          // radius of light_object on the y = 5.0f coplanar
float const light_velocity = 2.0f;                                                        // velocity of light_object
float light_angle = 0.0f;                                                                 // angle of light and axis
glm::vec3 lightPos(light_radius *sin(light_angle), 5.0f, light_radius *cos(light_angle)); // initial position(r,5,0)
glm::vec3 lightColor(0.0f, 0.0f, 0.0f);                                                   // Record the RGB of the light

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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Homework1 Teapot Naomi", NULL, NULL);
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
    // 当鼠标移动时调用这个函数
    // glfwSetCursorPosCallback(window, mouse_callback);
    // 当鼠标Scroll的时候调用这个函数
    // glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers 初始化GLAD
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("../shaders/vs.shader", "../shaders/fs.shader");
    Shader lightCubeShader("../shaders/light_cube.vs", "../shaders/light_cube.fs");

    // load models
    // -----------
    Model ourModel("../resources/teapot.obj");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
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

    float const green_value = 150.0f / 255.0f;
    float red_value = 1.0f;  //Initial Value Equals
    float blue_value = 0.0f; //Initial Value Equals 0

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    // glfwWindowShouldClose函数在我们每次循环的开始前检查一次GLFW是否被要求退出，
    // 如果是的话该函数返回true然后渲染循环便结束了，之后为我们就可以关闭应用程序了。
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame; // 两个帧之间的时间差
        lastFrame = currentFrame;
        // printf("deltaFrame = %f \n", deltaTime);

        // input
        // -----
        processInput(window);
        updateLight();

        // render
        // ------
        // 设置清空屏幕所用的颜色
        // IMP：这里修改背景颜色
        glClearColor((150.0f / 255.0f + cos(currentFrame) / 5) / 2 + cos(lightColor.x) / 2, green_value / 2 + cos(lightColor.y) / 2, (150.0f / 255.0f + sin(currentFrame) / 5) / 2 + (185.0f / 255.0f + cos(lightColor.z) / 3.63f) / 2, 0.6f);
        // 清空屏幕
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 让camera旋转
        glm::mat4 view = glm::mat4(10.0f); // make sure to initialize matrix to identity matrix first
        camera.RotateOnObject(radius);
        view = camera.GetViewMatrixR(glm::vec3(0.0, 2.0, 0.0)); // Set Center of Model is 2.0 instead of 0.0

        // don't forget to enable shader before setting uniforms
        ourShader.use();

        // Set Object Color
        red_value = 150.0f / 255.0f + cos(currentFrame) / 5;
        blue_value = 150.0f / 255.0f + sin(currentFrame) / 5;

        ourShader.setVec3("objectColor", red_value, green_value, blue_value);
        ourShader.setVec3("lightColor", cos(lightColor.x), cos(lightColor.y), cos(lightColor.z));
        ourShader.setVec3("lightPos", lightPos);
        ourShader.setVec3("viewPos", camera.Position);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        // draw the lamp object
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setVec3("lightCubeColor", cos(lightColor.x), cos(lightColor.y), 185.0f / 255.0f + cos(lightColor.z) / 3.63f);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.8f)); // a smaller cube
        lightCubeShader.setMat4("model", model);

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

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    // 当渲染循环结束后我们需要正确释放/删除之前的分配的所有资源。
    // 我们可以在main函数的最后调用glfwTerminate函数来完成。
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
    // camera.ProcessKeyboard(FORWARD, deltaTime);
    {
        radius = (radius <= nearest_distance) ? nearest_distance : radius;
        radius -= deltaTime * move_velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    // camera.ProcessKeyboard(BACKWARD, deltaTime);
    {
        radius = (radius >= furthest_distance) ? furthest_distance : radius;
        radius += deltaTime * move_velocity;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        light_angle -= deltaTime * light_velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        light_angle += deltaTime * light_velocity;

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