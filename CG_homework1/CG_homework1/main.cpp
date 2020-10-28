#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>

using namespace std;
#include <stdio.h>

#include "include/shader.h"
// #include "include/camera.h"
// #include "include/model.h"
// https://itxf.site/2020/031157774.html

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void updateLight();
float calColor(float base, float range, float curVal);

// // settings 屏幕大小
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// class Point
// {
// public:
//     float x;
//     float y;
//     float z;
//     Edge edge_1;
//     Edge edge_2;
//     Point point_1;
//     Point point_2;
// }

class Edge
{
public:
    int ymax;               // 判断是否有交点的依据就是看扫描线y是否大于这条边两个端点的y坐标值
    float xi;               // 下一个x
    float dx;               // 扫描线增加一次x坐标的增值
    // Edge* nextEdge;     
    // Edge() {
    //     nextEdge = NULL;
    // }
    Edge(int y, float x, float dx) {
        this->ymax = y;
        this->deltax = dx;
        this->x = x;
    }

};
const GLint WIDTH = 800, HEIGHT = 800;
// a 顶点数组， n 定点数量
// 创建边的分类表
int* creatET(map<int, Edge*>& ET, int* a, int n)
{

    int maxy, miny, ymax, ymin, x;
    float dx;
    maxy = a[1];        
    miny = a[1];
    //cout << n;
    for (int i = 0; i < n - 2; i = i + 2)
    {

        if (a[i + 1] > maxy)
        {
            maxy = a[i + 1];
        }
        if (a[i + 1] < miny)
        {
            miny = a[i + 1];
        }
        if (a[i + 1] != a[i + 3])//斜率为0
        {
            if (a[i + 1] < a[i + 3])
            {
                ymax = a[i + 3];
                ymin = a[i + 1];
                x = a[i];
            }
            else
            {
                ymax = a[i + 1];
                ymin = a[i + 3];
                x = a[i + 2];
            }

            dx = (float)(a[i + 2] - a[i]) / (a[i + 3] - a[i + 1]);
            if (ET[ymin] == NULL)
            {
                Edge* edge = new Edge(ymax, x, dx);
                ET[ymin] = edge;
            }
            else
            {
                Edge* p = ET[ymin];
                while (p->nextEdge != NULL)
                {
                    p = p->nextEdge;
                }
                Edge* edge = new Edge(ymax, x, dx);
                p->nextEdge = edge;
            }

        }

    }
    int* h = new int[2];
    h[0] = maxy;
    h[1] = miny;
    return h;
}
vector<GLfloat> filling(int miny, int maxy, map<int, Edge*>& ET) {
    GLfloat px, py;
    vector<GLfloat> fill;
    Edge* AET = new Edge();     // 活边表
    //扫描从所有图形最小y值到最大y值
    for (int i = miny; i < maxy; i++)
    {
        //如果 ET[y] 非空，则将其中的所有边取出并插入到AET中，按x（若x相等则按∆x）递增方向排序。
        while (ET[i] != NULL)
        {
            Edge* p = ET[i];
            Edge* AETp = AET;
            // 活边表中有next边
            while (AETp->nextEdge)
            {
                // 相交边的x值大于活边表中的一个边的x值
                if (p->x > AETp->nextEdge->x)
                {
                    // 活边表的
                    AETp = AETp->nextEdge;
                    continue;
                }
                if (p->x == AETp->nextEdge->x && p->deltax > AETp->nextEdge->deltax)
                {
                    AETp = AETp->nextEdge;
                    continue;
                }
                break;
            }
            ET[i] = p->nextEdge;

            p->nextEdge = AETp->nextEdge;

            AETp->nextEdge = p;

        }

        Edge* p = AET;
        // 若AET非空，将AET中的边按顺序两两配对并填色。
        while (p->nextEdge && p->nextEdge->nextEdge)
        {
            for (int x = (int)p->nextEdge->x + 1; x < (int)p->nextEdge->nextEdge->x; x++)
            {
                //cout << x << ' ' <<i<< endl;
                px = x * 1.0f / WIDTH;
                py = i * 1.0f / HEIGHT;
                //cout << px << ' ' << py << endl;
                fill.push_back(px);
                fill.push_back(py);
                fill.push_back(0.0f);
            }
            p = p->nextEdge->nextEdge;
        }

        //  删去AET中满足y=ymax+1的边。
        p = AET;
        while (p->nextEdge)
        {
            if (p->nextEdge->ymax == i+1)
            {

                Edge* pDelete = p->nextEdge;
                p->nextEdge = pDelete->nextEdge;
                pDelete->nextEdge = NULL;
                delete pDelete;
            }
            else
            {
                p = p->nextEdge;
            }
        }

        // 对于AET中所有边，赋值x = x + ∆x。
        p = AET;
        while (p->nextEdge)
        {
            p->nextEdge->x += p->nextEdge->deltax;
            p = p->nextEdge;
        }
        // y = y + 1
    }
    return fill;
}



int main()
{
    glfwInit(); // Initialize GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // For MacOS
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Homework2 Naomi", NULL, NULL);
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
    //glEnable(GL_DEPTH_TEST);

    map<int, Edge*> ET;         //变得分类表
    int a[] = { 0,0,173,100,100,100,91,121,70,130,49,121,40,
    100,49,79,70,70,93,77,100,100,173,100,141,141,100,173,0,
    200,-100,173,-141,141,-173,100,-200,0,-173,-100,-141,-141,
    -100,-173,0,-200,100,-173,141,-141,173,-100,0,0,110,0,119,
    21,140,30,161,21,170,0,161,-21,140,-30,119,-21,110,0,0,0};
    //吃豆人 输入坐标最后需要回到起点
    //int a[] = { 0,40,-20,80,-50,100,-100,100,-130,80,-150,30,-150,-30,-85,-120,0,-180,85,-120,150,-30,150,30,130,80,100,100,50,100,20,80 ,0,40 };
    int* large = creatET(ET, a, sizeof(a) / sizeof(a[0]));      //创建边的分类表，返回的large存最高点y左边，最低点y坐标
    vector<GLfloat> fill = filling(large[1], large[0], ET);     //large[1] 最小y值, large[0] 最大y值   
    GLfloat* vertices = fill.data();
    int size = fill.size();


    // build and compile shaders
    // -------------------------
    Shader ourShader("../shaders/vs.shader", "../shaders/fs.shader");
    
    

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    // Reading from txt------------------------------------------------------------------------------------
    const char* filePath = "../resources/overlapping.txt";
    float triangle_one[18], triangle_two[18], triangle_three[18], triangle_four[18];
    FILE* mFile = fopen(filePath, "r");

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
             std::cout << triangle_one[i] << "\t";
             std::cout << triangle_two[i] << "\t";
             std::cout << triangle_three[i] << "\t";
             std::cout << triangle_four[i] << "\t";
             std::cout << std::endl;
        }
    }

    unsigned int VBOs[4], VAOs[4], CVBOs[4], CVAOs[4];
    glGenVertexArrays(4, VAOs); // we can also generate multiple VAOs or buffers at the same time
    glGenBuffers(4, VBOs);

    // first triangle setup
    // --------------------
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_one), triangle_one, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Vertex attributes stay the same

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // // second triangle setup
    // // ---------------------
    glBindVertexArray(VAOs[1]);             // note that we bind to a different VAO now
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]); // and a different VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_two), triangle_two, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // // third triangle setup
    // // --------------------
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_three), triangle_three, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); // Vertex attributes stay the same

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // // fourth triangle setup
    // // ---------------------
    glBindVertexArray(VAOs[3]);             // note that we bind to a different VAO now
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]); // and a different VBO
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
    glDeleteVertexArrays(4, VAOs);
    glDeleteBuffers(4, VBOs);
    //glDeleteProgram(ourShader);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// // ---------------------------------------------------------------------------------------------------------
// // 检测特定的键是否被按下，并在每一帧做出处理。
// //void processInput(GLFWwindow *window)
// //{
// //    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
// //        glfwSetWindowShouldClose(window, true);
// //    //TODO
// //    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
// //    // camera.ProcessKeyboard(FORWARD, deltaTime);
// //    {
// //        radius = (radius <= nearest_distance) ? nearest_distance : radius;
// //        radius -= deltaTime * move_velocity;
// //    }
// //    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
// //    // camera.ProcessKeyboard(BACKWARD, deltaTime);
// //    {
// //        radius = (radius >= furthest_distance) ? furthest_distance : radius;
// //        radius += deltaTime * move_velocity;
// //    }
// //
// //    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
// //        light_angle -= deltaTime * light_velocity;
// //    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
// //        light_angle += deltaTime * light_velocity;
// //
// //    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
// //        lightColor.z -= deltaTime;
// //    // if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
// //    //     lightColor.y += deltaTime;
// //    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
// //        lightColor.z += deltaTime;
// //}
// //
// //void updateLight()
// //{
// //    lightPos.x = light_radius * sin(light_angle);
// //    lightPos.z = light_radius * cos(light_angle);
// //}

// // glfw: whenever the window size changed (by OS or user resize) this callback function executes
// // 当用户改变窗口的大小的时候，视口也应该被调整
// // ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    // 通过调用glViewport函数来设置窗口的维度(Dimension)
    glViewport(0, 0, width, height);
}

// // glfw: whenever the mouse moves, this callback is called
// // -------------------------------------------------------
// //void mouse_callback(GLFWwindow *window, double xpos, double ypos)
// //{
// //    if (firstMouse)
// //    {
// //        lastX = xpos;
// //        lastY = ypos;
// //        firstMouse = false;
// //    }
// //
// //    float xoffset = xpos - lastX;
// //    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
// //
// //    lastX = xpos;
// //    lastY = ypos;
// //
// //    camera.ProcessMouseMovement(xoffset, yoffset);
// //}
// //
// //// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// //// ----------------------------------------------------------------------
// //void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
// //{
// //    camera.ProcessMouseScroll(yoffset);
// //}

// struct Point
// {
//     int x;
//     int y;
//     Point(int a = 0, int b = 0)
//     {
//         x = a;
//         y = b;
//     }
// };
// struct Bian_list
// {
//     float jx;
//     float dx;
//     int ymin;
//     int ymax;
//     bool sp;
// };
// struct Xb
// {
//     float x[10];
//     int num;
// };
// struct Huo_list
// {
//     int num;
//     Bian_list *next[10];
// };
// void InitiateHuo_list(Huo_list *H)
// {
//     H->num = 0;
//     H->next[0] = NULL;
// }
// void InsertHuo_list(Huo_list *H, Bian_list *b_list)
// {
//     H->next[H->num] = b_list;
//     H->num++;
// }
// void Deleteb_list(Huo_list *H, int j)
// {
//     int i;
//     for (i = j; i < H->num; i++)
//     {
//         H->next[i] = H->next[i + 1];
//     }
//     H->num--;
// }
// void pai_xuHuo_list(Xb *xb)
// {
//     int n = xb->num;
//     int i, j;
//     float temp;
//     for (i = 0; i < n - 1; i++)
//     {
//         temp = xb->x[i + 1];
//         j = i;
//         while (j > -1 && temp < xb->x[j])
//         {
//             xb->x[j + 1] = xb->x[j];
//             j--;
//         }
//         xb->x[j + 1] = temp;
//     }
// }
// //ScanLine用于设多边形存储的顶点序列，顶点数等于边数

// void ScanLine(Point *Polygon_point, int num_point)
// {
//     //point_ymax,point_ymin,num_smx用来确定扫描线链表的个数
//     int i, j, point_ymax, point_ymin, num_smx;
//     point_ymax = Polygon_point[1].y;
//     point_ymin = Polygon_point[1].y;
//     for (i = 0; i < num_point; i++)
//     {
//         if (Polygon_point[i].y > point_ymax)
//             point_ymax = Polygon_point[i].y;
//         if (Polygon_point[i].y < point_ymin)
//             point_ymin = Polygon_point[i].y;
//     }
//     num_smx = point_ymax - point_ymin;
//     //建立、初始化边表
//     Bian_list *b_list = (Bian_list *)malloc(num_point * sizeof(Bian_list));
//     //n个点n-1条边
//     for (i = 0; i < num_point - 1; i++)
//     {
//         //jx其实是保存扫描线与边的当前交点值，即ymin对应的jx,Cpoint(jx,ymin)
//         if (Polygon_point[i].y < Polygon_point[i + 1].y)
//             b_list[i].jx = Polygon_point[i].x;
//         else
//             b_list[i].jx = Polygon_point[i + 1].x;
//         if (Polygon_point[i].y != Polygon_point[i + 1].y)
//         {
//             //忽略斜率为无穷，即忽略水平线
//             b_list[i].dx = (float)(Polygon_point[i].x - Polygon_point[i + 1].x) /
//                            (Polygon_point[i].y - Polygon_point[i + 1].y);
//             b_list[i].sp = FALSE;
//         }
//         else
//         {
//             //dx取较大的x
//             b_list[i].dx = (Polygon_point[i].x > Polygon_point[i + 1].x) ? Polygon_point[i].x : Polygon_point[i + 1].x;
//             //jx取较小的x
//             b_list[i].jx = (Polygon_point[i].x < Polygon_point[i + 1].x) ? Polygon_point[i].x : Polygon_point[i + 1].x;
//             b_list[i].sp = TRUE;
//         }
//         b_list[i].ymax = (Polygon_point[i].y > Polygon_point[i + 1].y) ? Polygon_point[i].y : Polygon_point[i + 1].y;
//         b_list[i].ymin = (Polygon_point[i].y < Polygon_point[i + 1].y) ? Polygon_point[i].y : Polygon_point[i + 1].y;
//     }
//     //建立、初始化新边表
//     Xb xb;
//     Huo_list *h_list = new (Huo_list);
//     h_list->num = 0;
//     for (i = point_ymin; i < point_ymax;)
//     {
//         for (j = 0; j < num_point - 1; j++)
//             if (i == b_list[j].ymin)
//             {
//                 InsertHuo_list(h_list, &b_list[j]);
//             }
//         int n = 0;
//         xb.num = 0;
//         while (n < h_list->num)
//         {
//             if (h_list->next[n]->sp)
//             {
//                 xb.x[xb.num] = h_list->next[n]->jx;
//                 xb.num++;
//                 xb.x[xb.num] = h_list->next[n]->dx;
//                 xb.num++;
//             }
//             else
//             {
//                 xb.x[xb.num] = h_list->next[n]->jx + h_list->next[n]->dx * (i - h_list->next[n]->ymin);
//                 xb.num++;
//             }
//             n++;
//         }
//         pai_xuHuo_list(&xb);
//         bool tc = FALSE;
//         for (j = 0; j < xb.num; j++, j++)
//         {
//             int x, x1, xr;
//             x1 = xb.x[j];
//             xr = xb.x[j + 1];
//             x = x1;
//             while (x <= xr)
//             {
//                 glColor3f(COLOR_NEW);
//                 glVertex2i(x, i);
//                 x++;
//             }
//         }
//         if (i == point_ymin)
//             i--;
//         i++;
//         for (j = 0; j < h_list->num; j++)
//             if (i == h_list->next[j]->ymax)
//             {
//                 Deleteb_list(h_list, j);
//             }
//         if (i == point_ymin)
//             i++;
//     }
// }
// void LineDDA(int x0, int y0, int x1, int y1)

// {
//     float dy, dx, x, y, m;
//     dx = x1 - x0;
//     dy = y1 - y0;
//     if (dx != 0)
//     {

//         m = dy / dx;
//         if (m <= 1 && m >= -1)
//         {
//             y = y0;
//             for (x = x0; x <= x1; x++)

//             {
//                 y = y0;
//                 for (x = x0; x <= x1; x++)

//                 {
//                     glVertex2i(x, int(y + 0.5));
//                     y += m;
//                 }
//             }
//             if (m > 1 || m < -1)
//             {

//                 m = 1 / m;
//                 x = x0;
//                 for (y = y0; y <= y1; y++)
//                 {
//                     glVertex2i(int(x + 0.5), y);
//                     x += m;
//                 }
//             }
//         }
//         else
//         {
//             int x = x0, y;
//             y = (y0 <= y1) ? y0 : y1;
//             int d = fabs((double)(y0 - y1));
//             while (d >= 0)
//             {
//                 glVertex2i(x, y);
//                 y++;
//                 d--;
//             }
//         }
//     }
// }

// void ScanLineSegment()
// {
//     glClear(GL_COLOR_BUFFER_BIT);
//     glColor3f(0.0, 0.0, 0.0);
//     glBegin(GL_POINTS);
//     Point spt[8];
//     spt[0] = Point(230, 230);
//     spt[1] = Point(290, 230);
//     spt[2] = Point(275, 280);
//     spt[3] = Point(230, 280);
//     spt[4] = Point(245, 260);
//     spt[5] = Point(250, 254);
//     spt[6] = Point(235, 245);
//     spt[7] = Point(230, 230);
//     ScanLine(spt, 8);
//     glEnd();
//     glFlush();
// }
// // void main(int argc, char *argv[])
// // {
// //     // glutInit(&argc, argv);
// //     // glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
// //     // glutInitWindowPosition(50, 100);
// //     // glutInitWindowSize(500, 500);
// //     // glutCreateWindow("扫描填充算法");
// //     // glClearColor(1.0, 1.0, 1.0, 0.0);
// //     // gluOrtho2D(0.0, 500.0, 0.0, 500.0);
// //     // glutDisplayFunc(ScanLineSegment);
// //     // glutMainLoop();
// // }

// int main()
// {
//     // glfw: initialize and configure
//     // ------------------------------
//     glfwInit(); // Initialize GLFW
//     // glfWindowHint 配置GFLFW，这里说明了使用OpenGL3.3
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

//     glMatrixMode(GL_PROJECTION);

//     // For MacOS
// #ifdef __APPLE__
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
// #endif

//     // glfw window creation
//     // --------------------
//     GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Homework2 Naomi", NULL, NULL);
//     if (window == NULL)
//     {
//         std::cout << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         return -1;
//     }
//     // 通知GLFW将我们窗口的上下文设置为当前线程的主上下文
//     glfwMakeContextCurrent(window);
//     // 注册这个函数，告诉GLFW我们希望每当窗口调整大小的时候调用这个函数
//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

//     // glad: load all OpenGL function pointers 初始化GLAD
//     // ---------------------------------------
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cout << "Failed to initialize GLAD" << std::endl;
//         return -1;
//     }

//     // configure global opengl state
//     // -----------------------------
//     //glEnable(GL_DEPTH_TEST);

//     // build and compile shaders
//     // -------------------------
//     Shader ourShader("../shaders/vs.shader", "../shaders/fs.shader");

//         // set up vertex data (and buffer(s)) and configure vertex attributes
//         // ------------------------------------------------------------------

//         // Reading from txt------------------------------------------------------------------------------------
//         const char *filePath = "../resources/overlapping.txt";
//     float triangle_one[18], triangle_two[18], triangle_three[18], triangle_four[18];
//     FILE *mFile = fopen(filePath, "r");

//     if (mFile == NULL)
//         perror("Error opening file");
//     else
//     {
//         int count = 0;
//         while (!feof(mFile))
//         {
//             //fscanf(mFile, "%f %f %f ", &vertices[count], &vertices[count + 1], &vertices[count + 2]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f ", &vertices[count + 6], &vertices[count + 7], &vertices[count + 8]);   //Vertex 2 for triangle
//             //fscanf(mFile, "%f %f %f ", &vertices[count + 12], &vertices[count + 13], &vertices[count + 14]);   //Vertex 3 for triangle
//             //fscanf(mFile, "%f %f %f ", &vertices[count + 3], &vertices[count + 4], &vertices[count + 5]);   // Color for vertex 1
//             //fscanf(mFile, "%f %f %f ", &vertices[count + 9], &vertices[count + 10], &vertices[count + 11]);   //Color for vertex 2
//             //fscanf(mFile, "%f %f %f\n", &vertices[count + 15], &vertices[count + 16], &vertices[count + 17]);   //Color for vertex 3
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_one[count], &triangle_one[count + 1], &triangle_one[count + 2], &triangle_one[count + 6], &triangle_one[count + 7], &triangle_one[count + 8], &triangle_one[count + 12], &triangle_one[count + 13], &triangle_one[count + 14]);                          //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_one[count + 3], &triangle_one[count + 4], &triangle_one[count + 5], &triangle_one[count + 9], &triangle_one[count + 10], &triangle_one[count + 11], &triangle_one[count + 15], &triangle_one[count + 16], &triangle_one[count + 17]);                   //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_two[count], &triangle_two[count + 1], &triangle_two[count + 2], &triangle_two[count + 6], &triangle_two[count + 7], &triangle_two[count + 8], &triangle_two[count + 12], &triangle_two[count + 13], &triangle_two[count + 14]);                          //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_two[count + 3], &triangle_two[count + 4], &triangle_two[count + 5], &triangle_two[count + 9], &triangle_two[count + 10], &triangle_two[count + 11], &triangle_two[count + 15], &triangle_two[count + 16], &triangle_two[count + 17]);                   //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_three[count], &triangle_three[count + 1], &triangle_three[count + 2], &triangle_three[count + 6], &triangle_three[count + 7], &triangle_three[count + 8], &triangle_three[count + 12], &triangle_three[count + 13], &triangle_three[count + 14]);        //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_three[count + 3], &triangle_three[count + 4], &triangle_three[count + 5], &triangle_three[count + 9], &triangle_three[count + 10], &triangle_three[count + 11], &triangle_three[count + 15], &triangle_three[count + 16], &triangle_three[count + 17]); //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_four[count], &triangle_four[count + 1], &triangle_four[count + 2], &triangle_four[count + 6], &triangle_four[count + 7], &triangle_four[count + 8], &triangle_four[count + 12], &triangle_four[count + 13], &triangle_four[count + 14]);                 //Vertex 1 for triangle
//             fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &triangle_four[count + 3], &triangle_four[count + 4], &triangle_four[count + 5], &triangle_four[count + 9], &triangle_four[count + 10], &triangle_four[count + 11], &triangle_four[count + 15], &triangle_four[count + 16], &triangle_four[count + 17]);          //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_two[count], &triangle_two[count + 1], &triangle_two[count + 2], &triangle_two[count + 3], &triangle_two[count + 4], &triangle_two[count + 5], &triangle_two[count + 6], &triangle_two[count + 7], &triangle_two[count + 8]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_two[count], &color_two[count + 1], &color_two[count + 2], &color_two[count + 3], &color_two[count + 4], &color_two[count + 5], &color_two[count + 6], &color_two[count + 7], &color_two[count + 8]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_three[count], &triangle_three[count + 1], &triangle_three[count + 2], &triangle_three[count + 3], &triangle_three[count + 4], &triangle_three[count + 5], &triangle_three[count + 6], &triangle_three[count + 7], &triangle_three[count + 8]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_three[count], &color_three[count + 1], &color_three[count + 2], &color_three[count + 3], &color_three[count + 4], &color_three[count + 5], &color_three[count + 6], &color_three[count + 7], &color_three[count + 8]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f ", &triangle_four[count], &triangle_four[count + 1], &triangle_four[count + 2], &triangle_four[count + 3], &triangle_four[count + 4], &triangle_four[count + 5], &triangle_four[count + 6], &triangle_four[count + 7], &triangle_four[count + 8]);   //Vertex 1 for triangle
//             //fscanf(mFile, "%f %f %f %f %f %f %f %f %f\n", &color_four[count], &color_four[count + 1], &color_four[count + 2], &color_four[count + 3], &color_four[count + 4], &color_four[count + 5], &color_four[count + 6], &color_four[count + 7], &color_four[count + 8]);   //Vertex 1 for triangle
//             count += 9; // finished 1 triangle
//         }
//         fclose(mFile);
//         for (int i = 0; i < 18; i++)
//         {
//             //cout << vertices[i] << "\t";
//             //vertices[i] /= 100;
//             if (i == 0 || i == 1 || i == 2 || i == 6 || i == 7 || i == 8 || i == 12 || i == 13 || i == 14)
//             {
//                 triangle_one[i] = triangle_one[i] / 100.0f;
//                 triangle_two[i] = triangle_two[i] / 100.0f;
//                 triangle_three[i] = triangle_three[i] / 100.0f;
//                 triangle_four[i] = triangle_four[i] / 100.0f;
//             }
//             /*cout << triangle_one[i] << "\t";
//             cout << triangle_two[i] << "\t";
//             cout << triangle_three[i] << "\t";
//             cout << triangle_four[i] << "\t";
//             cout << endl;*/
//         }
//     }

//     unsigned int VBOs[4], VAOs[4], CVBOs[4], CVAOs[4];
//     glGenVertexArrays(4, VAOs); // we can also generate multiple VAOs or buffers at the same time
//     glGenBuffers(4, VBOs);

//     // first triangle setup
//     // --------------------
//     glBindVertexArray(VAOs[0]);
//     glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_one), triangle_one, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // Vertex attributes stay the same

//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

//     // // second triangle setup
//     // // ---------------------
//     glBindVertexArray(VAOs[1]);             // note that we bind to a different VAO now
//     glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]); // and a different VBO
//     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_two), triangle_two, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out

//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

//     // // third triangle setup
//     // // --------------------
//     glBindVertexArray(VAOs[2]);
//     glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_three), triangle_three, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // Vertex attributes stay the same

//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

//     // // fourth triangle setup
//     // // ---------------------
//     glBindVertexArray(VAOs[3]);             // note that we bind to a different VAO now
//     glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]); // and a different VBO
//     glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_four), triangle_four, GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0); // because the vertex data is tightly packed we can also specify 0 as the vertex attribute's stride to let OpenGL figure it out

//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

//     // uncomment this call to draw in wireframe polygons.
//     //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//     // render loop
//     // -----------
//     while (!glfwWindowShouldClose(window))
//     {

//         // render
//         // ------
//         glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         //glUseProgram(shaderProgram);
//         ourShader.use();
//         // draw first triangle using the data from the first VAO
//         glBindVertexArray(VAOs[0]);
//         glDrawArrays(GL_TRIANGLES, 0, 3);
//         // then we draw the second triangle using the data from the second VAO
//         glBindVertexArray(VAOs[1]);
//         glDrawArrays(GL_TRIANGLES, 0, 3);
//         // draw third triangle using the data from the first VAO
//         glBindVertexArray(VAOs[2]);
//         glDrawArrays(GL_TRIANGLES, 0, 3);
//         // draw the fourth triangle using the data from the second VAO
//         glBindVertexArray(VAOs[3]);
//         glDrawArrays(GL_TRIANGLES, 0, 3);

//         ScanLineSegment();

//         // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//         // -------------------------------------------------------------------------------
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     // optional: de-allocate all resources once they've outlived their purpose:
//     // ------------------------------------------------------------------------
//     glDeleteVertexArrays(2, VAOs);
//     glDeleteBuffers(2, VBOs);
//     //glDeleteProgram(ourShader);

//     // glfw: terminate, clearing all previously allocated GLFW resources.
//     // ------------------------------------------------------------------
//     glfwTerminate();
//     return 0;
// }