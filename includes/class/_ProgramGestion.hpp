/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 21:53:06 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/29 19:17:51 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>
#include <map>

#define WIDTH 1600
#define HEIGHT 920

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
	std::optional<uint32_t>	graphicsFamily;
	
	bool isComplete()
	{
        return graphicsFamily.has_value();
    }
};

class ProgramGestion
{
	/****************************************************************************************/
	
	// Constructor - Destructor //
	public:
		ProgramGestion();
		ProgramGestion(const ProgramGestion &src);
		~ProgramGestion();
		
		ProgramGestion	&operator=(const ProgramGestion &src);
	
	/****************************************************************************************/

	// Public Methods //
	public:
		void	run(std::string name, bool resizeable);

	/****************************************************************************************/

	// Private Methods //
	private:
		void	initVulkan();
		void	mainLoop();
		void	cleanup();
		void	initWindow(std::string name, bool resizeable);

	/****************************************************************************************/
	// GPU //
		void	pickPhysicalDevice(); //- Find Graphic card -//
		bool	isDeviceSuitable(VkPhysicalDevice device);

	/****************************************************************************************/
	// Queue //
		QueueFamilyIndices	findQueueFamilies(VkPhysicalDevice device);

	/****************************************************************************************/
		void	createInstance();
		bool	checkValidationLayerSupport();
		void	setupDebugMessenger();

		std::vector<const char*>	getRequiredExtensions();

		void	DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
					const VkAllocationCallbacks* pAllocator);
		
		VkResult	CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT
						*pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		
		void	populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		
	/****************************************************************************************/
					
	// Private Attributes //
	private:
		GLFWwindow	*window;
		VkInstance	instance;
		
		VkDebugUtilsMessengerEXT	debugMessenger;
		VkPhysicalDevice			physicalDevice = VK_NULL_HANDLE; //- Stock graphic card -//	
};


