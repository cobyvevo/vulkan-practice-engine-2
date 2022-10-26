#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <vulkan/vulkan.hpp>

struct QueueFamilyInfo {
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;

	bool completed() {
		return (graphics.has_value() && present.has_value());
	}
};

struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	vk::SurfaceFormatKHR surfaceFormat;
	vk::PresentModeKHR presentMode;
		bool supported;
};

class VulkanCore {
	private:
	public:
		VulkanCore(uint32_t WIDTH, uint32_t HEIGHT);
		void GetNewSwapchain();

		GLFWwindow* window;
		vk::Instance instance;
		vk::DebugUtilsMessengerEXT debugMessenger;
		vk::DispatchLoaderDynamic instanceLoader;

		VkSurfaceKHR csurface;
		vk::UniqueSurfaceKHR surface;

		vk::PhysicalDevice gpu;
		vk::Device gpudevice;
		vk::PhysicalDeviceProperties gpuproperties;
		size_t uniformbuffer_alignment = 4;

		vk::Queue graphicsQueue;
		vk::Queue presentQueue;

		uint32_t windowWidth;
		uint32_t windowHeight;

		QueueFamilyInfo queueFamilies;
		SwapChainSupportDetails swapchainDetails;
		
		void cleanup();
};