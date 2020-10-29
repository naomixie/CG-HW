#include <GLFW/glfw3.h>
#include <../External/GL/glut.h>
#include <../External/glm-0.9.9.7/glm/glm.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/matrix_transform.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/type_ptr.hpp>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

GLfloat image[SCREEN_HEIGHT][SCREEN_WIDTH][3] = { 0 };

    // -> read data which should look like: 3 points for a triangle, 3 points for each point's color
    // -> transform data to : edgeTable -> saves a list of all edges
    //                        activeTable -> saves a list of edges that cross with the scanline
    //                        edge -> saves the 系数 for 两点间的三维空间内的直线函数，有一个给x计算z的函数
    // -> start:
    //          -> read all data
    //          -> for each triangle: calculate the three edges for each triangle, including the 系数，ymax,ymin,xmax,xmin,zmax,zmin,rx
    //          -> save all edges to the edge table
    //          -> calculate the triangle colors individually
    // -> fill: 
    //          -> for(y = 0 ; y < screen_height; y++)
    //              -> for(all edges)
    //                  -> if (y between edge->ymin edge->ymax) then scanline y and edge cross.
    //                      -> get the x value of cross point (use y and the 系数, calculate the x value)
    //                      -> save this x value to edge->rx
    //                      -> save this edge to activeTable (or use map to auto 排序)
    //              -> now we have the edges that cross with the scanline y
    //              -> for(all active edges in activeTable)
    //                  -> compare the rx values of these edges, 同一个三角形的才可以配对，计算每一个三角形的x值范围
    //                  -> 比较这几个范围是否有交集，如果有，则对交集进行深度检测
    //                      -> if(有交集)
    //                          -> for(交集最小x;x < 交际最大x;x++)
    //                              -> 计算两条边的x处的z，并且比较，谁更小取谁的颜色
    //                      -> 没有交集的地方
    //                          -> for(x小的edge的x -> x大的edge的x)
    //                              -> 取三角形颜色
    //                      
    //                      

class Edge{
    public:
        int ymax;       
        int ymin;
        GLfloat xmax;
        GLfloat xmin;
        GLfloat zmax;
        GLfloat zmin;

        GLfloat dxz;    //计算自己xz直线的系数
        GLfloat bxz;
        GLfloat dxy;    //计算自己xy直线的系数
        GLfloat bxy;
        GLfloat dyz;    //计算自己yz直线的系数
        GLfloat byz;
        int id;         //记录在哪个三角型内

        Edge(GLfloat point1x,int point1y,GLfloat point1z,GLfloat point2x,int point2y,GLfloat point2z){
            xmax = point1x > point2x ? point1x: point2x;
            xmin = point1x > point2x ? point2x: point1x;
            ymax = point1y > point2y ? point1y: point2y;
            ymin = point1y > point2y ? point2y: point1y;
            zmax = point1z > point2z ? point1z: point2z;
            zmin = point1z > point2z ? point2z: point1z;

            // 得到三个斜率
            calculateDXZ(point1x,point1z,point2x,point2z);
            calculateDXY(point1x,point1y,point2x,point2y);
            calculateDYZ(point1y,point1z,point2y,point2z);

            
        }
        /*输入y切面上的两个顶点，与此时的*/
        GLfloat getZfromX(GLfloat x, GLfloat x1,GLfloat z1,GLfloat x2,GLfloat z2){
            calculateDXZ(x1,z1,x2,z2);
            return x*dxz + b;
        }

        void calculateDXZ(GLfloat x1,GLfloat z1,GLfloat x2,GLfloat z2){
            dxz = (z1-z2)/ (x1-x2);
            bxz = z1 - dxz * x1;
        }
        void calculateDXY(GLfloat x1,GLfloat y1,GLfloat y2,GLfloat y2){
            dxy = (y1-y2)/ (x1-x2);
            bxy = y1 - dxy * x1;
        }
        void calculateDYZ(GLfloat y1,GLfloat z1,GLfloat y2,GLfloat z2){
            dyz = (z1-z2)/ (y1-y2);
            byz = z1 - dyz * y1;
        }
}

// 用于计算切面内两点的xz关系
void calculateDXZ2(GLfloat x1,GLfloat z1,GLfloat x2,GLfloat z2, GLfloat &dxz2, GLfloat &bxz2){
            dxz2 = (z1-z2)/ (x1-x2);
            bxz2 = z1 - dxz * x1;
}

// 三角形的struct{
//    setZfunc()

//}
// 



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);




    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            image[y][x][0] = image[y][x][1] = image[y][x][2] = 255;
        }
    }
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        /* the first answer*/
        glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_FLOAT, image);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
// ----------------------------------------------------------------------
