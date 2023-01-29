/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 21:53:06 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/29 00:10:06 by dracken24        ###   ########.fr       */
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


class ProgramGestion
{
	// Constructor - Destructor //
	public:
		ProgramGestion();
		ProgramGestion(const ProgramGestion &src);
		~ProgramGestion();
		
		ProgramGestion	&operator=(const ProgramGestion &src);
	
	// Public Methods //
	public:
		void	run();

	// Private Methods //
	private:
		void	initVulkan();
		void	mainLoop();
		void	cleanup();
		void	initWindow();

		void	createInstance();
		bool	checkValidationLayerSupport();
		void	setupDebugCallback();
		std::vector<const char*>	getRequiredExtensions();
		
		VkResult	CreateDebugUtilsMessengerEXT(VkInstance instance,
						const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
						const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback);

		void		DestroyDebugUtilsMessengerEXT(VkInstance instance,
						VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
						
	// Private Attributes //
	private:
		GLFWwindow			*window;
		vk::UniqueInstance	instance;
		
		VkDebugUtilsMessengerEXT	callback;		
};
