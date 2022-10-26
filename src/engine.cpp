#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "engine.hpp"
#include "meshtools.hpp"

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("file didnt open you mongoloid");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(),fileSize);

    file.close();

    return buffer;
}

vk::ShaderModule setupShader(const std::vector<char>& shader, vk::Device& gpudevice) {
	vk::ShaderModuleCreateInfo createinfo(
		{},
		shader.size(),
		reinterpret_cast<const uint32_t*>(shader.data())
	);

	vk::ShaderModule module = gpudevice.createShaderModule(createinfo);

	return module;
}

bool Mesh::Load(const char* path) {
	MeshTools::MeshData loading_mesh;
	loading_mesh.load_from_file(path);

	vertices.resize(loading_mesh.indices.size());
	for (int index : loading_mesh.indices) {
		MeshTools::MeshVertex* lm_v = &loading_mesh.vertices[index];
		vertices[index].pos = {lm_v->pos[0],lm_v->pos[1],lm_v->pos[2]};
	}

	//vertices.resize(3);
	//vertices[0].pos = {0.0f, 0.0f, 0.0f};
	//vertices[1].pos = {0.0f, 0.5f, 0.0f};
	//vertices[2].pos = {0.5f, 0.0f, 0.0f};

	//glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	//projection[1][1] *= -1;

	//vertices[0].pos *= projection;
	//vertices[1].pos *= projection;
	//vertices[2].pos *=	projection;

   // test.load_from_file("assets/cube.obj");

	return true;
}

void VertInputStateDesc::GetDefaultState() {
	vk::VertexInputBindingDescription mainBinding{};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(Vertex);
	mainBinding.inputRate = vk::VertexInputRate::eVertex;

	vk::VertexInputAttributeDescription posAtt{};
	posAtt.binding = 0;
	posAtt.location = 0;
	posAtt.format = vk::Format::eR32G32B32Sfloat;
	posAtt.offset = offsetof(Vertex,pos);

	bindings.push_back(mainBinding);
	attributes.push_back(posAtt);
}

AllocatedBuffer MainEngine::create_allocated_buffer(size_t allocSize, vk::Flags<vk::BufferUsageFlagBits> usageBits, VmaMemoryUsage memoryUsageFlag) {
	vk::BufferCreateInfo bufferinfo{};
	bufferinfo.size = allocSize;
	bufferinfo.usage = usageBits;

	VmaAllocationCreateInfo allocationinfo{};
	allocationinfo.usage = memoryUsageFlag;

	AllocatedBuffer newbuffer;

	if (vmaCreateBuffer(
		vallocator,
		reinterpret_cast<VkBufferCreateInfo*>(&bufferinfo),
		&allocationinfo,
		reinterpret_cast<VkBuffer*>(&newbuffer.buffer),
		&newbuffer.allocation,
		nullptr
		) != VK_SUCCESS) {
		throw std::runtime_error("buffer failed to be allocated");
	}

	return newbuffer;
}

void MainEngine::CreateAllocator() {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = core->gpu;
	allocatorInfo.device = core->gpudevice;
	allocatorInfo.instance = core->instance;
	vmaCreateAllocator(&allocatorInfo,&vallocator);
}

void MainEngine::CreateSwapchain() {

	core->GetNewSwapchain();
	swapchainDetails = &core->swapchainDetails;

	vk::SurfaceCapabilitiesKHR caps = swapchainDetails->capabilities;

	if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		swapchainExtent = caps.currentExtent;
	} else {

		int w,h;
		glfwGetFramebufferSize(core->window,&w,&h);

		swapchainExtent.width = std::clamp(
			(uint32_t) w, 
			caps.minImageExtent.width,
			caps.maxImageExtent.width
			);

		swapchainExtent.height = std::clamp(
			(uint32_t) h, 
			caps.minImageExtent.height,
			caps.maxImageExtent.height
			);

	}

	uint32_t swapimgcount = caps.minImageCount + 1;
	if (caps.maxImageCount > 0 && caps.minImageCount < swapimgcount) swapimgcount = caps.minImageCount;

	auto sharingmode = vk::SharingMode::eExclusive;
	uint32_t indexcount = 0;
	uint32_t queueindicesarray[] = {0,0};

	if (queueFamilies->graphics != queueFamilies->present) {
		sharingmode = vk::SharingMode::eConcurrent;
		indexcount = 2;
		queueindicesarray[0] = queueFamilies->graphics.value();
		queueindicesarray[1] = queueFamilies->present.value();
	}

	vk::SwapchainCreateInfoKHR swapCreateInfo{};
	swapCreateInfo.surface = core->surface.get();
	swapCreateInfo.minImageCount = swapimgcount;
	swapCreateInfo.imageFormat = swapchainDetails->surfaceFormat.format;
	swapCreateInfo.imageColorSpace = swapchainDetails->surfaceFormat.colorSpace;
	swapCreateInfo.imageExtent = swapchainExtent;
	swapCreateInfo.imageArrayLayers = 1;
	swapCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapCreateInfo.imageSharingMode = sharingmode;
	swapCreateInfo.queueFamilyIndexCount = indexcount;
	swapCreateInfo.pQueueFamilyIndices = queueindicesarray;
	swapCreateInfo.preTransform = caps.currentTransform;
	swapCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapCreateInfo.presentMode = swapchainDetails->presentMode;
	swapCreateInfo.clipped = true;
	swapCreateInfo.oldSwapchain = nullptr;

	swapchain = core->gpudevice.createSwapchainKHR(swapCreateInfo);
	std::cout << "the chain" << std::endl;

	swapchainImages = core->gpudevice.getSwapchainImagesKHR(swapchain);
	std::cout << "the chain imagres" << std::endl;

	swapchainImageViews.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++) {

		vk::ImageViewCreateInfo imgview{};

		imgview.image = swapchainImages[i];
		imgview.viewType = vk::ImageViewType::e2D;
		imgview.format = swapchainDetails->surfaceFormat.format;

		imgview.components.r = vk::ComponentSwizzle::eIdentity;
		imgview.components.g = vk::ComponentSwizzle::eIdentity;
		imgview.components.b = vk::ComponentSwizzle::eIdentity;
		imgview.components.a = vk::ComponentSwizzle::eIdentity;

		imgview.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		imgview.subresourceRange.baseMipLevel = 0;
		imgview.subresourceRange.levelCount = 1;
		imgview.subresourceRange.baseArrayLayer = 0;
		imgview.subresourceRange.layerCount = 1;

		swapchainImageViews[i] = core->gpudevice.createImageView(imgview);
		std::cout << "imgview" << i << std::endl;

	}

}

void MainEngine::CreateRenderpass() {

	
	//color
	vk::AttachmentDescription colorAttachment{};
	colorAttachment.format = swapchainDetails->surfaceFormat.format;
	colorAttachment.samples = vk::SampleCountFlagBits::e1;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;
	
	vk::AttachmentReference colorRef{};
	colorRef.attachment = 0;
	colorRef.layout = vk::ImageLayout::eColorAttachmentOptimal;
	//

	//subpass
	vk::SubpassDescription mainsubpass{};
	mainsubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	mainsubpass.colorAttachmentCount = 1;
	mainsubpass.pColorAttachments = &colorRef;

	vk::SubpassDependency mainsubpassDependency{};
	mainsubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; //everything before the render pass
	mainsubpassDependency.dstSubpass = 0;
	mainsubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; //wait for src color output to finish
	mainsubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; //once it finishes, run my color output
	mainsubpassDependency.srcAccessMask = {}; //src has no memory access types 
	mainsubpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; //im going to be writing to a color attachment
	//

	vk::RenderPassCreateInfo renderpassInfo{};
	renderpassInfo.flags = {};
	renderpassInfo.attachmentCount = 1;
	renderpassInfo.pAttachments = &colorAttachment;
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &mainsubpass;
	renderpassInfo.dependencyCount = 1;
	renderpassInfo.pDependencies = &mainsubpassDependency;

	renderpass = core->gpudevice.createRenderPass(renderpassInfo);
	std::cout << "rendeprass" << std::endl;

}

void MainEngine::CreateFramebuffer() {

	swapchainFramebuffer.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++) {

		vk::FramebufferCreateInfo fbinfo{};
		fbinfo.renderPass = renderpass;
		fbinfo.attachmentCount = 1;
		fbinfo.pAttachments = &swapchainImageViews[i];
		fbinfo.width = swapchainExtent.width;
		fbinfo.height = swapchainExtent.height;
		fbinfo.layers = 1;

		swapchainFramebuffer[i] = core->gpudevice.createFramebuffer(fbinfo);
		std::cout << "made framebuffer" << i << std::endl;

	}

}

void MainEngine::CreateCommandpool() {

	vk::CommandPoolCreateInfo cmdpoolinfo{};
	cmdpoolinfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	cmdpoolinfo.queueFamilyIndex = queueFamilies->graphics.value();

	cmdPool = core->gpudevice.createCommandPool(cmdpoolinfo);
	std::cout << "cmdpool created" << std::endl;

	vk::CommandBufferAllocateInfo bufferallocation{};
	bufferallocation.commandPool = cmdPool;
	bufferallocation.level = vk::CommandBufferLevel::ePrimary;
	bufferallocation.commandBufferCount = frameFlightNum;

	cmdBuffers = core->gpudevice.allocateCommandBuffers(bufferallocation);
	std::cout << "allocated buffers" << std::endl;

	//upload
	vk::CommandPoolCreateInfo uploadcmdpoolinfo{};
	uploadcmdpoolinfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	uploadcmdpoolinfo.queueFamilyIndex = queueFamilies->graphics.value();
	uploadCmdPool = core->gpudevice.createCommandPool(uploadcmdpoolinfo);

	vk::CommandBufferAllocateInfo uploadbufferallocation{};
	uploadbufferallocation.commandPool = cmdPool;
	uploadbufferallocation.level = vk::CommandBufferLevel::ePrimary;
	uploadbufferallocation.commandBufferCount = frameFlightNum;
	uploadCmdBuffer = core->gpudevice.allocateCommandBuffers(uploadbufferallocation).front();
	
	std::cout << "upload buffer done" << std::endl;

}

void MainEngine::CreateSyncObjects() {

	vk::SemaphoreCreateInfo seminfo{};
	vk::FenceCreateInfo fenceinfo(vk::FenceCreateFlagBits::eSignaled);

	swapimageavailable_semaphores.resize(frameFlightNum);
	rendersubmit_semaphores.resize(frameFlightNum);
	render_fences.resize(frameFlightNum);

	for (uint32_t i = 0; i < frameFlightNum; i++) {
		swapimageavailable_semaphores[i] = core->gpudevice.createSemaphore(seminfo);
		rendersubmit_semaphores[i] = core->gpudevice.createSemaphore(seminfo);
		render_fences[i] = core->gpudevice.createFence(fenceinfo);
	}

	vk::FenceCreateInfo ufenceinfo{};
	uploadFence = core->gpudevice.createFence(ufenceinfo);

}

void MainEngine::CreateGraphicsPipeline() {

	vk::PipelineLayoutCreateInfo defaultinfo{};
	defaultinfo.pPushConstantRanges = nullptr;
	defaultinfo.pushConstantRangeCount = 0;
	defaultinfo.setLayoutCount = 0;
	defaultinfo.pSetLayouts = nullptr;
	defaultPipelineLayout = core->gpudevice.createPipelineLayout(defaultinfo);

	//setup shaders
	auto vertexFile = readFile("shaders/vert.spv");
	auto fragFile = readFile("shaders/frag.spv");
	//vert
	vk::ShaderModule vertexShader = setupShader(vertexFile,core->gpudevice);
	vk::PipelineShaderStageCreateInfo vertInfo{};
	vertInfo.stage = vk::ShaderStageFlagBits::eVertex;
	vertInfo.module = vertexShader;
	vertInfo.pName = "main";
	//frag
	vk::ShaderModule fragmentShader = setupShader(fragFile,core->gpudevice);
	vk::PipelineShaderStageCreateInfo fragInfo{};
	fragInfo.stage = vk::ShaderStageFlagBits::eFragment;
	fragInfo.module = fragmentShader;
	fragInfo.pName = "main";

	vk::PipelineShaderStageCreateInfo stages[] = {vertInfo,fragInfo};
	//

	//dynamics
	std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,vk::DynamicState::eScissor};

	vk::PipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.dynamicStateCount = dynamicStates.size();
	dynamicState.pDynamicStates = dynamicStates.data();
	//

	//assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = false;
	//

	//viewport
	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	//

	//rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.depthClampEnable = false;
	rasterizer.rasterizerDiscardEnable = false;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.cullMode = vk::CullModeFlagBits::eNone;	
	rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizer.depthBiasEnable = false;
	rasterizer.lineWidth = 1.0f;
	//

	//coor blending
	vk::PipelineColorBlendAttachmentState colorattachmentstate{};
	colorattachmentstate.blendEnable = true;
	colorattachmentstate.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha; //alpha
	colorattachmentstate.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha; //1-alpha
	colorattachmentstate.colorBlendOp = vk::BlendOp::eAdd; // color*alpha + dstcolor*(1-alpha)
	colorattachmentstate.srcAlphaBlendFactor = vk::BlendFactor::eOne;
	colorattachmentstate.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	colorattachmentstate.alphaBlendOp = vk::BlendOp::eAdd;
	colorattachmentstate.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB;

	vk::PipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.logicOpEnable = false;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorattachmentstate;

	//multisampling
	vk::PipelineMultisampleStateCreateInfo multisample{};
	multisample.rasterizationSamples = vk::SampleCountFlagBits::e1;

	//vertex input

	VertInputStateDesc defaultdesc;
	defaultdesc.GetDefaultState();

	vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo{};
	vertexInputStateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateInfo.vertexAttributeDescriptionCount = 1;

	vertexInputStateInfo.pVertexBindingDescriptions = defaultdesc.bindings.data();
	vertexInputStateInfo.pVertexAttributeDescriptions = defaultdesc.attributes.data();

	//
	
	vk::GraphicsPipelineCreateInfo graphicsinfo{};

	graphicsinfo.stageCount = 2; //how many fragment, vertex, geometry etc
	graphicsinfo.pStages = stages;

	graphicsinfo.pVertexInputState = &vertexInputStateInfo;
	graphicsinfo.pInputAssemblyState = &inputAssembly;
	graphicsinfo.pTessellationState = nullptr;
	graphicsinfo.pViewportState = &viewportState;
	graphicsinfo.pRasterizationState = &rasterizer;
	graphicsinfo.pMultisampleState = &multisample;
	graphicsinfo.pDepthStencilState = nullptr;
	graphicsinfo.pColorBlendState = &colorBlendState;
	graphicsinfo.pDynamicState = &dynamicState;
	graphicsinfo.layout = defaultPipelineLayout;

	graphicsinfo.renderPass = renderpass;
	graphicsinfo.subpass = 0;
	graphicsinfo.basePipelineHandle = nullptr;
	graphicsinfo.basePipelineIndex = -1;

	vk::Result res;
	std::tie(res,graphicsPipeline) = core->gpudevice.createGraphicsPipeline(nullptr,graphicsinfo);
	std::cout << "fin gpipeline" << std::endl;

	core->gpudevice.destroyShaderModule(vertexShader);
	core->gpudevice.destroyShaderModule(fragmentShader);
	
}

void MainEngine::initial() {
	testmesh = new Mesh();

	testmesh->Load("assets/cube.obj");
	//testmesh->vertices.resize(3);
	//testmesh->vertices[0].pos = {0.0f, 0.0f, 0.0f};
	//testmesh->vertices[1].pos = {0.0f, 0.5f, 0.0f};
	//testmesh->vertices[2].pos = {0.5f, 0.0f, 0.0f};

	upload_mesh(testmesh);
}

void MainEngine::run_gpu_instruction(std::function<void(vk::CommandBuffer cmd)>&& function) {
	
	vk::CommandBufferBeginInfo begininfo{};
	begininfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	uploadCmdBuffer.begin(begininfo);
	function(uploadCmdBuffer);
	uploadCmdBuffer.end();

	vk::SubmitInfo submitinfo{};
	submitinfo.commandBufferCount = 1;
	submitinfo.pCommandBuffers = &uploadCmdBuffer;

	[[maybe_unused]] vk::Result res;
	res = core->graphicsQueue.submit(1, &submitinfo, uploadFence);
	res = core->gpudevice.waitForFences(1, &uploadFence, true, UINT32_MAX);
	res = core->gpudevice.resetFences(1,&uploadFence);

	core->gpudevice.resetCommandPool(uploadCmdPool);

}

bool MainEngine::upload_mesh(Mesh* target) {

	const size_t buffer_size = target->vertices.size() * sizeof(Vertex);

	AllocatedBuffer stagingbuffer = create_allocated_buffer(
		buffer_size,
		vk::BufferUsageFlagBits::eTransferSrc,
		VMA_MEMORY_USAGE_CPU_ONLY
	);

	target->vertexBuffer = create_allocated_buffer(
		buffer_size,
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		VMA_MEMORY_USAGE_GPU_ONLY
	);

	void* vertexdata;
	vmaMapMemory(vallocator, stagingbuffer.allocation, &vertexdata);
	memcpy(vertexdata, target->vertices.data(), target->vertices.size() * sizeof(Vertex));
	vmaUnmapMemory(vallocator, stagingbuffer.allocation);

	run_gpu_instruction([=](vk::CommandBuffer cmd) {
		vk::BufferCopy vertexbuffer_copy{};
		vertexbuffer_copy.dstOffset = 0;
		vertexbuffer_copy.srcOffset = 0;
		vertexbuffer_copy.size = buffer_size;
		cmd.copyBuffer(stagingbuffer.buffer, target->vertexBuffer.buffer, 1, &vertexbuffer_copy);
	});

	vmaDestroyBuffer(vallocator, stagingbuffer.buffer, stagingbuffer.allocation);

	std::cout << "uploaded testmesh" <<std::endl;

	return true;

}


void MainEngine::draw() {

	vk::Semaphore& swapchainavailable_S = swapimageavailable_semaphores[currentFlight];
	vk::Semaphore& rendersubmit_S = rendersubmit_semaphores[currentFlight];
	vk::Fence& fence = render_fences[currentFlight];

	[[maybe_unused]] vk::Result res;//bruh
	
	res = core->gpudevice.waitForFences(1,&fence,true,UINT64_MAX);

	uint32_t next_swap_image = 0; 
	res = core->gpudevice.acquireNextImageKHR(swapchain, UINT64_MAX, swapchainavailable_S, VK_NULL_HANDLE, &next_swap_image);
	res = core->gpudevice.resetFences(1, &fence);	

	vk::CommandBuffer* commandbuffer_current = &cmdBuffers[currentFlight];
	commandbuffer_current->reset();

	tick = (tick + 0.001f);
	if (tick>3.14159268) tick = 0;

	//float sine = sin(tick);

	vk::ClearValue clearcol{};
	clearcol.color = vk::ClearColorValue(std::array<float, 4>({{0.1f, 0.1f, 0.1f, 1.0f}}));

	vk::Rect2D renderrect;
	renderrect.extent = swapchainExtent;

	vk::CommandBufferBeginInfo commandbegininfo{};
	vk::RenderPassBeginInfo begininfo{};
	begininfo.renderPass = renderpass;
	begininfo.framebuffer = swapchainFramebuffer[next_swap_image];
	begininfo.renderArea = renderrect;
	begininfo.clearValueCount = 1;
	begininfo.pClearValues = &clearcol;

	commandbuffer_current->begin(commandbegininfo);
	commandbuffer_current->beginRenderPass(begininfo, vk::SubpassContents::eInline);

	vk::Viewport vp;
	vp.width = (float) swapchainExtent.width;
	vp.height = (float) swapchainExtent.height;
	vp.maxDepth = 1.0f;
	commandbuffer_current->setViewport(0, 1, &vp);
	commandbuffer_current->setScissor(0, 1, &renderrect);
	//vk::Rect2D scissorrect = renderrect;
	commandbuffer_current->bindPipeline(vk::PipelineBindPoint::eGraphics,graphicsPipeline);
	
	vk::DeviceSize offset = 0;
	commandbuffer_current->bindVertexBuffers(0,1,&testmesh->vertexBuffer.buffer,&offset);
	commandbuffer_current->draw(testmesh->vertices.size(),1,0,0);

	commandbuffer_current->endRenderPass();
	commandbuffer_current->end();

	vk::PipelineStageFlags wstages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

	vk::SubmitInfo sinfo{};
	sinfo.waitSemaphoreCount = 1;
	sinfo.pWaitSemaphores = &swapchainavailable_S;
	sinfo.pWaitDstStageMask = wstages;

	sinfo.commandBufferCount = 1;
	sinfo.pCommandBuffers = commandbuffer_current;

	sinfo.signalSemaphoreCount = 1;
	sinfo.pSignalSemaphores = &rendersubmit_S;

	res = core->graphicsQueue.submit(1, &sinfo, fence);

	//
	vk::PresentInfoKHR presentinfo{};
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pWaitSemaphores = &rendersubmit_S;
	presentinfo.swapchainCount = 1;
	presentinfo.pSwapchains = &swapchain;
	presentinfo.pImageIndices = &next_swap_image;

	res = core->presentQueue.presentKHR(presentinfo);

	currentFlight = (currentFlight+1)%frameFlightNum;

}

void MainEngine::cleanup() {
	core->gpudevice.waitIdle();

    core->gpudevice.destroySwapchainKHR(swapchain, nullptr);

    for (auto thing : swapchainImageViews) { //lol
    	core->gpudevice.destroyImageView(thing, nullptr);
    }
 	for (auto thing : swapchainFramebuffer) {
    	core->gpudevice.destroyFramebuffer(thing, nullptr);
    }

    for (uint32_t i = 0; i<frameFlightNum; i++) {
    	core->gpudevice.destroySemaphore(swapimageavailable_semaphores[i], nullptr);
		core->gpudevice.destroySemaphore(rendersubmit_semaphores[i], nullptr);
		core->gpudevice.destroyFence(render_fences[i], nullptr);
    }

    core->gpudevice.destroyFence(uploadFence, nullptr);
	core->gpudevice.destroyCommandPool(uploadCmdPool, nullptr);
    
    core->gpudevice.destroyCommandPool(cmdPool, nullptr);
    core->gpudevice.destroyRenderPass(renderpass, nullptr);

    core->gpudevice.destroyPipelineLayout(defaultPipelineLayout, nullptr);
    core->gpudevice.destroyPipeline(graphicsPipeline, nullptr);

	vmaDestroyBuffer(vallocator, testmesh->vertexBuffer.buffer, testmesh->vertexBuffer.allocation);
    vmaDestroyAllocator(vallocator);

    core->cleanup();
}

MainEngine::MainEngine(uint32_t WIDTH, uint32_t HEIGHT){
	std::cout << "making engine" << std::endl;
	core = new VulkanCore(WIDTH,HEIGHT);
	queueFamilies = &core->queueFamilies;

	CreateAllocator();
	CreateSwapchain();
	CreateRenderpass();
	CreateFramebuffer();
	CreateCommandpool();
	CreateSyncObjects();

	CreateGraphicsPipeline();

	initial();

	while (!glfwWindowShouldClose(core->window)) {
        glfwPollEvents();
        draw();
    }

    cleanup();
    delete core;
}