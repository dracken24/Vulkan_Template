/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/30 21:01:27 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/class/_ProgramGestion.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

/**********************************************************************************************/

// Constructor - Destructor //

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

/*********************************************************************************/
// Public Methods //

void	ProgramGestion::run(std::string name, bool resizeable)
{
	initWindow(name, resizeable);
	initVulkan();
	mainLoop();
	cleanup();
}

/* ************************************************************************** */
// Private Methods: Global Gestion //

void	ProgramGestion::initWindow(std::string name, bool resizeable)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, resizeable);

	window = glfwCreateWindow(WIDTH, HEIGHT, name.c_str(), nullptr, nullptr);
}

void	ProgramGestion::initVulkan()
{
	createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
	createSwapChain();
}

void	ProgramGestion::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void	ProgramGestion::cleanup()
{
	vkDestroySwapchainKHR(device, swapChain, nullptr);
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

/****************************************************************************************/
// Gestion GPU //

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

void	ProgramGestion::createLogicalDevice()
{
	QueueFamilyIndices	indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo>		queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =	{indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	// Create queue create infos //
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo	queueCreateInfo{};
		
		queueCreateInfo.sType =				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex =	queueFamily;
		queueCreateInfo.queueCount =		1;
		queueCreateInfo.pQueuePriorities =	&queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
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

/****************************************************************************************/
// Queue family //


ProgramGestion::QueueFamilyIndices	ProgramGestion::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
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
	createInfo.sType =				VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface =			surface;
	createInfo.minImageCount =		imageCount;
	createInfo.imageFormat =		surfaceFormat.format;
	createInfo.imageColorSpace =	surfaceFormat.colorSpace;
	createInfo.imageExtent =		extent;
	createInfo.imageArrayLayers =	1;
	createInfo.imageUsage =			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices	indices = findQueueFamilies(physicalDevice);
	uint32_t			queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	
	// If the graphics and present family are different, we need to use concurrent mode //
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// If the swap chain is transformed, we need to specify how it should be transformed //
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// Get the swap chain images //
	std::vector<VkImage>	swapChainImages;
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	// Create the swap chain //
	VkSwapchainKHR			swapChain;
	std::vector<VkImage> 	swapChainImages;
	VkFormat				swapChainImageFormat;
	VkExtent2D				swapChainExtent;

	swapChainImageFormat =	surfaceFormat.format;
	swapChainExtent =		extent;
}

//*********************************************************************************//
// Create surface //
void	ProgramGestion::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

//*********************************************************************************//
// Create instance and debug messenger //

void	ProgramGestion::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// Create the application info //
	VkApplicationInfo appInfo =		{};
	appInfo.sType =					VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName =		"DrackenLib";
	appInfo.applicationVersion =	VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName =			"DrackEngine";
	appInfo.engineVersion =			VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion =			VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo =	{};
	createInfo.sType =					VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo =		&appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount =		static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames =	extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	// If validation layers are enabled, add them to the instance //
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount =		static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames =	validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
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

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

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

//****************************************************************************************************//
// No member functions //

