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


//INTERNAL
struct VertInputStateDesc {
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;
	vk::PipelineVertexInputStateCreateFlags flags;

	void GetDefaultState();
};

struct Vertex {
	glm::vec3 pos;
	glm::vec2 uv;
	bool operator==(const Vertex& other) const {
		return (other.pos==pos && other.uv==uv);
	}
};

struct AllocatedBuffer {
	vk::Buffer buffer;
	VmaAllocation allocation;
};

struct AllocatedImage {
	vk::Image image;
	bool ImageViewExists = false;
	vk::ImageView imageview;
	VmaAllocation allocation;
};

struct Mesh {
	std::vector<Vertex> vertices;
	AllocatedBuffer vertexBuffer;
	//VertInputStateDesc meshInputState;
	bool Load(const char* path);
};

struct Texture {
	AllocatedImage image;
	vk::DescriptorSet descriptor;
	vk::Sampler sampler;
};

struct GPUObjectData {
	glm::mat4 transform;
};

struct WorldData {
	glm::mat4 viewproj;
};

struct FrameInfo {
	AllocatedBuffer cameraBuffer;
	AllocatedBuffer objectdataBuffer;
	vk::DescriptorSet descriptor;
	vk::DescriptorSet objectdescriptor;
};
//EXTERNAL ABSTRACTIONS

struct Material {
	Texture tex;
	vk::Pipeline* gpupipeline;

	void Setup(const char* texture_path);
};

struct Object {
	Mesh* mesh;
	Material* material;

	glm::mat4 transform;

	void Setup(const char* mesh_path);
};

struct Scene {

	std::unordered_map<std::string,Material> materials;
	std::unordered_map<std::string,Object> objects;
	std::unordered_map<std::string,Mesh> meshes;

	void New_Material(const char* texturepath, std::string name);
	Object New_Object(const char* meshpath, std::string name, std::string material_name);

};

//create object
//create material
//add different object types (object, camera, etc)
//add scene + scene rendering
//scene storing / loading


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

	//DESCRIPTOR SETS

	vk::DescriptorPool descriptorPool;
	vk::DescriptorSetLayout descriptorSetLayout;
	vk::DescriptorSetLayout descriptorSetLayout_objectdata;
	vk::DescriptorSetLayout descriptorSetLayout_texture;

	//DEPTH BUFFER
	vk::Format depthFormat;
	AllocatedImage depthImage;

	//MISC
	Mesh* testmesh;
	AllocatedImage testimage;
	Texture testtexture;
	Scene testscene;

	std::vector<FrameInfo> frames;

public:
	bool windowResized = false;
	uint32_t frameFlightNum = 3;
	uint32_t currentFlight = 0;
	float tick = 0;

	MainEngine(uint32_t WIDTH, uint32_t HEIGHT);

	void CreateAllocator();

	void CreateSwapchain();
	void CreateRenderpass();
	void CreateFramebuffer();
	void CreateCommandpool();
	void CreateDescriptorSets();
	void CreateSyncObjects();
	void CreateGraphicsPipeline();

	void ReCreateSwapchain();

	AllocatedBuffer create_allocated_buffer(size_t allocSize, vk::Flags<vk::BufferUsageFlagBits> usageBits, VmaMemoryUsage memoryUsageFlag);
	void destroy_allocated_buffer(AllocatedBuffer* buffer);

	AllocatedImage create_allocated_image(vk::Format format, vk::Flags<vk::ImageUsageFlagBits> imageusage, vk::Extent3D extent, VmaMemoryUsage memoryUsageFlag, bool create_an_imageview, vk::ImageAspectFlagBits imageaspect);
	void destroy_allocated_image(AllocatedImage* image);

	void run_gpu_instruction(std::function<void(vk::CommandBuffer cmd)>&& function);

	AllocatedImage load_texture_file(const char* file);

	Texture create_texture_from_allimage(AllocatedImage* target);
	void create_texture(Texture* tex);
	
	void destroy_texture(Texture* target);

	bool upload_mesh(Mesh* target);

	void initial();

	void draw();
	void cleanup();
};
