/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/28 22:14:08 by dracken24         #+#    #+#             */
/*   Updated: 2023/02/02 00:46:04 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/class/_ProgramGestion.hpp"

#define STB_IMAGE_IMPLEMENTATION

int		main(void)
{
    ProgramGestion app;

    try 
	{
        app.run("Vulkan", true);
    }
	catch (const std::exception& e)
	{
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
