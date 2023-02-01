/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/31 19:27:55 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/class/_ProgramGestion.hpp"
#include "../../includes/engine.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

//******************************************************************************************************//
//										Constructor - Destructor							    		//
//******************************************************************************************************//



ProgramGestion::ProgramGestion() :
window(nullptr)
{
	return ;
}

ProgramGestion::ProgramGestion(const ProgramGestion &src)
{
	*this = src;
	
	return ;
}

ProgramGestion::~ProgramGestion()
{
	return ;
}

ProgramGestion  &ProgramGestion::operator=(const ProgramGestion &src)
{
	(void)src;
	
	return *this;
}

//******************************************************************************************************//
//											Public Methods									    		//
//******************************************************************************************************//

void	ProgramGestion::run(std::string name, bool resizeable)
{
	initWindow(name, resizeable);
	initVulkan();
	mainLoop();
	cleanup();
}

//******************************************************************************************************//
//											Private Methods									    		//
//******************************************************************************************************//

//---------------------------------- Private Methods: Global Gestion -----------------------------------//
		
void	ProgramGestion::initWindow(std::string name, bool resizeable)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, resizeable);

	window = glfwCreateWindow(WIDTH, HEIGHT, name.c_str(), nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void	ProgramGestion::initVulkan()
{
	createInstance();			// Create Vulkan instance //
    setupDebugMessenger();		
    createSurface();			// Create surface for communication with selected GPU //
    pickPhysicalDevice();		// Select GPU //
    createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();	
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createCommandBuffers();
	createSyncObjects();
}

void	ProgramGestion::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(device);
}

void	ProgramGestion::cleanup()
{
	cleanupSwapChain();

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

    vkDestroyRenderPass(device, renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);

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
//-------------------------------------------- Gestion GPU ---------------------------------------------//

//- Check if the graphic card is suitable - 'No member function' -//
int rateDeviceSuitability(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties	deviceProperties;
	VkPhysicalDeviceFeatures	deviceFeatures;
	
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
	uint32_t	deviceCount = 0;
	
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
	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			int score = rateDeviceSuitability(device);
        	candidates.insert(std::make_pair(score, device));
			
			// Print device info //
			std::cout << "GPU[" << i << "] name: " << device << " ,GPU score: " << score << std::endl;
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
	std::cout << "Phisical Device Selected: " << physicalDevice << std::endl << std::endl;
}

bool	ProgramGestion::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices	indices = findQueueFamilies(device);

	// Verify that all required extensions are supported //
	bool	extensionsSupported = checkDeviceExtensionSupport(device);

	// Verify that swap chain support is adequate //
	bool	swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails	swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

//- Check if the graphic card support the extension -//
bool	ProgramGestion::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties>	availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string>	requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	
    for (const auto& extension : availableExtensions)
	{
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

//- Find the queue family -//
void	ProgramGestion::createLogicalDevice()
{
	QueueFamilyIndices	indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo>		queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =	{indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	// Create queue create infos //
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo	queueCreateInfo{};
		
		queueCreateInfo.sType =					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;	// Type of structure //
		queueCreateInfo.queueFamilyIndex =		queueFamily;								// Index of the queue family to create a queue from //
		queueCreateInfo.queueCount =			1;											// Number of queues to create //
		queueCreateInfo.pQueuePriorities =		&queuePriority;								// Array of queue priorities //
		queueCreateInfos.push_back(queueCreateInfo);										// Add queue create info to the vector //
	}

	VkPhysicalDeviceFeatures	deviceFeatures{};

	VkDeviceCreateInfo			createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount	=		static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos =			queueCreateInfos.data();

	createInfo.pEnabledFeatures	=			&deviceFeatures;

	createInfo.enabledExtensionCount =		static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames =	deviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount =		static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames =	validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount	= 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

//******************************************************************************************************//
//------------------------------------------- Queue family ---------------------------------------------//

//- Find the queue family -//
ProgramGestion::QueueFamilyIndices	ProgramGestion::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	// Find queue family that support graphics //
	for (const auto& queueFamily : queueFamilies)
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
ProgramGestion::SwapChainSupportDetails		ProgramGestion::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails	details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	
	// Get the number of format //
	uint32_t	formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	// If formatCount is 0, there are no formats supported //
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// Get the number of present mode //
	uint32_t	presentModeCount;
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
VkSurfaceFormatKHR	ProgramGestion::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		// If the format is VK_FORMAT_B8G8R8A8_SRGB and the color space is VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, then it is the best format //
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

// Choose the best present mode //
VkPresentModeKHR	ProgramGestion::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
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
VkExtent2D	ProgramGestion::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
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
            static_cast<uint32_t>(height)
        };

		// bound the values of and between the allowed minimum and maximum extents that are supported by the implementation //
        actualExtent.width	= std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height	= std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

// Create the swap chain //
void	ProgramGestion::createSwapChain()
{
    SwapChainSupportDetails	swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR		surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR		presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D				extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Get the number of image in the swap chain //
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// If maxImageCount is 0, there is no limit //
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Create the swap chain //
	VkSwapchainCreateInfoKHR		createInfo{};
	createInfo.sType =				VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;	// The type of the structure //
	createInfo.surface =			surface;										// The surface to associate the swap chain with //
	
	createInfo.minImageCount =		imageCount;										// The minimum number of images in the swap chain //
	createInfo.imageFormat =		surfaceFormat.format;							// The format of the image //
	createInfo.imageColorSpace =	surfaceFormat.colorSpace;						// The color space of the image //
	createInfo.imageExtent =		extent;											// The size of the image //
	createInfo.imageArrayLayers =	1;												// The number of layers each image consists of //
	createInfo.imageUsage =			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;			// The bit field describing the intended usage of the image //

	QueueFamilyIndices	indices = findQueueFamilies(physicalDevice);
	uint32_t			queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	
	// If the graphics and present family are different, we need to use concurrent mode //
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;	// The image can be used across multiple queue families without explicit ownership transfers //
		createInfo.queueFamilyIndexCount = 2;						// The number of queue families that will access this image //
		createInfo.pQueueFamilyIndices = queueFamilyIndices;		// An array of queue family indices //
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;		// An image is owned by one queue family at a time and ownership must be explicitly transfered before using it in another queue family //
		// createInfo.queueFamilyIndexCount = 0; // Optional			// The number of queue families that will access this image //
		// createInfo.pQueueFamilyIndices = nullptr; // Optional		// An array of queue family indices //
	}

	// If the swap chain is transformed, we need to specify how it should be transformed //
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;	// The transformation to apply to the image as part of the presentation //
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;				// The alpha compositing mode to use with this swap chain //
	createInfo.presentMode = presentMode;										// The presentation mode to use for the swap chain //
	createInfo.clipped = VK_TRUE;												// Whether the images are clipped //
	
	createInfo.oldSwapchain = VK_NULL_HANDLE;									// The old swap chain to replace //

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// Get the swap chain images //
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat =	surfaceFormat.format;
	swapChainExtent =		extent;
}

// Create the image views //
void	ProgramGestion::createImageViews()
{
	// Resize the vector to the number of swap chain images //
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		
		VkImageViewCreateInfo createInfo{};
		
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;	// Specify the type of the structure //
		createInfo.image = swapChainImages[i];							// Specify the image to create a view for //
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;					// Specify the type of the image view //
		createInfo.format = swapChainImageFormat;						// Specify the format of the image data //

		// Specify the components to use for color channel RGBA//
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Specify the purpose of the image and which part of the image should be accessed //
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// The aspect of the image that the view is created for //
		createInfo.subresourceRange.baseMipLevel = 0;						// The first mipmap level accessible to the view //
		createInfo.subresourceRange.levelCount = 1;							// The number of mipmap levels (and array layers) accessible to the view //
		createInfo.subresourceRange.baseArrayLayer = 0;						// The first array layer accessible to the view //
		createInfo.subresourceRange.layerCount = 1;							// The number of array layers accessible to the view //

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create image views!");
		}
	}
}

void	ProgramGestion::cleanupSwapChain()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void	ProgramGestion::recreateSwapChain()
{
	int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0)
	{
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
	
	// Wait until the device is idle before recreating the swap chain //
    vkDeviceWaitIdle(device);

	cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();
}

//******************************************************************************************************//
//------------------------------------------ Create surface --------------------------------------------//

void	ProgramGestion::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
	
}

//******************************************************************************************************//
//-------------------------------- Create instance and debug messenger ---------------------------------//

// Create the instance //
void	ProgramGestion::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Create the application info //
	VkApplicationInfo appInfo =		{};
	appInfo.sType =					VK_STRUCTURE_TYPE_APPLICATION_INFO;	// Specify the type of the structure //
	appInfo.pApplicationName =		"DrackenLib";						// The name of the application //
	appInfo.applicationVersion =	VK_MAKE_VERSION(1, 0, 0);			// The version of the application //
	appInfo.pEngineName =			"DrackEngine";						// The name of the engine //
	appInfo.engineVersion =			VK_MAKE_VERSION(1, 0, 0);			// The version of the engine //
	appInfo.apiVersion =			VK_API_VERSION_1_0;					// The version of the Vulkan API //

	VkInstanceCreateInfo createInfo =	{};
	createInfo.sType =					VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;			// Specify the type of the structure //
	createInfo.pApplicationInfo =		&appInfo;										// The application info //

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount =		static_cast<uint32_t>(extensions.size());	// The number of extensions //
	createInfo.ppEnabledExtensionNames =	extensions.data();							// The extensions //

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// If validation layers are enabled, add them to the instance //
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount =		static_cast<uint32_t>(validationLayers.size());	// The number of layers //
		createInfo.ppEnabledLayerNames =	validationLayers.data();						// The layers //

		populateDebugMessengerCreateInfo(debugCreateInfo);									// The debug messenger create info //
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;			// The debug messenger create info //
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

bool	ProgramGestion::checkValidationLayerSupport()
{
    uint32_t	layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties>	availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if all the layers in the validationLayers vector are in the availableLayers vector //
    for (const char* layerName : validationLayers)
	{
    	bool layerFound = false;
		
		// Check if the layer is in the availableLayers vector //
		for (const auto& layerProperties : availableLayers)
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
static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT	messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT	messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void* pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

// Create the debug messenger //
void	ProgramGestion::setupDebugMessenger()
{
    if (!enableValidationLayers)
		return;

    VkDebugUtilsMessengerCreateInfoEXT	createInfo;
    populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
    	throw std::runtime_error("failed to set up debug messenger!");
	}
}

// Create the debug messenger //
std::vector<const char*>	ProgramGestion::getRequiredExtensions()
{
    uint32_t		glfwExtensionCount = 0;
    const char**	glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*>	extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
	{
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// Destroy the debug messenger //
void	ProgramGestion::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
			const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    
	if (func != nullptr)
	{
        func(instance, debugMessenger, pAllocator);
    }
}

// Chose value for the debug messenger //
void	ProgramGestion::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		
    createInfo.pfnUserCallback = debugCallback;
}

//	Load the debug messenger //
VkResult	ProgramGestion::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT
				*pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

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
// Getters //


//******************************************************************************************************//
//												Draw										    		//
//******************************************************************************************************//

// Pipeline only for triangle //
void	ProgramGestion::createGraphicsPipeline(void)
{
	auto vertShaderCode = readFile("srcs/shaders/vert.spv");
    auto fragShaderCode = readFile("srcs/shaders/frag.spv");

	// std::cout << "vertShaderCode.size() = " << vertShaderCode.size() << std::endl;
	// std::cout << "fragShaderCode.size() = " << fragShaderCode.size() << std::endl;
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};								// Create the shader stage //
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// Set the type //
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;								// Set the stage //
	vertShaderStageInfo.module = vertShaderModule;										// Set the module //
	vertShaderStageInfo.pName = "main";													// Set the name //

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};								// Create the shader stage //
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;	// Set the type //
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;							// Set the stage //
	fragShaderStageInfo.module = fragShaderModule;										// Set the module //
	fragShaderStageInfo.pName = "main";													// Set the name //

	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
	
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)	swapChainExtent.width;
	viewport.height = (float)	swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChainExtent;
	
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();
	
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	// rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	// rasterizer.depthBiasClamp = 0.0f; // Optional
	// rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling //
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	// multisampling.minSampleShading = 1.0f; // Optional
	// multisampling.pSampleMask = nullptr; // Optional
	// multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	// multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	// colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	// colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	// colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	// colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	// colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	// colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	// colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	// colorBlending.blendConstants[0] = 0.0f; // Optional
	// colorBlending.blendConstants[1] = 0.0f; // Optional
	// colorBlending.blendConstants[2] = 0.0f; // Optional
	// colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// pipelineLayoutInfo.setLayoutCount = 0; // Optional
	// pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	// pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	// pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Graphic pipeline //

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	// pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	// pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	// pipelineInfo.basePipelineIndex = -1; // Optional

	// Create the pipeline //
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}
	
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

// Shader //
VkShaderModule	ProgramGestion::createShaderModule(const std::vector<char> &code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;		// Type of the structure //
	createInfo.codeSize = code.size();									// Size of the code //
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());	// Code //

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
    std::ifstream	file(filename , std::ios::ate | std::ios::binary);

    if (!file.is_open())
	{
        throw std::runtime_error("failed to open file!");
    }

	// Get the size of the file //
	size_t				fileSize = (size_t) file.tellg();
	std::vector<char>	buffer(fileSize);

	// Read the file //
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

// Multisampling //
void	ProgramGestion::createRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

// Create Framebuffers //
void	ProgramGestion::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

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
void	ProgramGestion::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

// Create command buffer //
void	ProgramGestion::createCommandBuffers()
{
	// Resize the command buffer //
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

// Record command buffer //
void	ProgramGestion::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
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
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

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

		vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	// End the render pass //
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

// Core for draw on screen //
void	ProgramGestion::drawFrame()
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
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame],  0);
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Wait for the image to be available //
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	// Signal that the image is ready to be rendered //
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// Present the image //
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// Specify the swap chain to present the image to //
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

// Create semaphores //
void	ProgramGestion::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	// Create the semaphore info //
    VkSemaphoreCreateInfo	semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Create the fence info //
    VkFenceCreateInfo	fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

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
void	ProgramGestion::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

	// Copy the vertex data to the vertex buffer //
    void* data;
    vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
    vkUnmapMemory(device, vertexBufferMemory);
}

uint32_t	ProgramGestion::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties	memProperties;
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

void	ProgramGestion::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo	bufferInfo{};
	// Size of the buffer //
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	
	// Buffer will be used as a vertex buffer //
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Create the buffer //
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
        throw std::runtime_error("failed to create vertex buffer!");
    }

	// Get the memory requirements for the buffer //
	VkMemoryRequirements	memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	// Allocate the memory for the buffer //
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	// Allocate the memory //
	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}
