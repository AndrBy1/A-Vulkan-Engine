#pragma once //pragma once prevents multiple inclusions of the same header file similar to #ifndef guards

#include "ave_camera.hpp"
#include "ave_pipeline.hpp"
#include "ave_device.hpp"
#include "ave_game_object.hpp"
#include "ave_frame_info.hpp"

#include <memory>
#include <vector>

namespace ave {
	class PointLightSystem {
	public:
		PointLightSystem(AveDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete; //disable copy constructor
		PointLightSystem& operator=(const PointLightSystem&) = delete;

		void update(FrameInfo& frameInfo, GlobalUbo& ubo);
		void render(FrameInfo& frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);

		//order matters here since they are initialized in order listed
		AveDevice& aveDevice;
		std::unique_ptr<AvePipeline> avePipeline;
		VkPipelineLayout pipelineLayout;
	};
}
//rendere manages swapchain, command buffers and draw frame
//simple render system sets up pipeline pipeline layout, simple push constants struct and render game objects