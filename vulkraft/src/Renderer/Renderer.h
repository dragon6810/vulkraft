#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class Renderer
{
private:
    bool initialized = false;
    int framenum = 0;
    bool stop = false;

    GLFWwindow *win = 0;
    VkExtent2D windowdims = { 1280, 720 };
private:
    void MakeWindow(void);
public:
    void Initialize(void);
    void Launch(void);
    void Kill(void);
};