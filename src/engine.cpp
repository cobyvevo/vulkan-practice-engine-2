#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <vk_mem_alloc.h>
#include <stb_image.h>

#include "engine.hpp"

//test
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

size_t pad_uniform_buffer_size(size_t original, size_t minUboAlignment) {
	if (minUboAlignment > 0) {
		return (original + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}
	return original;
}//vkguide.dev

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
	int count = 0;
	for (int index : loading_mesh.indices) {
		MeshTools::MeshVertex* lm_v = &loading_mesh.vertices[index];

		vertices[count].pos = {lm_v->pos[0],lm_v->pos[1],lm_v->pos[2]};
		vertices[count].uv = {lm_v->uv[0],1-lm_v->uv[1]};

		std::cout << lm_v->pos[0] << ", " << lm_v->pos[1] << ", " << lm_v->pos[2] << std::endl;
		count++;
	}



	//vertices.resize(3)
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

void Material::Setup(const char* texture_path) {
	//tex.image = load_texture_file(texture_path);
	//tex = create_texture(&tex);
	//gpupipeline = &graphicsPipeline;
}

void Object::Setup(const char* mesh_path) {
	//mesh loading*******
	//mesh.Load(mesh_path);
	//upload_mesh(mesh);
}

//SCENE CONTROLS
/*
void Scene::New_Material(const char* texturepath, std::string name) {
	//mat
	auto find_mat = materials.find(name);
	if (find_mat == materials.end()) {
		Material newmaterial;	
		newmaterial.Setup(texturepath);

		materials[name] = newmaterial;
	} else {
		//newobject.material = &(*find_mat).second;
	}

}*/

void MainEngine::SCENE_new_material(Scene* sc,const char* texture_path, std::string name) { //well, this sucks

	auto find_mat = sc->materials.find(name);
	if (find_mat == sc->materials.end()) {
		Material newmaterial;	
		newmaterial.tex.image = load_texture_file(texture_path);
		create_texture(&newmaterial.tex);
		newmaterial.gpupipeline = graphicsPipeline_textured;

		sc->materials[name] = newmaterial;

	} else {
		//newobject.material = &(*find_mat).second;
	}

}

Object* MainEngine::SCENE_new_object(Scene* sc,const char* meshpath, std::string name, std::string material_name) {
	//Object obj = sc->New_Object(meshpath.name,material_name);
	Object* newobject = new Object;
	newobject->name = name;
	newobject->transform = glm::translate(glm::mat4(1.f), glm::vec3(0.0f,0.0f,0.0f));
	newobject->colour = glm::vec4(1.0f,0.0f,0.0f,1.0f);
	//mesh
	auto find_mesh = sc->meshes.find(name);

	if (find_mesh == sc->meshes.end()) {
		//couldnt be found, make new mesh
	
		Mesh newmesh;
		newmesh.Load(meshpath);
		upload_mesh(&newmesh);

		sc->meshes[name] = newmesh;
		newobject->mesh = &sc->meshes[name];

	} else {
		//set mesh to already existing
		newobject->mesh = &(*find_mesh).second;
	}

	//mat
	auto find_mat = sc->materials.find(material_name);
	if (find_mat == sc->materials.end()) {
		//could not be found
		std::cout << "could not find material for object" << std::endl;
		newobject->material = nullptr;
	} else {
		newobject->material = &(*find_mat).second;
	}
	sc->objects.push_back(newobject);

	/*
	you have NO FUCKING IDEA the hours of pain i just went though. turns out, a pointer to an object within
	a vector is prone to completely crashing your program at random times, and ive spend about 4 hours 
	trying to figure this out. the worst part is that i saw quite a few people recommend doing this, so i didnt
	even think that this wouldve been a problem.
	you live you learn, i guess. jesus fucking christ
	*/

	return newobject;
}

Object Scene::New_Object(const char* meshpath, std::string name, std::string material_name) { //UNUSED
	Object newobject;
	//mesh
	auto find_mesh = meshes.find(name);

	if (find_mesh == meshes.end()) {
		//couldnt be found, make new mesh
	
		Mesh newmesh;
		newmesh.Load(meshpath);
		//engine_target->upload_mesh(newmesh);

		//upload_mesh(newmesh); NEEDS MAIN ENGINE CONNECTION

		meshes[name] = newmesh;
		newobject.mesh = &meshes[name];

	} else {
		//set mesh to already existing
		newobject.mesh = &(*find_mesh).second;
	}

	//mat
	auto find_mat = materials.find(material_name);
	if (find_mat == materials.end()) {
		//could not be found
		std::cout << "could not find material for object" << std::endl;
	} else {
		newobject.material = &(*find_mat).second;
	}

	return newobject;
}

//


//mesh = new Mesh();
//testmesh->Load("assets/testasset.obj");
//testimage = load_texture_file("assets/tex.png");
//testtexture = create_texture_from_allimage(&testimage);

//testmesh->vertices.resize(3);
//testmesh->vertices[0].pos = {0.0f, 0.0f, 0.0f};
//testmesh->vertices[1].pos = {0.0f, 0.5f, 0.0f};
//testmesh->vertices[2].pos = {0.5f, 0.0f, 0.0f};

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

	vk::VertexInputAttributeDescription uvAtt{};
	uvAtt.binding = 0;
	uvAtt.location = 1;
	uvAtt.format = vk::Format::eR32G32Sfloat;
	uvAtt.offset = offsetof(Vertex,uv);

	bindings.push_back(mainBinding);
	attributes.push_back(posAtt);
	attributes.push_back(uvAtt);
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

void MainEngine::destroy_allocated_buffer(AllocatedBuffer* abuffer) {
	vmaDestroyBuffer(vallocator, abuffer->buffer, abuffer->allocation);
}

AllocatedImage MainEngine::create_allocated_image(vk::Format format, vk::Flags<vk::ImageUsageFlagBits> imageusage, vk::Extent3D extent, VmaMemoryUsage memoryUsageFlag, bool create_an_imageview, vk::ImageAspectFlagBits imageaspect = vk::ImageAspectFlagBits::eDepth) {

	vk::ImageCreateInfo image{};
	image.imageType = vk::ImageType::e2D;
	image.format = format;
	image.extent = extent;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = vk::SampleCountFlagBits::e1;
	image.tiling = vk::ImageTiling::eOptimal;
	image.usage = imageusage;

	VmaAllocationCreateInfo allocationinfo{};
	allocationinfo.usage = memoryUsageFlag;
	allocationinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	AllocatedImage newallocimage;

	if (vmaCreateImage(
		vallocator, 
		reinterpret_cast<VkImageCreateInfo*>(&image), 
		&allocationinfo,
		reinterpret_cast<VkImage*>(&newallocimage.image),
		&newallocimage.allocation,
		nullptr
		) != VK_SUCCESS) {
		throw std::runtime_error("image failed to be allocated");
	}

	if (create_an_imageview == true) {
		newallocimage.ImageViewExists = true;
		vk::ImageViewCreateInfo imgview{};

		imgview.image = newallocimage.image;
		imgview.viewType = vk::ImageViewType::e2D;
		imgview.format = format;

		imgview.subresourceRange.aspectMask = imageaspect;//vk::ImageAspectFlagBits::eColor;
		imgview.subresourceRange.baseMipLevel = 0;
		imgview.subresourceRange.levelCount = 1;
		imgview.subresourceRange.baseArrayLayer = 0;
		imgview.subresourceRange.layerCount = 1;	

		newallocimage.imageview = core->gpudevice.createImageView(imgview);
	}

	return newallocimage;

}

void MainEngine::destroy_allocated_image(AllocatedImage* aimage) {
	if (aimage->ImageViewExists==true) {
		core->gpudevice.destroyImageView(aimage->imageview,nullptr);
	}
	vmaDestroyImage(vallocator, static_cast<VkImage>(aimage->image), aimage->allocation);
}

AllocatedImage MainEngine::load_texture_file(const char* file) {

	int texture_width, texture_height, texture_channels;

	stbi_uc* pixels = stbi_load(file, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
	if (!pixels) {
		std::cout << "image failed to be loaded in" << std::endl;	
		//make a default missing texture and then return it here
	}

	vk::DeviceSize texturesize = texture_width * texture_height * 4;
	vk::Format textureformat = vk::Format::eR8G8B8A8Srgb;

	AllocatedBuffer stagingbuffer = create_allocated_buffer(
		texturesize,
		vk::BufferUsageFlagBits::eTransferSrc,
		VMA_MEMORY_USAGE_CPU_ONLY
	);

	void* data;
	void* pixelvoid = pixels;
	vmaMapMemory(vallocator,stagingbuffer.allocation,&data);
	memcpy(data, pixelvoid, static_cast<size_t>(texturesize));
	vmaUnmapMemory(vallocator,stagingbuffer.allocation);

	stbi_image_free(pixels);

	vk::Extent3D textureExtent;
	textureExtent.width = static_cast<uint32_t>(texture_width);
	textureExtent.height = static_cast<uint32_t>(texture_height);
	textureExtent.depth = 1;

	AllocatedImage newtexture = create_allocated_image(
		textureformat, 
		vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, 
		textureExtent, 
		VMA_MEMORY_USAGE_GPU_ONLY, 
		true,
		vk::ImageAspectFlagBits::eColor
	);

	run_gpu_instruction([=](vk::CommandBuffer cmd) {
		
		vk::ImageSubresourceRange range;
		range.aspectMask = vk::ImageAspectFlagBits::eColor;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		vk::ImageMemoryBarrier transfer_barrier{};
		transfer_barrier.oldLayout = vk::ImageLayout::eUndefined;
		transfer_barrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
		transfer_barrier.image = newtexture.image;
		transfer_barrier.subresourceRange = range;
		transfer_barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eTransfer,
			{},
			0,
			nullptr,
			0,
			nullptr,
			1,
			&transfer_barrier
		);

		vk::BufferImageCopy imagebuffer_copy{};
		imagebuffer_copy.bufferOffset = 0;
		imagebuffer_copy.bufferRowLength = 0;
		imagebuffer_copy.bufferImageHeight = 0;
		imagebuffer_copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		imagebuffer_copy.imageSubresource.mipLevel = 0;
		imagebuffer_copy.imageSubresource.baseArrayLayer = 0;
		imagebuffer_copy.imageSubresource.layerCount = 1;
		imagebuffer_copy.imageExtent = textureExtent;

		cmd.copyBufferToImage(stagingbuffer.buffer, newtexture.image, vk::ImageLayout::eTransferDstOptimal, 1, &imagebuffer_copy);

		vk::ImageMemoryBarrier transfer_barrier_readable{};
		transfer_barrier_readable.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		transfer_barrier_readable.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		transfer_barrier_readable.image = newtexture.image;
		transfer_barrier_readable.subresourceRange = range;
		transfer_barrier_readable.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		transfer_barrier_readable.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmd.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			{},
			0,
			nullptr,
			0,
			nullptr,
			1,
			&transfer_barrier_readable
		);

	});
	
	destroy_allocated_buffer(&stagingbuffer);

	return newtexture;
}

Texture MainEngine::create_texture_from_allimage(AllocatedImage* target) {
	Texture tex;

	vk::DescriptorSetAllocateInfo allocinfo{};
	allocinfo.descriptorPool = descriptorPool;
	allocinfo.descriptorSetCount = 1;
	allocinfo.pSetLayouts = &descriptorSetLayout_texture; //image sampler layout
	tex.descriptor = core->gpudevice.allocateDescriptorSets(allocinfo).front();

	vk::SamplerCreateInfo sampler{};
	sampler.magFilter = vk::Filter::eNearest;
	sampler.minFilter = vk::Filter::eNearest;
	//sampler.addressModeU = vk::SamplerAddressMode::eRepeat;
	//sampler.addressModeV = vk::SamplerAddressMode::eRepeat;	
	//sampler.addressModeW = vk::SamplerAddressMode::eRepeat;

	tex.sampler = core->gpudevice.createSampler(sampler);

	vk::DescriptorImageInfo imageInfo{};
	imageInfo.sampler = tex.sampler;
	imageInfo.imageView = target->imageview;
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::WriteDescriptorSet write{};
	write.dstBinding = 0;
	write.descriptorCount = 1;
	write.dstSet = tex.descriptor;
	write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	write.pImageInfo = &imageInfo;

	core->gpudevice.updateDescriptorSets(1,&write,0,nullptr);

	return tex;
}

void MainEngine::create_texture(Texture* tex) {
	vk::DescriptorSetAllocateInfo allocinfo{};
	allocinfo.descriptorPool = descriptorPool;
	allocinfo.descriptorSetCount = 1;
	allocinfo.pSetLayouts = &descriptorSetLayout_texture; //image sampler layout
	tex->descriptor = core->gpudevice.allocateDescriptorSets(allocinfo).front();

	vk::SamplerCreateInfo sampler{};
	sampler.magFilter = vk::Filter::eNearest;
	sampler.minFilter = vk::Filter::eNearest;
	//sampler.addressModeU = vk::SamplerAddressMode::eRepeat;
	//sampler.addressModeV = vk::SamplerAddressMode::eRepeat;	
	//sampler.addressModeW = vk::SamplerAddressMode::eRepeat;

	tex->sampler = core->gpudevice.createSampler(sampler);

	vk::DescriptorImageInfo imageInfo{};
	imageInfo.sampler = tex->sampler;
	imageInfo.imageView = tex->image.imageview;
	imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

	vk::WriteDescriptorSet write{};
	write.dstBinding = 0;
	write.descriptorCount = 1;
	write.dstSet = tex->descriptor;
	write.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	write.pImageInfo = &imageInfo;

	core->gpudevice.updateDescriptorSets(1,&write,0,nullptr);
}

void MainEngine::destroy_texture(Texture* target) {
	core->gpudevice.destroySampler(target->sampler, nullptr);
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
	//std::cout << "the chain" << std::endl;

	swapchainImages = core->gpudevice.getSwapchainImagesKHR(swapchain);
	//std::cout << "the chain imagres" << std::endl;

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

	//make depth
	depthFormat = vk::Format::eD32Sfloat;

	vk::Extent3D depthImgExtent = {swapchainExtent.width,swapchainExtent.height,1};
	
	depthImage = create_allocated_image(
		depthFormat, 
		vk::ImageUsageFlagBits::eDepthStencilAttachment, 
		depthImgExtent, 
		VMA_MEMORY_USAGE_GPU_ONLY, 
		true, 
		vk::ImageAspectFlagBits::eDepth
	);

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

	//depth
	vk::AttachmentDescription depthAttachment{};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = vk::SampleCountFlagBits::e1;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
	depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	
	vk::AttachmentReference depthRef{};
	depthRef.attachment = 1;
	depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	//

	//subpass
	vk::SubpassDescription mainsubpass{};
	mainsubpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	mainsubpass.colorAttachmentCount = 1;
	mainsubpass.pColorAttachments = &colorRef;
	mainsubpass.pDepthStencilAttachment = &depthRef;

	vk::SubpassDependency mainsubpassDependency{};
	mainsubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; //everything before the render pass
	mainsubpassDependency.dstSubpass = 0;
	mainsubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; //wait for src color output to finish
	mainsubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput; //once it finishes, run my color output
	mainsubpassDependency.srcAccessMask = {}; //src has no memory access types 
	mainsubpassDependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; //im going to be writing to a color attachment
	
	vk::SubpassDependency depthsubpassDependency{};
	depthsubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL; //everything before the render pass
	depthsubpassDependency.dstSubpass = 0;
	depthsubpassDependency.srcStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
	depthsubpassDependency.dstStageMask = vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests; 
	depthsubpassDependency.srcAccessMask = {};
	depthsubpassDependency.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite; 
	//

	vk::AttachmentDescription passAttachments[2] = {colorAttachment,depthAttachment};
	vk::SubpassDependency passDependencies[2] = {mainsubpassDependency,depthsubpassDependency};

	vk::RenderPassCreateInfo renderpassInfo{};
	renderpassInfo.flags = {};
	renderpassInfo.attachmentCount = 2;
	renderpassInfo.pAttachments = &passAttachments[0];
	renderpassInfo.subpassCount = 1;
	renderpassInfo.pSubpasses = &mainsubpass;
	renderpassInfo.dependencyCount = 2;
	renderpassInfo.pDependencies = &passDependencies[0];

	renderpass = core->gpudevice.createRenderPass(renderpassInfo);
	std::cout << "rendeprass" << std::endl;

}

void MainEngine::CreateFramebuffer() {

	swapchainFramebuffer.resize(swapchainImageViews.size());

	for (size_t i = 0; i < swapchainImageViews.size(); i++) {

		vk::ImageView imageview_attachments[2] = {swapchainImageViews[i],depthImage.imageview};

		vk::FramebufferCreateInfo fbinfo{};
		fbinfo.renderPass = renderpass;
		fbinfo.attachmentCount = 2;
		fbinfo.pAttachments = imageview_attachments;
		fbinfo.width = swapchainExtent.width;
		fbinfo.height = swapchainExtent.height;
		fbinfo.layers = 1;

		swapchainFramebuffer[i] = core->gpudevice.createFramebuffer(fbinfo);
		//std::cout << "made framebuffer" << i << std::endl;

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

void MainEngine::CreateDescriptorSets() {

	//make pool

	std::vector<vk::DescriptorPoolSize> sizes = {
		{vk::DescriptorType::eUniformBuffer, 10},
		{vk::DescriptorType::eStorageBuffer, 10},
		{vk::DescriptorType::eCombinedImageSampler, 10},
	};

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.maxSets = 10;
	poolInfo.poolSizeCount = (uint32_t)sizes.size();
	poolInfo.pPoolSizes = sizes.data();
	descriptorPool = core->gpudevice.createDescriptorPool(poolInfo);

	//create bindings

	vk::DescriptorSetLayoutBinding cameraBufferBinding{};
	cameraBufferBinding.binding = 0;
	cameraBufferBinding.descriptorCount = 1;
	cameraBufferBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	cameraBufferBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

	vk::DescriptorSetLayoutCreateInfo setinfo{};
	setinfo.bindingCount = 1;
	setinfo.pBindings = &cameraBufferBinding;

	//object data

	vk::DescriptorSetLayoutBinding objectdataBufferBinding{};
	objectdataBufferBinding.binding = 0;
	objectdataBufferBinding.descriptorCount = 1;
	objectdataBufferBinding.descriptorType = vk::DescriptorType::eStorageBuffer;
	objectdataBufferBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

	vk::DescriptorSetLayoutCreateInfo setinfo_objectdata{};
	setinfo_objectdata.bindingCount = 1;
	setinfo_objectdata.pBindings = &objectdataBufferBinding;	

	//texture

	vk::DescriptorSetLayoutBinding textureBufferBinding{};
	textureBufferBinding.binding = 0;
	textureBufferBinding.descriptorCount = 1;
	textureBufferBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	textureBufferBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutCreateInfo setinfo_texture{};
	setinfo_texture.bindingCount = 1;
	setinfo_texture.pBindings = &textureBufferBinding;

	descriptorSetLayout = core->gpudevice.createDescriptorSetLayout(setinfo);
	descriptorSetLayout_objectdata = core->gpudevice.createDescriptorSetLayout(setinfo_objectdata);
	descriptorSetLayout_texture = core->gpudevice.createDescriptorSetLayout(setinfo_texture);

	frames.resize(frameFlightNum);

	for (uint32_t i = 0; i < frameFlightNum; i++) {

		frames[i].cameraBuffer = create_allocated_buffer(
			sizeof(WorldData),
			vk::BufferUsageFlagBits::eUniformBuffer,
			VMA_MEMORY_USAGE_CPU_TO_GPU //change this
		);

		frames[i].objectdataBuffer = create_allocated_buffer(
			sizeof(GPUObjectData) * MAX_OBJECTS,
			vk::BufferUsageFlagBits::eStorageBuffer,
			VMA_MEMORY_USAGE_CPU_TO_GPU //also this
		);

		//allocate

		//cam
		vk::DescriptorSetAllocateInfo allocateinfo{};
		allocateinfo.descriptorPool = descriptorPool;
		allocateinfo.descriptorSetCount = 1;
		allocateinfo.pSetLayouts = &descriptorSetLayout;

		frames[i].descriptor = core->gpudevice.allocateDescriptorSets(allocateinfo).front();

		//obj
		vk::DescriptorSetAllocateInfo allocateinfo_obj{};
		allocateinfo_obj.descriptorPool = descriptorPool;
		allocateinfo_obj.descriptorSetCount = 1;
		allocateinfo_obj.pSetLayouts = &descriptorSetLayout_objectdata;

		frames[i].objectdescriptor = core->gpudevice.allocateDescriptorSets(allocateinfo_obj).front();

		//write allocation

		//cam
		vk::DescriptorBufferInfo camerabufferinfo{};
		camerabufferinfo.buffer = frames[i].cameraBuffer.buffer;
		camerabufferinfo.offset = 0;
		camerabufferinfo.range = sizeof(WorldData);

		vk::WriteDescriptorSet camerawrite{};
		camerawrite.dstSet = frames[i].descriptor;
		camerawrite.dstBinding = 0;
		camerawrite.descriptorCount = 1;
		camerawrite.descriptorType = vk::DescriptorType::eUniformBuffer;
		camerawrite.pBufferInfo = &camerabufferinfo;

		//obj
		vk::DescriptorBufferInfo objectbufferinfo{};
		objectbufferinfo.buffer = frames[i].objectdataBuffer.buffer;
		objectbufferinfo.offset = 0;
		objectbufferinfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

		vk::WriteDescriptorSet objectbufferwrite{};
		objectbufferwrite.dstSet = frames[i].objectdescriptor;
		objectbufferwrite.dstBinding = 0;
		objectbufferwrite.descriptorCount = 1;
		objectbufferwrite.descriptorType = vk::DescriptorType::eStorageBuffer;
		objectbufferwrite.pBufferInfo = &objectbufferinfo;

		//update allocation
		core->gpudevice.updateDescriptorSets(1,&camerawrite,0,nullptr);	
		core->gpudevice.updateDescriptorSets(1,&objectbufferwrite,0,nullptr);	
	}

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

void MainEngine::Create_New_Pipeline(MainEnginePipelineInfo& main_pipeline_info, vk::PipelineLayout& target_layout, vk::Pipeline& target_pipeline) {

	//vk::DescriptorSetLayout dslayouts[3];
	std::vector<vk::DescriptorSetLayout> dslayouts;

	if (main_pipeline_info.Textured == true) {
		dslayouts.resize(3);
		dslayouts = {descriptorSetLayout,descriptorSetLayout_texture,descriptorSetLayout_objectdata};
	} else {
		dslayouts.resize(2);
		dslayouts = {descriptorSetLayout,descriptorSetLayout_objectdata};
	}

	vk::PipelineLayoutCreateInfo defaultinfo{};
	defaultinfo.pPushConstantRanges = nullptr;
	defaultinfo.pushConstantRangeCount = 0;
	defaultinfo.setLayoutCount = 3;
	defaultinfo.pSetLayouts = dslayouts.data();

	target_layout = core->gpudevice.createPipelineLayout(defaultinfo);

	//setup shaders
	auto vertexFile = readFile(main_pipeline_info.VertexShaderPath);
	auto fragFile = readFile(main_pipeline_info.FragmentShaderPath);
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

	//coLor blending
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
	vertexInputStateInfo.vertexBindingDescriptionCount = defaultdesc.bindings.size();
	vertexInputStateInfo.vertexAttributeDescriptionCount = defaultdesc.attributes.size();

	vertexInputStateInfo.pVertexBindingDescriptions = defaultdesc.bindings.data();
	vertexInputStateInfo.pVertexAttributeDescriptions = defaultdesc.attributes.data();
	//

	//depth stencil
	vk::PipelineDepthStencilStateCreateInfo depthstencil{};
	depthstencil.depthTestEnable = true;
	depthstencil.depthWriteEnable = true;
	depthstencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depthstencil.depthBoundsTestEnable = false;
	//basically you can check if an incoming fragment/pixels depth is inbetween 2 given values
	//if they arent, you can discard them. i guess this is good for lighting
	//since you can just completely disregard things that are not within the vicinity of a 
	//light source
	depthstencil.stencilTestEnable = false;
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
	graphicsinfo.pDepthStencilState = &depthstencil;
	graphicsinfo.pColorBlendState = &colorBlendState;
	graphicsinfo.pDynamicState = &dynamicState;
	graphicsinfo.layout = target_layout;

	graphicsinfo.renderPass = renderpass;
	graphicsinfo.subpass = 0;
	graphicsinfo.basePipelineHandle = nullptr;
	graphicsinfo.basePipelineIndex = -1;

	vk::Result res;
	std::tie(res,target_pipeline) = core->gpudevice.createGraphicsPipeline(nullptr,graphicsinfo);
	std::cout << "fin gpipeline" << std::endl;

	core->gpudevice.destroyShaderModule(vertexShader);
	core->gpudevice.destroyShaderModule(fragmentShader);
	
}

void MainEngine::CreateGraphicsPipeline() {
	MainEnginePipelineInfo info;
	info.VertexShaderPath = "shaders/vert.spv";
	info.FragmentShaderPath = "shaders/frag.spv";
	info.Textured = true;

	MainEnginePipelineInfo info2;
	info2.VertexShaderPath = "shaders/vert.spv";
	info2.FragmentShaderPath = "shaders/frag_untex.spv";
	info2.Textured = true;


	Create_New_Pipeline(info, pipelineLayout_textured, graphicsPipeline_textured);	
	Create_New_Pipeline(info2, pipelineLayout_untextured, graphicsPipeline_untextured);	
}

void MainEngine::initial() // ######## I WILL REMOVE THIS LATER ###############

	//testscene.engine_target = &this;
	//testscene.New_Material("assets/tex.png", "SmileTexture");



	//SCENE_new_material(&testscene,"assets/tex.png", "SmileTexture");
	//SCENE_new_material(&testscene,"assets/tabletexture.png", "Table_Texture");

	//testobject = SCENE_new_object(&testscene,"assets/cube.obj", "Cube", "SmileTexture");
  //  SCENE_new_object(&testscene,"assets/table.obj", "TestAsset", "Table_Texture");
  //  SCENE_new_object(&testscene,"assets/ball.obj", "TestAsset2", "none");



//	testmesh = new Mesh();
	//testmesh->Load("assets/testasset.obj");

	//testimage = load_texture_file("assets/tex.png");
	//testtexture = create_texture_from_allimage(&testimage);

	//testmesh->vertices.resize(3);
	//testmesh->vertices[0].pos = {0.0f, 0.0f, 0.0f};
	//testmesh->vertices[1].pos = {0.0f, 0.5f, 0.0f};
	//testmesh->vertices[2].pos = {0.5f, 0.0f, 0.0f};

	//upload_mesh(testmesh);
}

void MainEngine::step() { // ######## I WILL REMOVE THIS LATER ###############

	//glm::vec3 objcenter = {sin(tick*4) * 10.0f,0.0f,0.0f};
	//glm::vec3 sc = {cos(tick*2) * 5.0f,1.0f,1.0f};

	//testobject->transform = glm::translate(glm::mat4(1.f), objcenter);
	//testobject->transform = glm::scale(testobject->transform,sc);

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

	destroy_allocated_buffer(&stagingbuffer);

	std::cout << "uploaded testmesh" <<std::endl;

	return true;

}

static void WindowResizedCallback(GLFWwindow* win, int w, int h) {
	auto eng = reinterpret_cast<MainEngine*>(glfwGetWindowUserPointer(win));
	eng->windowResized = false;
}

void MainEngine::ReCreateSwapchain() {

	core->gpudevice.waitIdle();
	//vk::Result res = core->gpudevice.waitForFences(frameFlightNum, render_fences.data(), true, UINT32_MAX);

    for (auto thing : swapchainImageViews) { //lol
    	core->gpudevice.destroyImageView(thing, nullptr);
    }
 	for (auto thing : swapchainFramebuffer) {
    	core->gpudevice.destroyFramebuffer(thing, nullptr);
    }

    core->gpudevice.destroySwapchainKHR(swapchain, nullptr);

    destroy_allocated_image(&depthImage);

    CreateSwapchain();
    CreateFramebuffer();

}

void MainEngine::draw() {
//	std::cout << "draw starting" << std::endl; 

	vk::Semaphore& swapchainavailable_S = swapimageavailable_semaphores[currentFlight];
	vk::Semaphore& rendersubmit_S = rendersubmit_semaphores[currentFlight];
	vk::Fence& fence = render_fences[currentFlight];

	[[maybe_unused]] vk::Result res;//bruh
	
	res = core->gpudevice.waitForFences(1,&fence,true,UINT64_MAX);
	uint32_t next_swap_image = 0; 
	res = core->gpudevice.acquireNextImageKHR(swapchain, UINT64_MAX, swapchainavailable_S, VK_NULL_HANDLE, &next_swap_image);
	
	//resizing
	if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || windowResized == false) {
		windowResized = true;
		ReCreateSwapchain();
		return;
	} else if (res != vk::Result::eSuccess) {
		throw std::runtime_error("swapchain died lol");
	}

	res = core->gpudevice.resetFences(1, &fence);	

	vk::CommandBuffer* commandbuffer_current = &cmdBuffers[currentFlight];
	commandbuffer_current->reset();

	//running cool code before draw
	FrameInfo currentframe = frames[currentFlight];
	tick = (tick + 0.0001f);
	if (tick>3.14159268) tick = 0;

	WorldData newworlddata;

	glm::vec3 campos = {0.0f,-2.0f,-10.0f};
	glm::vec3 center = {0.0f,0.0f,0.0f};
	glm::mat4 defaultmat = glm::mat4(1.f);

	glm::mat4 rotatemat = glm::rotate(defaultmat,tick*2, glm::vec3(0,1,0));
	glm::mat4 view = glm::translate(rotatemat, center);

	glm::mat4 offsetview = glm::translate(defaultmat,campos);

	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

	newworlddata.viewproj = projection * (offsetview*view);

	void* data;
	vmaMapMemory(vallocator, currentframe.cameraBuffer.allocation, &data);
	memcpy(data, &newworlddata, sizeof(WorldData));
	vmaUnmapMemory(vallocator, currentframe.cameraBuffer.allocation);
	void* objectdata;
	vmaMapMemory(vallocator, currentframe.objectdataBuffer.allocation, &objectdata);

	//ADD OBJECT DATA INTO BUFFER
	
	GPUObjectData* objdata_mapped = (GPUObjectData*)objectdata;
	
	for (size_t i = 0; i < testscene.objects.size(); i++) {
		objdata_mapped[i].transform = testscene.objects[i]->transform;
		objdata_mapped[i].colour = testscene.objects[i]->colour;
	}

	vmaUnmapMemory(vallocator, currentframe.objectdataBuffer.allocation);

	vk::ClearValue clearcol{};
	clearcol.color = vk::ClearColorValue(std::array<float, 4>({{0.1f, 0.1f, 0.1f, 1.0f}}));

	vk::ClearValue depthclearcol{};
	depthclearcol.depthStencil.depth = 1.0f;
	vk::ClearValue clearvalues[2] = {clearcol,depthclearcol};

	vk::Rect2D renderrect;
	renderrect.extent = swapchainExtent;

	vk::CommandBufferBeginInfo commandbegininfo{};
	vk::RenderPassBeginInfo begininfo{};
	begininfo.renderPass = renderpass;
	begininfo.framebuffer = swapchainFramebuffer[next_swap_image];
	begininfo.renderArea = renderrect;
	begininfo.clearValueCount = 2;
	begininfo.pClearValues = &clearvalues[0];

	commandbuffer_current->begin(commandbegininfo);
	commandbuffer_current->beginRenderPass(begininfo, vk::SubpassContents::eInline);

	vk::Viewport vp;
	vp.width = (float) swapchainExtent.width;
	vp.height = (float) swapchainExtent.height;
	vp.maxDepth = 1.0f;
	commandbuffer_current->setViewport(0, 1, &vp);
	commandbuffer_current->setScissor(0, 1, &renderrect);
	
	Material* oldmaterial = nullptr;
	Mesh* oldmesh = nullptr;

	//std::cout << "beginning render" << std::endl;

	for (size_t i = 0; i < testscene.objects.size(); i++) {
		Object* obj = testscene.objects[i];
		//std::cout << obj->name << std::endl;
		if (oldmaterial != obj->material) {
			//bind new material
			
			if (obj->material != nullptr) {
				//std::cout << "textured" << std::endl;
				commandbuffer_current->bindPipeline(vk::PipelineBindPoint::eGraphics,obj->material->gpupipeline);
				
				commandbuffer_current->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout_textured,
					1,1,
					&obj->material->tex.descriptor,
					0,
					nullptr
				);

				commandbuffer_current->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout_textured,
					0,1,
					&currentframe.descriptor,
					0,
					nullptr
				);
				
				commandbuffer_current->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout_textured,
					2,1,
					&currentframe.objectdescriptor,
					0,
					nullptr
				);

			} else {
				//std::cout << "untextured" << std::endl;
				commandbuffer_current->bindPipeline(vk::PipelineBindPoint::eGraphics,graphicsPipeline_untextured);
				
				commandbuffer_current->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout_untextured,
					0,1,
					&currentframe.descriptor,
					0,
					nullptr
				);
				
				commandbuffer_current->bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout_untextured,
					2,1,
					&currentframe.objectdescriptor,
					0,
					nullptr
				);

			}

			oldmaterial = obj->material;

		} else {
			//std::cout << "same material" << std::endl;
		}
		if (oldmesh != obj->mesh) {
			vk::DeviceSize offset = 0;
			commandbuffer_current->bindVertexBuffers(0,1,&obj->mesh->vertexBuffer.buffer,&offset);
			oldmesh = obj->mesh;
		}else {
			//std::cout << "same mesh" << std::endl;
		}
		//std::cout << "draw" << std::endl;
		commandbuffer_current->draw(obj->mesh->vertices.size(),1,0,i);
			
	}	

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

	vk::PresentInfoKHR presentinfo{};
	presentinfo.waitSemaphoreCount = 1;
	presentinfo.pWaitSemaphores = &rendersubmit_S;
	presentinfo.swapchainCount = 1;
	presentinfo.pSwapchains = &swapchain;
	presentinfo.pImageIndices = &next_swap_image;

	res = core->presentQueue.presentKHR(presentinfo);
	
	currentFlight = (currentFlight+1)%frameFlightNum;
}

void MainEngine::SCENE_cleanup(Scene* sc) {

	for (std::pair<std::string,Material> material : sc->materials) {
		destroy_allocated_image(&material.second.tex.image);
		destroy_texture(&material.second.tex);
	}

	for (std::pair<std::string,Mesh> mesh : sc->meshes) {
		destroy_allocated_buffer(&mesh.second.vertexBuffer);
	}

}

void MainEngine::cleanup() {
	core->gpudevice.waitIdle();

	destroy_allocated_image(&depthImage);

	SCENE_cleanup(&testscene);
	
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
		destroy_allocated_buffer(&frames[i].cameraBuffer);	
		destroy_allocated_buffer(&frames[i].objectdataBuffer);	
    }

    core->gpudevice.destroyFence(uploadFence, nullptr);
	core->gpudevice.destroyCommandPool(uploadCmdPool, nullptr);
    
    core->gpudevice.destroyCommandPool(cmdPool, nullptr);
    core->gpudevice.destroyRenderPass(renderpass, nullptr);

    core->gpudevice.destroyDescriptorPool(descriptorPool, nullptr);
	core->gpudevice.destroyDescriptorSetLayout(descriptorSetLayout, nullptr);
	core->gpudevice.destroyDescriptorSetLayout(descriptorSetLayout_objectdata, nullptr);
	core->gpudevice.destroyDescriptorSetLayout(descriptorSetLayout_texture, nullptr);
	
    core->gpudevice.destroyPipelineLayout(pipelineLayout_textured, nullptr);
    core->gpudevice.destroyPipeline(graphicsPipeline_textured, nullptr);

    core->gpudevice.destroyPipelineLayout(pipelineLayout_untextured, nullptr);
    core->gpudevice.destroyPipeline(graphicsPipeline_untextured, nullptr);

	//vmaDestroyBuffer(vallocator, testmesh->vertexBuffer.buffer, testmesh->vertexBuffer.allocation);
    vmaDestroyAllocator(vallocator);

    core->cleanup();
    delete core;
}

bool MainEngine::StepEngine() {
	if (glfwWindowShouldClose(core->window)) {
		return false;
	} else {
		glfwPollEvents();
        step();
        draw();
		return true;
	}
}

MainEngine::MainEngine(uint32_t WIDTH, uint32_t HEIGHT){
	std::cout << "making engine" << std::endl;

	core = new VulkanCore(WIDTH,HEIGHT);
	queueFamilies = &core->queueFamilies;
	glfwSetWindowUserPointer(core->window,this);
	glfwSetFramebufferSizeCallback(core->window,WindowResizedCallback);

	CreateAllocator();
	CreateSwapchain();
	CreateRenderpass();
	CreateFramebuffer();
	CreateCommandpool();
	CreateDescriptorSets();
	CreateSyncObjects();

	CreateGraphicsPipeline();

	initial();

}