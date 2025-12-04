#include "Mesh.h"

using namespace std;

void Mesh::SetVertices(const std::vector<Vertex>& i_vertices)
{
    m_vertices.resize(i_vertices.size());
    m_vertices = i_vertices;
}

void Mesh::SetIndices(const std::vector<unsigned int>& i_indices)
{
    m_indices.resize(i_indices.size());
    m_indices = i_indices;
}

void Mesh::SetTexture(const std::vector<Texture>& i_textures)
{
    m_textures.resize(i_textures.size());
    m_textures = i_textures;
}

void Mesh::SetBoneInfo(const std::vector<BoneInfo>& i_bones)
{
    m_bones.resize(i_bones.size());
    m_bones = i_bones;
}

void Mesh::SetVertexBoneData(const std::vector<VertexBoneData>& i_vertexBoneData)
{
    m_vertexBoneData.resize(i_vertexBoneData.size());
    m_vertexBoneData = i_vertexBoneData;
}

// render the mesh
void Mesh::Draw(const Shader& i_shader)
{
    // Set the vertex buffers and its attribute pointers.
    this->InitializeBuffer();

    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    for (unsigned int i = 0; i < m_textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                                          // retrieve texture number (the N in diffuse_textureN)
        string number;
        string name = m_textures[i].type;
        if (name == "texture_diffuse")
        {
            number = std::to_string(diffuseNr++);
        }
        else if (name == "texture_specular")
        {
            number = std::to_string(specularNr++); // transfer unsigned int to stream
        }
        else if (name == "texture_normal")
        {
            number = std::to_string(normalNr++); // transfer unsigned int to stream
        }
        else if (name == "texture_height")
        {
            number = std::to_string(heightNr++); // transfer unsigned int to stream
        }
        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(i_shader.ID, (name + number).c_str()), i);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::InitializeBuffer()
{
    // create buffers/arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_vertexData_vbo);
    glGenBuffers(1, &m_EBO);
    glGenBuffers(1, &m_vertexBones_vbo);

    glBindVertexArray(m_VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexData_vbo);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to
    // a glm::vec3/2 array which again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0],
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0],
                 GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    // glEnableVertexAttribArray(3);
    // glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( void* )offsetof( Vertex,
    // Tangent ) );
    // vertex bitangent
    // glEnableVertexAttribArray(4);
    // glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( void* )offsetof( Vertex,
    // Bitangent ) );

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBones_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexBoneData) * m_vertexBoneData.size(),
                 &m_vertexBoneData[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneData),
                           (const GLvoid*)0); // Int values only
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
                          (void*)offsetof(VertexBoneData, Weights));
    glBindVertexArray(0);
}