#include <GLFW/glfw3.h>
#include <../External/GL/glut.h>
#include <../External/glm-0.9.9.7/glm/glm.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/matrix_transform.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/type_ptr.hpp>

#include <iostream>
#include<map>
#include<array>
#include<list>
using namespace::std;
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

// 三角形的边
class Edge {
public:
    GLfloat ox, oy;  //对角顶点
    GLfloat or , og, ob;    //对角顶点rgb值
    GLfloat bo;     //对角平行线截距

    GLfloat ymax;
    GLfloat ymin;
    GLfloat xmax;
    GLfloat xmin;
    GLfloat zmax;
    GLfloat zmin;

    GLfloat t;

    GLfloat rx;     //相交点的x值
    GLfloat rz;     //相交点的y值

    GLfloat dxz;    //计算自己xz直线的系数
    GLfloat bxz;
    GLfloat dxy;    //计算自己xy直线的系数
    GLfloat bxy;
    GLfloat dyz;    //计算自己yz直线的系数
    GLfloat byz;

    int id;         //记录在哪个三角型内

    Edge() {

    }

    Edge(GLfloat point1x, GLfloat point1y, GLfloat point1z, GLfloat point2x, GLfloat point2y, GLfloat point2z,int id, GLfloat ox, GLfloat oy, GLfloat or, GLfloat og, GLfloat ob) {
        xmax = point1x > point2x ? point1x : point2x;
        xmin = point1x > point2x ? point2x : point1x;
        ymax = point1y > point2y ? point1y : point2y;
        ymin = point1y > point2y ? point2y : point1y;
        zmax = point1z > point2z ? point1z : point2z;
        zmin = point1z > point2z ? point2z : point1z;

        this->ox = ox;
        this->oy = oy;
        this-> or = or;
        this->og = og;
        this->ob = ob;

        this->id = id;

        // 得到三个斜率
        calculateDXZ(point1x, point1z, point2x, point2z);
        calculateDXY(point1x, point1y, point2x, point2y);
        calculateDYZ(point1y, point1z, point2y, point2z);
        calculateBo();


    }
    /*Get x axis value from y axis value for this edge(when scanline is scanning)*/
    GLfloat getXfromY(GLfloat y) {
        if (isinf(dxy)|| dxy ==0.0f) {
            rx = xmin;
            return rx;
        }
        rx = (y - bxy) / dxy;
        /*cout << "Get x from y: " << rx << endl;
        cout << "dxy  = " << dxy << "bxy = " << bxy<<endl;*/
        
        return rx;
    }

    /*Get z axis value from y axis value for this edge(when scanline is scanning and we
    require the z value of the 相交点 so that we could calculate the dxz of y切面上的xz直线)*/
    GLfloat getZfromY(GLfloat y) {
        //cout << "Value of dyz: " << dyz <<"zman: "<<zmax<<"zmin: "<<zmin<<"ymax: "<<ymax<<"ymin: "<<ymin<< endl;
        if (isinf(dyz)|| dyz == 0.0f) {
            rz = zmin;
            return rz;
        }
        rz = y * dyz +byz;
        return rz;
    }

    void calculateDXZ(GLfloat x1, GLfloat z1, GLfloat x2, GLfloat z2) {
        dxz = (z1 - z2) / (x1 - x2);
        bxz = z1 - dxz * x1;
        if (x1 == x2) {
            bxz = zmin;
        }
    }
    // x横坐标，y纵坐标
    void calculateDXY(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2) {
        dxy = (y1 - y2) / (x1 - x2);
        bxy = y1 - dxy * x1;
        if (x1 == x2) {
            //垂直
            //bxy = ymin;
            bxy = y1;
            t = ymin;
        }
        if (dxy == 0.0f) {
            bxy = y1;
        }

    }
    // y横坐标 z纵坐标
    void calculateDYZ(GLfloat y1, GLfloat z1, GLfloat y2, GLfloat z2) {
        dyz = (z1 - z2) / (y1 - y2);
        byz = z1 - dyz * y1;
        if (y1 == y2) {
            //byz = zmin;
            byz = 0;
        }
    }
    // 计算经过对角顶点的，与该边平行的直线的截距
    void calculateBo() {
        bo = (dxy == 0.0f || isinf(dxy))  ? ox : oy - dxy * ox;
        bo = oy - dxy * ox;
        if (dxy == 0.0f) {
            bo = oy;
        }
        else if (isinf(dxy))
        {
            bo = ox;
        }
    }

    void calculateColors(GLfloat y, GLfloat x, GLfloat& red, GLfloat& green, GLfloat& blue) {
        GLfloat nbo  = y - dxy * x;
        if (dxy == 0.0f) {
            nbo = y;
        }
        else if(isinf(dxy))
        {
            nbo = x;
        }
        
        GLfloat totalLength = bxy > bo ? bxy - bo: bo - bxy ;     //
        GLfloat pLength = bxy > nbo ? bxy - nbo : nbo - bxy ;
        GLfloat ratio = pLength / totalLength;
        /*cout << "ratio: " << ratio << endl;
        cout << "x = " << x << "\ty =  " << y << "\tdxy =  " << dxy << endl;
        cout << "nbo = " << nbo << "\tbxy =  " << bxy << "\tbo =  " << bo << "\ttotalLength = " << totalLength << "\tpLength = " << pLength << endl;*/
        red = or *ratio;
        green = og * ratio;
        blue = ob * ratio;
    }
};

// 切面上的边
class QEdge {
    public:
        GLfloat dxz;    //xz斜率
        GLfloat bxz;    //截距

        GLfloat xmax;
        GLfloat xmin;
        GLfloat zmax;
        GLfloat zmin;

        GLfloat rz;

        QEdge(){}

        // 输入两个顶点数据求出函数数据
        QEdge(GLfloat x1, GLfloat z1, GLfloat x2, GLfloat z2) {
            xmax = x1 > x2 ? x1 : x2;
            xmin = x1 > x2 ? x2 : x1;
            zmax = z1 > z2 ? z1 : z2;
            zmin = z1 > z2 ? z2 : z1;
            dxz = (z1 - z2) / (x1 - x2);
            bxz = z1 - dxz * x1;
            if (x1 = x2) {
                //bxz = zmin;

            }
        }

        // 返回该切线内x值的点的深度
        GLfloat getZfromX(GLfloat x) {
            //cout << "Value of dyz: " << dxz << endl;
            if (isinf(dxz) || dxz == 0) {
                rz = zmin;
                return rz;
            }
            return x*dxz + bxz;
        }
};
struct infos {
    pair<GLfloat, GLfloat> minx;
    pair<GLfloat, GLfloat> maxx;
    int id;
};
class Range {
public:
    infos info;
    bool intersect = false;
    QEdge cutEdge;

    Range(){}

    Range(int id, GLfloat minx, GLfloat maxx, GLfloat minxz, GLfloat maxxz) {
        this->info.id = id;
        if (minx < maxx) {
            this->info.minx = pair<GLfloat, GLfloat>(minx, minxz);
            this->info.maxx = pair<GLfloat, GLfloat>(maxx, maxxz);
        }
        if (minx > maxx) {
            this->info.maxx = pair<GLfloat, GLfloat>(minx, minxz);
            this->info.minx = pair<GLfloat, GLfloat>(maxx, maxxz);
        }
        cutEdge = QEdge(minx, minxz, maxx, maxxz);
    }
};

Edge EdgeTable[12];      // Register all the edges in the graph
GLfloat Scanrangeminx = SCREEN_WIDTH, Scanrangemaxx = 0, Scanrangeminy = SCREEN_HEIGHT, Scanrangemaxy = 0;

void mixColors(int id, GLfloat x, GLfloat y, GLfloat& red, GLfloat& green, GLfloat& blue) {
    red = 0.0f, green = 0.0f, blue = 0.0f;
    for(int i = id*3; i < id*3 +3; i++){
        GLfloat tred = 0.0f, tgreen = 0.0f, tblue = 0.0f;
        EdgeTable[i].calculateColors(x, y, tred, tgreen, tblue);
        red += tred;
        green += tgreen;
        blue += tblue;
        //cout << "Edgeid: " << i << " \tColor for position (" << x << "," << y << ") is ( " << tred << ", " << tgreen << ", " << tblue << ")\n";

    }
    /*GLfloat total = red + green + blue;
    red = red / total;
    green = green / total;
    blue = blue / total;*/
    //cout << "id: "<<id<<" \tColor for position (" << x << "," << y << ") is ( " << red << ", " << green << ", " << blue << ")\n";

}

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



    // 背景为白色
    for (int x = 0; x < SCREEN_WIDTH; ++x) {
        for (int y = 0; y < SCREEN_HEIGHT; ++y) {
            image[y][x][0] = image[y][x][1] = image[y][x][2] = 1.0f;
        }
    }

    // Reading from txt------------------------------------------------------------------------------------
    const char* filePath = "../resources/overlapping.txt";
    GLfloat triangles[4][18];
    FILE* mFile = fopen(filePath, "r");
    int cnt = 0;
    if (mFile == NULL)
        perror("Error opening file");
    else
    {
        for (int i = 0; i < 4 && !feof(mFile); i++) {
            for (int j = 0; j < 18 && !feof(mFile); j++) {
                fscanf(mFile, "%f ", &triangles[i][j]);
                cout << triangles[i][j]<<"\t";
                if (j < 9 && j != 2 && j != 5 && j != 8) {
                    triangles[i][j] += 100;
                    triangles[i][j] *= 4;
                }
                if (j == 0 || j == 3 || j == 6) {
                    if (Scanrangemaxx < triangles[i][j]) Scanrangemaxx = triangles[i][j];
                    if (Scanrangeminx > triangles[i][j]) Scanrangeminx = triangles[i][j];
                }
                if (j == 1 || j == 4 || j == 7) {
                    if (Scanrangemaxy < triangles[i][j]) Scanrangemaxy = triangles[i][j];
                    if (Scanrangeminy > triangles[i][j]) Scanrangeminy = triangles[i][j];
                }
            }            
            // Register the edges
            Edge e1(triangles[i][0], triangles[i][1], triangles[i][2], triangles[i][3], triangles[i][4], triangles[i][5],i,triangles[i][6],triangles[i][7],triangles[i][15],triangles[i][16],triangles[i][17]);
            EdgeTable[cnt] = e1; // i indicates which triangle this edge belongs to
            Edge e2(triangles[i][3], triangles[i][4], triangles[i][5], triangles[i][6], triangles[i][7], triangles[i][8],i, triangles[i][0], triangles[i][1], triangles[i][9], triangles[i][10], triangles[i][11]);
            EdgeTable[cnt+1] = e2; // i indicates which triangle this edge belongs to
            Edge e3(triangles[i][0], triangles[i][1], triangles[i][2], triangles[i][6], triangles[i][7], triangles[i][8],i,triangles[i][3], triangles[i][4], triangles[i][12], triangles[i][13], triangles[i][14]);
            EdgeTable[cnt+2] = e3; // i indicates which triangle this edge belongs to
            cnt += 3;
            
        }
    }

    /*cout << "\nTriangles" << " contains edges: \n";
    for (int i = 0; i < 12 ; i++) {
        cout << "Edge " << i << ":\t\t xmin: "<< EdgeTable[i].xmin << "\txmax: " << EdgeTable[i].xmax << "\tymin: " << EdgeTable[i].ymin << "\tymax: " << EdgeTable[i].ymax << endl;
    }*/

    //cout << "\n Scanrange: minx: " << Scanrangeminx << "\t maxx: " << Scanrangemaxx << "\t miny: " << Scanrangeminy << "\t maxy: " << Scanrangemaxy << endl;

    // Scan the data
    for (int y = 0; y < /*(int)Scanrangemaxy*/800; y++) {
       /* cout << "----------BEGIN ONE SCAN ROUTINE---------------" << endl;
        cout << "Current ScanLine Position y = " << y << endl;*/

        list<Edge> ActiveTable;
        // 得到活边表
        /*cout << "SECTION COLLISION" << endl;
        cout << "------------------------------------------------" << endl;
        */
        for (int i = 0; i < 12; i++) {
            if (EdgeTable[i].ymin< y && EdgeTable[i].ymax >= y) {
                //说明扫描线与该边相交
                ActiveTable.push_back(EdgeTable[i]);
                //cout << "Colliding with line :" << i <<"\tid: "<< EdgeTable[i].id<< "\txmax: "<< EdgeTable[i].xmax<<"\txmin: " << EdgeTable[i].xmin << "\tymax: " << EdgeTable[i].ymax << "\tymin: " << EdgeTable[i].ymin << "\tzmax: " << EdgeTable[i].zmax << "\tzmin: " << EdgeTable[i].zmin << endl;
            }
        }
        /*cout << "SECTION RANGES" << endl;
        cout << "------------------------------------------------" << endl;*/

        // 配对
        list<Range> ranges;
        while (!ActiveTable.empty()) {
            //cout << "Check Active Table: size = " << ActiveTable.size() << endl;
            
            Edge e = ActiveTable.back();
            ActiveTable.pop_back();
            for (list<Edge>::iterator it = ActiveTable.begin(); it != ActiveTable.end(); it++) {
                if (it->id == e.id) {
                    // 配对成功,生成区间
                    /*cout << "Genererating Space : id =  "<< e.id<<"\t" << e.getXfromY(y) << " <-> " << it->getXfromY(y) << endl;
                    cout << "getXfromY dxy" << it->xmin << endl;
                    cout << "Creating QEdge with points: x1: " << e.getXfromY(y) << "\tx2: " << it->getXfromY(y) << "\tz1: " << e.getZfromY(y) << "\tz2: " << it->getZfromY(y)<<endl;*/
                    Range r(e.id, e.getXfromY(y), it->getXfromY(y),e.getZfromY(y),it->getZfromY(y));
                    ranges.push_back(r);
                    ActiveTable.erase(it);
                    break;
                }
            }
        }

        //cout << "SECTION Intersection" << endl;
        //cout << "------------------------------------------------" << endl;
        // 看各个区间是否有相交
        bool io = false;
        for (list<Range>::iterator it1 = ranges.begin(); it1 != ranges.end(); it1++) {
            ////cout << "it1: minx: " << it1->info.minx.first << "\tmaxx: " << it1->info.maxx.first << endl;
            for (int i = (int)it1->info.minx.first; i < (int)it1->info.maxx.first; i++) {
                /*cout << "---------------DRAWING SECTION---------------" << endl;
                cout << "Drawing pixel with y = " << y << " i = " << i << "with a color id: " << it1->info.id << endl;*/
                GLfloat r, g, b;
                mixColors(it1->info.id, y, i, r, g, b);
                image[y][i][0] = r; image[y][i][1] = g; image[y][i][2] = b;
            }
        }
        for (list<Range>::iterator it1 = ranges.begin(); it1 != ranges.end(); it1++) {
                
            for (list<Range>::iterator it2 = it1; it2 != ranges.end(); it2++) {
                if (it1 == it2) continue;
                //cout << "These are the 2 ranges that we are analyzing: " << endl;
                //cout << "Range 1: [" << it1->info.minx.first << " , " << it1->info.maxx.first<<" ] " << endl;
                //cout << "Range 2: [" << it2->info.minx.first << " , " << it2->info.maxx.first << " ] " << endl;
                if ((it1->info.minx.first > it2->info.minx.first && it1->info.minx.first < it2->info.maxx.first ) || (it1->info.maxx.first > it2->info.minx.first && it1->info.maxx.first < it2->info.maxx.first )
                    || (it2->info.minx.first > it1->info.minx.first && it2->info.minx.first < it1->info.maxx.first) || (it2->info.maxx.first > it1->info.minx.first && it2->info.maxx.first < it1->info.maxx.first)) {
                    //两个区间有交集
                    //排序得出三个区间
                    //cout << "There is an Intersection between these two ranges, divide them into 3 intersects: " << endl;
                    infos r1,r2,r3;
                    r1.minx = it1->info.minx.first < it2->info.minx.first ? it1->info.minx : it2->info.minx;
                    r1.id = it1->info.minx.first < it2->info.minx.first ? it1->info.id : it2->info.id;
                    r3.maxx = it1->info.maxx.first > it2->info.maxx.first ? it1->info.maxx : it2->info.maxx;
                    r3.id = it1->info.maxx.first > it2->info.maxx.first ? it1->info.id : it2->info.id;
                    r2.minx = r1.minx == it1->info.minx ? it2->info.minx : it1->info.minx;
                    r1.maxx = r2.minx;
                    r3.minx = r3.maxx == it1->info.maxx ? it2->info.maxx : it1->info.maxx;
                    r2.maxx = r3.minx;
                    //cout << "interect 1: [" << r1.minx.first << " , " << r1.maxx.first << " ] " << endl;
                    //cout << "intersect 2: [" << r2.minx.first << " , " << r2.maxx.first << " ] " << endl;
                    //cout << "intersect 3: [" << r3.minx.first << " , " << r3.maxx.first << " ] " << endl;

                    io = true;
                    for (int i = (int)r1.minx.first; i < (int)r1.maxx.first; i++) {
                        GLfloat r, g, b;
                        mixColors(r1.id, y, i, r, g, b);
                        image[y][i][0] = r; image[y][i][1] = g; image[y][i][2] = b;
                    }
                    for (int i = (int)r3.minx.first; i < (int)r3.maxx.first; i++) {
                        GLfloat r, g, b;
                        mixColors(r3.id, y, i, r, g, b);
                        image[y][i][0] = r; image[y][i][1] = g; image[y][i][2] = b;
                    }
                    //cout << "it1->info.id = " << it1->info.id << "\tit2->info.id = " << it2->info.id << endl;
                    for (int i = (int)r2.minx.first; i < (int)r2.maxx.first; i++) {
                        //cout << "it1->cutEdge.getZfromX(i) = " << it1->cutEdge.getZfromX(i) << "\tit2->cutEdge.getZfromX(i) = " << it2->cutEdge.getZfromX(i) << endl;
                        if (it1->cutEdge.getZfromX(i) > it2->cutEdge.getZfromX(i)) {
                            GLfloat r, g, b;
                            mixColors(it1->info.id, y, i, r, g, b);
                            image[y][i][0] = r; image[y][i][1] = g; image[y][i][2] = b;
                        }
                        else
                        {
                            GLfloat r, g, b;
                            mixColors(it2->info.id, y, i, r, g, b);
                            image[y][i][0] = r; image[y][i][1] = g; image[y][i][2] = b;
                        }
                    }
                }
                
                
            }
            
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

