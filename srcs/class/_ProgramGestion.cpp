/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:20:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/28 22:54:52 by dracken24        ###   ########.fr       */
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
	initWindows();
	initVulkan();
	mainLoop();
	cleanup();
}

/* ************************************************************************** */
// Private Methods //

void	ProgramGestion::cleanup()
{
	vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void	ProgramGestion::initWindows()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void	ProgramGestion::initVulkan()
{
	createInstance();
}

void	ProgramGestion::mainLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}

void ProgramGestion::createInstance()
{
    VkApplicationInfo appInfo{};
	
    appInfo.sType = 				VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName =		"DrackenApp";
    appInfo.applicationVersion =	VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName =			"DrackEngine";
    appInfo.engineVersion =			VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion =			VK_API_VERSION_1_0;

	VkInstanceCreateInfo			createInfo{};
	uint32_t						glfwExtensionCount = 0;
	const char						**glfwExtensions;
	
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;
	
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
    	throw std::runtime_error("failed to create instance!");
	}

	// Check for extensions //
	
	// uint32_t extensionCount = 0;
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	// std::vector<VkExtensionProperties> extensions(extensionCount);
	// vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	
	// std::cout << "available extensions:\n";

	// for (const auto& extension : extensions)
	// {
	// 	std::cout << '\t' << extension.extensionName << '\n';
	// }
}
