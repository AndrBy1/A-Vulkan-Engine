#pragma once //pragma once prevents multiple inclusions of the same header file similar to #ifndef guards
#include "ave_window.hpp"
#include "ave_device.hpp"
#include "ave_game_object.hpp"
#include "ave_renderer.hpp"
#include "ave_descriptors.hpp"

#include <memory>
#include <vector>

namespace ave {
    class FirstApp {
        public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        //becuase this is managing Vulkan objects for pipeline layout and command buffers, we should delete copy constructors 
        FirstApp(const FirstApp&) = delete; // Disable copy constructor
        FirstApp& operator=(const FirstApp&) = delete;

        void run();

        private:
        void loadGameObjects();

        //order here matters
        AveWindow aveWindow{WIDTH, HEIGHT, "HELLO VULKAN!"};
        AveDevice aveDevice{aveWindow};
        AveRenderer aveRenderer{aveWindow, aveDevice};

        //order of declaration matters, need to be destroyed in reverse order of creation
        std::unique_ptr<AveDescriptorPool> globalPool{};
        std::vector<VkDescriptorImageInfo> imageInfos;
        std::vector<VkDescriptorSetLayout> setLayouts;

        AveGameObject::Map gameObjects;
    };
}

//rendere manages swapchain, command buffers and draw frame
//simple render system sets up pipeline pipeline layout, simple push constants struct and render game objects