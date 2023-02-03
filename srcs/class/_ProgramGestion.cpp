/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/02/03 14:00:34 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "../../includes/class/_ProgramGestion.hpp"
#include "../../includes/engine.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

int		keyPress(GLFWwindow *window);
void	waitTime(void);
float	deltaTime(void);


extern ProgramGestion app;

//******************************************************************************************************//
//										Constructor - Destructor							    		//
//******************************************************************************************************//

ProgramGestion::ProgramGestion() : window(nullptr)
{
	return;
}

ProgramGestion::ProgramGestion(const ProgramGestion &src)
{
	*this = src;

	return;
}

ProgramGestion::~ProgramGestion()
{
	return;
}

ProgramGestion &ProgramGestion::operator=(const ProgramGestion &src)
{
	(void)src;

	return *this;
}

//******************************************************************************************************//
//											Public Methods									    		//
//******************************************************************************************************//

void ProgramGestion::run(std::string name, bool resizeable)
{
	initWindow(name, resizeable);
	initVulkan();
	mainLoop();
	cleanup();
}

// Clean up all Vulkan resources //
void ProgramGestion::cleanup()
{
	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);

	vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

	// Destroy the descriptor set layout //
	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	// Destroy buffers //
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);

	// Destroy semaphores //
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	// Destroy the debug messenger //
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}

//******************************************************************************************************//
//											Private Methods									    		//
//******************************************************************************************************//

//------------------------------------------- Global Gestion -------------------------------------------//

void ProgramGestion::initWindow(std::string name, bool resizeable)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, resizeable);

	window = glfwCreateWindow(widith, height, name.c_str(), nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void ProgramGestion::initVulkan()
{
	createInstance(); // Create Vulkan instance //
	setupDebugMessenger();
	createSurface();	  // Create surface for communication with selected GPU //
	pickPhysicalDevice(); // Select GPU //
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

//******************************************************************************************************//
//										 		Main loop							  		    		//
//******************************************************************************************************//

void ProgramGestion::mainLoop()
{
	// static int		i = 0;
	
	while (_quit == 1 && glfwWindowShouldClose(window) == 0)
	{
		glfwPollEvents();

		keyPress(window);
		
		drawFrame();
		
		waitTime();	// Wait for 16.666 ms to have 60 fps //

		// std::cout << "Time: " << time << "      I: " << i << std::endl;
		// i++;
	}
	vkDeviceWaitIdle(device);
}

//******************************************************************************************************//
//										 	Control functions						  		    		//
//******************************************************************************************************//

void	waitTime(void)
{
	static float	time = 0;

	time = deltaTime();
	while (time < 0.016666667)
	{
		time += deltaTime();
		if (time > 0.016666667)
			return ;
		sleep(0.001);
	}
	// time = 0;
}

float		deltaTime(void)
{
	static float	lastFrame = 0.0f;
	float			currentFrame = glfwGetTime();
	float			deltaTime = currentFrame - lastFrame;
	
	lastFrame = currentFrame;
	
	return (deltaTime);
}

void	key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// std::cout << "Key pressed: " << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		app._quit = 0;
	}
}

int	keyPress(GLFWwindow *window)
{
	glfwSetKeyCallback(window, key_callback);	// Key Hook //
	return (0);
}

//******************************************************************************************************//
//										 	Gestion GPU								  		    		//
//******************************************************************************************************//

//- Check if the graphic card is suitable - 'No member function' -//
int		ProgramGestion::rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader)
	{
		return 0;
	}

	return score;
}

//- Find graphic card -//
void	ProgramGestion::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;

	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	int i = 0;
	std::cout << std::endl;
	// Check for suitable GPU //
	for (const auto &device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));

			// Get device properties, name of GPU... //
			VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

			// Print device info //
			std::cout << "GPU[" << i << "] name: " << deviceProperties.deviceName << " ,GPU score: " << score << std::endl;
		}
		else
		{
			std::cout << "GPU not found" << std::endl;
		}
		i++;
	}
	std::cout << std::endl;

	// Check if a GPU has been found //
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0)
	{
		physicalDevice = candidates.rbegin()->second;
	}
	else
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	// Get selected device properties, name of GPU... //
	VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	std::cout << "Phisical Device Selected : " << deviceProperties.deviceName << std::endl;
	std::cout << "API Version              : " << deviceProperties.apiVersion << std::endl;
	std::cout << "Driver Version           : " << deviceProperties.driverVersion << std::endl;
	std::cout << "Vendor ID                : " << deviceProperties.vendorID << std::endl;
	std::cout << "Device ID                : " << deviceProperties.deviceID << std::endl;
	std::cout << "Device Type              : " << deviceProperties.deviceType << std::endl;
	std::cout << "Device LUID              : " << deviceProperties.limits.maxImageDimension2D << std::endl << std::endl;
}

bool ProgramGestion::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	// Verify that all required extensions are supported //
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// Verify that swap chain support is adequate //
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

//- Check if the graphic card support the extension -//
bool ProgramGestion::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	// Get the list of extensions //
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// Create a set of required extensions //
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	// Check if the extension is supported //
	for (const auto &extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

//- Find the queue family -//
void ProgramGestion::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	// Create queue create infos //
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; // Type of structure //
		queueCreateInfo.queueFamilyIndex = queueFamily;						// Index of the queue family to create a queue from //
		queueCreateInfo.queueCount = 1;										// Number of queues to create //
		queueCreateInfo.pQueuePriorities = &queuePriority;					// Array of queue priorities //
		queueCreateInfos.push_back(queueCreateInfo);						// Add queue create info to the vector //
	}

	VkPhysicalDeviceFeatures deviceFeatures{};								// Structure for specifying physical device features //
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	
	VkDeviceCreateInfo createInfo{};										// Structure for specifying logical device creation parameters //
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;				// Type of structure //

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());	// Number of queue create infos //
	createInfo.pQueueCreateInfos = queueCreateInfos.data();								// Array of queue create infos //

	createInfo.pEnabledFeatures = &deviceFeatures;										// Pointer to a structure containing physical device features to be enabled for the logical device //

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());	// Number of enabled logical device extensions //
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();						// Array of enabled logical device extensions //

	// Create validation layers //
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());	// Number of enabled validation layers //
		createInfo.ppEnabledLayerNames = validationLayers.data();						// Array of enabled validation layers //
	}
	else
	{
		createInfo.enabledLayerCount = 0;												// Number of enabled validation layers //
	}

	// Create the logical device //
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);		// Get the queue handle //
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);			// Get the queue handle //
}

//******************************************************************************************************//
//										 	Queue family							  		    		//
//******************************************************************************************************//

//- Find the queue family -//
ProgramGestion::QueueFamilyIndices ProgramGestion::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	// Find queue family that support graphics //
	for (const auto &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

// Find good supported surface format //
ProgramGestion::SwapChainSupportDetails ProgramGestion::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// Get the number of format //
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If formatCount is 0, there are no formats supported //
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Get the number of present mode //
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	// If presentModeCount is 0, there are no present mode supported //
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

// Choose the best surface format //
VkSurfaceFormatKHR ProgramGestion::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		// If the format is VK_FORMAT_B8G8R8A8_SRGB and the color space is VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, then it is the best format //
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

// Choose the best present mode //
VkPresentModeKHR ProgramGestion::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		// If the present mode is VK_PRESENT_MODE_MAILBOX_KHR, then it is the best present mode //
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

// Choose the best resolution for the window //
VkExtent2D ProgramGestion::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
	// If the current extent is at numeric limits, then the extent can vary //
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)};

		// bound the values of and between the allowed minimum and maximum extents that are supported by the implementation //
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

// Create the swap chain //
void ProgramGestion::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Get the number of image in the swap chain //
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// If maxImageCount is 0, there is no limit //
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Create the swap chain //
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR; // The type of the structure //
	createInfo.surface = surface;									// The surface to associate the swap chain with //

	createInfo.minImageCount = imageCount;						 	// The minimum number of images in the swap chain //
	createInfo.imageFormat = surfaceFormat.format;				 	// The format of the image //
	createInfo.imageColorSpace = surfaceFormat.colorSpace;		 	// The color space of the image //
	createInfo.imageExtent = extent;							 	// The size of the image //
	createInfo.imageArrayLayers = 1;							 	// The number of layers each image consists of //
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; 	// The bit field describing the intended usage of the image //

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// If the graphics and present family are different, we need to use concurrent mode //
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; 	// The image can be used across multiple queue families without explicit ownership transfers //
		createInfo.queueFamilyIndexCount = 2;					  	// The number of queue families that will access this image //
		createInfo.pQueueFamilyIndices = queueFamilyIndices;	  	// An array of queue family indices //
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; 	// An image is owned by one queue family at a time and ownership must be explicitly transfered before using it in another queue family //
	}

	// If the swap chain is transformed, we need to specify how it should be transformed //
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform; // The transformation to apply to the image as part of the presentation //
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;			  // The alpha compositing mode to use with this swap chain //
	createInfo.presentMode = presentMode;									  // The presentation mode to use for the swap chain //
	createInfo.clipped = VK_TRUE;											  // Whether the images are clipped //

	createInfo.oldSwapchain = VK_NULL_HANDLE; // The old swap chain to replace //

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// Get the swap chain images //
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

// Create the image views //
void ProgramGestion::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

// Cleaning the swap chain //
void ProgramGestion::cleanupSwapChain()
{
	vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	for (auto imageView : swapChainImageViews)
	{
		vkDestroyImageView(device, imageView, nullptr);
	}

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

// Recreate the swap chain //
void ProgramGestion::recreateSwapChain()
{
	int width = 0, height = 0;
	
	glfwGetFramebufferSize(window, &width, &height);
	// Wait until the window is not minimized //
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	// Wait until the device is idle before recreating the swap chain //
	vkDeviceWaitIdle(device);

	// Recreate the swap chain //
	cleanupSwapChain();

	// Recreate the swap chain //
	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();

	// // Recreate the command buffers //
	// createUniformBuffers();
	// createDescriptorPool();
	// createCommandBuffers();
}

//******************************************************************************************************//
//										 	Create surface							  		    		//
//******************************************************************************************************//

// Create the surface for comunicate between the window and the GPU //
void ProgramGestion::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

//******************************************************************************************************//
//								  	Create instance and debug messenger				  		    		//
//******************************************************************************************************//

// Create the instance //
void ProgramGestion::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Create the application info //
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;	   		// Specify the type of the structure //
	appInfo.pApplicationName = "DrackenLib";			   		// The name of the application //
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); 		// The version of the application //
	appInfo.pEngineName = "DrackEngine";				   		// The name of the engine //
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);	   		// The version of the engine //
	appInfo.apiVersion = VK_API_VERSION_1_0;			   		// The version of the Vulkan API //

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 	// Specify the type of the structure //
	createInfo.pApplicationInfo = &appInfo;					   	// The application info //

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size()); // The number of extensions //
	createInfo.ppEnabledExtensionNames = extensions.data();						 // The extensions //

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// If validation layers are enabled, add them to the instance //
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()); 	// The number of layers //
		createInfo.ppEnabledLayerNames = validationLayers.data();					   	// The layers //

		populateDebugMessengerCreateInfo(debugCreateInfo);						   		// The debug messenger create info //
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo; 		// The debug messenger create info //
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

bool ProgramGestion::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if all the layers in the validationLayers vector are in the availableLayers vector //
	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;

		// Check if the layer is in the availableLayers vector //
		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

// Get the required extensions //
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *pUserData)
{

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

// Create the debug messenger //
void ProgramGestion::setupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

// Create the debug messenger //
std::vector<const char *> ProgramGestion::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// Destroy the debug messenger //
void ProgramGestion::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
												   const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

// Chose value for the debug messenger //
void ProgramGestion::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
	| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	createInfo.pfnUserCallback = debugCallback;
}

//	Load the debug messenger //
VkResult ProgramGestion::CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator,
		VkDebugUtilsMessengerEXT *pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

//******************************************************************************************************//
//												Getters										    		//
//******************************************************************************************************//



//******************************************************************************************************//
//												Draw										    		//
//******************************************************************************************************//

// Pipeline only for triangle //
void ProgramGestion::createGraphicsPipeline(void)
{
	auto vertShaderCode = readFile("srcs/shaders/vert.spv");	// Read the shader code //
	auto fragShaderCode = readFile("srcs/shaders/frag.spv");	// Read the shader code //

	// Create the shader modules //
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);	// Create the shader module //
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);	// Create the shader module //

	// Create the shader stages //
	VkPipelineShaderStageCreateInfo	vertShaderStageInfo{};								// Create the shader stages //
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// Set the type //
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;								// Set the stage //			
	vertShaderStageInfo.module = vertShaderModule;										// Set the module //
	vertShaderStageInfo.pName = "main";													// Set the name //

	// Create the shader stages //
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};								// Create the shader stages //
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// Set the type //
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;							// Set the stage //
	fragShaderStageInfo.module = fragShaderModule;										// Set the module //
	fragShaderStageInfo.pName = "main";													// Set the name //

	// Create the shader stages //
	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	// Create the vertex input //
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};								// Create the vertex input //
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;														// Create the vertex input //
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Create the input assembly //
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};								// Create the input assembly //
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;	
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Create the viewport //
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;		// Create the viewport //
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Create the rasterizer //
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;	// Create the rasterizer //
	rasterizer.depthClampEnable = VK_FALSE;											// Disable the depth clamp //
	rasterizer.rasterizerDiscardEnable = VK_FALSE;									// Disable the rasterizer discard //
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;									// Set the polygon mode //
	rasterizer.lineWidth = 1.0f;													// Set the line width //
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;									// Set the cull mode //
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;							// Set the front face //
	rasterizer.depthBiasEnable = VK_FALSE;											// Disable the depth bias //

	// Create the multisampling //
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;	// Create the multisampling //
	multisampling.sampleShadingEnable = VK_FALSE;									// Disable the sample shading //
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;						// Set the rasterization samples //

	// Create the color blending //
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
		| VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;									// Create the color blending //
	colorBlendAttachment.blendEnable = VK_FALSE;												// Disable the blending //

	// Create the color blending //
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;	// Create the color blending //
	colorBlending.logicOpEnable = VK_FALSE;											// Disable the logic operation //
	colorBlending.logicOp = VK_LOGIC_OP_COPY;										// Set the logic operation //
	colorBlending.attachmentCount = 1;												// Set the attachment count //
	colorBlending.pAttachments = &colorBlendAttachment;								// Set the attachments //
	colorBlending.blendConstants[0] = 0.0f;											// Set the blend constants //
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Create the dynamic states //
	std::vector<VkDynamicState> dynamicStates = {	// Create the dynamic states //
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	
	// Create the dynamic states //	
	VkPipelineDynamicStateCreateInfo dynamicState{};								// Create the dynamic states //
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Create the pipeline layout //
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};							// Create the pipeline layout //
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;	
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

	// Create the pipeline layout //
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Create the graphics pipeline //
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;	// Create the graphics pipeline //
	pipelineInfo.stageCount = 2;											// Set the stage count //
	pipelineInfo.pStages = shaderStages;									// Set the stages //
	pipelineInfo.pVertexInputState = &vertexInputInfo;						// Set the vertex input state //
	pipelineInfo.pInputAssemblyState = &inputAssembly;						// Set the input assembly state //
	pipelineInfo.pViewportState = &viewportState;							// Set the viewport state //
	pipelineInfo.pRasterizationState = &rasterizer;							// Set the rasterization state //
	pipelineInfo.pMultisampleState = &multisampling;						// Set the multisampling state //
	pipelineInfo.pColorBlendState = &colorBlending;							// Set the color blending state //
	pipelineInfo.pDynamicState = &dynamicState;								// Set the dynamic state //
	pipelineInfo.layout = pipelineLayout;									// Set the pipeline layout //
	pipelineInfo.renderPass = renderPass;									// Set the render pass //
	pipelineInfo.subpass = 0;												// Set the subpass //
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;						// Set the base pipeline handle //

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;	// Create the depth stencil //
	depthStencil.depthTestEnable = VK_TRUE;												// Enable the depth test //
	depthStencil.depthWriteEnable = VK_TRUE;											// Enable the depth write //
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;									// Set the depth compare operation //
	depthStencil.depthBoundsTestEnable = VK_FALSE;										// Disable the depth bounds test //
	depthStencil.minDepthBounds = 0.0f; // Optional										// Set the min depth bounds //
	depthStencil.maxDepthBounds = 1.0f; // Optional										// Set the max depth bounds //
	depthStencil.stencilTestEnable = VK_FALSE;											// Disable the stencil test //
	depthStencil.front = {}; // Optional												// Set the front //
	depthStencil.back = {}; // Optional													// Set the back //

	pipelineInfo.pDepthStencilState = &depthStencil;						// Set the depth stencil //	

	// Create the graphics pipeline //
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

// Shader //
VkShaderModule ProgramGestion::createShaderModule(const std::vector<char> &code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;		// Type of the structure //
	createInfo.codeSize = code.size();									// Size of the code //
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data()); // Code //

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

// Read a file and return a vector of char //
static std::vector<char> readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	// Get the size of the file //
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Read the file //
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

// Multisampling //
void ProgramGestion::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};											// Create the color attachment //
	colorAttachment.format = swapChainImageFormat;										// Set the format //
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;									// Set the sample count //
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;								// Set the load operation //
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;								// Set the store operation //
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;					// Set the stencil load operation //
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;					// Set the stencil store operation //
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;							// Set the initial layout //
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;						// Set the final layout //

	VkAttachmentDescription depthAttachment{};											// Create the depth attachment //
	depthAttachment.format = findDepthFormat();											// Set the format //					
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;									// Set the sample count //
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;								// Set the load operation //
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;							// Set the store operation //
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;					// Set the stencil load operation //
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;					// Set the stencil store operation //
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;							// Set the initial layout //
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// Set the final layout //

	VkAttachmentReference colorAttachmentRef{};											// Create the color attachment reference //
	colorAttachmentRef.attachment = 0;													// Set the attachment //
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;				// Set the layout //

	VkAttachmentReference depthAttachmentRef{};											// Create the depth attachment reference //
	depthAttachmentRef.attachment = 1;													// Set the attachment //
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;		// Set the layout //

	VkSubpassDescription subpass{};														// Create the subpass //
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;						// Set the pipeline bind point //
	subpass.colorAttachmentCount = 1;													// Set the color attachment count //
	subpass.pColorAttachments = &colorAttachmentRef;									// Set the color attachment //
	subpass.pDepthStencilAttachment = &depthAttachmentRef;								// Set the depth attachment //

	VkSubpassDependency dependency{};																						// Create the subpass dependency //
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;																			// Set the source subpass //
	dependency.dstSubpass = 0;																								// Set the destination subpass //
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// Set the source stage mask //
	dependency.srcAccessMask = 0;																							// Set the source access mask //
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// Set the destination stage mask //
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;			// Set the destination access mask //

	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};	// Create the attachments //
	VkRenderPassCreateInfo renderPassInfo{};													// Create the render pass info //
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;							// Set the structure type //
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());					// Set the attachment count //
	renderPassInfo.pAttachments = attachments.data();											// Set the attachments //
	renderPassInfo.subpassCount = 1;															// Set the subpass count //
	renderPassInfo.pSubpasses = &subpass;														// Set the subpasses //
	renderPassInfo.dependencyCount = 1;															// Set the dependency count //
	renderPassInfo.pDependencies = &dependency;													// Set the dependencies //

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}
}

// Create Framebuffers //
void ProgramGestion::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	// Create the framebuffers //
	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		// VkImageView attachments[] = { swapChainImageViews[i] };

		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		// Create the framebuffer info //
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;				// Set the structure type //
		framebufferInfo.renderPass = renderPass;										// Set the render pass //
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());	// Set the attachment count //
		framebufferInfo.pAttachments = attachments.data();								// Set the attachments //
		framebufferInfo.width = swapChainExtent.width;									// Set the width //
		framebufferInfo.height = swapChainExtent.height;								// Set the height //
		framebufferInfo.layers = 1;														// Set the layers //

		// Create the framebuffer //
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

//******************************************************************************************************//
//										Fuctions for draw on screen							    		//
//******************************************************************************************************//

// Draw on screen //
void ProgramGestion::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	// Create the command pool //
	VkCommandPoolCreateInfo poolInfo{};										// Command pool //
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;			// Type of the structure //
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;		// Command pool creation flags //
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();	// Index of the queue family //

	// Create the command pool //
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

// Create command buffer //
void ProgramGestion::createCommandBuffers()
{
	// Resize the command buffer //
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	// Create the command buffer //
	VkCommandBufferAllocateInfo allocInfo{};							// Command buffer //
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;	// Type of the structure //
	allocInfo.commandPool = commandPool;								// Command pool //
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// Level of the command buffer //
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();		// Number of command buffers //

	// Create the command buffer //
	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

// Record command buffer //
void ProgramGestion::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	// beginInfo.flags = 0; // Optional
	// beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;	// Type of the structure //
	renderPassInfo.renderPass = renderPass;								// Render pass //
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];		// Framebuffer //
	renderPassInfo.renderArea.offset = {0, 0};							// Render area //
	renderPassInfo.renderArea.extent = swapChainExtent;					// Render area //

	// VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
	clearValues[1].depthStencil = {1.0f, 0};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Start the render pass //
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Link the pipeline to the command buffer //
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		// Set the viewport //
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapChainExtent.width);
		viewport.height = static_cast<float>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		// Set the scissor //
		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapChainExtent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// Bind the vertex buffer //
		VkBuffer vertexBuffers[] = {vertexBuffer};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		// Bind the index buffer //
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// Bind the descriptor sets //
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

		// Draw the object //
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	// End the render pass //
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

// Core for draw on screen //
void ProgramGestion::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX,
						imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	// Only reset the fence if we are submitting work
	updateUniformBuffer(currentFrame);
	
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Wait for the image to be available //
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};				// Semaphore to wait for //
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};	// Stage to wait for //
	submitInfo.waitSemaphoreCount = 1;														// Number of semaphores to wait for //
	submitInfo.pWaitSemaphores = waitSemaphores;											// Semaphores to wait for //
	submitInfo.pWaitDstStageMask = waitStages;												// Stages to wait for //

	submitInfo.commandBufferCount = 1;														// Number of command buffers to submit //
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];								// Command buffers to submit //

	// Signal that the image is ready to be rendered //
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};				// Semaphore to signal //
	submitInfo.signalSemaphoreCount = 1;													// Number of semaphores to signal //
	submitInfo.pSignalSemaphores = signalSemaphores;										// Semaphores to signal //

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// Present the image //
	VkPresentInfoKHR presentInfo{};								// Present info //
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;		// Type of present info //

	presentInfo.waitSemaphoreCount = 1;							// Number of semaphores to wait for //
	presentInfo.pWaitSemaphores = signalSemaphores;				// Semaphores to wait for //

	// Specify the swap chain to present the image to //
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;								// Number of swap chains //
	presentInfo.pSwapchains = swapChains;						// Swap chains to present to //

	presentInfo.pImageIndices = &imageIndex;					// Index of the image to present //

	result = vkQueuePresentKHR(presentQueue, &presentInfo);		// Present the image //

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;	// Update the current frame //
}

// Create semaphores //
void ProgramGestion::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);			// Resize the image available semaphores //
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);			// Resize the render finished semaphores //
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);					// Resize the in flight fences //

	// Create the semaphore info //
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;	// Type of semaphore info //

	// Create the fence info //
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;			// Type of fence info //
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;					// Fence flags //

	// Create the semaphores and fences //
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		// Create the semaphores //
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

//******************************************************************************************************//
//											Vertex Buffers									    		//
//******************************************************************************************************//

// Create the vertex buffer //
void ProgramGestion::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Create the staging buffer //
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Copy the vertex data to the vertex buffer //
	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create the vertex buffer //
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

uint32_t ProgramGestion::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// Find the memory type that matches the requirements //
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return (i);
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void ProgramGestion::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
								  VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
	// Size of the buffer //
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;		// Type of buffer info //
	bufferInfo.size = size;											// Size of the buffer //

	// Buffer will be used as a vertex buffer //
	bufferInfo.usage = usage;										// Buffer usage //
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;				// Buffer sharing mode //

	// Create the buffer //
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	// Get the memory requirements for the buffer //
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Allocate the memory for the buffer //
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;									// Type of memory allocation info //
	allocInfo.allocationSize = memRequirements.size;											// Size of the allocation //
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);		// Memory type index //

	// Allocate the memory //
	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

// Copy the buffer //
void ProgramGestion::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	// Create the command buffer //
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Copy the buffer //
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	// End the command buffer //
    endSingleTimeCommands(commandBuffer);
}

//******************************************************************************************************//
//											Index Buffers									    		//
//******************************************************************************************************//

// Indices for the vertices //
void ProgramGestion::createIndexBuffer()
{
	VkDeviceSize	bufferSize = sizeof(indices[0]) * indices.size();

	// Create the staging buffer //
	VkBuffer		stagingBuffer;
	VkDeviceMemory	stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Copy the data to the staging buffer //
	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create the index buffer //
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

//******************************************************************************************************//
//											Uniform Buffers									    		//
//******************************************************************************************************//

void ProgramGestion::createDescriptorSetLayout()
{
	// Uniform buffer //
	VkDescriptorSetLayoutBinding uboLayoutBinding{};									// Uniform buffer //
	uboLayoutBinding.binding = 0;														// Binding point //
	uboLayoutBinding.descriptorCount = 1;												// Number of descriptors //
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// Type of descriptor //
	uboLayoutBinding.pImmutableSamplers = nullptr;										// Not used //
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;							// Shader stage //

	// Sampler //
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};								// Sampler //
	samplerLayoutBinding.binding = 1;													// Binding point //
	samplerLayoutBinding.descriptorCount = 1;											// Number of descriptors //
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;	// Type of descriptor //
	samplerLayoutBinding.pImmutableSamplers = nullptr;									// Not used //
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;						// Shader stage //

	// Create the descriptor set layout //
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {							// Create the descriptor set layout //
		uboLayoutBinding, samplerLayoutBinding
	};
	
	VkDescriptorSetLayoutCreateInfo layoutInfo{};										// Create the descriptor set layout //
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;				// Create the descriptor set layout //
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());					// Create the descriptor set layout //
	layoutInfo.pBindings = bindings.data();												// Create the descriptor set layout //

	// Create the descriptor set layout //
	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void ProgramGestion::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	// Create the uniform buffers //
	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i], uniformBuffersMemory[i]);

		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}

// Update the uniform buffer //
void ProgramGestion::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	// Get the current time //
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	// Update the uniform buffer //
	UniformBufferObject ubo{};
	//                                                     / Rotate Speed /
	ubo.model = glm::rotate(glm::mat4(0.20f), time * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));						// Rotate the model //
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));			// Camera position //
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);	// Perspective projection //
	ubo.proj[1][1] *= -1;																									// Flip the Y axis //

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));															// Copy the data to the uniform buffer //
}

// Create the descriptor pool //
void ProgramGestion::createDescriptorPool()
{
	// Create the descriptor pool //
	std::array<VkDescriptorPoolSize, 2> poolSizes{};								// Create the descriptor pool //
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;							// Uniform buffer //
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);		// Number of descriptors //
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;					// Sampler //
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);		// Number of descriptors //

	// Create the descriptor pool //
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	// Create the descriptor pool //
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

// Create the descriptor sets //
void	ProgramGestion::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;						// Allocate the descriptor sets //
	allocInfo.descriptorPool = descriptorPool;												// Descriptor pool //
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);				// Number of descriptor sets //
	allocInfo.pSetLayouts = layouts.data();													// Descriptor set layout //

	// Allocate the descriptor sets //
	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	// Update the descriptor sets //
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		// Update the descriptor sets //
		VkDescriptorBufferInfo bufferInfo{};												// Uniform buffer //
		bufferInfo.buffer = uniformBuffers[i];												// Buffer //
		bufferInfo.offset = 0;																// Offset //
		bufferInfo.range = sizeof(UniformBufferObject);										// Size //

		VkDescriptorImageInfo imageInfo{};													// Image //
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;					// Image layout //
		imageInfo.imageView = textureImageView;												// Image view //
		imageInfo.sampler = textureSampler;													// Sampler //

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;					// Uniform buffer //
		descriptorWrites[0].dstSet = descriptorSets[i];										// Descriptor set //
		descriptorWrites[0].dstBinding = 0;													// Binding //
		descriptorWrites[0].dstArrayElement = 0;											// Array element //
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;				// Descriptor type //
		descriptorWrites[0].descriptorCount = 1;											// Number of descriptors //
		descriptorWrites[0].pBufferInfo = &bufferInfo;										// Buffer info //

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;					// Image //
		descriptorWrites[1].dstSet = descriptorSets[i];										// Descriptor set //
		descriptorWrites[1].dstBinding = 1;													// Binding //
		descriptorWrites[1].dstArrayElement = 0;											// Array element //
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;		// Descriptor type //
		descriptorWrites[1].descriptorCount = 1;											// Number of descriptors //
		descriptorWrites[1].pImageInfo = &imageInfo;										// Image info //

		// Update the descriptor sets //
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}
}

//******************************************************************************************************//
//											Texture mapping									    		//
//******************************************************************************************************//
 
void	ProgramGestion::createTextureImage()
{
	// Load the image //
	int texWidth, texHeight, texChannels;
    // stbi_uc* 		pixels = stbi_load("srcs/textures/ichigo.png", &texWidth, &texHeight,
	// 							&texChannels, STBI_rgb_alpha);
	stbi_uc	*pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight,
						&texChannels, STBI_rgb_alpha);
    VkDeviceSize	imageSize = texWidth * texHeight * 4;
	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	// Check if the image has been loaded //
    if (!pixels)
	{
        throw std::runtime_error("failed to load texture image!");
    }

	// Create the mipmaps //

	// Create the staging buffer //
    VkBuffer		stagingBuffer;
    VkDeviceMemory	stagingBufferMemory;
    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Copy the image data to the staging buffer //
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

	// Free the image data //
    stbi_image_free(pixels);

	// Create the image //
    createImage(texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);
//
	// Copy the staging buffer to the image //
	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight));

	// // Transition the image layout //
	// transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	// 	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

//
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
	
	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
}

void	ProgramGestion::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
			VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image,
				VkDeviceMemory& imageMemory)
{
	// Create the image //
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;		// The type of the structure //
    imageInfo.imageType = VK_IMAGE_TYPE_2D;						// The type of the image //
    imageInfo.extent.width = width;								// The width of the image //
    imageInfo.extent.height = height;							// The height of the image //
    imageInfo.extent.depth = 1;									// The depth of the image //
    imageInfo.mipLevels = mipLevels;							// The number of mipmap levels //
    imageInfo.arrayLayers = 1;									// The number of layers //
    imageInfo.format = format;									// The format of the image //
    imageInfo.tiling = tiling;									// The tiling of the image //
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// The initial layout of the image //
    imageInfo.usage = usage;									// The usage of the image //
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;					// The number of samples //
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;			// The sharing mode of the image //

	// Create the image //
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
        throw std::runtime_error("echec de la creation d'une image!");
    }

	// Allocate the memory for the image //
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

	// Allocate the memory //
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	// Bind the memory to the image //
    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
        throw std::runtime_error("echec de l'allocation de la memoire d'une image!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

VkCommandBuffer	ProgramGestion::beginSingleTimeCommands()
{
	// Create the command pool //
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;	// The type of the structure //
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;					// The level of the command buffer //
    allocInfo.commandPool = commandPool;								// The command pool //
    allocInfo.commandBufferCount = 1;									// The number of command buffers //

	// Allocate the command buffer //
    VkCommandBuffer commandBuffer;										// The command buffer //
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);		// Allocate the command buffer //

	// Begin the command buffer //
    VkCommandBufferBeginInfo beginInfo{};								// The structure //
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;		// The type of the structure //
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;		// The flags //

	// Begin the command buffer //
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void	ProgramGestion::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

	// Create the submit info //
    VkSubmitInfo submitInfo{};										// The structure //
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;				// The type of the structure //
    submitInfo.commandBufferCount = 1;								// The number of command buffers //
    submitInfo.pCommandBuffers = &commandBuffer;					// The command buffers //

	// Submit the command buffer //
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);	// Submit the command buffer //
    vkQueueWaitIdle(graphicsQueue);									// Wait for the queue to be idle //

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void	ProgramGestion::transitionImageLayout(VkImage image, VkFormat format,
			VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
	// Create the command buffer //
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Create the image barrier //
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;				// The type of the structure //
	barrier.oldLayout = oldLayout;										// The old layout of the image //
	barrier.newLayout = newLayout;										// The new layout of the image //
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// The source queue family //
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// The destination queue family //
	
	// Create the image barrier //
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// The aspect mask of the image //
	barrier.subresourceRange.baseMipLevel = 0;							// The base mipmap level //
	barrier.subresourceRange.levelCount = mipLevels;					// The number of mipmap levels //
	barrier.subresourceRange.baseArrayLayer = 0;						// The base array layer //
	barrier.subresourceRange.layerCount = 1;							// The number of array layers //
	// barrier.srcAccessMask = 0; // TODO
	// barrier.dstAccessMask = 0; // TODO

	// Create the image barrier //
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	// Create the image barrier //
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	endSingleTimeCommands(commandBuffer);
}

void	ProgramGestion::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Create the image barrier //
	VkBufferImageCopy region{};											// The region of the buffer //
	region.bufferOffset = 0;											// The offset of the buffer //
	region.bufferRowLength = 0;											// The row length of the buffer //
	region.bufferImageHeight = 0;										// The image height of the buffer //

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// The aspect mask of the image //
	region.imageSubresource.mipLevel = 0;								// The mipmap level of the image //
	region.imageSubresource.baseArrayLayer = 0;							// The base array layer of the image //
	region.imageSubresource.layerCount = 1;								// The number of array layers of the image //

	region.imageOffset = {0, 0, 0};										// The offset of the image //
	region.imageExtent = {												// The extent of the image //
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void	ProgramGestion::createTextureImageView()
{
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

VkImageView	ProgramGestion::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	// Create the image view //
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;		// The type of the structure //
	viewInfo.image = image;											// The image to create the view for //
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// The type of the image view //
	viewInfo.format = format;										// The format of the image //
	viewInfo.subresourceRange.aspectMask = aspectFlags;				// The aspect mask of the image //
	viewInfo.subresourceRange.baseMipLevel = 0;						// The base mipmap level //
	viewInfo.subresourceRange.levelCount = mipLevels;				// The number of mipmap levels //
	viewInfo.subresourceRange.baseArrayLayer = 0;					// The base array layer //
	viewInfo.subresourceRange.layerCount = 1;						// The number of array layers //

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void	ProgramGestion::createTextureSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;				// The type of the structure //
	samplerInfo.magFilter = VK_FILTER_LINEAR;								// The magnification filter //
	samplerInfo.minFilter = VK_FILTER_LINEAR;								// The minification filter //
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;				// The addressing mode for the U coordinate //
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;				// The addressing mode for the V coordinate //
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;				// The addressing mode for the W coordinate //
	samplerInfo.anisotropyEnable = VK_FALSE;								// Enable anisotropic filtering //
	// samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;		// The maximum anisotropy //
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;				// The border color //
	samplerInfo.unnormalizedCoordinates = VK_FALSE;							// Enable unnormalized coordinates //
	samplerInfo.compareEnable = VK_FALSE;									// Enable compare operations //
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;							// The compare operation //
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;					// The mipmap mode //
	samplerInfo.minLod = 0.0f;												// The minimum level of detail //
    samplerInfo.maxLod = static_cast<float>(mipLevels);						// The maximum level of detail //
    samplerInfo.mipLodBias = 0.0f;											// The mipmap level of detail bias //

	// Create the sampler //
	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

//******************************************************************************************************//
//											Deapth buffer									    		//
//******************************************************************************************************//

// Find a supported depth format //
void	ProgramGestion::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	
	// Create the depth image //
	createImage(swapChainExtent.width, swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
}

// Find a supported depth format //
VkFormat	ProgramGestion::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	// Loop through the candidates //
	for (VkFormat format : candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		// Check if the format is supported //
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

// Find a supported depth format //
VkFormat ProgramGestion::findDepthFormat()
{
    return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

// Check if the format has a stencil component //
bool	ProgramGestion::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

//******************************************************************************************************//
//											Charging models									    		//
//******************************************************************************************************//

// Load the model //
void	ProgramGestion::loadModel()
{
	tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str()))
	{
        throw std::runtime_error(warn + err);
    }

	// For stock vertex once //
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	// Stock each vertex position //
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{};

			
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

//******************************************************************************************************//
//												Minimaps									    		//
//******************************************************************************************************//

// Generate the minimaps //
void	ProgramGestion::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth,
			int32_t texHeight, uint32_t mipLevels)
{
	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// Transition the image layout to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL to prepare for blitting //
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;				// Structure type //
	barrier.image = image;												// Image to transition //
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family //
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family //
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Aspect mask //
	barrier.subresourceRange.baseArrayLayer = 0;						// Base array layer //
	barrier.subresourceRange.layerCount = 1;							// Layer count //
	barrier.subresourceRange.levelCount = 1;							// Level count //

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	// Generate the minimaps //
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;						// Base mip level //
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;			// Old layout //
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;			// New layout //
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;				// Source access mask //
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;				// Destination access mask //

		// Transition the image layout //
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,				// Command buffer //
			VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);	// Pipeline stage //

		// Blit the image //
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};																	// Source offsets //
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1};													// Source offsets //
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;										// Aspect mask //
		blit.srcSubresource.mipLevel = i - 1;															// Mip level //
		blit.srcSubresource.baseArrayLayer = 0;															// Base array layer //
		blit.srcSubresource.layerCount = 1;																// Layer count //
		blit.dstOffsets[0] = {0, 0, 0};																	// Destination offsets //
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };	// Destination offsets //
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;										// Aspect mask //
		blit.dstSubresource.mipLevel = i;																// Mip level //
		blit.dstSubresource.baseArrayLayer = 0;															// Base array layer //
		blit.dstSubresource.layerCount = 1;																// Layer count //

		// Blit the image //
		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,		// Command buffer //
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);	// Filter //

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;						// Old layout //
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;					// New layout //
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;							// Source access mask //
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;								// Destination access mask //

		vkCmdPipelineBarrier(commandBuffer,												// Command buffer //
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)																// If the width is greater than 1 //
			mipWidth /= 2;
		if (mipHeight > 1)																// If the height is greater than 1 //
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;								// Base mip level //
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;							// Old layout //
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;						// New layout //
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;								// Source access mask //
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;									// Destination access mask //

	vkCmdPipelineBarrier(commandBuffer,													// Command buffer //
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandBuffer);												// End the command buffer //
}
