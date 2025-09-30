#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <chrono>
#include <SOIL.h>
#include <Windows.h>

using namespace std;
int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Ask for OpenGL 3.3 Core Profile (common baseline)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "GLEW + GLFW Test", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    // Make context current before using GLEW
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = GL_TRUE; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Create some primitive
    float vertices[] = {
        // Position Color Texcoords
        -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // Top-left
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // Top-right
        0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f // Bottom-left
    };
         
    //Unsigned int elements referring to vertices bound to GL_ARRAY_BUFFER if we want to draw them in order
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
	};  
    //We need a VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Unsigned int to identify our primitive
    GLuint vbo;
    //Set id in buffer
    glGenBuffers(1, &vbo);
    //Upload actual data 
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    //Make it active GL_STATIC_DRAW = uploaded once drawn many times
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);


    //Handles attributes as they appear in the vertex array, positions, and 3d Transformations
    //Model matrix: position of model to real world
	//View matrix: position of camera to real world
	//Projection matrix: 3D to 2D Razterization
    const char* vertexSource = R"glsl(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec3 color;
        layout(location = 2) in vec2 texcoord;

        out vec3 Color;
        out vec2 Texcoord;    
                
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;
        void main(){
            Color = color;
            Texcoord = texcoord;
            gl_Position = proj * view *  model * vec4(position,0.0, 1.0);
        }
    )glsl";
    //Handles coloring of pixels using glsl
	//sampler2D = texture, samples at certain points based on mix func. which linearly interpolates between two values based on a third value
    const char* fragmentSource = R"glsl(
        #version 330 core
        in vec3 Color;
        in vec2 Texcoord;
        out vec4 outColor;
        uniform sampler2D texKitten;
        uniform sampler2D texPuppy;
        uniform float time;
        
        void main() {
            vec4 colKitten = texture(texKitten, Texcoord);
            vec4 colPuppy = texture(texPuppy, Texcoord);
            outColor = mix(colKitten, colPuppy, time);
        }
    )glsl";
    //Create Id to store the shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //Upload actual data 
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    //Compile the shader into code
    glCompileShader(vertexShader);

    //Check if shader is working
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    char buffer[1024];
    glGetShaderInfoLog(vertexShader, 1024, NULL, buffer);
    if (!status) {
        std::cerr << "Vertex Shader Error:\n" << buffer << "\n";
    }

    //Create Id to store the shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    //Upload actual data 
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    //Compile the shader into code
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(fragmentShader, 1024, NULL, buffer);
    if (!status) {
        std::cerr << "Fragment Shader Error:\n" << buffer << "\n";
    }

    //Creates a shaderProgram attaches both 
    GLuint shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    //sets to which location writing to  Use glDrawBuffers when rendering multiple buffers
    glBindFragDataLocation(shaderProgram, 0, "outColor");

    //Connections made project is linked!
    glLinkProgram(shaderProgram);
    // Check link status
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (!status) {
        glGetProgramInfoLog(shaderProgram, 1024, NULL, buffer);
        std::cerr << "Shader Program Link Error:\n" << buffer << "\n";
    }

    glUseProgram(shaderProgram);
    //We need to order our attributes
    //Get attribute location signed int
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    if (posAttrib == -1) {
        // fallback if the attribute wasn't found (but shader uses layout(location = 0), so 0 is correct)
        std::cerr << "Warning: 'position' attribute not found. Using location 0 as fallback.\n";
        posAttrib = 0;
    }

    //Set how the input is to be achieved, 2 = number of values, GL_FLOAT is the type of each component clamped to -1.0, and 1.0 
   //Last two most important, sets how the attribuates laid out, first = stride: how many bytes are between each position attribute, last = offset: how many bytes from the start of the array
   //Also stores the vbo currently bound in the GL_ARRAY_BUFFER
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

    //The method says what it does...
    glEnableVertexAttribArray(posAttrib);
    // Print some info
    std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // Optional: make background a non-black color so triangle is obvious
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //Get attribute location signed int
    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");


    //Defines an array of vertex attribute data 	
    // GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    //Enables a generic vertex attribute array
    glEnableVertexAttribArray(colAttrib);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(texAttrib);

    std::cout << "posAttrib: " << posAttrib << "\n";
    std::cout << "colAttrib: " << colAttrib << "\n";
    std::cout << "texAttrib: " << texAttrib << "\n";
    //Texture
	//Texture coorinates clamped between 0.0 and 1.0 where (0, 0) is bottom left and (1, 1) is top right
    //Retrieving texture at the pixel = sampling
    GLuint textures[2];
    glGenTextures(2, textures);
    int width, height;
	unsigned char* image;
	//Gotta activate before binding
    glActiveTexture(GL_TEXTURE0);

	//Atmost 48 textures can be bound at once
    glBindTexture(GL_TEXTURE_2D, textures[0]);
	//Cat loading in via SOIL
    image = SOIL_load_image("textures/cat.png", &width, &height, 0, SOIL_LOAD_RGB);
    if (!image) {
        std::cerr << "Failed to load cat.png: " << SOIL_last_result() << "\n";
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    SOIL_free_image_data(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Dog loaded in via SOIL
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    image = SOIL_load_image("textures/puppy.png", &width, &height, 0, SOIL_LOAD_RGB);
    if (!image) {
        std::cerr << "Failed to load cat.png: " << SOIL_last_result() << "\n";
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


   
        
    GLint uniTrans = glGetUniformLocation(shaderProgram, "model");    
    
    //View Transformation, Camera matrix, Simulates a moving camera
    glm::mat4 view = glm::lookAt(
        glm::vec3(1.2f, 1.2f, 1.2f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    );
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        1.0f,
        10.0f
	);
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
    // Main loop
    auto t_start = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		GLint timeUnform = glGetUniformLocation(shaderProgram, "time");
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));
        model = glm::rotate(model, time, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::vec4 result = model * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        printf("%f, %f, %f\n", result.x, result.y, result.z);

        model = glm::rotate(
            model,
            time * glm::radians(180.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

        glUniform1f(timeUnform, (sin(time) + 1.0f) / 2.0f);
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    }

    // Cleanup
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
/*
* 1. Initialize GLFW, GLEW, MAKE WINDOWS
        glfwInit()
        glfwWindowHint(...)
        glfwCreateWindow(...)
        glfwMakeContextCurrent(window)
        glewInit()
  2. Setup arrays (Vertex Buffer Object, Vertex Array Object, Element Buffer Object)
        glGenVertexArrays, glBindVertexArray
        glGenBuffers, glBindBuffer, glBufferData
        Compile + link shaders → glUseProgram
        Setup vertex attributes → glVertexAttribPointer, glEnableVertexAttribArray
  3. Setup texture information
        glGenTextures, glBindTexture
        glTexParameteri
        SOIL_load_image
        glTexImage2D
        glGenerateMipmap
        SOIL_free_image_data
        glUniform1i (set sampler to texture unit 0)
  4. Main loop: 
        a. glClear
        b. glActiveTexture(GL_TEXTURE0)
        c. glBindTexture(GL_TEXTURE_2D, tex)
        d. glUseProgram 
        e. glBindVertexArray  
        f. glDrawElements
        g. glfwSwapBuffers, glfwPollEvents
  5. Disposing
        Cleanup: delete VAO/VBO/EBO/texture/shaders
        glfwDestroyWindow
        glfwTerminate()
*/