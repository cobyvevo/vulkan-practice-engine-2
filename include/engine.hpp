#pragma once

//#define VMA_IMPLEMENTATION
//#include <vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//3rd party
#include <vk_mem_alloc.h>
//

#include <iostream>
#include <vector>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/hash.hpp>

#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <vulkan/vulkan.hpp>

#include "vulkancore.hpp"

struct VertInputStateDesc {
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;
	vk::PipelineVertexInputStateCreateFlags flags;

	void GetDefaultState();
};

struct Vertex {
	glm::vec3 pos;
	bool operator==(const Vertex& other) const {
		return other.pos==pos;
	}
};

struct AllocatedBuffer {
	vk::Buffer buffer;
	VmaAllocation allocation;
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;
	//VertInputStateDesc meshInputState;
	bool Load();
};

class MainEngine {
private:	
	VulkanCore* core;	
	QueueFamilyInfo* queueFamilies;
		
	//SWAPCHAIN
	SwapChainSupportDetails* swapchainDetails;
	vk::Extent2D swapchainExtent;
	vk::SwapchainKHR swapchain;

	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	std::vector<vk::Framebuffer> swapchainFramebuffer;
	//RENDERPASS
	vk::RenderPass renderpass;

	//CMDPOOLS
	vk::CommandPool cmdPool;
	std::vector<vk::CommandBuffer> cmdBuffers;
		//UPLOAD
		vk::Fence uploadFence;
		vk::CommandPool uploadCmdPool;
		vk::CommandBuffer uploadCmdBuffer;

	//SYNC OBJECTS
	std::vector<vk::Semaphore> swapimageavailable_semaphores;
	std::vector<vk::Semaphore> rendersubmit_semaphores;
	std::vector<vk::Fence> render_fences;

	//GPIPELINE
	vk::Pipeline graphicsPipeline;
	vk::PipelineLayout defaultPipelineLayout;

	//ALLOCATOR
	VmaAllocator vallocator;

	//MISC
	Mesh* testmesh;


public:
	uint32_t frameFlightNum = 3;
	uint32_t currentFlight = 0;
	float tick = 0;

	MainEngine(uint32_t WIDTH, uint32_t HEIGHT);

	void CreateAllocator();

	void CreateSwapchain();
	void CreateRenderpass();
	void CreateFramebuffer();
	void CreateCommandpool();
	void CreateSyncObjects();

	void CreateGraphicsPipeline();

	AllocatedBuffer create_allocated_buffer(size_t allocSize, vk::Flags<vk::BufferUsageFlagBits> usageBits, VmaMemoryUsage memoryUsageFlag);

	void run_gpu_instruction(std::function<void(vk::CommandBuffer cmd)>&& function);
	bool upload_mesh(Mesh* target);

	void initial();

	void draw();
	void cleanup();
};
