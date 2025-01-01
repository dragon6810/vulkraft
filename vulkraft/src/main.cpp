#include <stdio.h>

#include <Renderer/Renderer.h>

int main(int argc, char** argv)
{
    Renderer renderer;

    printf("Hello, Vulkan!\n");

    renderer.Initialize();
    renderer.Launch();
    renderer.Kill();
}