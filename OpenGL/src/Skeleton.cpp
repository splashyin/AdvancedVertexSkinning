#include <GL/glew.h>
#include "Skeleton.h"


Skeleton::Skeleton( vec3_map i_skeletonMap )
{
	m_skeletonMap = i_skeletonMap;
	initSkeleton();
	setupSkeleton();
}

//render the Skeleton
void Skeleton::Draw( Shader i_shader )
{
	i_shader.use();
	glBindVertexArray(skeletonVAO);
	//glDrawArrays(GL_POINTS, 0, skeleton.size());
	glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

void Skeleton::initSkeleton()
{
	/*skeleton.resize(skeleton_map.size());
	indices.resize(skeleton_map.size());*/
	for (auto it = m_skeletonMap.cbegin(); it != m_skeletonMap.cend(); it++)
	{
		indices.push_back(it->first);
		skeleton.push_back(it->second);
	}
}

void Skeleton::setupSkeleton()
{
	glGenVertexArrays(1, &skeletonVAO);
	glBindVertexArray(skeletonVAO);

	glGenBuffers(1, &EBO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, skeleton.size() * sizeof(glm::vec3), &skeleton[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}
