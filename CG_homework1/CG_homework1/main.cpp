#include <GLFW/glfw3.h>
#include <../External/GL/glut.h>
#include <../External/glm-0.9.9.7/glm/glm.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/matrix_transform.hpp>
#include <../External/glm-0.9.9.7/glm/gtc/type_ptr.hpp>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

GLubyte image[SCREEN_HEIGHT][SCREEN_WIDTH][3] = { 0 };

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
        glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, image);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
// ----------------------------------------------------------------------
