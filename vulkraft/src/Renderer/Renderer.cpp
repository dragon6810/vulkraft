#include <Renderer/Renderer.h>

#include <stdio.h>
#include <assert.h>

#define RENDERER_DOVALIDATIONLAYERS true

void Renderer::MakeWindow(void)
{
    assert(!this->win);
    assert(glfwInit());

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    this->win = glfwCreateWindow(this->windowdims.width, this->windowdims.height, "vulkraft", nullptr, nullptr);
}

void Renderer::Kill(void)
{

}

void Renderer::Launch(void)
{
    while(!glfwWindowShouldClose(win))
    {
        glfwPollEvents();
    }
}

void Renderer::Initialize(void)
{
    assert(!this->initialized);
    
    MakeWindow();

    printf("renderer initialize.\n");
    this->initialized = true;
}