#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "include/shader.h"
#include "include/camera.h"
#include "include/model.h"
// #define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>

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

GLfloat RscrnConversion(GLfloat screenY);

GLfloat LscrnConversion(GLfloat screenX);


void drawBezierPoints();

#pragma region OldVars
// settings 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera 摄像机位置
Camera camera(glm::vec3(0.0f, 1.0f, 0.05f), glm::vec3(0.0, 1.0, 0.0));
glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
// glm::mat4 view = camera.GetViewMatrixR(glm::vec3(-1.0, 0.0, 0.0)); // Set Center of Model is 2.0 instead of 0.0
glm::mat4 view = glm::lookAt(glm::vec3(camera.Position.x, camera.Position.y, camera.Position.z), glm::vec3(0.0, 0.0, 0.0), glm::vec3(-1.0, 0.0, 0.0));

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
float const light_radius = 8.0f;         // radius of light_object on the y = 5.0f coplanar
float const light_velocity = 2.0f;       // velocity of light_object
float light_angle = 0.0f;                // angle of light and axis
glm::vec3 lightPos(0.44f, 0.2f, -0.57f); // initial position(r,5,0)
glm::vec3 lightColor(0.0f, 0.0f, 0.0f);  // Record the RGB of the light

#pragma endregion

#define STEP 6
#define SAMPLE_CNT (360 / STEP)

#define INITIAL_RADIUS 0.2f
#define INITIAL_HEAD -0.4f
#define INITIAL_END 0.4f

#define SCREEN_RADIUS 0.5f
#define SCREEN_HEAD -0.75f
#define SCREEN_END 0.75f

map<GLfloat, GLfloat> cylinder_base_line;
map<GLfloat, GLfloat> bezier_curve;
typedef map<GLfloat, GLfloat>::iterator map_iterator;
typedef map<GLfloat, GLfloat>::reverse_iterator rmap_iterator;
bool Beziermode = false;
bool Cameramode = true;
bool ShowBezierCurve = false;
bool steelMaterial = true;
bool particle = false;

GLfloat mouseX,
    mouseY;

glm::vec2 BPs[4];

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//Bezier variables
int BPss = 0;
GLfloat bmX, bmY;

#define MAX_PARTICLES 100

typedef struct //structrue for particle
{
    bool active;      //active or not
    float life;       //last time
    float fade;       //describe the decreasing of life
    float r, g, b;    //color
    float x, y, z;    //the position
    float xi, yi, zi; //what direction to move
    float xg, yg, zg; //gravity
} particles;

particles paticle[MAX_PARTICLES];
glm::vec3 spc(0.53, 0.54, 0.55);
glm::vec3 wpc(0.75, 0.67, 0.62);

bool sp, rp; //space button and return button pressed or not?
float slowdown = 2.0f;
float xspeed, yspeed;

void InitializeCylinder()
{
    cylinder_base_line.clear();
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(-0.4f, 0.2001f));
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(0.4f, 0.2001f));
    cylinder_base_line.insert(pair<GLfloat, GLfloat>(0.0f, 0.1500f));
}

int Mul(int n)
{
    if (n < 1)
        return 1;
    return n * Mul(n - 1);
}

double Over(double u, int n)
{
    double sum = 1.0f;
    if (n == 0)
        return 1;
    for (int i = 0; i < n; i++)
    {
        sum *= u;
    }
    return sum;
}

double Shuffle(int n, int i)
{
    return ((double)Mul(n)) / ((double)(Mul(i) * Mul(n - i)));
}

void drawBezier()
{
    // printf("Drawing Bezier Line\n");
    if (BPss <= 0 || !ShowBezierCurve)
        return;

    GLfloat u = 0, x, y;
    glClear(GL_DEPTH_BUFFER_BIT);
    glPointSize(4.0);

    glBegin(GL_POINTS);
    glColor3f(0.0, 0.0, 0.0);
    for (u = 0; u < 1; u = u + 0.001)
    {
        x = 0;
        y = 0;
        for (int i = 0; i < BPss; i++)
        {
            x += Shuffle(BPss - 1, i) * Over(u, i) * Over(1 - u, (BPss - 1 - i)) * (-BPs[i].x);
            y += Shuffle(BPss - 1, i) * Over(u, i) * Over(1 - u, (BPss - 1 - i)) * BPs[i].y;
        }
        printf("x = %f\ty = %f\tBPss = %i\n", x, y, BPss);
        glVertex3f(x, y, 0.0);
        bezier_curve.insert(std::pair<GLfloat, GLfloat>(x, y));
    }

    //BPss = 0;

    glEnd();
    glFlush();
    return;
}
int main()
{

#pragma region Initializaion

    glfwInit(); // Initialize GLFW
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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
    Shader metalShader("../shaders/metal.vs", "../shaders/metal.fs");

    // load models
    Model ourModel("../resources/knife.obj");

    // load and create a texture
    // -------------------------
    unsigned int texture1 = loadTexture("../resources/wood_cut_s.png");
    unsigned int texture2 = loadTexture("../resources/wood_cut_s.png");
    unsigned int diffuseMapWood = loadTexture("../resources/wood_cut_s.png");
    unsigned int specularMapWood = loadTexture("../resources/wood_cut_s.png");

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
    InitializeCylinder();
    // cylinder_base_line.insert(pair<GLfloat, GLfloat>(-0.2f, 0.2006f));
    // cylinder_base_line.insert(pair<GLfloat, GLfloat>(0.0f, 0.2008f));

    map_iterator iter = cylinder_base_line.begin();
    map_iterator iter2 = cylinder_base_line.begin();
    iter2++;

    // STEP = 每几度取一个点， 360/STEP = 一圈要几个点 ， 360/STEP*cylinder_base_line size = 整个圆柱体要取的点。
    GLfloat vertex_list[2 * SAMPLE_CNT][3];
    GLfloat n_vertex_list[2 * SAMPLE_CNT][3];
    GLfloat texture_vertex_list[SAMPLE_CNT * 3][2];

    GLfloat face_vertex[SAMPLE_CNT + 1][3];
    GLfloat n_face_vertex[SAMPLE_CNT + 1][3];
    GLfloat texture_face_vertex[SAMPLE_CNT * 3 / 21][2];

    GLfloat end_vertex[SAMPLE_CNT + 1][3];
    GLfloat n_end_vertex[SAMPLE_CNT + 1][3];
    GLfloat texture_end_vertex[SAMPLE_CNT * 3 / 2][2];

    GLuint index_list[SAMPLE_CNT * 3];
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
        GLfloat sum = sqrt((face_vertex[j][0] * face_vertex[j][0]) + (face_vertex[j][1] * face_vertex[j][1]) + (face_vertex[j][2] * face_vertex[j][2]));

        n_face_vertex[j][0] = face_vertex[j][0] / sum;
        n_face_vertex[j][0] = face_vertex[j][1] / sum;
        n_face_vertex[j][0] = face_vertex[j][2] / sum;
    }
    face_vertex[SAMPLE_CNT][0] = 0.0f;
    face_vertex[SAMPLE_CNT][1] = 0.0f;
    face_vertex[SAMPLE_CNT][2] = iter->first;
    for (int i = 0, j = 0; i < SAMPLE_CNT * 3 / 2; i += 3, j++)
    {
        face_index[i] = j;
        face_index[i + 1] = j + 1;
        face_index[i + 2] = SAMPLE_CNT;

        texture_face_vertex[i][0] = 0.0f;
        texture_face_vertex[i][1] = 0.0f;
        texture_face_vertex[i + 1][0] = 1.0f;
        texture_face_vertex[i + 1][1] = 0.0f;
        texture_face_vertex[i + 2][0] = 0.5f;
        texture_face_vertex[i + 2][1] = 1.0f;

        if (j == SAMPLE_CNT / 2 - 1)
        {
            face_index[i] = j;
            face_index[i + 1] = 0;
            face_index[i + 2] = SAMPLE_CNT;
        }

        printf("face_index[%i] = %i \t face_index[%i] = %i \t face_index[%i] = %i \n",
               i, face_index[i], i + 1, face_index[i + 1], i + 2, face_index[i + 2]);
    }

    unsigned int FVAO, FVBO, FEBO, FNVBO, FTVBO;
    glGenVertexArrays(1, &FVAO);
    glGenBuffers(1, &FVBO);
    glGenBuffers(1, &FEBO);
    glGenBuffers(1, &FNVBO);
    glGenBuffers(1, &FTVBO);

    glBindVertexArray(FVAO);

    glBindBuffer(GL_ARRAY_BUFFER, FVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(face_vertex), face_vertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, FEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(face_index), face_index, GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 3 * sizeof(unsigned int), (void *)0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, FNVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(n_face_vertex), n_face_vertex, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    if (!steelMaterial)
    {

        glBindBuffer(GL_ARRAY_BUFFER, FTVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_face_vertex), texture_face_vertex, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(2);
    }

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

        GLfloat sum = sqrt((end_vertex[j][0] * end_vertex[j][0]) + (end_vertex[j][1] * end_vertex[j][1]) + (end_vertex[j][2] * end_vertex[j][2]));

        n_end_vertex[j][0] = end_vertex[j][0] / sum;
        n_end_vertex[j][0] = end_vertex[j][1] / sum;
        n_end_vertex[j][0] = end_vertex[j][2] / sum;
    }
    end_vertex[SAMPLE_CNT][0] = 0.0f;
    end_vertex[SAMPLE_CNT][1] = 0.0f;
    end_vertex[SAMPLE_CNT][2] = rend->first;
    for (int i = 0, j = 0; i < SAMPLE_CNT * 3 / 2; i += 3, j++)
    {
        end_index[i] = j;
        end_index[i + 1] = j + 1;
        end_index[i + 2] = SAMPLE_CNT;

        texture_end_vertex[i][0] = 0.0f;
        texture_end_vertex[i][1] = 0.0f;
        texture_end_vertex[i + 1][0] = 1.0f;
        texture_end_vertex[i + 1][1] = 0.0f;
        texture_end_vertex[i + 2][0] = 0.5f;
        texture_end_vertex[i + 2][1] = 1.0f;

        if (j == SAMPLE_CNT / 2 - 1)
        {
            end_index[i] = j;
            end_index[i + 1] = 0;
            end_index[i + 2] = SAMPLE_CNT;
        }

        printf("end_index[%i] = %i \t end_index[%i] = %i \t end_index[%i] = %i \n",
               i, end_index[i], i + 1, end_index[i + 1], i + 2, end_index[i + 2]);
    }

    unsigned int EVAO, EVBO, EEBO, ENVBO, ETVBO;
    glGenVertexArrays(1, &EVAO);
    glGenBuffers(1, &EVBO);
    glGenBuffers(1, &EEBO);
    glGenBuffers(1, &ENVBO);
    glGenBuffers(1, &ETVBO);

    glBindVertexArray(EVAO);

    glBindBuffer(GL_ARRAY_BUFFER, EVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(end_vertex), end_vertex, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(end_index), end_index, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertex_list)));
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, ENVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(n_end_vertex), n_end_vertex, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);

    if (!steelMaterial)
    {

        glBindBuffer(GL_ARRAY_BUFFER, ETVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texture_end_vertex), texture_end_vertex, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(2);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

#pragma region Draw lamp
    // draw the lamp object
    //glDrawElementsBaseVertex(GL_TRIANGLE_FAN, SAMPLE_CNT, GL_UNSIGNED_INT, (GLvoid*)(0), 1);

    // draw the lamp object
    lightCubeShader.use();
    // glBindVertexArray(CVAO);
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setVec3("lightCubeColor", cos(lightColor.x), cos(lightColor.y), 185.0f / 255.0f + cos(lightColor.z) / 3.63f);
    lightCubeShader.setMat4("view", view);
    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    // model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));     // it's a bit too big for our scene, so scale it down

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(1.2f)); //
    lightCubeShader.setMat4("model", model);

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
#pragma endregion

#pragma endregion
    GLfloat angle = 0;

    while (!glfwWindowShouldClose(window))
    {
        angle += 5;

        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame; // 两个帧之间的时间差
        lastFrame = currentFrame;
        // printf("deltaFrame = %f \n", deltaTime);

        GLfloat SaveX = mouseX;
        GLfloat SaveY = mouseY;

        GLfloat screenX = RscrnConversion(mouseX);
        GLfloat screenY = LscrnConversion(mouseY);

        processInput(window);
        // updateLight();

#pragma region Bezier

        if (Beziermode)
        {
            // printf("INSIDE BEZIERMODE\n");
            // glClearColor(0.6f, 0.6f, 0.6f, 0.8f);
            glClear(GL_DEPTH_BUFFER_BIT);

            glUseProgram(0);

            glBegin(GL_TRIANGLE_STRIP);
            glColor4f(0.2, 0.2, 0.2, 0.5); // Red

            glVertex3f(0.75, 0.5, 0.0);

            glVertex3f(-0.75, 0.5, 0.0);

            glVertex3f(0.75, 0.0, 0.0);

            glVertex3f(-0.75, 0.0, 0.0);

            glEnd();
            glFlush();

            glClear(GL_DEPTH_BUFFER_BIT);
            glPointSize(5);

            glBegin(GL_POINTS);
            glColor3f(1.0, 1.0, 1.0);

            for (int i = 0; i < BPss; i++)
            {
                // printf("Drawing with %ith point with x = %f  y = %f\n", i, BPs[i].x, BPs[i].y = mouseY);

                glVertex3f(-BPs[i].x, BPs[i].y, 0.0);
            }
            glEnd();
            glFlush();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
#pragma endregion
        else
        {

            // render
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#pragma region Draw Cylinder Body
            // don't forget to enable shader before setting uniforms

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

                    GLfloat sum = sqrt((vertex_list[i][0] * vertex_list[i][0]) + (vertex_list[i][1] * vertex_list[i][1]) + (vertex_list[i][2] * vertex_list[i][2]));

                    n_vertex_list[i][0] = vertex_list[i][0] / sum;
                    n_vertex_list[i][0] = vertex_list[i][1] / sum;
                    n_vertex_list[i][0] = vertex_list[i][2] / sum;

                    GLfloat sum2 = sqrt((vertex_list[i + 1][0] * vertex_list[i + 1][0]) + (vertex_list[i + 1][1] * vertex_list[i + 1][1]) + (vertex_list[i + 1][2] * vertex_list[i + 1][2]));

                    n_vertex_list[i + 1][0] = vertex_list[i + 1][0] / sum;
                    n_vertex_list[i + 1][0] = vertex_list[i + 1][1] / sum;
                    n_vertex_list[i + 1][0] = vertex_list[i + 1][2] / sum;

                    // printf("vertex_list[%i][0] = %f \t vertex_list[%i][1] = %f \t vertex_list[%i][2] = %f \n", i, cos(p) * iter->second,
                    //        i, sin(p) * iter->second,
                    //        i, iter->first);

                    // printf("vertex_list[%i][0] = %f \t vertex_list[%i][1] = %f \t vertex_list[%i][2] = %f \n", i + 1, cos(p) * iter2->second,
                    //        i + 1, sin(p) * iter2->second,
                    //        i + 1, iter2->first);
                }

                for (int i = 0, j = 0; i < (360 / STEP) * 3; i += 3, j++)
                {

                    index_list[i] = j;
                    index_list[i + 1] = j + 1;
                    index_list[i + 2] = j + 2;

                    texture_vertex_list[i][0] = 0.0f;
                    texture_vertex_list[i][1] = 0.0f;
                    texture_vertex_list[i + 1][0] = 1.0f;
                    texture_vertex_list[i + 1][1] = 0.0f;
                    texture_vertex_list[i + 2][0] = 0.5f;
                    texture_vertex_list[i + 2][1] = 1.0f;

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

                unsigned int CVAO, CVBO, CEBO, CNVBO, CTVBO;
                glGenVertexArrays(1, &CVAO);
                glGenBuffers(1, &CVBO);
                glGenBuffers(1, &CEBO);
                glGenBuffers(1, &CNVBO);
                glGenBuffers(1, &CTVBO);

                glBindVertexArray(CVAO);

                glBindBuffer(GL_ARRAY_BUFFER, CVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_list), vertex_list, GL_STATIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_list), index_list, GL_STATIC_DRAW);

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
                //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(vertex_list)));
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, CNVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(n_vertex_list), n_vertex_list, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(1);

                if (!steelMaterial)
                {

                    glBindBuffer(GL_ARRAY_BUFFER, ETVBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertex_list), texture_vertex_list, GL_STATIC_DRAW);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
                    glEnableVertexAttribArray(2);
                }

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
#pragma region material Selection
                if (!steelMaterial)
                {

                    ourShader.use();


                    ourShader.setVec3("objectColor", spc.x, spc.y, spc.z);
                    ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
                    ourShader.setVec3("lightPos", lightPos);
                    ourShader.setVec3("viewPos", camera.Position);
                    // view/projection transformations
                    ourShader.setMat4("projection", projection);
                    ourShader.setMat4("view", view);
                    ourShader.setVec3("light.position", lightPos);

                    // light properties
                    ourShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
                    ourShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
                    ourShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
                    ourShader.setFloat("material.shininess", 100.0f);
                    model = glm::mat4(1.0f);
                    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));


                    ourShader.setMat4("model", model);

                    // bind diffuse map
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, diffuseMapWood);
                    // bind specular map
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, specularMapWood);
                }
                else
                {
                    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));

                    metalShader.use();

                    metalShader.setVec3("objectColor", 0.53f, 0.54f, 0.55f);
                    metalShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
                    metalShader.setVec3("lightPos", lightPos);
                    metalShader.setVec3("viewPos", camera.Position);
                    // view/projection transformations
                    metalShader.setMat4("projection", projection);
                    metalShader.setMat4("view", view);
                    model = glm::mat4(1.0f);


                    metalShader.setMat4("model", model);
                }

                glBindVertexArray(CVAO);
                glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));
                glBindVertexArray(FVAO);
                glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));
                glBindVertexArray(EVAO);
                glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, (GLvoid *)(0));

                glDeleteVertexArrays(1, &CVAO);
                glDeleteVertexArrays(1, &FVAO);
                glDeleteVertexArrays(1, &EVAO);

                glDeleteBuffers(1, &CVBO);
                glDeleteBuffers(1, &FVBO);
                glDeleteBuffers(1, &EVBO);

                glDeleteBuffers(1, &CEBO);
                glDeleteBuffers(1, &FEBO);
                glDeleteBuffers(1, &EEBO);

                glDeleteBuffers(1, &CNVBO);
                glDeleteBuffers(1, &FNVBO);
                glDeleteBuffers(1, &ENVBO);

                glDeleteBuffers(1, &CTVBO);
                glDeleteBuffers(1, &FTVBO);
                glDeleteBuffers(1, &ETVBO);
            }

// ourModel.Draw(ourShader);
// RenderCylinder();
#pragma endregion

            glUseProgram(0);
            //drawBezier();

#pragma region Detect Cut
            if (ShowBezierCurve)
            {
                GLfloat aX = 0.0f, aY = 0.0f, bX = 0.0f, bY = 0.0f;

                // Detect if this line is outside Bezier Line
                map_iterator itr = bezier_curve.begin();
                map_iterator itr2 = bezier_curve.begin();
                drawBezier();
                itr2++;
                for (; itr2 != bezier_curve.end(); itr++, itr2++)
                {
                    if (itr->first < mouseX && itr2->first > mouseX)
                    {
                        aX = itr->first;
                        aY = itr->second;
                        bX = itr2->first;
                        bY = itr2->second;
                        break;
                    }

                    GLfloat k = (aY - bY) / (aX - bX);
                    GLfloat inception = aY - k * aX;
                    GLfloat calY = k * mouseX + inception;

                    if (calY > mouseY)
                    {
                        // printf("[CUT]THIS POINT DOEN'T EXISTS AND IT HAS A LARGER RADIUS THAN KNIFE, mouseY = %f \t calY = %f\n", mouseY, calY);
                        // cylinder_base_line.insert(std::pair<GLfloat, GLfloat>(mouseX, mouseY));
                        mouseY = calY;
                    }
                    else
                    {
                        // printf("[NOT CUT]THIS POINT DOESN'T EXISTS AND IT HAS A SMALLER RADIUS THAN KNIFE, mouseY = %f \t calY = %f\n", mouseY, calY);
                    }
                }
            }
            // printf("saveX = %f\t saveY = %f\n", mouseX, mouseY);

            if (screenX > INITIAL_HEAD && screenX < INITIAL_END && SaveY < 0.5 && SaveY > 0.5 / 2)
            {

                // printf("mouseX = %f\t mouseY = %f\n", mouseX, mouseY);

                // printf("INSIDE REASONABLE RANGE\n");
                if (cylinder_base_line.find(screenX) != cylinder_base_line.end())
                {
                    if (cylinder_base_line.find(screenX)->second > screenY)
                    {
                        //printf("[CUT]THIS POINT EXISTS AND IT HAS A LARGER RADIUS THAN KNIFE, mouseY = %f \t pointY = %f\n", screenY, cylinder_base_line.find(screenX)->second);
                        cylinder_base_line.find(screenX)->second = screenY;
                        particle = true;
                    }
                    else
                    {
                        particle = false;
                        //printf("[NOT CUT]THIS POINT EXISTS BUT IT HAS A SMALLER RADIUS THAN KNIFE, mouseY = %f \t pointY = %f\n", screenY, cylinder_base_line.find(screenX)->second);
                    }
                }
                else
                {
                    //printf("THIS POINT DOESN'T EXIST\n");
                    iter = cylinder_base_line.begin();
                    iter2 = cylinder_base_line.begin();
                    iter2++;
                    GLfloat aX = 0.0f, aY = 0.0f, bX = 0.0f, bY = 0.0f;

                    for (; iter2 != cylinder_base_line.end(); ++iter2, ++iter)
                    {

                        //printf("mouseX = %f\t mouseY = %f\t iter->first = %f\t iter2->first = %f\n", screenX, screenY, iter->first, iter2->first);

                        if (iter->first < screenX && iter2->first > screenX)
                        {
                            //printf("[FOUND MOUSEX = %f RANGE] between x1 = %f\t x2 = %f\n", screenX, iter->first, iter2->first);
                            aX = iter->first;
                            aY = iter->second;
                            bX = iter2->first;
                            bY = iter2->second;
                            break;
                        }
                    }
                    GLfloat k = (aY - bY) / (aX - bX);
                    GLfloat inception = aY - k * aX;
                    GLfloat calY = k * screenX + inception;
                    //printf("aX = %f\t aY = %f\t bX = %f\t bY = %f\t k = %f\t inception = %f\t calY = %f\n", aX, aY, bX, bY, k, inception, calY);

                    if (calY > screenY)
                    {
                        //printf("[CUT]THIS POINT DOEN'T EXISTS AND IT HAS A LARGER RADIUS THAN KNIFE, mouseY = %f \t calY = %f\n", screenY, calY);
                        cylinder_base_line.insert(std::pair<GLfloat, GLfloat>(screenX, screenY));
                        particle = true;
                    }
                    else
                    {
                        //printf("[NOT CUT]THIS POINT DOESN'T EXISTS AND IT HAS A SMALLER RADIUS THAN KNIFE, mouseY = %f \t calY = %f\n", screenY, calY);
                        particle = false;
                    }

                    // This point is a unique point
                }
            }
#define ParticleCNT 50
#pragma region Paricle System

            if (particle)
            {
                // printf("Acitvated Particle System\n");
                for (int loop = 0; loop < MAX_PARTICLES; ++loop)
                {
                    if (paticle[loop].active)
                        continue;
                    paticle[loop].x = mouseX;
                    paticle[loop].y = mouseY;
                    paticle[loop].z = 0;

                    paticle[loop].active = true;
                    paticle[loop].life = 10.0f; //full life is 1.0f
                    //life decrease rate(a random value add 0.003f : never equals 0)
                    paticle[loop].fade = float(rand() % 100) / 1000.0f + 0.003f;
                    if (steelMaterial)
                    {
                        paticle[loop].r = spc.x;
                        paticle[loop].g = spc.y;
                        paticle[loop].b = spc.z;
                    }
                    else
                    {
                        paticle[loop].r = wpc.x;
                        paticle[loop].g = wpc.y;
                        paticle[loop].b = wpc.z;
                    }

                    if (mouseX < 0)
                    {
                        paticle[loop].xi = float((rand() % 2)) - float((rand() % 2));
                        paticle[loop].yi = float((rand() % 5)) - float((rand() % 5));
                        paticle[loop].zi = float((rand() % 5)) - float((rand() % 5));
                        paticle[loop].xg = float((rand() % 10)) / 10;
                        paticle[loop].yg = float((rand() % 10)) / 10;
                        paticle[loop].zg = -float((rand() % 10)) / 10;
                    }
                    else
                    {
                        paticle[loop].xi = float((rand() % 2)) - float((rand() % 2));
                        paticle[loop].yi = float((rand() % 5)) - float((rand() % 5));
                        paticle[loop].zi = -float((rand() % 5)) - float((rand() % 5));
                        paticle[loop].xg = float((rand() % 10)) / 10;
                        paticle[loop].yg = float((rand() % 10)) / 10;
                        paticle[loop].zg = float((rand() % 10)) / 10;
                    }
                }
            }
            // Draw
            // glClear(GL_DEPTH_BUFFER_BIT);
            for (int loop = 0; loop < MAX_PARTICLES; ++loop)
            {
                if (paticle[loop].active) //the particle is alive
                {
                    float x = -paticle[loop].x;
                    float y = paticle[loop].y;
                    //our scene is moved into the screen
                    float z = paticle[loop].z;

                    glColor4f(paticle[loop].r,
                              paticle[loop].g,
                              paticle[loop].r,
                              //use life as alpha value:
                              //as particle dies,it becomes more and more transparent
                              paticle[loop].life);

                    //printf("Drawing Particle i = %i from x = %f\t y = %f z = %f\n", loop, x, y, z);
                    //glUseProgram(1);
                    //draw particles : use triangle strip instead of quad to speed
                    glBegin(GL_TRIANGLE_STRIP);
                    //top right
                    // glTexCoord2d(1, 1);
                    glVertex3f(x + 0.005f, y + 0.005f, z);
                    //top left
                    // glTexCoord2d(0, 1);
                    glVertex3f(x - 0.005f, y + 0.005f, z);
                    //bottom right
                    // glTexCoord2d(1, 0);
                    glVertex3f(x + 0.005f, y - 0.005f, z);
                    //bottom left
                    //glTexCoord2d(0, 0);
                    glVertex3f(x - 0.005f, y - 0.005f, z);
                    glEnd();

                    //Move On The X Axis By X Speed
                    paticle[loop].x += paticle[loop].xi / (slowdown * 1000);
                    //Move On The Y Axis By Y Speed
                    paticle[loop].y += paticle[loop].yi / (slowdown * 1000);
                    //Move On The Z Axis By Z Speed
                    paticle[loop].z += paticle[loop].zi / (slowdown * 1000);

                    //add gravity or resistance : acceleration
                    paticle[loop].xi += paticle[loop].xg;
                    paticle[loop].yi += paticle[loop].yg;
                    paticle[loop].zi += paticle[loop].zg;

                    //decrease particles' life
                    paticle[loop].life -= paticle[loop].fade;

                    //if particle is dead,rejuvenate it
                    if (paticle[loop].life < 0.0f)
                    {
                        paticle[loop].active = false;
                    }
                }
            }

#pragma endregion

            GLfloat offX = RscrnConversion(-0.13);
            GLfloat offY = LscrnConversion(-0.43);
            // render the loaded model
            metalShader.use();
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-screenY + offY, 0, screenX + offX)); // translate it down so it's at the center of the scene
            model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));                       // it's a bit too big for our scene, so scale it down
            //model = glm::
            metalShader.setMat4("model", model);
            ourModel.Draw(metalShader);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            // glfwSwapBuffers函数会交换颜色缓冲（它是一个储存着GLFW窗口每一个像素颜色值的大缓冲），
            // 它在这一迭代中被用来绘制，并且将会作为输出显示在屏幕上。
            glfwSwapBuffers(window);
            // glfwPollEvents函数检查有没有触发什么事件（比如键盘输入、鼠标移动等）
            // 更新窗口状态，并调用对应的回调函数（可以通过回调方法手动设置）。
            glfwPollEvents();

#pragma endregion
        }
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
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime * 10);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime * 10);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime * 10);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime * 10);

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        lightPos.x -= deltaTime;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        lightPos.x += deltaTime;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        lightPos.y -= deltaTime;
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
        lightPos.y += deltaTime;
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
        lightPos.z -= deltaTime;
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        lightPos.z += deltaTime;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        InitializeCylinder();

    // printf("lightpos: x = %f\t y = %f\t z = %f\n", lightPos.x, lightPos.y, lightPos.z);
    // }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
    {
        //printf("Start Bezier Mode\n");
        // Beziermode = !Beziermode;
        BPss = 0;
        Beziermode = true;
    }

    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
    {
        //printf("Exit Bezier Mode\n");
        // Beziermode = !Beziermode;
        Beziermode = false;
    }

    if (Beziermode && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && BPss == 4)
    {
        // display();
        ShowBezierCurve = true;

        drawBezier();

        //BPss = 0;
        Beziermode = false;
    }

    if (ShowBezierCurve && glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        ShowBezierCurve = false;
    // printf("camera X = %f \t Y = %f \tZ = %f\n", camera.Position.x, camera.Position.y, camera.Position.z);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        steelMaterial = true;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        steelMaterial = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        lightColor.z -= deltaTime;
    // if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    //     lightColor.y += deltaTime;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        lightColor.z += deltaTime;
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

#define By1 0.0f
#define By2 0.5f
#define Bx1 -0.75f
#define Bx2 0.75f
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && Beziermode && (mouseX > Bx1 && mouseX < Bx2 && mouseY > By1 && mouseY < By2))
    {
        if (BPss > 4)
        {
            // BPss = 0;
            return;
        }

        BPs[BPss].x = mouseX;
        BPs[BPss].y = mouseY;
        // printf("Drawing with %ith point with x = %f  y = %f\n", BPss, BPs[BPss].x, BPs[BPss].y);
        printf("mouseButtonCallback: \n");
        BPss++;
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    printf("Location of mouse x = %f y = %f\n", -(xpos - 400) / 400, -(ypos - 300) / 300);
    mouseX = -(xpos - 400) / 400;
    mouseY = -((ypos - 300) / 300);
    bmX = xpos;
    bmY = ypos;
}

GLfloat RscrnConversion(GLfloat screenY)
{
    if (screenY > SCREEN_END || screenY < SCREEN_HEAD)
    {
        //printf("ERROR: THIS SCREENY IS OUT OF RANGE 0.75f\n");
    }
    //printf("Location of Screeny = %f\n", screenY / SCREEN_END * INITIAL_END);

    return screenY / SCREEN_END * INITIAL_END;
}

GLfloat LscrnConversion(GLfloat screenX)
{
    if (screenX < 0.1f && screenX > 0.0f)
    {
        //printf("ERROR: THIS SCREENX IS TOO CLOSE\n");
    }
    if (screenX < 0.0f)
    {
        //printf("ERROR: THIS SCREENX IS ON THE WRONG SIDE\n");
    }
    //printf("Location of Screenx = %f\n", screenX / SCREEN_RADIUS * INITIAL_RADIUS);

    return screenX / SCREEN_RADIUS * INITIAL_RADIUS;
}
