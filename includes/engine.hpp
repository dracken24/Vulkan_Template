/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   engine.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dracken24 <dracken24@student.42.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/30 21:41:29 by dracken24         #+#    #+#             */
/*   Updated: 2023/02/01 21:38:45 by dracken24        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ENGINE_HPP
# define ENGINE_HPP

#include "./class/_ProgramGestion.hpp"

typedef unsigned char   uchar;


//****************************************** Basic data types *****************************************//

typedef struct  Vector2
{
	float	x;
	float	y;
}               Vector2;

typedef struct  Vector3
{
	float	x;
	float	y;
	float	z;
}               Vector3;

typedef struct  Vector4
{
	float	x;
	float	y;
	float	z;
	float	w;
}               Vector4;

typedef struct  Matrix4
{
	float	m[4][4];
}               Matrix4;

typedef struct  Quaternion
{
	float	x;
	float	y;
	float	z;
	float	w;
}               Quaternion;

typedef struct  Transform
{
	Matrix4		transform;      // Local transform matrix
	Matrix4		world;          // World transform matrix
}               Transform;

typedef struct  Transform2D
{
	Vector2		position;       // Local position
	float		rotation;       // Local rotation
	Vector2		scale;          // Local scale
	Vector2		worldPosition;  // World position
	float		worldRotation;  // World rotation
	Vector2		worldScale;     // World scale
}               Transform2D;

typedef struct  Transform3D
{
	Vector3		position;       // Local position
	Quaternion	rotation;       // Local rotation
	Vector3		scale;          // Local scale
	Vector3		worldPosition;  // World position
	Quaternion	worldRotation;  // World rotation
	Vector3		worldScale;     // World scale
}               Transform3D;


//****************************************** Basic 2D shapes ******************************************//

typedef struct  Rectangle
{
	int		x;
	int		y;
	int		width;
	int		height;
}               Rectangle;

typedef struct	triangle
{
	Vector2	v1;
	Vector2	v2;
	Vector2	v3;
}				triangle;

typedef struct  Circle
{
	Vector2	center;
	float	radius;
}               Circle;

typedef struct	ellipsis
{
	Vector2	center;
	float	radius;
	float	radius2;
}				ellipsis;

typedef struct	Line
{
	Vector2	start;
	Vector2	end;
}				Line;


//****************************************** Basic 3D shapes ******************************************//

typedef struct	Cube
{
	Vector3	center;
	float	radius;
}				Cube;

typedef struct	Sphere
{
	Vector3	center;
	float	radius;
}				Sphere;

typedef struct	Cylinder
{
	Vector3	center;
	float	radius;
	float	height;
}				Cylinder;

typedef struct	Cone
{
	Vector3	center;
	float	radius;
	float	height;
}				Cone;

typedef struct	Plane
{
	Vector3	center;
	float	radius;
	float	height;
}				Plane;

typedef struct	Line3
{
	Vector3	start;
	Vector3	end;
}				Line3;


//********************************************** Raycast **********************************************//

typedef struct  Ray
{
	Vector2	position;
	Vector2	direction;
}               Ray;

typedef struct  Ray3
{
	Vector3	position;
	Vector3	direction;
}               Ray3;


//****************************************** Color data type ******************************************//

typedef struct  Color
{
	uint	r;
	uint	g;
	uint	b;
	uint	a;
}               Color;

typedef struct  Image
{
	void	*data;         			// Image raw data
	int		width;          		// Image base width
	int		height;         		// Image base height
	int		mipmaps;        		// Mipmap levels, 1 by default
	int		format;         		// Data format (PixelFormat type)
}               Image;

typedef struct  Texture2D
{
	uint	id;    					// OpenGL texture id
	int		width;          		// Texture base width
	int		height;         		// Texture base height
	int		mipmaps;        		// Mipmap levels, 1 by default
	int		format;         		// Data format (PixelFormat type)
}               Texture2D;

typedef struct  RenderTexture2D
{
	uint		id;            	  	// OpenGL Framebuffer Object (FBO) id
	Texture2D	texture;          	// Color buffer attachment texture
	Texture2D	depth;            	// Depth buffer attachment texture
}               RenderTexture2D;

typedef struct  Texture3D
{
	uint	id;    					// OpenGL texture id
	int		width;          		// Texture base width
	int		height;         		// Texture base height
	int		depth;          		// Texture base depth
	int		mipmaps;       			// Mipmap levels, 1 by default
	int		format;         		// Data format (PixelFormat type)
}               Texture3D;

typedef struct  TextureCubemap
{
	uint	id;    					// OpenGL texture id
	int		size;           		// Texture base size (cubemap width/height)
	int		mipmaps;        		// Mipmap levels, 1 by default
	int		format;         		// Data format (PixelFormat type)
}               TextureCubemap;

typedef struct  NPatchInfo
{
	// Source image rectangle
	Rectangle	sourceRec;
	// Nine-patch layout info
	int			left;       		// left border offset
	int			top;        		// top border offset
	int			right;      		// right border offset
	int			bottom;     		// bottom border offset
	int			type;       		// layout of the 9-patch: 3x3, 1x3 or 3x1
}               NPatchInfo;


//************************************ Vertex data definning types ************************************//

typedef struct  CharInfo
{
	int		value;          	// Character value (Unicode)
	int		offsetX;        	// Character offset X when drawing
	int		offsetY;        	// Character offset Y when drawing
	int		advanceX;       	// Character advance position X
	Image	image;        		// Character image data
}               CharInfo;

typedef struct  _Font
{
	int			baseSize;       // Base size (default chars height)
	int			charsCount;     // Number of characters
	Texture2D	texture;  		// Characters texture atlas
	Rectangle	*recs;    		// Characters rectangles in texture
	CharInfo	*chars;    		// Characters info data
}               _Font;


//***************************** Shader and material properties data types *****************************//

typedef struct  Mesh
{
	int		vertexCount;        // Number of vertices stored in arrays
	int		triangleCount;      // Number of triangles stored (indexed or not)

	// Default vertex data
	float	*vertices;     		// Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
	float	*texcoords;    		// Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
	float	*texcoords2;   		// Vertex second texture coordinates (useful for lightmaps) (shader-location = 5)
	float	*normals;      		// Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
	float	*tangents;     		// Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
	uchar	*colors;  			// Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
	ushort	*indices;			// Vertex indices (in case vertex data comes indexed)

	// Animation vertex data
	float	*animVertices;    	// Animated vertex positions (after bones transformations)
	float	*animNormals;     	// Animated normals (after bones transformations)
	int		*boneIds;           // Vertex bone ids, up to 4 bones influence by vertex (skinning)
	float	*boneWeights;     	// Vertex bone weight, up to 4 bones influence by vertex (skinning)

	// OpenGL identifiers
	uint	vaoId;     			// OpenGL Vertex Array Object id
	uint	*vboId;    			// OpenGL Vertex Buffer Objects id (default vertex data)
}               Mesh;

typedef struct  Shader
{
	uint	id;        			// Shader program id
	int		*locs;              // Shader locations array (MAX_SHADER_LOCATIONS)
}               Shader;

typedef struct  MaterialMap
{
	Texture2D texture;      	// Material map texture
	Color	color;            	// Material map color
	float	value;            	// Material map value
}               MaterialMap;

typedef struct  Material
{
	Shader		shader;         // Material shader
	MaterialMap	*maps;      	// Material maps array (MAX_MATERIAL_MAPS)
	float		*params;       	// Material generic parameters (if required)
}               Material;

typedef struct  BoneInfo
{
	char	name[32];          	// Bone name
	int		parent;             // Bone parent
}               BoneInfo;


//***************************************** Camera data types *****************************************//

typedef struct  Camera2D
{
	Vector2	offset;     	// Camera offset (displacement from target)
	Vector2	target;     	// Camera target (rotation and zoom origin)
	float	rotation;     	// Camera rotation in degrees
	float	zoom;         	// Camera zoom (scaling), should be 1.0f by default
}               Camera2D;

typedef struct  Camera3D
{
	Vector3	position;   	// Camera position
	Vector3	target;     	// Camera target it looks-at
	Vector3	up;         	// Camera up vector (rotation over its axis)
	float	fovy;         	// Camera field-of-view apperture in Y (degrees) in perspective, used as near plane width in orthographic
	int		type;           // Camera type, defines projection type: CAMERA_PERSPECTIVE or CAMERA_ORTHOGRAPHIC
}               Camera3D;


//******************************************************************************************************//
//												Functions									    		//
//******************************************************************************************************//

//***************************** Open, Read, write and Close file functions *****************************//

static std::vector<char>	readFile(const std::string& filename);


//***************************************** Pipeline functions *****************************************//

VkShaderModule	createShaderModule(ProgramGestion *engine, const std::vector<char> &code);
void			createGraphicsPipeline(ProgramGestion *engine);

#endif
