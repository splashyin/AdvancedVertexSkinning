#pragma once

#include "MeshData.inl"
#include "Shader.h"
#include <string>
#include <vector>

//------------------------------------------------------
// GLMesh CLASS
//------------------------------------------------------

class Mesh
{
  public:
    // Ctor
    Mesh()
    {
    }

    // Draw  call - handle rendering the mesh
    void Draw(const Shader& i_shader);

    void SetVertices(const std::vector<Vertex>& i_vertices);

    void SetIndices(const std::vector<unsigned int>& i_vertices);

    void SetTexture(const std::vector<Texture>& i_textures);

    void SetBoneInfo(const std::vector<BoneInfo>& i_vertices);

    void SetVertexBoneData(const std::vector<VertexBoneData>& i_vertices);

    // Initialize all buffer objects and arrays
    void InitializeBuffer();

  private:
    unsigned int m_VAO = 0;
    unsigned int m_EBO = 0;
    unsigned int m_vertexData_vbo = 0;
    unsigned int m_vertexBones_vbo = 0;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;
    std::vector<BoneInfo> m_bones;
    std::vector<VertexBoneData> m_vertexBoneData;
};