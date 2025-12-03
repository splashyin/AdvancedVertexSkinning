#pragma once

#include <GL/glew.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>
#include <gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_CTOR_INIT
#include "Mesh.h"
#include "Shader.h"
#include <gtx/dual_quaternion.hpp>
#include <gtx/quaternion.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

unsigned int TextureFromFile(const char *path, const std::string &directory);
glm::mat3x4 convertMatrix(glm::mat4 s);
glm::quat quatcast(glm::mat4 t);

typedef std::map<std::string, const aiNodeAnim *> nodeAnimationMap;
typedef std::map<std::string, nodeAnimationMap> AnimationMap;

class Model {
public:
  // Ctor
  Model() = delete;
  Model(const std::string &i_path);

  // Copy Ctor
  Model(const Model &i_model) = delete;

  // Move Ctor
  Model(Model &&i_model) = delete;

  // Dtor
  ~Model();

  /*  Model Data */
  std::vector<Texture>
      textures_loaded; // stores all the textures loaded so far, optimization to
                       // make sure textures aren't loaded more than once.

  /*Bone Data*/
  unsigned int m_NumBones = 0;
  std::map<std::string, unsigned int> Bone_Mapping;
  AnimationMap Animations;
  std::map<unsigned int, glm::vec3> skeleton_pose;
  std::map<std::string, unsigned int> Node_Mapping;
  std::vector<BoneInfo> m_BoneInfo;

  glm::fdualquat IdentityDQ = glm::fdualquat(glm::quat(1.f, 0.f, 0.f, 0.f),
                                             glm::quat(0.f, 0.f, 0.f, 0.f));

  // Draws the model, and thus all its meshes
  void Draw(const Shader &i_shader);

  void BoneTransform(const float &i_timeInSeconds,
                     std::vector<glm::mat4> &i_transforms,
                     std::vector<glm::fdualquat> &io_dqs);

private:
  // Model has ownership over the loaded scene
  // The application is now responsible for deleting the scene
  // The scene data is now heap allocated, so it requires application uses the
  // same heap as Assimp
  aiScene *m_scene;

  // Directory of the scene file
  std::string m_directory;

  // A number of meshes of the model
  std::vector<Mesh> m_meshes;

  // loads a model with supported ASSIMP extensions from file and stores the
  // resulting meshes in the meshes vector.
  void loadModel(const std::string &i_path);

  void loadBones(aiNode *node);

  // processes a node in a recursive fashion. Processes each individual mesh
  // located at the node and repeats this process on its children nodes (if
  // any).
  void processNode(aiNode *node);

  // process a mesh object (does a copy)
  void processMesh(aiMesh *mesh);

  // checks all material textures of a given type and loads the textures if
  // they're not loaded yet. the required info is returned as a Texture struct.
  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            const std::string &typeName);

  void loadMeshBones(aiMesh *mesh, std::vector<VertexBoneData> &vertexBoneData);

  // get animation from the bone
  // populate the animation map : animation_map[animation_name][bone_name] ->
  // animation
  void loadAnimations(const std::string &BoneName, AnimationMap &o_animations);

  void ReadNodeHeirarchy(float AnimationTime, const aiNode *pNode,
                         const glm::mat4 &ParentTransform,
                         const glm::fdualquat &ParentDQ, glm::vec3 startpos);

  void CalcInterpolatedScaling(aiVector3D &Out, float AnimationTime,
                               const aiNodeAnim *pNodeAnim);

  void CalcInterpolatedRotaion(aiQuaternion &Out, float AnimationTime,
                               const aiNodeAnim *pNodeAnim);

  void CalcInterpolatedPosition(aiVector3D &Out, float AnimationTime,
                                const aiNodeAnim *pNodeAnim);

  unsigned int FindScaling(float AnimationTime, const aiNodeAnim *pNodeAnim);

  unsigned int FindRotation(float AnimationTime, const aiNodeAnim *pNodeAnim);

  unsigned int FindPosition(float AnimationTime, const aiNodeAnim *pNodeAnim);
};