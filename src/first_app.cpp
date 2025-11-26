#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"

#include "ave_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "ave_buffer.hpp"
#include "ave_image.hpp"

#include <stdexcept>

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <cassert>
#include <iostream>

namespace ave { 
    
    FirstApp::FirstApp() {
        globalPool = AveDescriptorPool::Builder(aveDevice)
            .setMaxSets(AveSwapChain::MAX_FRAMES_IN_FLIGHT + 1)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, AveSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1) //for texture images
            .build();
        loadGameObjects(); // Load the model data before creating the pipeline
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run() {
        //noncoherentAtomSize is the smallest memory range the device allows when syncing between host and device memory
        std::vector<std::unique_ptr<AveBuffer>> uboBuffers(AveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < uboBuffers.size(); i++){
            uboBuffers[i] = std::make_unique<AveBuffer>(
                aveDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffers[i]->map();
        }

        auto globalSetLayout = AveDescriptorSetLayout::Builder(aveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            //.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //for texture images
            .build();
        setLayouts.push_back(globalSetLayout->getDescriptorSetLayout());
        
        auto textureSetLayout = AveDescriptorSetLayout::Builder(aveDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        setLayouts.push_back(textureSetLayout->getDescriptorSetLayout());

        //AveImage textureImage{aveDevice};
        //textureImage.createTextureImage("images/texture.jpg");

        std::vector<VkDescriptorSet> globalDescriptorSets(AveSwapChain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < globalDescriptorSets.size(); i++){
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            //auto imageInfo = textureImage.descriptorInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            AveDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                //.writeBuffer(1, &bufferInfo) //might not be necessary
                //.writeImage(1, &imageInfo) //for texture images
                .build(globalDescriptorSets[i]);
        }

        std::vector<VkDescriptorSet> textureDescriptorSets(imageInfos.size());
        for(int i = 0; i < imageInfos.size(); i++) {
            //VkDescriptorImageInfo roomImageInfo = models[i].getTextureImage().descriptorInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            //std::cout << "ImageInfo sampler: " << roomImageInfo.sampler << ", imageView: " << roomImageInfo.imageView << ", layout: " << roomImageInfo.imageLayout << std::endl;
            AveDescriptorWriter(*textureSetLayout, *globalPool)
                .writeImage(0, &imageInfos[i])
                .build(textureDescriptorSets[i]);
        }
        
        SimpleRenderSystem simpleRenderSystem{aveDevice, aveRenderer.getSwapChainRenderPass(), setLayouts};

        PointLightSystem pointLightSystem{aveDevice, aveRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};

        int ind = 0;
        for(auto& kv : gameObjects) {
            auto& obj = kv.second;
            if (obj.model == nullptr) continue;
            //TODO: make this moore efficient
            //if (obj.model->getTextureImage() == nullptr) continue;
            if (ind >= static_cast<int>(textureDescriptorSets.size())) break;
            obj.model->setTextureDescriptor(textureDescriptorSets[ind++]);
        }

        AveCamera camera{};

        auto viewerObject = AveGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now(); //returns a high precision value representing current time

        std::cout << "sizeof(GlobalUbo): " << sizeof(GlobalUbo) << "\n";

        while (!aveWindow.shouldClose()) {
            //std::cout << "New frame\n";
            glfwPollEvents(); //checks and processes window level events such as keyboard and mouse input

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count(); 
            currentTime = newTime; 

            frameTime = glm::min(frameTime, 0.1f); //add max allowable frame time

            cameraController.moveInPlaneXZ(aveWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = aveRenderer.getAspectRatio();
            //camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            if(auto commandBuffer = aveRenderer.beginFrame()){
                int frameIndex = aveRenderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects};

                //update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                
                pointLightSystem.update(frameInfo, ubo);
                
                //std::cout << "buffer data: " << typeid(&ubo).name() << std::endl;
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                //render
                aveRenderer.beginSwapChainRenderPass(commandBuffer);

                //order here matters
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);
                
                
                aveRenderer.endSwapChainRenderPass(commandBuffer);
                aveRenderer.endFrame();
            }
        }
        vkDeviceWaitIdle(aveDevice.device()); //waits for the device to finish all operations before destroying resources
    }

    void FirstApp::loadGameObjects() {
        //smooth uses smooth shading for vertex normals where the normal was calculated as if there was a smooth surface 
        //flat uses flat shading where the normal is the same for the entire face
        std::shared_ptr<AveModel> AveModel = AveModel::createModelFromFile(aveDevice, "models/flat_vase.obj");
        auto flatVase = AveGameObject::createGameObject();
        flatVase.model = AveModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        AveModel = AveModel::createModelFromFile(aveDevice, "models/smooth_vase.obj");
        auto smoothVase = AveGameObject::createGameObject();
        smoothVase.model = AveModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        AveModel = AveModel::createModelFromFile(aveDevice, "models/viking_room.obj");
        AveModel->attachTextureFromFile("textures/viking_room.png"); 
        VkDescriptorImageInfo roomImageInfo = AveModel->getTextureImage().descriptorInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        imageInfos.push_back(roomImageInfo);
        auto room = AveGameObject::createGameObject();
        room.model = AveModel;
        room.transform.translation = {0.f, 0.f, 0.f};
        room.transform.scale = {1.f, 1.f, 1.f};
        gameObjects.emplace(room.getId(),  std::move(room));

        AveModel = AveModel::createModelFromFile(aveDevice, "models/quad.obj");
        auto floor = AveGameObject::createGameObject();
        floor.model = AveModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  //
        };

        for(int i = 0; i < lightColors.size(); i++){
            auto pointLight = AveGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}