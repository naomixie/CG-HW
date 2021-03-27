# 旋转车床加工模拟小软件

计算机图形学最后一次作业报告。

exe 文件在/build 目录下。

## 目录

- [使用说明](#使用说明)
  - [键盘操作](#键盘操作)
  - [鼠标操作](#鼠标操作)
- [渲染圆柱体](#渲染圆柱体)
- [切割](#切割)
- [Bezier 曲线](#Bezier曲线)
- [灯光与贴图](#灯光与贴图)
- [颗粒效果](#颗粒效果)
- [其他](#其他)

## 使用说明

这一章节将说明作业项目使用键盘外设的相关操作。

### 键盘操作

- [ 5，6，7，8，9，0 ] 控制灯光位置
- [ B ] 开启 Bezier 曲线绘画模式
- [ Enter ] （在 Bezier 曲线绘画模式下）画出 Bezier 曲线，并且渲染在频幕上
- [ R ] 重置得到基础圆柱体

### 鼠标操作

- [ 移动 ] 鼠标移动会连带刀具一起移动
- [ 左键 ] 鼠标左键用来在 Bezier 模式下画点

## 渲染圆柱体

对于圆柱体使用了一个比较简洁的数据结构。

    map <float,float>(x_coord, radius)

简单的来说就是讲圆柱体看成了由无数个圆（横截面）所组成的物体，因此只要记录特殊点的 x 轴位置与其半径便可以由 cos 跟 sin 函数来求得其圆上各个点的位置，由此来绘制曲面。

    face_vertex[j][0] = cos(p) * iter->second;
    face_vertex[j][1] = sin(p) * iter->second;
    face_vertex[j][2] = iter->first;

这个方法有一个坏处就是我一开始最简单的便是添加了两个节点，也就是物体的两端，结果发现在第一刀切下去时的效果不是很理想，因为他会直接绘制两端跟中点的曲面，这个效果往往是不符合常理的。因此在这之后我便添加了几个作为参考节点来增加他的精确度。

这个方法的好处便是我绕开了数组（Buffer）不能使用变量去定义这一问题。

在原始的方法中我们可能定义了确定的点数来渲染圆柱体，而每次有切割时我们不能够再往里面添加点了，只能取最近的点并改变其位置。

但我的方法是遍历了 map 中的所有点，并两辆进行绘制（曲面），这样的话实际上中间是可以灵活的添加和去除点的，不仅提高了精度（可以绘制无数个点），也不用在意绘制点数的多少。

## 切割

首先为了符合实际情况我规定了切割半径不能超过其半径的一般，并且不能从两端开始切割，不能从下边切割。
其次我们在切割时基本上就是继续使用 map 去寻找这个刀的 x 坐标在哪两个点的区间内，并使用这两个点的 x 坐标与半径去生成一个直线方程来得出在该 x 坐标上其表面的 y 坐标的值。再将其与刀的 y 坐标进行对比，得出其是否被切割。

![r1](/pics/r1.PNG)

## Bezier 曲线

Bezier 曲线实现参照了原理的公式，想要进入绘画 Bezier 曲线的模式需要按 B 键等待画板出现后须在画板位置上点四个点，再按 Enter 键来生成 Bezier 曲线。

为了让我们能够看清楚 Bezier 曲线的实际效果，我把 Bezier 曲线的看板直接放在了可以切削的范围内，使用 B 按键进入绘画模式（此时其他操作全部禁止），在点满四个点后按 Enter 便会在切割画面中显示一条 Bezier 曲线。

对于 Bezier 曲线的限制我们直接在碰撞检测（切削部分）之前对刀具的 y 坐标与 Bezier 曲线在该点的 y 坐标（用切割一样的方法计算），进行比较，如果判断刀具超过了 Bezier 曲线，则直接将 Bezier 曲线在此的 y 坐标取代刀具的 y 坐标。

## 灯光与贴图

打光参照了 OpenGl 教程，融合了 Ambient Lighting，Specular Lighting 与 diffuse Lighting 进行计算，在此不多加赘述。

对于木质材料与金属材料两种表面，我选择对他们的亮度进行了修改，以及两种材质选择了不同的贴图。

但我在实际应用中发现或许是我的绘画方式的原因，我生成的圆柱体的贴图并不是很自然，因此对于金属质地的材料我选择了用颜色进行绘制，木质材料则选择了木材。

对于两中材料的灯照也进行了选择，对与由于金属材质选择了颜色因此反光是直接使用颜色进行反射计算的，而对于木质材料则是选择分析了材质的 rgb 进行计算的。

## 颗粒效果

首先是给每一个小的颗粒设计了一个类，如下：

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
    
  分别设置了激活时间，是否在激活状态，消亡速度，颜色，位置，移动方向（三维），重力的变量，用来描述每个颗粒的运动状态。项目中设置了颗粒总数为100.
  
  设置了一个布尔值来判断是否改生成颗粒效果，它的值与切割是否发生等值，在判断进入了切割模式后再遍历所有的颗粒，如果已经处于激活状态则直接渲染其按照目前的运动状态在这一帧的位置，如果其没有在激活状态则按照目前切割的位置去初始化他的各项数值。
  
  这一帧的渲染结果是基于其上一帧的位置，加上移动（三维）的速度去生成的。
  
       //Move On The X Axis By X Speed
       paticle[loop].x += paticle[loop].xi / (slowdown * 1000);
       //Move On The Y Axis By Y Speed
       paticle[loop].y += paticle[loop].yi / (slowdown * 1000);
       //Move On The Z Axis By Z Speed
       paticle[loop].z += paticle[loop].zi / (slowdown * 1000);
       
   而颗粒的存活时间被用作其颜色的alpha值，效果就是剩下的存活时间越短其透明效果越明显
   
     glColor4f(paticle[loop].r，
               paticle[loop].g, 
               paticle[loop].b,
               //use life as alpha value, as particle dies,it becomes more and more transparent
               paticle[loop].life);
               
   最后根据每个颗粒不同的消亡速度（在初始化时给了每个颗粒不同的随机的消亡时间）去减少其存活时间，存活时间小于0后将其设为没有被激活，以备下一次渲染重新激活。
   
    //decrease particles' life
    paticle[loop].life -= paticle[loop].fade;

    //if particle is dead,rejuvenate it
    if (paticle[loop].life < 0.0f)
    {
        paticle[loop].active = false;
    }
    
## 其他

  第一次做这样规模的OpenGL项目，代码还是有很多漏洞的，希望自己以后有时间来优化一下。
  最后感谢肖老师跟助教们在课程过程中的帮助！
