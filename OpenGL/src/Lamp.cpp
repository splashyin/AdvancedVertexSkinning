#include <GL/glew.h>

#include "Lamp.h"

const float vertices[] = {
	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,

	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,

	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,

	-0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f, -0.5f,
	 0.5f, -0.5f,  0.5f,
	 0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,

	-0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f, -0.5f,
	 0.5f,  0.5f,  0.5f,
	 0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
};


Lamp::Lamp( const glm::vec3& i_position, const glm::vec3& i_color )
	: m_position( i_position ), m_color( i_color )
{
	setupLight();
}

glm::vec3 Lamp::getPosition()
{
	return m_position;
}

glm::vec3 Lamp::getColor()
{
	return m_color;
}

void Lamp::setupLight()
{
	glGenVertexArrays( 1, &m_lightVAO );
	glBindVertexArray( m_lightVAO );
	glGenBuffers( 1, &m_lightVBO );
	glBindBuffer( GL_ARRAY_BUFFER, m_lightVBO);
	glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), ( void* )0 );
	glEnableVertexAttribArray( 0 );
}


void Lamp::Draw( Shader* i_shader )
{
	i_shader->use();
	glBindVertexArray( m_lightVAO );
	glDrawArrays( GL_TRIANGLES, 0, 36 );
	glBindVertexArray( 0 );
}
