/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 21:53:06 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/31 14:45:05 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#if _WIN32
	#define GLFW_EXPOSE_NATIVE_WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#endif
#if __APPLE__
	#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#if __linux__
	#define GLFW_EXPOSE_NATIVE_X11
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3native.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <set>
#include <map>
#include <fstream>



#define WIDTH 1500
#define HEIGHT 920

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


class ProgramGestion
{
	/****************************************************************************************/
	
	// Queue family indices //
	struct QueueFamilyIndices
	{
		std::optional<uint32_t>		graphicsFamily;
		std::optional<uint32_t>		presentFamily;
		
		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	// Swap chain support details //
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	// Constructor - Destructor //
	public:
		ProgramGestion();
		ProgramGestion(const ProgramGestion &src);
		~ProgramGestion();
		
		ProgramGestion	&operator=(const ProgramGestion &src);
	
	/****************************************************************************************/

	// Public Methods //
	public:
		void		run(std::string name, bool resizeable);

	// Setters//

	// Getters//
		// VkDevice	getDevice() const;
		// VkExtent2D	getSwapChainExtent() const;
		// VkPipelineLayout	*getPipelineLeyout();	

	/****************************************************************************************/

	// Private Methods //
	private:
		void	initWindow(std::string name, bool resizeable);
		void	initVulkan();
		void	mainLoop();
		void	cleanup();

	/****************************************************************************************/
	// GPU //
		void	createLogicalDevice();
		void	pickPhysicalDevice(); //- Find Graphic card -//
		
		bool	checkDeviceExtensionSupport(VkPhysicalDevice device);
		bool	isDeviceSuitable(VkPhysicalDevice device);

	/****************************************************************************************/
	// Queue //
		VkPresentModeKHR			chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkSurfaceFormatKHR 			chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkExtent2D					chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		SwapChainSupportDetails		querySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices			findQueueFamilies(VkPhysicalDevice device);
		
		void	createSwapChain();
		void	createImageViews();

	/****************************************************************************************/
	// Surface //
		void	createSurface();

	/****************************************************************************************/
	// Instance creation and debug messages //
		void	createInstance();
		bool	checkValidationLayerSupport();
		void	setupDebugMessenger();

		std::vector<const char*>	getRequiredExtensions();

		void		DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
						const VkAllocationCallbacks* pAllocator);
		
		VkResult	CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT
						*pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		
		void		populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	
	/****************************************************************************************/
	// Pipeline functions //

		VkShaderModule	createShaderModule(const std::vector<char> &code);
		void			createGraphicsPipeline();
		void			createRenderPass();
		void			createFramebuffers();

	/****************************************************************************************/
	// Command pool and buffer //
		void	recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void	createCommandBuffers();
		void	createCommandPool();

		void	createSyncObjects();
		void	drawFrame();
					
	// Private Attributes //
	private:
		GLFWwindow	*window;	//- Stock window -//
		VkInstance	instance;	//- Stock instance -//
		
		VkDebugUtilsMessengerEXT	debugMessenger;	//- Debug messenger -//
		
		VkPhysicalDevice			physicalDevice = VK_NULL_HANDLE; //- Stock graphic card -//	
		VkSurfaceKHR				surface; 				//- Stock surface -//
		VkDevice					device; 				//- Stock logical device -//
		
		VkQueue						graphicsQueue;			//- Stock queue -//
		VkQueue						presentQueue;			//- Stock queue -//

		std::vector<VkImageView>	swapChainImageViews;	//- Stock swap chain image views -//
		std::vector<VkImage> 		swapChainImages;		//- Stock swap chain images -//
		VkSwapchainKHR				swapChain;				//- Stock swap chain -//
		VkExtent2D					swapChainExtent; 		//- Stock swap chain extent -//
		VkFormat					swapChainImageFormat;	//- Stock swap chain image format -//
		
		VkRenderPass 				renderPass;				//- Stock render pass -//
		VkPipelineLayout			pipelineLayout;			//- Stock pipeline layout -//
		VkPipeline					graphicsPipeline;		//- Stock graphics pipeline -//
		std::vector<VkFramebuffer>	swapChainFramebuffers;	//- Stock swap chain framebuffers -//

		// Command buffers //
		VkCommandPool				commandPool;			//- Stock command pool -//
		VkCommandBuffer				commandBuffer;			//- Stock command buffer -//

		// Stocking semaphores //
		VkSemaphore					imageAvailableSemaphore;	//- Stock image available semaphore -//
		VkSemaphore					renderFinishedSemaphore;	//- Stock render finished semaphore -//
		VkFence						inFlightFence;				//- Stock in flight fence -//
};
