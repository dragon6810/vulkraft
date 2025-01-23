#include <stdio.h>

#include <Renderer/Renderer.h>
#include <Threading/Threads.h>

int main(int argc, char** argv)
{
    Renderer renderer;

    printf("Hello, Vulkan!\n");

    renderer.Initialize();
    renderer.Launch();
    renderer.Kill();
}