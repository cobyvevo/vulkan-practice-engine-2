#include "vulkancore.hpp"

const std::vector<const char*> valLayers = {
    "VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

std::vector<const char*> getGLFWExtensions(bool enableValLayers) {
	uint32_t glfwExtensionCount = 0;

	const char** glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> allextensions(glfwExts,glfwExts+glfwExtensionCount);

	if (enableValLayers == true) {
		allextensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	for (const char* extension: allextensions) {
		std::cout << extension << std::endl;
	}

	return allextensions;
}

void vkdo(VkResult x, const char* error) {
	if (x != VK_SUCCESS) {throw std::runtime_error(error);}
}

bool checkDeviceSupport(vk::PhysicalDevice* device) {
	std::vector<vk::ExtensionProperties> device_extensions = device->enumerateDeviceExtensionProperties();
	std::set<std::string> required_extensions(deviceExtensions.begin(),deviceExtensions.end());
	
	for (const vk::ExtensionProperties& extension: device_extensions) {
		required_extensions.erase(extension.extensionName);
		std::cout << "got extension support for" << extension.extensionName << std::endl;
	}

	if (required_extensions.empty()) std::cout << "successful" << std::endl;

	return required_extensions.empty();
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
    

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    
    return VK_FALSE;
}

SwapChainSupportDetails getSwapchainDetails(vk::PhysicalDevice* device, vk::UniqueSurfaceKHR* uniquesurface) {
	SwapChainSupportDetails details;
	auto surface = uniquesurface->get();

	details.capabilities = device->getSurfaceCapabilitiesKHR(surface);
	std::vector<vk::SurfaceFormatKHR> formats = device->getSurfaceFormatsKHR(surface);
	std::vector<vk::PresentModeKHR> presentmodes = device->getSurfacePresentModesKHR(surface);
		
	if (formats.empty() || presentmodes.empty()) {
		details.supported = false;
		return details;
	}
	details.supported = true;

	for (const vk::SurfaceFormatKHR& available: formats) {
		if (available.format == vk::Format::eB8G8R8A8Srgb && available.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			details.surfaceFormat = available;
			break;
		}
	}
	
	for (const vk::PresentModeKHR& available: presentmodes) {
		if (available == vk::PresentModeKHR::eMailbox) {
			details.presentMode = available;
			break;
		}
	}

	return details;
}

QueueFamilyInfo getQueueFamsFromDevice(vk::PhysicalDevice* device, vk::UniqueSurfaceKHR* surface) {
	QueueFamilyInfo indices;
	//get list of queue families
	//pick out graphics family and present family
	//uint32_t qfc = 0; 
	std::vector<vk::QueueFamilyProperties> queueFams = device->getQueueFamilyProperties();

	int i = 0;
	for (const auto& family : queueFams) {

		std::cout << vk::to_string(family.queueFlags) << std::endl;
		if (family.queueFlags & vk::QueueFlagBits::eGraphics) {
			std::cout << "found graphics" << std::endl;
			indices.graphics = i;
		} 

		if (device->getSurfaceSupportKHR(i, surface->get())) {
			std::cout << "found present > " << i << std::endl;
			indices.present = i;
		}

		if (indices.completed()) break;
		i++;

	}

	return indices;
}
//vulkan
VulkanCore::VulkanCore(uint32_t WIDTH, uint32_t HEIGHT) {

	std::cout << "creating boilerplate" << std::endl;
	//window stuff
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(WIDTH,HEIGHT, "epic gaming", nullptr, nullptr);

	windowWidth = WIDTH;
	windowHeight = HEIGHT;
	//

	bool validationLayersEnabled = true;

	std::vector<const char*> glfwExt = getGLFWExtensions(validationLayersEnabled);

	vk::ApplicationInfo appinfo{};
	appinfo.pApplicationName = "App";
	appinfo.applicationVersion = 1;
	appinfo.pEngineName = "cobert engine v1";
	appinfo.engineVersion = 1;
	appinfo.apiVersion = VK_API_VERSION_1_3;

	vk::InstanceCreateInfo createinfo{};
	createinfo.pApplicationInfo = &appinfo;
	createinfo.enabledLayerCount = valLayers.size();
	createinfo.ppEnabledLayerNames = valLayers.data();
	createinfo.enabledExtensionCount = glfwExt.size();
	createinfo.ppEnabledExtensionNames = glfwExt.data();

	instance = vk::createInstance(createinfo);

	//making surface
	vkdo(
		glfwCreateWindowSurface(instance, window, nullptr, &csurface),
		"The window surface failed"
	);

	surface = vk::UniqueSurfaceKHR(csurface, instance);

	//creating debug messenger
	vk::DebugUtilsMessageSeverityFlagsEXT severities(
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	);

	vk::DebugUtilsMessageTypeFlagsEXT types(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | 
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
	);

	vk::DebugUtilsMessengerCreateInfoEXT debugcreateinfo{};
	debugcreateinfo.messageSeverity = severities;
	debugcreateinfo.messageType = types;
	debugcreateinfo.pfnUserCallback = &debugCallback;

	instanceLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

	debugMessenger = instance.createDebugUtilsMessengerEXT(debugcreateinfo,nullptr,instanceLoader);

	//get device
	std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
	for (auto& device : devices) {
		//get queue family information
		std::cout << device.getProperties().deviceName << std::endl;
		
		QueueFamilyInfo indices = getQueueFamsFromDevice(&device, &surface);
		bool supportAdequate = checkDeviceSupport(&device);
		SwapChainSupportDetails swapdetails = getSwapchainDetails(&device, &surface);

		if (indices.completed() && supportAdequate && swapdetails.supported) {
		
			std::cout << "chosen" << std::endl;

			queueFamilies = indices;
			swapchainDetails = swapdetails;
			gpu = device;
			gpuproperties = device.getProperties();
			uniformbuffer_alignment = gpuproperties.limits.minUniformBufferOffsetAlignment;

		}

	}

	std::cout << "this GPU allows minimum buffer alighment of" << gpuproperties.limits.minUniformBufferOffsetAlignment << std::endl;

	//create logical device
	std::vector<vk::DeviceQueueCreateInfo> queueCreateinfos;
	std::set<uint32_t> queueFamilySet = {queueFamilies.graphics.value(),queueFamilies.present.value()};

	float priority = 1.0f;
	for (uint32_t family : queueFamilySet) {
		vk::DeviceQueueCreateInfo queuecreateinfo{};
		queuecreateinfo.queueFamilyIndex = family;
		queuecreateinfo.queueCount = 1;
		queuecreateinfo.pQueuePriorities = &priority;

		queueCreateinfos.push_back(queuecreateinfo);
	}

	std::vector<const char*> desiredValidationLayers;
	if (validationLayersEnabled) desiredValidationLayers = valLayers;

	vk::PhysicalDeviceShaderDrawParametersFeatures features{};
	features.shaderDrawParameters = true;//TODO

	vk::DeviceCreateInfo deviceCreateinfo{};
	deviceCreateinfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateinfos.size());
	deviceCreateinfo.pQueueCreateInfos = queueCreateinfos.data();

	deviceCreateinfo.enabledLayerCount = static_cast<uint32_t>(desiredValidationLayers.size());
	deviceCreateinfo.ppEnabledLayerNames = desiredValidationLayers.data();

	deviceCreateinfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateinfo.ppEnabledExtensionNames = deviceExtensions.data();

	vk::StructureChain<vk::DeviceCreateInfo,vk::PhysicalDeviceShaderDrawParametersFeatures> chain = {deviceCreateinfo,features};

	//deviceCreateinfo.pEnabledFeatuers = features;

	gpudevice = gpu.createDevice(chain.get<vk::DeviceCreateInfo>()); //fuck chain
	graphicsQueue = gpudevice.getQueue(queueFamilies.graphics.value(),0);
	presentQueue = gpudevice.getQueue(queueFamilies.present.value(),0);
	std::cout << "created logical device" << std::endl;

}

void VulkanCore::GetNewSwapchain() {
	//swapchainDetails = getSwapchainDetails(&gpu, &surface);
	swapchainDetails.capabilities = gpu.getSurfaceCapabilitiesKHR(surface.get());
}

void VulkanCore::cleanup() {

	gpudevice.waitIdle();

	std::cout << "cleaning up vulkan boilerplate" << std::endl;

	instance.destroySurfaceKHR(csurface,nullptr);
	instance.destroyDebugUtilsMessengerEXT(debugMessenger,nullptr,instanceLoader);
	
	gpudevice.destroy();
	instance.destroy();
	glfwDestroyWindow(window);
	glfwTerminate();

	std::cout << "cleaning up successful" << std::endl;

}