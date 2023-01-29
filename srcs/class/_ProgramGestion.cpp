/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/29 14:45:20 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/class/_ProgramGestion.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

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

void	ProgramGestion::run()
{
	initWindow();
	initVulkan();
	mainLoop();
	cleanup();
}

/* ************************************************************************** */
// Private Methods //

//- Find Graphic card -//
// void	ProgramGestion::pickPhysicalDevice()
// {
// 	uint32_t deviceCount = 0;
// 	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
// }

void	ProgramGestion::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void ProgramGestion::initVulkan()
{
	createInstance();
	// setupDebugMessenger();
}

void ProgramGestion::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void ProgramGestion::cleanup()
{	
	vkDestroyInstance(instance, nullptr);
	
	glfwDestroyWindow(window);
	glfwTerminate();
}

//*********************************************************************************//

void	ProgramGestion::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
        throw std::runtime_error("validation layers requested, but not available!");
    }

	
	VkApplicationInfo	appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "DrackenLib";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "DrackEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo	createInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
    	throw std::runtime_error("failed to create instance!");
	}


	// //- See avaliable extensions -//
	// uint32_t extensionCount = 0;
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	
	// std::vector<VkExtensionProperties> extensions(extensionCount);
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	// std::cout << "available extensions:\n";

	// for (const auto& extension : extensions)
	// {
    // std::cout << '\t' << extension.extensionName << std::endl;
	// }
	
	// //- See avaliable layers -//
	// uint32_t layerCount = 0;
	// vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	
	// std::vector<VkLayerProperties> layers(layerCount);
	// vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	// std::cout << "\navailable layers:" << std::endl;
	
	// for (const auto& layer : layers)
	// {
	// std::cout << '\t' << layer.layerName << std::endl;
	// }
}

bool	ProgramGestion::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

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



//****************************************************************************************************//
// No member functions //


