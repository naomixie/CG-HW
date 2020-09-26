## Shaders

从基本意义上来说，着色器只是一种把输入转化为输出的程序。

### GLSL

着色器是使用一种叫 GLSL 的类 C 语言写成的。GLSL 是为图形计算量身定制的，它包含一些针对向量和矩阵操作的有用特性。

        #version version_number             //着色器的开头总是要声明版本
        in type in_variable_name;           //输入 变量
        in type in_variable_name;           //每个输入变量也叫顶点属性

        out type out_variable_name;         //输出变量

        uniform type uniform_name;          //uniform 函数

        // main函数，每个着色器的入口点都是main函数，在这个函数中我们处理所有的输入变量，
        // 并将结果输出到输出变量中。
        int main()
        {
        // 处理输入并进行一些图形操作
        ...
        // 输出处理过的结果到输出变量
        out_variable_name = weird_stuff_we_processed;
        }

我们能声明的顶点属性是有上限的，它一般由硬件来决定。OpenGL 确保至少有 16 个包含 4 分量的顶点属性可用，但是有些硬件或许允许更多的顶点属性，你可以查询 GL_MAX_VERTEX_ATTRIBS 来获取具体的上限：

        int nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;

通常情况下它至少会返回 16 个，大部分情况下是够用了。
