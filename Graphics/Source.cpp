#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

void Main() {

}
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
        0.0f, 0.5f, // Vertex 1 (X,Y)
        0.5f, -0.5f, // Vertex 2 (X,Y)
        -0.5f, -0.5f // Vertex 3 (X,Y)
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

    //Print id
    printf("%u\n", vbo);

    //Vertex shader source
    const char* vertexSource = R"glsl(
        #version 330 core

        layout(location = 0) in vec2 position;
        
        void main(){
            gl_Position = vec4(position,0.0, 1.0);
        }
    )glsl";
    const char* fragmentSource = R"glsl(
    #version 330 core
    out vec4 outColor;

    void main() {
        outColor = vec4(1.0, 0.5, 0.2, 1.0);
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
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    //The method says what it does...
    glEnableVertexAttribArray(posAttrib);

    // Print some info
    std::cout << "OpenGL Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL Version:  " << glGetString(GL_VERSION) << "\n";
    std::cout << "GLSL Version:    " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    // Optional: make background a non-black color so triangle is obvious
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

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