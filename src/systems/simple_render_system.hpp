//this render system is for rendering simple game objects with a single color shader
//renderer manages swapchain, command buffers and draw frame
//simple render system sets up pipeline pipeline layout, simple push constants struct and render game objects

#pragma once //pragma once prevents multiple inclusions of the same header file similar to #ifndef guards

#include "ave_camera.hpp"
#include "ave_pipeline.hpp"
#include "ave_device.hpp"
#include "ave_game_object.hpp"
#include "ave_frame_info.hpp"

//#include "ave_model.hpp"
#include <memory>
#include <vector>
#include <string>

namespace ave {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(AveDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
		~SimpleRenderSystem();

		//becuase this is managing Vulkan objects for pipeline layout and command buffers, we should delete copy constructors 
		SimpleRenderSystem(const SimpleRenderSystem&) = delete; //disable copy constructor
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete; //disable copy assignment operator

		void renderGameObjects(FrameInfo& frameInfo);
		//VkPipelineLayout& getPipelineLayout() { return pipelineLayouts[0]; }

	private:
		void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayout);
		void createPipeline(VkRenderPass renderPass);

		//order matters here since they are initialized in order listed
		AveDevice& aveDevice;
		std::unique_ptr<AvePipeline> avePipeline;
		VkPipelineLayout pipelineLayout;
	};
}