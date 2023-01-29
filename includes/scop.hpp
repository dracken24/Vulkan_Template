/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   scop.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 13:00:56 by dracken24         #+#    #+#             */
/*   Updated: 2023/01/28 19:51:03 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# pragma once

// Vulkan Library //
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h> // GLFWwindow

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "/usr/include/GL/gl.h"

// Standard Library //
#include <iostream>

// My Library //


// Defined Constants //
#define WIDTH 1600
#define HEIGHT 920

// Principal Struct //
typedef struct Scop
{
	Scop(void) {};
	~Scop(void) {};

	GLFWwindow	*window;
} Scop;
