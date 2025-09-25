#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
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

	// Black/White Checkboard Texture
    float pixels[] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
    };
    //We need a VAO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //Texture
	//Texture coorinates clamped between 0.0 and 1.0 where (0, 0) is bottom left and (1, 1) is top right
    //Retrieving texture at the pixel = sampling
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
   
    //Constraints for texture coordinates outside the range [0.0, 1.0]: gotta repeat, mirrored repeat, clamp to edge, clamp to border
	//glTexParameteri sets texture parameters for the currently bound texture
	//GL_TEXTURE_WRAP_S = x axis, GL_TEXTURE_WRAP_T = y axis
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float color[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    //fv expects a float, i expects an int
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	//Gotta filter the texture to match pixels
	//GL_NEAREST = returns the pixel that is closest
	//GL_LINEAR = returns the weighted average of the 4 pixels closest to the texture coordinate
	//GL_MIPMAP_NEAREST = picks the mipmap that most closely matches the size of the pixel being textured and uses GL_NEAREST to sample from that mipmap
	//GL_MIPMAP_LINEAR = picks the two mipmaps that most closely match the size of the pixel being textured and uses GL_LINEAR to sample from them and then blends the two samples together
	//Lienar interpolation is best for 8 bit graphics
	// GL_TEXTURE_MIN_FILTER = used when the pixel being textured maps to an area greater than one texture element, i.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Upload texture to GPU, target = 2d texture, level = 0 no mipmaps, internal format = RGB, width = 2, height = 2, border = 0, format = RGB, type = float, pixels = actual data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_FLOAT, pixels);

	//SOIL Texture Loading
    int width, height;
	
    //SOIL_LOAD_RGB forces the image to load as Red Green Blue, 0 = generate a new texture ID: Creates texture form image
    unsigned char* image = SOIL_load_image("V:/Graphics/x64/Debug/textures/cat.png", &width, &height,0, SOIL_LOAD_RGB);


    if (image == 0) {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
    }
    //Defines the texture image
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	
    //Cleans up the image data
    SOIL_free_image_data(image);

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
	glDrawElements(GL_TRIANGLES,6, GL_UNSIGNED_INT, 0);

    //Print id
    printf("%u\n", vbo);

    //Handles attributes as they appear in the vertex array, positions, and 3d Transformations
    const char* vertexSource = R"glsl(
        #version 330 core
        in vec2 position;
        in vec3 color;
        in vec2 texcoord;
        out vec3 Color;
        out vec2 Texcoord;    
        void main(){
            Texcoord = texcoord;
            Color = color;
            gl_Position = vec4(position,0.0, 1.0);
        }
    )glsl";
    //Handles coloring of pixels
    const char* fragmentSource = R"glsl(
        #version 330 core
        in vec3 Color;
        in vec2 Texcoord;
        out vec4 outColor;
        
        uniform sampler2D tex;
        void main() {
            outColor = texture(tex, Texcoord ) * vec4(Color, 1.0);
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
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);

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
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(2*sizeof(float)));
    //Enables a generic vertex attribute array
    glEnableVertexAttribArray(colAttrib);

    GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(texAttrib);
    
    // Main loop
    auto t_start = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);


        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();
        glUniform3f(colAttrib, 1.0f, 1.0f, 0.0f);

        
        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
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