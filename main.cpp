#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

// Define a custom Vertex type
typedef struct Vertex
{
    vec2 pos;
    vec3 col;
} Vertex;

// Define a list of verticies (using the Vertex type) for our triangle
static const Vertex vertices[3] =
{
    { { -0.6f, -0.4f }, { 1.f, 0.f, 0.f } },
    { {  0.6f, -0.4f }, { 0.f, 1.f, 0.f } },
    { {   0.f,  0.6f }, { 0.f, 0.f, 1.f } }
};

// Vertex shader code (written in OpenGL Shading Language (GLSL))
static const char* vertex_shader_text =
"#version 330\n"        // GLSL version, OpenGL 3.3
"uniform mat4 MVP;\n"   // uniform (const vals passed from app to shader during draw call) (MVP -> model-view-projection matrix)
"in vec3 vCol;\n"       // Input for vertex color (e.g. RGB)
"in vec2 vPos;\n"       // Input for vertex position
"out vec3 color;\n"     // output variable that passes from vertex shader to the next pipeline stage (frag shader, likely)
"void main()\n"         // main function
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"   // assigns to built in variable for clip-space position of the vertex
"    color = vCol;\n"                               // assigns the color
"}\n";

static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"  // Fragment color output (rgba)
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"    // Returns color with a=1
"}\n";

// Error callback function for GLFW
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Key callback function for GLFW that catches user input
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    // Setup the error callback
    glfwSetErrorCallback(error_callback);

    // Try to init GLFW, exit on failure
    if (!glfwInit())
        exit(EXIT_FAILURE);

    // Setup Window Hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);      // Required OpenGL version minimum
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // Default core

    // Try to create window
    GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        // Exit on failure (window is NULL on failure)
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // Key callbacks - used for input
    glfwSetKeyCallback(window, key_callback);

    // Sets the context for OpenGL to draw
    glfwMakeContextCurrent(window);
    
    // Loads OpenGL through GLAD
    gladLoadGL();

    // Buffer intervals
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    // Setup vertex buffer object for vertex data storage
    GLuint vertex_buffer;                                                       // Holds the ID of the buffer objects
    glGenBuffers(1, &vertex_buffer);                                            // Generates X buffer objects (set to 1 in this example, assigned to vertex_bffer))
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);                               // binds the array buffer to the created vertex buffer - all subsequent operations on GL_ARRAY_BUFFER are for this array
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // initializes buffer data -> GL_ARRAY_BUFFER specifies the target, sizeof(verticies) is size of the data, verticies is vertex dat, GL_STATIC_DRAW is how the data is used

    // Setup the vertex shader
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);  // Creates a new shader of type GL_VERTEX_SHADER
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);    // Source of shader -> (source, number of strings in shader source, pointer to the shader source string, string length auto)
    glCompileShader(vertex_shader);                                 // Compiles the shader source code in vertex_shader

    // Setup the fragment (pixel) shader - basically same as above
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    // Creates the shader program and attaches the shaders to it.
    const GLuint program = glCreateProgram();       // Creates program and gives it unique ID
    glAttachShader(program, vertex_shader);         // attach vertex shader
    glAttachShader(program, fragment_shader);       // attach fragment shader
    glLinkProgram(program);                         // Links the attached shaders to the program

    const GLint mvp_location = glGetUniformLocation(program, "MVP");    // Gets the MVP (model-view-projection) location 
    const GLint vpos_location = glGetAttribLocation(program, "vPos");   // the vertex position location
    const GLint vcol_location = glGetAttribLocation(program, "vCol");   // the vertex color location

    // Sets up Vertex Array object (VAO) to manage vertex attribute configs
    GLuint vertex_array;                        // UID for vertex array
    glGenVertexArrays(1, &vertex_array);        // generates X (1, here) arrays and assigns it's id to vertex array
    glBindVertexArray(vertex_array);            // bind the VAO - vertex attributes or buffer configs are stored in it
    glEnableVertexAttribArray(vpos_location);   // enables vertex attributes at location
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,     // specify vertex attribute layouts
        sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(vcol_location);   // enables vertex attributes at location
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,     // specify vertex attribute layouts
        sizeof(Vertex), (void*)offsetof(Vertex, col));

    // While the window should not close
    while (!glfwWindowShouldClose(window))
    {
        // Gets the framebuffer with the GLFW window and sets aspect ratio
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float)height;

        // Defines the area of the window (0,0 = bottom of viewport)
        glViewport(0, 0, width, height);

        // Clears the color buffer and resets to predefined color
        glClear(GL_COLOR_BUFFER_BIT);

        // Sets up a 4x4 matrix
        mat4x4 m, p, mvp;   // m = model matrix (transforms object coordinates) // p = proejction matrix (handles perspective) // mvp = model-view-projection matrix, combines pm & p
        mat4x4_identity(m); // sets "m" to be identity matrix (starting point for transformations)
        mat4x4_rotate_Z(m, m, (float)glfwGetTime());    // apply rotation - rotates "m" around z axis by angle derived from glfwGetTime()
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);   // create orthographic project matrix
        mat4x4_mul(mvp, p, m);  // Combine the matricies

        glUseProgram(program);  // activates the specified shader for subsequent OpenGL rendering calls
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&mvp);    // Sends the "mvp" matrix to the GPU
        glBindVertexArray(vertex_array);    // binds the vertex array object so OpenGL can interpret the vertex data
        glDrawArrays(GL_TRIANGLES, 0, 3);   // Draw the object (GL_TRIANGLES is the type of object to draw)

        glfwSwapBuffers(window);    // Swaps front and back buffers
        glfwPollEvents();           // process all pending events in the event queue (inputs, e.g.)
    }

    // Destroy window on "esc"
    glfwDestroyWindow(window);

    // Terminate GLFW
    glfwTerminate();

    // Exit, with success code
    exit(EXIT_SUCCESS);
}