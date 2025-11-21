#pragma once 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> //for dealing with windows and input

#include <string>
namespace ave {
    class AveWindow {

    public:
        AveWindow(int w, int h, const char* name);
        ~AveWindow();

        AveWindow(const AveWindow&) = delete; // Disable copy constructor
        AveWindow& operator=(const AveWindow&) = delete;

        bool shouldClose() const { return glfwWindowShouldClose(window); }

        //VkExtent2D is a struct that holds width and height as uint32_t
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized(){return framebufferResized;}
        void resetWindowResizedFlag(){framebufferResized = false;}
        GLFWwindow* getGLFWwindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
    private:
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool framebufferResized = false;

        const char* windowName;
        GLFWwindow* window;

    };
}