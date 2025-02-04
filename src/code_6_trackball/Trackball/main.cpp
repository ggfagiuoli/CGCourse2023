#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "../../common/debugging.h"
#include "../../common/renderable.h"
#include "../../common/shaders.h"
#include "../../common/simple_shapes.h"
#include "../../common/matrix_stack.h"


#include <glm/glm.hpp>
#include <glm/ext.hpp>


/* projection matrix*/
glm::mat4 proj;

/* view matrix*/
glm::mat4 view;

glm::vec3 lastPointOnSphere;

bool isLastPointSet = false;
/* a bool variable that indicates if we are currently rotating the trackball*/
bool is_trackball_dragged;


float calculateRotation(glm::vec3 p0, glm::vec3 p1, glm::vec3 center){
    float x = glm::asin(glm::length(glm::cross(p0-center,p1-center))/(glm::length(p0-center)*glm::length(p1-center)));
    return x;
}

glm::vec3 calculateVector(glm::vec3 p0, glm::vec3 p1, glm::vec3 center){
    float len = (float)glm::length(glm::cross(p0-center,p1-center));
    if(len==0)return glm::vec3(0,0,0);
    glm::vec3 vector3 = glm::cross(p0-center,p1-center)/len;
    return glm::normalize(vector3);
}

/* callbakc function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
   // std::cout << xpos << " " << ypos << " " << std::endl;
	if (!is_trackball_dragged)
		return;

    //std::cout <<"drag "<< std::endl;
	/* here the code to create the rotation to apply before rendering the scene.
	1. build the ray from (0,0,0) in view space going through the window into the scene
	2. check if the ray intersect the sphere centered in (0,0,0), in world space. 
	   Try different values for the sphere radius. radius = 2 will be fine.
	   You also need to store the previous intersection (found in the previous invocation of
	   this function) in order to have p0 and p1
	3. with p0 and p1 compute the rotation vector and angle as seen in the slides
	4. with glm::rotate create the rotation matrix
	
	*BE CAREFUL at point 2*: the ray and sphere must be in the same  frame when computing
	the intersection. 

	*/

    //intersezione sfera=length(cross(direction,position)) / length(direction) < radius
    glm::vec3 rayStart=glm::unProject(glm::vec3(xpos,800-ypos,0),glm::mat4(1.f)*view,proj,glm::vec4(0,0,1000,800));
    glm::vec3 rayEnd=glm::unProject(glm::vec3(xpos,800-ypos,1),glm::mat4(1.f)*view,proj,glm::vec4(0,0,1000,800));
    glm::vec3 ray = glm::vec3(rayEnd - rayStart);
    bool intersects = glm::length(glm::cross(ray,glm::vec3(glm::inverse(view)[3]))) / glm::length(ray) < 2;
    //std::cout<<intersects<<"\n";
    if(!intersects){isLastPointSet = false; return;}
    glm::vec3 point = glm::vec3(0,0,0) - ray*2.f;
    if(!isLastPointSet){lastPointOnSphere = point; isLastPointSet = true;}
    float rotAngle = calculateRotation(lastPointOnSphere,point,glm::vec3(0,0,0));
    glm::vec3 rotVec = calculateVector(lastPointOnSphere,point,glm::vec3(0,0,0));
    if(rotVec[0] == 0.f && rotVec[1] == 0.f && rotVec[2] == 0.f){return;}
    std::cout<<rotVec[0]<<" "<<rotVec[1]<<" "<<rotVec[2];
    view =view * glm::inverse(glm::rotate(glm::mat4(1.f),glm::radians(rotAngle*50),rotVec));
    //std::cout<<rotAngle<<"\n";
    lastPointOnSphere = point;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		/* here the button is pressed  and hence the dragging of the trackball can start*/
        //std::cout << " GLFW_MOUSE_BUTTON_LEFT PRESSED" << std::endl;
        is_trackball_dragged = true;
	}
	else 
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            /* here the button is released and hence the dragging of the trackball ends*/
            //std::cout << " GLFW_MOUSE_BUTTON_LEFT RELEASED" << std::endl;
            isLastPointSet = false;
            is_trackball_dragged = false;
	}
}


int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(1000, 800, "code_6_trackball", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	/* declare the callback functions on mouse events */
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();

	printout_opengl_glsl_info();

	/* load the shaders */
	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	basic_shader.bind("uP");
	basic_shader.bind("uV");
	basic_shader.bind("uT");
	basic_shader.bind("uColor");
	check_shader(basic_shader.vs);
	check_shader(basic_shader.fs);
	validate_shader_program(basic_shader.pr);

	/* Set the uT matrix to Identity */
	glUseProgram(basic_shader.pr);
	glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	check_gl_errors(__LINE__, __FILE__);

	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube = shape_maker::cube(0.5, 0.3, 0.0);

	/* create 3 lines showing the reference frame*/
	renderable r_frame = shape_maker::frame(4.0);

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
    proj = glm::frustum(-1.f, 1.f, -0.8f, 0.8f, 2.f, 100.f);
    view = glm::lookAt(glm::vec3(0,6,8.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	glEnable(GL_DEPTH_TEST);

	matrix_stack stack;

	/* define the viewport  */
    glViewport(0, 0, 1000, 800);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
	

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
			glUseProgram(basic_shader.pr);
			glUniformMatrix4fv(basic_shader["uP"], 1, GL_FALSE, &proj[0][0]);
			glUniformMatrix4fv(basic_shader["uV"], 1, GL_FALSE, &view[0][0]);
			check_gl_errors(__LINE__, __FILE__);


			r_cube.bind();
			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 1.0, 0.2)));
			glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.0, 1.0, 0.0);
			glDrawElements(GL_TRIANGLES, r_cube.in, GL_UNSIGNED_INT, 0);
			stack.pop();

			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1, 0.2, 0.2)));
			glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
			glDrawElements(GL_TRIANGLES, r_cube.in, GL_UNSIGNED_INT, 0);
			stack.pop();

			stack.push();
			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2f, 0.2f, 1.f)));
			glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
			glDrawElements(GL_TRIANGLES, r_cube.in, GL_UNSIGNED_INT, 0);
			stack.pop();
			/* ******************************************************/

		glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], -1.0, 0.0, 1.0);
		r_frame.bind();
		glDrawArrays(GL_LINES, 0, 6);

		check_gl_errors(__LINE__, __FILE__);
		glUseProgram(0);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
