#include "ave_window.hpp"

#include <stdexcept>
namespace ave {
    AveWindow::AveWindow(int w, int h, const char* name) : width(w), height(h), windowName(name){
        initWindow();
    }

    AveWindow::~AveWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    
    void AveWindow::initWindow() {
        glfwInit();
        //glfwWindowHint sets window properties before creation, GLFW_CLIENT_API specifies which client API to create context for, here we specify no context. GLFW_RESIZABLE allows window to be resizable
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName, nullptr, nullptr); //last two args are monitor and share. 
        //glfwSetWindowUserPointer pairs glfw window object with arbituary pointer value
        glfwSetWindowUserPointer(window, this);
        //the glfw library allows to register callback function that when the window is resized, the function will be called with the arguments being the window pointer and new width and height
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }

    void AveWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void AveWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height){
        //reinterpret_cast allows convertion of one pointer type to another. Risky to use. 
        auto aveWindow = reinterpret_cast<AveWindow *>(glfwGetWindowUserPointer(window));
        aveWindow->framebufferResized = true;
        aveWindow->width = width;
        aveWindow->height = height;
    }
}

