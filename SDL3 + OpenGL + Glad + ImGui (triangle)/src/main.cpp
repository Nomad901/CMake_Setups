#define SDL_MAIN_HANDLED
#include <iostream>
#include <format>
#include <vector>
#include <SDL3/SDL.h>
#include <glad/glad.h>

int32_t screenWidth = 1280;
int32_t screenHeight = 720;

SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

bool gQuit = false;

// VAO
GLuint gVertexArrayObject = 0;
// VBP
GLuint gVertexBufferObject = 0;
// Program Object for our shaders 
GLuint gGraphicsPipelineProgram = 0;

const std::string gVertexShaderCode = R"(
	#version 410 core
	in vec4 position;
	void main()
	{
		gl_Position = vec4(position.x, position.y, position.z, position.w);
	}
)";
const std::string gVertexFragmentCode = R"(
	#version 410 core
	out vec4 color;
	void main()
	{
		color = vec4(1.0f, 0.5f, 0.0f, 1.0f);
	}
)";


void getOpenGLInfo()
{
	std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n'; 
	std::cout << "nRenderer: " << glGetString(GL_RENDERER) << '\n';
	std::cout << "Version: " << glGetString(GL_VERSION) << '\n';
	std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
}

void initializeProgram()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Couldnt initilize the video!\n";
		exit(1);
	}

	// SETTING VERSION 4.6
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4); // VERSION 4
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6); // VERSION .6 

	// TURNING OFF OLD FUNCTIONS 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	// DOUBLE BUFFER AND SMOOTH UPDATING 
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	gGraphicsApplicationWindow = SDL_CreateWindow("OpenGl", 
												  screenWidth, screenHeight,
												  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!gGraphicsApplicationWindow)
	{
		std::cout << "Couldnt initialize the window!\n";
		exit(1);
	}

	gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);
	if (!gOpenGLContext)
	{
		std::cout << "Couldnt initialize the gl context!\n";
		exit(1);
	}
	if (SDL_GL_MakeCurrent(gGraphicsApplicationWindow, gOpenGLContext) < 0)
	{
		std::cout << "Couldnt make the openGlContext current!\n";
		exit(1);
	}

	// loading function pointers into SDL from glad (initializing glad)
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		std::cout << "glad wasnt initialized!\n";
		exit(1);
	}
}

void vertexSpecification()
{
	//
	// lives on CPU.
	// setting our vertex positions
	//
	const std::vector<GLfloat> vertexPos =
	{
		// x    y     z
		-0.8f, -0.8f, 0.0f, // left vertex position
		 0.8f, -0.8f, 0.0f, // right vertex position
		 0.0f,  0.8f, 0.0f  // top vertex position 
	};

	// 
	// VAO
	//
	// generating VAO
	glGenVertexArrays(1, &gVertexArrayObject);
	// selecting VAO
	glBindVertexArray(gVertexArrayObject);

	// Start generating VBO
	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	// coppying our data on the CPU
	glBufferData(GL_ARRAY_BUFFER,
				 vertexPos.size() * sizeof(GLfloat), // in bytes
				 vertexPos.data(), 
				 GL_STATIC_DRAW /*how we intend to use our data*/);

	// how our information will be used 
	glEnableVertexAttribArray(0);
	// is used in order to figure out how we will be moving through our data
	glVertexAttribPointer(0,
						  /*x,y,z*/3,
						  GL_FLOAT,
						  GL_FALSE,
						  0,
						  (void*)0);
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
}

GLuint compileShader(GLuint pType, const std::string& pSourceCode)
{
	GLuint shaderObject = 0;

	if (pType == GL_VERTEX_SHADER)
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	else if (pType == GL_FRAGMENT_SHADER)
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// compilation proccess 
	const char* src = pSourceCode.c_str();
	glShaderSource(shaderObject, 1, &src, nullptr);
	glCompileShader(shaderObject);

	int32_t result;
	// retrieving compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int32_t length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length];
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);
		
		if (pType == GL_VERTEX_SHADER)
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed! " << errorMessages << '\n';
		else if(pType == GL_FRAGMENT_SHADER)
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed! " << errorMessages << '\n';
	
		delete[] errorMessages;
		glDeleteShader(shaderObject);
		return 0;
	}

	return shaderObject;
}

GLuint createShaderProgram(std::string_view pVertexShaderSource,
						   std::string_view pFragmentShaderSource)
{
	// create a new program object
	GLuint programObject = glCreateProgram();
	GLuint myVertexShader = compileShader(GL_VERTEX_SHADER, std::string(pVertexShaderSource));
	GLuint myFragmentShader = compileShader(GL_FRAGMENT_SHADER, std::string(pFragmentShaderSource));

	// merging our two shaders together into the one
	glAttachShader(programObject, myVertexShader);
	glAttachShader(programObject, myFragmentShader);
	glLinkProgram(programObject);

	//validate our program
	glValidateProgram(programObject);
	
	// once our final program object was created 
	// we can detach and erase our individual shaders
	glDetachShader(programObject, myVertexShader);
	glDetachShader(programObject, myFragmentShader);
	//
	// deleting our shaders
	//
	glDeleteShader(myVertexShader);
	glDeleteShader(myFragmentShader);

	return programObject;
}

void createGraphicsPipeline()
{
	gGraphicsPipelineProgram = createShaderProgram(gVertexShaderCode, gVertexFragmentCode);
}

void input()
{
	SDL_Event events;
	while (SDL_PollEvent(&events) != 0)
	{
		if (events.type == SDL_EVENT_QUIT)
		{
			gQuit = true;
			return;
		}
	}

}

void preDraw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// the background of the screen 
	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(1.f, 1.f, 0.f, 1.f);
	
	//
	// clear color buffer and depth buffer (the screen)
	//
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	// use our shader
	glUseProgram(gGraphicsPipelineProgram);
}

void draw()
{
	// enable our attributes
	glBindVertexArray(gVertexArrayObject);
	// select the VBO which we wanna use
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

	// render the data. 0 the beginning of the array, 3 - the end
	glDrawArrays(GL_TRIANGLES, 0, 3);
	// stopping using our current graphics pipeline
	glUseProgram(0);
}

void mainLoop()
{
	while (!gQuit)
	{
		input();
		preDraw();
		draw();

		// Update the screen
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}

void cleanUp()
{
	SDL_DestroyWindow(gGraphicsApplicationWindow);

	SDL_Quit();
}

int main(int argc, char* argv[])
{
	initializeProgram();
	vertexSpecification(); // giving some vertex data on gpu
	createGraphicsPipeline();
	getOpenGLInfo();
	mainLoop();
	cleanUp();

	return 0;
}
