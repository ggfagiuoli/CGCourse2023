#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../../common/debugging.h"

int create_box2d(int x, int y){
    int nVertex = (x+1) * (y+1);
    float arr[5 * nVertex];
    float xPos[x+1];
    float yPos[y+1];
    //2 * (x+1) * (y+1) vertici
    //x, y, r, g, b
    float xStep = 0.2/x, yStep = 0.2/y;
    for(int i=0; i<x+1; i++){
        xPos[i] = -0.1 + (xStep * (i % (x+1)));
    }
    for(int i=0; i<y+1; i++){
        yPos[i] = -0.1 + (yStep * (i % (y+1)));
    }
    int colorPos = 0;
    for(int i=0; i<nVertex; i++){
        int temp = colorPos % 3;
        arr[5*i] = xPos[i % (x+1)];
        arr[5*i+1] = yPos[((int)i/(x+1))];
        arr[5*i+2] = 0.0;
        arr[5*i+3] = 0.5;
        arr[5*i+4] = 1.0;
    }
    GLuint indexes[x*y*6];
    int actualIndex = 0;
    for(int i=0; i<nVertex; i++){
        if((i+1)%(x+1)==0){continue;}
        if(((int)i/(x+1))==y){break;}
        //upper left triangle
        indexes[6*actualIndex] = i;
        indexes[6*actualIndex+1] = i+1;
        indexes[6*actualIndex+2] = i+x+1;
        //lower right triangle
        indexes[6*actualIndex+3] = i+x+1;
        indexes[6*actualIndex+4] = i+x+2;
        indexes[6*actualIndex+5] = i+1;
        actualIndex++;
    }


    GLuint positionIndex = 0;
    GLuint colorIndex = 1;

    GLuint indexBuffer;
    glCreateBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * x*y*6, indexes, GL_STATIC_DRAW);


    GLuint buffer;
    glCreateBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER,20 * nVertex, arr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionIndex);
    glEnableVertexAttribArray(colorIndex);
    glVertexAttribPointer(positionIndex, 2, GL_FLOAT, false, 20, 0);
    glVertexAttribPointer(colorIndex, 3, GL_FLOAT, false, 20, (void*)8);
    return x*y*6;
}

int main(void)
{

    GLFWwindow* window;

    if(!glfwInit())
        return -1;

    window = glfwCreateWindow(1000,800,"box2d",NULL,NULL);

    if(!window){
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    int nindexes = create_box2d(2,3);

    std::string vertex_shader_src = "#version 330\n \
    in vec2 aPosition;\
    in vec3 aColor;\
    out vec3 vColor;\
    void main(void)\
    {\
    gl_Position = vec4(aPosition, 0.0, 1.0);\
    vColor = aColor;\
    }";

    const GLchar* vshader_source = (const GLchar*)vertex_shader_src.c_str();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vshader_source, NULL);
    glCompileShader(vertex_shader);
    check_shader(vertex_shader);

    std::string fragment_shader_src ="#version 330\n \
    out vec4 color;\
    in vec3 vColor;\
    void main(void)\
    {\
    color = vec4(vColor, 1.0);\
    }";

    const GLchar* fshader_source = (const GLchar*)fragment_shader_src.c_str();
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fshader_source, NULL);
    glCompileShader(fragment_shader);
    check_shader(fragment_shader);

    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glBindAttribLocation(shader_program, 0, "aPosition");
    glBindAttribLocation(shader_program, 1, "aColor");
    glLinkProgram(shader_program);

    while(!glfwWindowShouldClose(window)){
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glDrawElements(GL_TRIANGLES,nindexes,GL_UNSIGNED_INT,NULL);
        glUseProgram(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
