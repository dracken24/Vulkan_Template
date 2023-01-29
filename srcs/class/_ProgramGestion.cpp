/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/29 00:12:28 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../includes/class/_ProgramGestion.hpp"

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

VkResult	ProgramGestion::CreateDebugUtilsMessengerEXT(VkInstance instance,
				const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
				const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void	ProgramGestion::DestroyDebugUtilsMessengerEXT(VkInstance instance,
			VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	
	if (func != nullptr)
	{
		func(instance, callback, pAllocator);
	}
}

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
	setupDebugCallback();
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
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(*instance, callback, nullptr);
	}

	// NOTE: instance destruction is handled by UniqueInstance

	glfwDestroyWindow(window);

	glfwTerminate();
}

void	ProgramGestion::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	auto appInfo = vk::ApplicationInfo(
		"DrackenLib",
		VK_MAKE_VERSION(1, 0, 0),
		"DrackEngine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	);
	
	auto extensions = getRequiredExtensions();

	auto createInfo = vk::InstanceCreateInfo( vk::InstanceCreateFlags(),
		&appInfo, 0, nullptr,	static_cast<uint32_t>(extensions.size()), extensions.data());

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	try
	{
		instance = vk::createInstanceUnique(createInfo, nullptr);
	}
	catch (vk::SystemError err)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

void	ProgramGestion::setupDebugCallback()
{
	if (!enableValidationLayers)
		return;

	auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT(vk::DebugUtilsMessengerCreateFlagsEXT(),
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
		| vk::DebugUtilsMessageSeverityFlagBitsEXT::eError, vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
		| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
		nullptr);

	if (CreateDebugUtilsMessengerEXT(*instance, reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>
		(&createInfo), nullptr, &callback) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug callback!");
	}
}

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

bool	ProgramGestion::checkValidationLayerSupport()
{
	auto availableLayers = vk::enumerateInstanceLayerProperties();
	
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL	debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											VkDebugUtilsMessageTypeFlagsEXT messageType,
											const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
											void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
