#include "stdafx.h"

#include "core/Log.h"
#include "core/Timer.h"
#include "renderer/InputLayout.h"
#include "renderer/Surface.h"
#include "renderer/RenderDevice_GL.h"

struct Vertex_PosColor
{
    float x, y, z;
    uint32_t color;         // ABGR 
};

static const Vertex_PosColor cubeVertices[] = {
    { -1.0f,  1.0f,  1.0f, 0xff000000 },
    {  1.0f,  1.0f,  1.0f, 0xff0000ff },
    { -1.0f, -1.0f,  1.0f, 0xff00ff00 },
    {  1.0f, -1.0f,  1.0f, 0xff00ffff },
    { -1.0f,  1.0f, -1.0f, 0xffff0000 },
    {  1.0f,  1.0f, -1.0f, 0xffff00ff },
    { -1.0f, -1.0f, -1.0f, 0xffffff00 },
    {  1.0f, -1.0f, -1.0f, 0xffffffff }
};

static const unsigned short cubeIndices[] = {
    0, 1, 2, 1, 3, 2,
    4, 6, 5, 5, 6, 7,
    0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3,
    0, 4, 1, 4, 5, 1,
    2, 3, 6, 6, 3, 7
};

renderer::InputElement vertexLayout[] = {
    { renderer::Attrib_Position, renderer::Format_RGB_F32,       0 },
    { renderer::Attrib_Color0,   renderer::Format_RGBA_UI8Norm, 12 }
}; 

GLFWwindow *OpenWindow(uint32_t width, uint32_t height, const char *title)
{
    static bool isInitialized = false;

    if (!isInitialized) {
        glfwSetErrorCallback([](int, const char *msg) { throw core::FatalException(msg); });
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    }

    GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(window);
    //glfwSwapInterval(0);

    if (!isInitialized) {
        glewExperimental = true;
        GLenum err = glewInit();
        THROW_FATAL_COND(err != GLEW_OK, std::string( "glew init: ") + (char *)glewGetErrorString(err));
        DEBUG_LOG_TEXT("Using OpenGL version %s", glGetString(GL_VERSION));
        DEBUG_LOG_TEXT("Shader language %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        DEBUG_LOG_TEXT("GLEW %s", glewGetString(GLEW_VERSION));
        isInitialized = true;
    }

    return window;
}

int main()
{ 
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
#endif

    int returnValue = 0;

    try {
        GLFWwindow *window = OpenWindow(1024, 760, "Cyb engine test");

        auto device = std::make_shared<renderer::RenderDevice_GL>();
  
        renderer::Surface surf;
        surf.geometry.vertexBuffer = device->CreateBuffer(renderer::Buffer_Vertex, cubeVertices, sizeof(cubeVertices));
        surf.geometry.inputLayout = { vertexLayout, _countof(vertexLayout) };
        surf.geometry.indexBuffer = device->CreateBuffer(renderer::Buffer_Index, cubeIndices, sizeof(cubeIndices));
        surf.geometry.indexCount = _countof(cubeIndices);
        surf.geometry.winding = renderer::Winding_CW;
        surf.shader = device->CreateShaderSet({ device->LoadBuiltinShader(renderer::Shader_Vertex,   renderer::VShader_MVP),
                                                device->LoadBuiltinShader(renderer::Shader_Fragment, renderer::FShader_Gouraud) });

        device->SetProjection(glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f));
        glm::mat4 view = glm::lookAt(
            glm::vec3(0, 0, -3),    // position
            glm::vec3(0, 0, 0),     // target
            glm::vec3(0, 1, 0));    // up
    
        char titleBuffer[64];
        double startTime = core::Timer::GetSeconds();
        double previousFrametime = startTime;

        while (!glfwWindowShouldClose(window)) {
            double currentTime = core::Timer::GetSeconds() - startTime;
            double frametime = currentTime - previousFrametime;
            previousFrametime = currentTime;

            _snprintf_s(titleBuffer, sizeof(titleBuffer), "FrameTime: %.0fms", frametime * core::Timer::MsPerSecond);
            glfwSetWindowTitle(window, titleBuffer);

            device->Clear(0.7f, 0.7f, 0.82f, 1.0f, 1.0f);

            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)(currentTime*0.2), glm::vec3(1.0f, 0.0f, 0.0f));
            rotate = glm::rotate(glm::mat4(rotate), (float)(currentTime*0.33), glm::vec3(0.0f, 1.0f, 0.0f));
            device->Render(&surf, view * rotate);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    } catch (const std::exception &e) {
        DEBUG_LOG_TEXT("*** Error: %s", e.what());
        MessageBox(0, e.what(), 0, MB_OK | MB_ICONERROR | MB_SETFOREGROUND); 
        returnValue = 1;
    }

    glfwTerminate();
    core::LogSaveToFile("debuglog.txt");
    return returnValue;;
}