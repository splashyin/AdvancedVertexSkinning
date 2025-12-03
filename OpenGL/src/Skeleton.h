#pragma once

#include "Shader.h"

#include <glm.hpp>
#include <vector>
#include <map>

typedef std::map< unsigned int, glm::vec3 > vec3_map;

class Skeleton 
{
public:
	// Ctor
	Skeleton( const vec3_map& i_skeletonMap );
	
	// Dtor
	~Skeleton() {};

	//render the Skeleton
	void Draw( Shader& i_shader );

	std::vector< unsigned int > indices;
	std::vector< glm::vec3 > skeleton;

private:
	void initSkeleton();

	void setupSkeleton();

	vec3_map m_skeletonMap;

	unsigned int skeletonVAO;
	unsigned int VBO;
	unsigned int EBO;
};