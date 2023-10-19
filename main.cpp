
/*
Student Information
Student ID: 1155190882
Student Name: Zerlina Zilan LAI
*/

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/stb_image/stb_image.h"

#include "Shader.h"
#include "Texture.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

// screen setting
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

GLint programID;
double mousex, mousey;
double dx = 0;
double dy = 0;
double dz = 0;

// struct for storing the obj file
struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
	glm::vec3 normal;
};

struct Model {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

Model loadOBJ(const char* objPath)
{
	// function to load the obj file
	// Note: this simple function cannot load all obj files.

	struct V {
		// struct for identify if a vertex has showed up
		unsigned int index_position, index_uv, index_normal;
		bool operator == (const V& v) const {
			return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
		}
		bool operator < (const V& v) const {
			return (index_position < v.index_position) ||
				(index_position == v.index_position && index_uv < v.index_uv) ||
				(index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
		}
	};

	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	std::map<V, unsigned int> temp_vertices;

	Model model;
	unsigned int num_vertices = 0;

	std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

	std::ifstream file;
	file.open(objPath);

	// Check for Error
	if (file.fail()) {
		std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
		exit(1);
	}

	while (!file.eof()) {
		// process the object file
		char lineHeader[128];
		file >> lineHeader;

		if (strcmp(lineHeader, "v") == 0) {
			// geometric vertices
			glm::vec3 position;
			file >> position.x >> position.y >> position.z;
			temp_positions.push_back(position);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			// texture coordinates
			glm::vec2 uv;
			file >> uv.x >> uv.y;
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			// vertex normals
			glm::vec3 normal;
			file >> normal.x >> normal.y >> normal.z;
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			// Face elements
			V vertices[3];
			for (int i = 0; i < 3; i++) {
				char ch;
				file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
			}

			// Check if there are more than three vertices in one face.
			std::string redundency;
			std::getline(file, redundency);
			if (redundency.length() >= 5) {
				std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
				std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
				std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
				exit(1);
			}

            // change to opengl format
            
			for (int i = 0; i < 3; i++) {
				if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
					// the vertex never shows before
					Vertex vertex;
					vertex.position = temp_positions[vertices[i].index_position - 1];
					vertex.uv = temp_uvs[vertices[i].index_uv - 1];
					vertex.normal = temp_normals[vertices[i].index_normal - 1];

					model.vertices.push_back(vertex);
					model.indices.push_back(num_vertices);
					temp_vertices[vertices[i]] = num_vertices;
					num_vertices += 1;
				}
				else {
					// reuse the existing vertex
					unsigned int index = temp_vertices[vertices[i]];
					model.indices.push_back(index);
				}
			} // for
		} // else if
		else {
			// it's not a vertex, texture coordinate, normal or face
			char stupidBuffer[1024];
			file.getline(stupidBuffer, 1024);
		}
	}
	file.close();

	std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
	return model;
}

GLuint loadTexture(const char* texturePath){
    // tell stb_image.h to flip loaded texture's on the y-axis.
    stbi_set_flip_vertically_on_load(true);
    // load the texture data into "data"
    int Width, Height, BPP;
    unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
    GLenum format=3;
    switch (BPP) {
        case 1: format = GL_RED; break;
        case 3: format = GL_RGB; break;
        case 4: format = GL_RGBA; break;
    }
    // Create a OGL texture
    // bind (all future texture fxs will modify this texture)
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    // send image to OGL then free our version
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load texture: " << texturePath << std::endl;
        exit(1);
    }

    std::cout << "Load " << texturePath << " successfully!" << std::endl;
    return texID;
}


void get_OpenGL_info()
{
	// OpenGL information
	const GLubyte* name = glGetString(GL_VENDOR);
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* glversion = glGetString(GL_VERSION);
	std::cout << "OpenGL company: " << name << std::endl;
	std::cout << "Renderer name: " << renderer << std::endl;
	std::cout << "OpenGL version: " << glversion << std::endl;
}

bool checkStatus(
    GLuint objectID,
    PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
    GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* buffer = new GLchar[infoLogLength];

        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
        std::cout << buffer << std::endl;

        delete[] buffer;
        return false;
    }
    return true;
}

bool checkShaderStatus(GLuint shaderID) {
    return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
    return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
    std::ifstream meInput(fileName);
    if (!meInput.good()) {
        std::cout << "File failed to load ... " << fileName << std::endl;
        exit(1);
    }
    return std::string(
        std::istreambuf_iterator<char>(meInput),
        std::istreambuf_iterator<char>()
    );
}

void installShaders() {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* adapter[1];
    //adapter[0] = vertexShaderCode;
//    std::string temp = readShaderCode("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/VertexShaderCode.glsl");
    std::string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);
    //adapter[0] = fragmentShaderCode;
//    temp = readShaderCode("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/FragmentShaderCode.glsl");
    temp = readShaderCode("FragmentShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
        return;

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    if (!checkProgramStatus(programID))
        return;

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    glUseProgram(programID);
}

// global vars
GLuint vao1, vao2, vao3;
GLuint vbo1, vbo2, vbo3;
GLuint ebo1, ebo2, ebo3;

Model tiger;
Model ground;
Model bluejay;

void loadBluejay() {
    // Load .obj file:
    bluejay = loadOBJ("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/Birds/Nutcracker.obj");
    
    // VAO
    glGenVertexArrays(1, &vao3);
    glBindVertexArray(vao3);
    
    // VBO
    glGenBuffers(1, &vbo3);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glBufferData(GL_ARRAY_BUFFER, bluejay.vertices.size()*sizeof(Vertex), &bluejay.vertices[0], GL_STATIC_DRAW);
    
    // EBO
    glGenBuffers(1, &ebo3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bluejay.indices.size()*sizeof(unsigned int), &bluejay.indices[0], GL_STATIC_DRAW);
    
    // send VBO attributes to shader
    // position
    glBindBuffer(GL_ARRAY_BUFFER, vbo3);
    glEnableVertexAttribArray(0);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // 2nd attrib
    glEnableVertexAttribArray(1);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        
    // 3rd attrib
    glEnableVertexAttribArray(2);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadTiger() {
    // Load .obj file for cat:
    tiger = loadOBJ("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/cat/cat_2.obj");
    
    // VAO
    glGenVertexArrays(1, &vao1);
    glBindVertexArray(vao1);
    
    // VBO
    glGenBuffers(1, &vbo1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glBufferData(GL_ARRAY_BUFFER, tiger.vertices.size()*sizeof(Vertex), &tiger.vertices[0], GL_STATIC_DRAW);
    
    // EBO
    glGenBuffers(1, &ebo1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tiger.indices.size()*sizeof(unsigned int), &tiger.indices[0], GL_STATIC_DRAW);
    
    // send VBO attributes to shader
    // position
    glBindBuffer(GL_ARRAY_BUFFER, vbo1);
    glEnableVertexAttribArray(0);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // 2nd attrib
    glEnableVertexAttribArray(1);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        
    // 3rd attrib
    glEnableVertexAttribArray(2);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void loadGround(){
    // load ground
    ground = loadOBJ("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/ground/ground.obj");
    
    // vao
    // VAO
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    
    // VBO
    glGenBuffers(1, &vbo2);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glBufferData(GL_ARRAY_BUFFER, ground.vertices.size()*sizeof(Vertex), &ground.vertices[0], GL_STATIC_DRAW);
    
    // EBO
    glGenBuffers(1, &ebo2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ground.indices.size()*sizeof(unsigned int), &ground.indices[0], GL_STATIC_DRAW);
    
    // send VBO attributes to shader
    // position
    glBindBuffer(GL_ARRAY_BUFFER, vbo2);
    glEnableVertexAttribArray(0);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // 2nd attrib
    glEnableVertexAttribArray(1);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
        
    // 3rd attrib
    glEnableVertexAttribArray(2);
    // attribute, size, type, normalized, stride, array buffer offset
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void sendDataToOpenGL()
{
    //Load objects and bind to VAO and VBO
    //Load textures
    
    loadTiger();
    loadBluejay();
    loadGround();
  
}


void initializedGL(void) //run only once
{
	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW not OK." << std::endl;
	}

	get_OpenGL_info();
	sendDataToOpenGL();
    installShaders();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

// more global vars
GLuint TextureTiger1;
GLuint TextureTiger2;
GLuint groundTexture1;
GLuint groundTexture2;
GLuint BluejayTexture;
int tigtex = 1;
int groundtex = 3;
GLfloat dirBrightness = 0;
int tigerMove = 0;
int tigerTurn = 0;

void paintGL(void)
{
    // always run
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  //specify the background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // view matrix
    // lookAt params: eye position, center position, up vector
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f + 0.1f * dx, 1.2f + 0.1f * dy, -8.0f - dz), glm::vec3(0.0f + 0.05f * dx, 1.0f + 0.05f * dy, 0.0f- dz),  glm::vec3(0.0f, 1.0f, 0.0f));
    GLint viewMatrixUniformLoc = glGetUniformLocation(programID, "viewMatrix");
    glUniformMatrix4fv(viewMatrixUniformLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    
    GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePosition");
    glm::vec3 eyePosition(0.0f + 0.1f * dx, 1.2f + 0.1f * dy, -8.0f - dz);
    glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

	//Set lighting information, such as position and color of lighting source
    
    // diffuse lights location
    GLint lightPositionUniformLocation = glGetUniformLocation(programID, "LightPositionWorld");
    glm::vec3 lightPosition(50.0f, 15.0f, -50.0f);
    glUniform3fv(lightPositionUniformLocation, 1, &lightPosition[0]);
	
    GLint diffuse2UniformLocation = glGetUniformLocation(programID, "diffuse2Position");
    glm::vec3 diffuse2pos(-5.0f, 10.0f, 8.0f);
    glUniform3fv(diffuse2UniformLocation, 1, &diffuse2pos[0]);
    
    //task 3a: lighting brightness control
    GLint dirBrightnessUniformLocation = glGetUniformLocation(programID, "dirBrightness");
    glUniform1f(dirBrightnessUniformLocation, dirBrightness);

    
    //Set transformation matrix
	//Bind different textures
    
    // Cat
    glBindVertexArray(vao1);
    
    //scale transformation
    glm::mat4 scaleMatrix = glm::mat4(1.0f);
    scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.05,0.05,0.05));
    GLint scaleMatrixUniformLocation = glGetUniformLocation(programID, "scaleMatrix");
    glUniformMatrix4fv(scaleMatrixUniformLocation, 1, GL_FALSE, &scaleMatrix[0][0]);
    
    // rotation to orient tiger head as front
    glm::mat4 rotateTigMatrix = glm::rotate(glm::mat4(1.0f), 0.78f, glm::vec3(0, 1, 0));
    GLint rotateTigMatrixUniformLocation = glGetUniformLocation(programID, "rotateTigMatrix");
    glUniformMatrix4fv(rotateTigMatrixUniformLocation, 1, GL_FALSE, &rotateTigMatrix[0][0]);
    
    // rotate tiger when L/R arrow keys pressed
    glm::mat4 rotateMatrix = glm::rotate(glm::mat4(1.0f), tigerTurn * -0.2f, glm::vec3(0, 1, 0));
    GLint rotateMatrixUniformLocation = glGetUniformLocation(programID, "rotateMatrix");
    glUniformMatrix4fv(rotateMatrixUniformLocation, 1, GL_FALSE, &rotateMatrix[0][0]);
    
    // translate tiger when up/down arrow keys
    glm::mat4 translateMatrix = glm::mat4(1.0f);
    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, -6.f, 0.3f * tigerMove));
    GLint translateMatrixUniformLocation =
    glGetUniformLocation(programID, "translateMatrix");
    glUniformMatrix4fv(translateMatrixUniformLocation, 1,
                       GL_FALSE, &translateMatrix[0][0]);
    
    // tiger texture
    unsigned int slot = 0;
    GLuint texID = glGetUniformLocation(programID, "textureSampler0");
    glActiveTexture(GL_TEXTURE0+slot);
    if(tigtex == 1){
        glBindTexture(GL_TEXTURE_2D, TextureTiger1);
    }
    else{glBindTexture(GL_TEXTURE_2D, TextureTiger2); }
    glUniform1i(texID, slot);
    
    glDrawElements(GL_TRIANGLES, tiger.indices.size(), GL_UNSIGNED_INT, 0);
    
    
    glBindVertexArray(0);
    
    // ground texture
    glBindVertexArray(vao2);

    //  remove tiger transforms
    // rotation to orient tiger head as front
    rotateTigMatrix = glm::mat4(1.0f);
    rotateTigMatrixUniformLocation = glGetUniformLocation(programID, "rotateTigMatrix");
    glUniformMatrix4fv(rotateTigMatrixUniformLocation, 1, GL_FALSE, &rotateTigMatrix[0][0]);
    rotateMatrix = glm::mat4(1.0f);
    rotateMatrixUniformLocation = glGetUniformLocation(programID, "rotateMatrix");
    glUniformMatrix4fv(rotateMatrixUniformLocation, 1, GL_FALSE, &rotateMatrix[0][0]);

    translateMatrix = glm::mat4(1.0f);
    translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 0.7f, 0.0f));
    translateMatrixUniformLocation =
    glGetUniformLocation(programID, "translateMatrix");
    glUniformMatrix4fv(translateMatrixUniformLocation, 1,
                       GL_FALSE, &translateMatrix[0][0]);
    
    //scale transformation
    scaleMatrix = glm::mat4(1.0f);
    scaleMatrixUniformLocation = glGetUniformLocation(programID, "scaleMatrix");
    glUniformMatrix4fv(scaleMatrixUniformLocation, 1, GL_FALSE, &scaleMatrix[0][0]);
    
    texID = glGetUniformLocation(programID, "textureSampler0");
    glActiveTexture(GL_TEXTURE0+slot);
    if(groundtex == 3){
        glBindTexture(GL_TEXTURE_2D, groundTexture1);
    }
    else{
        glBindTexture(GL_TEXTURE_2D, groundTexture2);
    }
    glUniform1i(texID, slot);
    glDrawElements(GL_TRIANGLES, ground.indices.size(), GL_UNSIGNED_INT, 0);
    
    // circle of birds
    glBindVertexArray(vao3);
    
    // have 8 floating birds surrounding the cat in a circle
    // 45 degrees = 0.79 rad
    
    // texture
    texID = glGetUniformLocation(programID, "textureSampler0");
    glActiveTexture(GL_TEXTURE0+slot);
    glBindTexture(GL_TEXTURE_2D, BluejayTexture);
    glUniform1i(texID, slot);
    
    float x = 0;
    float z = 0;

    for (int i = 0; i < 8; i++){
        
        rotateMatrix = glm::rotate(glm::mat4(1.0f), 0.79f * i, glm::vec3(0, 1, 0));
        rotateMatrixUniformLocation = glGetUniformLocation(programID, "rotateMatrix");
        glUniformMatrix4fv(rotateMatrixUniformLocation, 1, GL_FALSE, &rotateMatrix[0][0]);
        
        translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, -0.8f, z));
        translateMatrixUniformLocation =
        glGetUniformLocation(programID, "translateMatrix");
        glUniformMatrix4fv(translateMatrixUniformLocation, 1,
                           GL_FALSE, &translateMatrix[0][0]);
        
        // it is tiny. make much bigger
        //scale transformation
        scaleMatrix = glm::mat4(1.0f);
        scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(.2,.2,.2));
        scaleMatrixUniformLocation = glGetUniformLocation(programID, "scaleMatrix");
        glUniformMatrix4fv(scaleMatrixUniformLocation, 1, GL_FALSE, &scaleMatrix[0][0]);
    
    glDrawElements(GL_TRIANGLES, bluejay.indices.size(), GL_UNSIGNED_INT, 0);
    }
    
    // unbind vao
    glBindVertexArray(0);
}

bool lmb = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // detect whether pressed lmb
    glfwGetCursorPos(window, &mousex, &mousey);
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
    {
        lmb = true;
    }
    else { lmb = false;
        glfwGetCursorPos(window, &mousex, &mousey);
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    // uncomment below to remove cursor and lock to window:
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if (lmb) {
        // while left mouse is held down, detect change in position per frame
        dx += x - mousex;
        dy += y - mousey;
        mousex = x;
        mousey = y;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// move camera along z.
    dz += yoffset * 0.4;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    
    // switch between tiger textures
    if ((key == GLFW_KEY_1 || key == GLFW_KEY_2) && action == GLFW_PRESS) {
        tigtex += 1;
        tigtex = tigtex % 2;
    }
    
    // switch between ground textures
    if ((key == GLFW_KEY_3 || key == GLFW_KEY_4) && action == GLFW_PRESS) {
        if (groundtex == 3){
            groundtex = 4;
        }
        else {groundtex = 3;}
    }
    
    // task 3a lighting brightness control
    if (key == GLFW_KEY_W && action == GLFW_PRESS && dirBrightness <= 2) {
        dirBrightness += 1;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS && dirBrightness >= -3) {
        dirBrightness -= 1;
    }
    
    // task 3c tiger control: l/r rotate, u/d move
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        tigerTurn -= 1;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        tigerTurn += 1;
    }
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        tigerMove += 1;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        tigerMove -= 1;
    }
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        tigerTurn += 1 + rand() % 10;
        tigerMove += 1 + rand() % 20;
    }
}

int main(int argc, char* argv[])
{
	GLFWwindow* window;

	/* Initialize the glfw */
	if (!glfwInit()) {
		std::cout << "Failed to initialize GLFW" << std::endl;
		return -1;
	}
	/* glfw: configure; necessary for MAC */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
	if (!window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
    
	/* Make the window's context current */
	glfwMakeContextCurrent(window);
    
    glfwGetCursorPos(window, &mousex, &mousey);

	/*register callback functions*/
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);                                                                  //
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	initializedGL();
    
    // load textures once
    // bottom commented section is pathfiles for my local use
    TextureTiger1 = loadTexture("resources/cat/Cat_diffuse.jpg");
    TextureTiger2 = loadTexture("resources/cat/Cat_bump.jpg");
    groundTexture1 = loadTexture("resources/ground/ground_01.jpg");
    groundTexture2 = loadTexture("resources/ground/ground_02.jpg");
    BluejayTexture = loadTexture("resources/Birds/Nutcracker_model.png");
    
//    TextureTiger1 = loadTexture("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/cat/Cat_diffuse.jpg");
//    TextureTiger2 = loadTexture("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/cat/Cat_bump.jpg");
//    groundTexture1 = loadTexture("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/ground/ground_01.jpg");
//    groundTexture2 = loadTexture("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/ground/ground_02.jpg");
//    BluejayTexture = loadTexture("/Users/zerlinalai/Desktop/Graphics/test1/assignment 2/resources/Birds/Nutcracker_model.png");
    
    

	while (!glfwWindowShouldClose(window)) {
        
        // projection matrix
        // perspective params: (fov, aspect, near, far);
        glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), 1.33f, 1.0f, 500.0f);
        GLint projMatrixUniformLoc = glGetUniformLocation(programID, "projMatrix");
        glUniformMatrix4fv(projMatrixUniformLoc, 1, GL_FALSE, &projMatrix[0][0]);
        
		/* Render here */
		paintGL();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}






