/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   _ProgramGestion.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 21:53:06 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/28 22:49:31 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

#define WIDTH 1600
#define HEIGHT 920


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
		void	initWindows();

		void	createInstance();

	// Private Attributes //
	private:
		GLFWwindow	*window;
		VkInstance	instance;
};
