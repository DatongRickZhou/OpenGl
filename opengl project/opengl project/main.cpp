#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

//window resize call back function, which will change glViewport to match new size
void windowResizeCallBack(GLFWwindow* window, int width, int height);

//user input function
void processInputs(GLFWwindow* window);

//show frames per second
void showFPS(GLFWwindow* window);


//Vertex Shader Program Source code
const char* vertexShadersource =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n" //e.g aPos representing positions can access like this aPos.x, aPos.y, aPos.z or even aPos[0] for the x, etc
"\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"//main job of vertex shader is to pass a gl_position to help with drawing, gl_position takes a vec4. this position is used to help construct geometry
"}\n\0";//\0 tells a character string where to end

//Fragment Shader Program Source code
const char* fragmentShadersource =
"#version 330 core\n"
"out vec4 FragColor; \n"//value passed onto next shader when this one is done
"\n"
"void main()\n"
"{\n"
"FragColor = vec4(1.0f,0.5f, 0.2f, 1.0f);\n"
"}\n\0";


void main(){
	glfwInit();
	//tell glfw which version of openGL to use: 3.3 core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//the first 3 in 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//the .3 of 3.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//core profile

	//build a window! 800x600 window
	GLFWwindow* window = glfwCreateWindow(800, 600, "My first openGL thing", NULL, NULL);
	//did it fail?
	if (window == NULL){
		cout << "Window failed to build" << endl;
		glfwTerminate();
		system("pause");
		return;
	}
	//make this the main window
	glfwMakeContextCurrent(window);

	//setup GLAD to help initialise the openGL stuff
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		//if it fails
		cout << "GLAD failed to initialise" << endl;
		glfwTerminate();
		system("pause");
		return;
	}
	//tell opengl which part of our 800,600 window is used for opengl
	glViewport(0, 0, 800, 600);//x,y,w,h

	//add window resize call back too the window, so when that event occurs, this function will be called for us
	glfwSetFramebufferSizeCallback(window, windowResizeCallBack);

	//SHADERS
	//generate a shader ID for our vertex shader
	int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);//gives id of new vertex shader
	//give this vertex shader our source code
	//params: shader id to work with
	//		how many shader source code strings do you want to give it?
	//		the source code you want to give it
	//      length of source string, or NULL if source code ends in a \0
	glShaderSource(vertexShaderID, 1, &vertexShadersource, NULL);
	//compile the code
	glCompileShader(vertexShaderID);

	//check for errors
	int success; //failed or not
	char infoLog[512]; //store error information
	//check compile status
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	//if failed
	if (!success){
		//get error message and display it
		glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
		cout << "Vertex Shader error!" << infoLog << endl;
		system("pause");
	}
	
	//fragment shader!
	int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentShadersource, NULL);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
		cout << "Fragment shader error!: " << infoLog << endl;
		system("pause");
	}
	//create a shader program which will link our whole graphics pipeline together
	int shaderProgramID = glCreateProgram();
	//attach our shaders
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);
	//link our shaders
	glLinkProgram(shaderProgramID);
	//check for shader linking errors
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(shaderProgramID, 512, NULL, infoLog);
		cout << "Shader Program Linking Error!: " << infoLog << endl;
		system("pause");
	}
	//once its linked together into 1 big shader program, we no longer need the individual smaller shaders
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	//setup triangle vertices
	//a vertex is a 3d coordinate, colour, texture coordinates, normal directions, etc
	//3 points of our triangle as xyz
	float vertices[] = {
		0.5f, -0.5f, 0, //bottom right
		-0.5f, -0.5f, 0, //bottom left
		0.0f, 0.5f, 0 //top
	};
	//these current positions are based on the the NDC(normalised device coordinates where x and y span -1 to 1)

	//VBO - Vertex Buffer Object
	//currently vertices is stored in RAM and we want stored on the GPU somewhere. So we make a buffer object for it to store this data on the gpu
	unsigned int vboID1; //we need to store the id of the created vertex buffer object somewhere to use it in our code when we want to
	//generate a VBO and set the new VBO's ID into our vboID1 variable.
	glGenBuffers(1, &vboID1);//params: how many VBO's to make, where to store the new VBO's ids
	//example of generating multiple at a time
	//use array instead: unsigned int VBOs[6];
	//glGenBuffers(6, VBOs);//will feed the 6 new VBO ids into the 6 slots of the array

	//generate a Vertex Array Object(VAO), helps describe what the raw VBO data is
	unsigned int vaoID1;
	glGenVertexArrays(1, &vaoID1);

	//bind this vao to be the current working vao right now
	glBindVertexArray(vaoID1);

	//you can only have 1 VBO bound at a time in opengl, but you can swap whenever you want. Lets bind our first VBO
	glBindBuffer(GL_ARRAY_BUFFER, vboID1);//params: type of buffer to bind, the id of the buffer I want to bind to

	//pass data to the currently binded VBO on the graphics card
	//params: type of buffer,
	//		  size of data to give to the buffer
	//        data to give to the buffer
	//		  type of data: static, dynamic, stream
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//using current VAO, record how to pass vertex data from our VBO to our Vertex Shader
	//params: 0 = location 0 slot of the vertex shader (for us, thats where aPos is located)
	//		 aPos is expecting 3 values (vec3), so we'll pass it 3 values at a time
	//       each of the 3 values are GL_FLOATs
	//		 Normalise it? (if  < -1 or >1, cut the numbers down), false means no, dont screw with my vertex numbers
	//		 stride, size of how many floats to jump at a time to find the next vertex (3 values make up x,y,z so to get to the second ,x,y,z we jump 3)
	//		 how many floats into this array is where I should start reading this information (0 means start) NOTE: must always be void* for this one
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//turn on the location 0 slot(aPos) in the vertex shader
	glEnableVertexAttribArray(0);// this is where vec3 aPos is located, location = 0

	//bind to no VAO so we dont accidentally screw up our VAO info
	glBindVertexArray(0);


	//GAME LOOP
	while (!glfwWindowShouldClose(window)){
		//user inputs
		processInputs(window);
		//RENDER STUFF
		//set clear colour
		glClearColor(0, 0, 1, 1);//rgba, values 0-1
		//clear screen
		glClear(GL_COLOR_BUFFER_BIT);

		//DRAW THE MOTHERFUCKIN TRIANGLE!!
		//tell it which shader to use
		glUseProgram(shaderProgramID);
		//we then bind the VAO we want to draw from
		glBindVertexArray(vaoID1);
		//draw the vertex array
		//params: primitive type (triangles, points, line_strip)
		//		  starting index to start drawing from in our VBO data
		//		  how many vertexes to draw from our VBO
		glDrawArrays(GL_TRIANGLES,	0, 3);

		//process any polled events
		glfwPollEvents();

		//openGL uses double buffering, so to make these render changes visual
		//we swap the working buffer with the currently showing buffer
		glfwSwapBuffers(window);

		showFPS(window);
	}

	glfwTerminate();
}

//window resize call back function, which will change glViewport to match new size
void windowResizeCallBack(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

//user input function
void processInputs(GLFWwindow* window){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
	}
}

//show frames per second
void showFPS(GLFWwindow* window){
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime();//number of seconds since start of program

	elapsedSeconds = currentSeconds - previousSeconds;
	if (elapsedSeconds > 0.25){
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		stringstream ss;
		ss.precision(3);//3  decimal places
		ss << fixed << "FPS: " << fps << " Frame Time: " << msPerFrame << "(ms)" << endl;

		glfwSetWindowTitle(window, ss.str().c_str());

		frameCount = 0;
	}

	frameCount++;
}