#include "Model.h"
#include "Log.h"

#include <assimp/Importer.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace
{
	glm::fdualquat MakeDualQuat(const glm::fquat& rotation, const glm::vec3& translation)
	{
		glm::fdualquat dq;
		dq.real = rotation;
		glm::fquat translationQuat(0.0f, translation.x, translation.y, translation.z);
		dq.dual = 0.5f * translationQuat * rotation;
		return dq;
	}
}

//----------------------------------------------------------------

Model::Model( const std::string& i_path )
{
	// Retrieve the directory path of the filepath
	m_directory = i_path.substr( 0, i_path.find_last_of( '/' ) );

	// Load model data
	loadModel( i_path );

	std::cout << "[Model] Bones detected: " << m_NumBones << std::endl;
}

//----------------------------------------------------------------

Model::~Model()
{
	// Free the heap allocated scene
	delete m_scene;
}

//----------------------------------------------------------------

void Model::Draw( const Shader& i_shader )
{
	for ( Mesh& mesh : m_meshes )
	{
		mesh.Draw( i_shader );
	}
}

//----------------------------------------------------------------

void Model::BoneTransform( const float& i_timeInSeconds, std::vector<glm::mat4>& io_transforms, std::vector<glm::fdualquat>& io_dqs ) 
{
	if ( !m_scene->HasAnimations() )
	{
		return;
	}

	glm::mat4 Identity = glm::mat4( 1.0f );
	unsigned int numPosKeys = m_scene->mAnimations[0]->mChannels[0]->mNumPositionKeys;

	float TicksPerSecond = m_scene->mAnimations[0]->mTicksPerSecond != 0 ?
		m_scene->mAnimations[0]->mTicksPerSecond : 25.0f;

	float TimeInTicks = i_timeInSeconds * TicksPerSecond;
	float AnimationTime = fmod( TimeInTicks, m_scene->mAnimations[0]->mChannels[0]->mPositionKeys[numPosKeys - 1].mTime );

	ReadNodeHeirarchy( AnimationTime, m_scene->mRootNode, Identity, IdentityDQ, glm::vec3( 0.0f, 0.0f, 0.0f ) );

	io_transforms.resize( m_NumBones );
	io_dqs.resize( m_NumBones );

	for ( unsigned int i = 0; i < m_NumBones; ++i )
	{
		io_transforms[i] = glm::mat4( 1.0f );
		io_transforms[i] = m_BoneInfo[i].FinalTransformation;
	}

	for ( unsigned int i = 0; i < io_dqs.size(); ++i )
	{
		io_dqs[i] = IdentityDQ;
		io_dqs[i] = m_BoneInfo[i].FinalTransDQ;

		#ifdef DEBUG_PRINT()
			LOG_DUALQUAT( io_dqs[i] );
		#endif
	}
}

//----------------------------------------------------------------

void Model::loadModel( const std::string& i_path )
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile( i_path, 
											  aiProcess_Triangulate | 
											  aiProcess_FlipUVs | 
											  aiProcess_CalcTangentSpace );
	// Error checking
	if ( scene == nullptr 
		|| scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
		|| scene->mRootNode == nullptr )
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	// Takes ownership of the loaded scene (heap allocated)
	m_scene = importer.GetOrphanedScene();

	// Load bone data from Assimp
	loadBones( m_scene->mRootNode );

	// Load mesh from nodes recursively
	processNode( m_scene->mRootNode );
}

//----------------------------------------------------------------

void Model::loadBones( aiNode* i_node )
{
	for ( unsigned int i = 0; i < i_node->mNumChildren; ++i )
	{
		aiNode* child = i_node->mChildren[i];
		std::string nodeName = child->mName.data;

		unsigned int boneIndex = 0;
		if (Bone_Mapping.find( nodeName ) == Bone_Mapping.end())
		{
			boneIndex = m_NumBones;
			m_NumBones++;
			Bone_Mapping[nodeName] = boneIndex;
		}

		loadBones( child );
	}
}

//----------------------------------------------------------------

void Model::processNode( aiNode* node )
{
	m_BoneInfo.resize( Bone_Mapping.size() );
	// process each mesh located at the current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* aiMesh_ptr = m_scene->mMeshes[node->mMeshes[i]];
		processMesh( aiMesh_ptr );
	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode( node->mChildren[i] );
	}
}

//----------------------------------------------------------------

void Model::processMesh( aiMesh* aiMesh )
{
	Mesh mesh;

	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < aiMesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;

		//retreive positions
		vector.x = aiMesh->mVertices[i].x;
		vector.y = aiMesh->mVertices[i].y;
		vector.z = aiMesh->mVertices[i].z;
		vertex.Position = vector;

		//retreive normals
		vector.x = aiMesh->mNormals[i].x;
		vector.y = aiMesh->mNormals[i].y;
		vector.z = aiMesh->mNormals[i].z;
		vertex.Normal = vector;

		//retreive texture coordinates
		if ( aiMesh->mTextureCoords[0] )
		{
			glm::vec2 vec;
			vec.x = aiMesh->mTextureCoords[0][i].x;
			vec.y = aiMesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
		{
			vertex.TexCoords = glm::vec2( 0.0f, 0.0f );
		}
		vertices.push_back( vertex );
	}

	//retreive indices
	for (unsigned int i = 0; i < aiMesh->mNumFaces; i++)
	{
		aiFace face = aiMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back( face.mIndices[j] );
		}
	}

	// process materials
	aiMaterial* material = m_scene->mMaterials[aiMesh->mMaterialIndex];
	// 1. diffuse maps
	std::vector<Texture> diffuseMaps = loadMaterialTextures( material, aiTextureType_DIFFUSE, "texture_diffuse" );
	textures.insert( textures.end(), diffuseMaps.begin(), diffuseMaps.end() );
	// 2. specular maps
	std::vector<Texture> specularMaps = loadMaterialTextures( material, aiTextureType_SPECULAR, "texture_specular" );
	textures.insert( textures.end(), specularMaps.begin(), specularMaps.end() );
	// 3. normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures( material, aiTextureType_HEIGHT, "texture_normal" );
	textures.insert( textures.end(), normalMaps.begin(), normalMaps.end() );
	// 4. height maps
	std::vector<Texture> heightMaps = loadMaterialTextures( material, aiTextureType_AMBIENT, "texture_height" );
	textures.insert( textures.end(), heightMaps.begin(), heightMaps.end() );
	//retreive bone information
	std::vector<VertexBoneData> bones(aiMesh->mNumVertices);
	loadMeshBones( aiMesh, bones );
	
	mesh.SetVertices( vertices );
	mesh.SetIndices( indices );
	mesh.SetTexture( textures );
	mesh.SetBoneInfo( m_BoneInfo );
	mesh.SetVertexBoneData( bones );
	m_meshes.push_back( mesh );
}

//----------------------------------------------------------------

std::vector<Texture> Model::loadMaterialTextures( aiMaterial* mat, aiTextureType type, const std::string& typeName )
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount( type ); i++)
	{
		aiString str;
		mat->GetTexture( type, i, &str );
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp( textures_loaded[j].path.data(), str.C_Str() ) == 0)
			{
				textures.push_back( textures_loaded[j] );
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.id = TextureFromFile( str.C_Str(), m_directory );
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back( texture );
			textures_loaded.push_back( texture );  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

//----------------------------------------------------------------

void Model::loadMeshBones( aiMesh* i_aiMesh, std::vector<VertexBoneData>& vertexBoneData )
{
	for (unsigned int i = 0; i < i_aiMesh->mNumBones; i++)
	{
		unsigned int BoneIndex = 0;
		std::string BoneName( i_aiMesh->mBones[i]->mName.data );

		if (Bone_Mapping.find( BoneName ) != Bone_Mapping.end())
		{
			BoneIndex = Bone_Mapping[BoneName];
			//BoneInfo bi;
			//m_BoneInfo.push_back(bi);

			aiMatrix4x4 tp1 = i_aiMesh->mBones[i]->mOffsetMatrix;
			m_BoneInfo[BoneIndex].offset = glm::transpose( glm::make_mat4( &tp1.a1 ) );
			glm::fquat offsetRot = glm::normalize( glm::quat_cast( m_BoneInfo[BoneIndex].offset ) );
			glm::vec3 offsetTrans( m_BoneInfo[BoneIndex].offset[3][0], m_BoneInfo[BoneIndex].offset[3][1], m_BoneInfo[BoneIndex].offset[3][2] );
			m_BoneInfo[BoneIndex].offsetDQ = glm::normalize( MakeDualQuat( offsetRot, offsetTrans ) );
		}

		for (unsigned int n = 0; n < i_aiMesh->mBones[i]->mNumWeights; n++)
		{
			unsigned int vid = i_aiMesh->mBones[i]->mWeights[n].mVertexId;
			float weight = i_aiMesh->mBones[i]->mWeights[n].mWeight;
			vertexBoneData[vid].AddBoneData( BoneIndex, weight );
		}
		loadAnimations( BoneName, Animations );
	}

	// Normalize bone weights per vertex to ensure proper blending (especially for DQS).
	for (VertexBoneData& vertex : vertexBoneData)
	{
		float totalWeight = 0.0f;
		for (unsigned int j = 0; j < NUM_BONES_PER_VERTEX; ++j)
		{
			totalWeight += vertex.Weights[j];
		}

		if (totalWeight > 0.0f)
		{
			for (unsigned int j = 0; j < NUM_BONES_PER_VERTEX; ++j)
			{
				vertex.Weights[j] /= totalWeight;
			}
		}
	}
}

//----------------------------------------------------------------

void Model::loadAnimations( const std::string& BoneName, AnimationMap& o_animations )
{
	for (unsigned int i = 0; i < m_scene->mNumAnimations; ++i )
	{
		const aiAnimation* pAnimation = m_scene->mAnimations[i];
		std::string animName = pAnimation->mName.data;

		for ( unsigned int j = 0; j < pAnimation->mNumChannels; ++j )
		{
			if ( pAnimation->mChannels[j]->mNodeName.data == BoneName )
			{
				o_animations[animName][BoneName] = pAnimation->mChannels[j];
				break;
			}
		}
	}
}

//----------------------------------------------------------------

void Model::ReadNodeHeirarchy( float AnimationTime, const aiNode* pNode,
	const glm::mat4& ParentTransform, const glm::fdualquat& ParentDQ, glm::vec3 startpos )
{
	std::string NodeName( pNode->mName.data );
	const aiAnimation* pAnimation = m_scene->mAnimations[0];
	glm::mat4 NodeTransformation = glm::mat4( 1.0f );
	aiMatrix4x4 tp1 = pNode->mTransformation;
	NodeTransformation = glm::transpose( glm::make_mat4( &tp1.a1 ) );

	const aiNodeAnim* pNodeAnim = nullptr;
	pNodeAnim = Animations[pAnimation->mName.data][NodeName];

	if (pNodeAnim) {

		//Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotaion( RotationQ, AnimationTime, pNodeAnim );


		aiMatrix3x3 tp = RotationQ.GetMatrix();
		glm::mat4 RotationM = glm::transpose( glm::make_mat3( &tp.a1 ) );

		//Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition( Translation, AnimationTime, pNodeAnim );
		glm::mat4 TranslationM = glm::mat4( 1.0f );
		TranslationM = glm::translate( TranslationM, glm::vec3( Translation.x, Translation.y, Translation.z ) );

		NodeTransformation = TranslationM * RotationM;
	}

	glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;
	glm::fquat nodeRotation = glm::normalize( glm::quat_cast( NodeTransformation ) );
	glm::vec3 nodeTranslation( NodeTransformation[3][0], NodeTransformation[3][1], NodeTransformation[3][2] );
	glm::fdualquat NodeDQ = glm::normalize( MakeDualQuat( nodeRotation, nodeTranslation ) );
	glm::fdualquat GlobalDQ = glm::normalize( ParentDQ * NodeDQ );

	unsigned int ID = 0;


	if (Bone_Mapping.find( NodeName ) != Bone_Mapping.end()) {
		startpos.x = GlobalTransformation[3][0];
		startpos.y = GlobalTransformation[3][1];
		startpos.z = GlobalTransformation[3][2];
		ID = Bone_Mapping[NodeName];
		skeleton_pose[ID] = startpos;
	}

	if (Bone_Mapping.find( NodeName ) != Bone_Mapping.end()) {
		unsigned int NodeIndex = Bone_Mapping[NodeName];
		m_BoneInfo[NodeIndex].FinalTransformation = GlobalTransformation * m_BoneInfo[NodeIndex].offset;

		glm::fdualquat finalDQ = glm::normalize( GlobalDQ * m_BoneInfo[NodeIndex].offsetDQ );
		m_BoneInfo[NodeIndex].FinalTransDQ = finalDQ;
	}
	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy( AnimationTime, pNode->mChildren[i], GlobalTransformation, GlobalDQ, startpos );
	}
}

//----------------------------------------------------------------

void Model::CalcInterpolatedScaling( aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	if (pNodeAnim->mNumScalingKeys == 1) 
	{
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = FindScaling( AnimationTime, pNodeAnim );
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert( NextScalingIndex < pNodeAnim->mNumScalingKeys );
	float DeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert( Factor >= 0.0f && Factor <= 1.0f );
	const aiVector3D& Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

//----------------------------------------------------------------

void Model::CalcInterpolatedRotaion( aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = FindRotation( AnimationTime, pNodeAnim );
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert( NextRotationIndex < pNodeAnim->mNumRotationKeys );
	float DeltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert( Factor >= 0.0f && Factor <= 1.0f );
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate( Out, StartRotationQ, EndRotationQ, Factor );
	Out = Out.Normalize();//normalized
}

//----------------------------------------------------------------

void Model::CalcInterpolatedPosition( aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = FindPosition( AnimationTime, pNodeAnim );
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert( NextPositionIndex < pNodeAnim->mNumPositionKeys );
	float DeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert( Factor >= 0.0f && Factor <= 1.0f );
	const aiVector3D& Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	Out = Start + Factor * Delta;
}

//----------------------------------------------------------------

unsigned int Model::FindScaling( float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	assert( pNodeAnim->mNumScalingKeys > 0 );

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) 
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) 
		{
			return i;
		}
	}
	assert( 0 );
	return 0;
}

//----------------------------------------------------------------

unsigned int Model::FindRotation( float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	assert( pNodeAnim->mNumRotationKeys > 0 );

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) 
	{
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
		{
			return i;
		}
	}

	assert( 0 );
	return 0;
}

//----------------------------------------------------------------

unsigned int Model::FindPosition( float AnimationTime, const aiNodeAnim* pNodeAnim )
{
	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) 
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
		{
			return i;
		}
	}
	assert( 0 );
	return 0;
}

//----------------------------------------------------------------
// HELPER FUNCTIONS
//----------------------------------------------------------------

unsigned int TextureFromFile( const char* path, const std::string& directory )
{
	std::string filename = std::string( path );
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures( 1, &textureID );

	int width, height, nrComponents;
	unsigned char* data = stbi_load( filename.c_str(), &width, &height, &nrComponents, 0 );
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture( GL_TEXTURE_2D, textureID );
		glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
		glGenerateMipmap( GL_TEXTURE_2D );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		stbi_image_free( data );
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free( data );
	}

	return textureID;
}

//----------------------------------------------------------------

glm::quat quatcast( glm::mat4 t ) {
	glm::quat q;
	float T = 1 + t[0][0] + t[1][1] + t[2][2];
	float S, X, Y, Z, W;

	if (T > 0.0000001f) 
	{
		S = glm::sqrt( T ) * 2.f;
		X = (t[1][2] - t[2][1]) / S;
		Y = (t[2][0] - t[0][2]) / S;
		Z = (t[0][1] - t[1][0]) / S;
		W = 0.25f * S;
	}
	else
	{
		if (t[0][0] > t[1][1] && t[0][0] > t[2][2])
		{
			// Column 0 :
			S = sqrt( 1.0f + t[0][0] - t[1][1] - t[2][2] ) * 2.f;
			X = 0.25f * S;
			Y = (t[0][1] + t[1][0]) / S;
			Z = (t[2][0] + t[0][2]) / S;
			W = (t[1][2] - t[2][1]) / S;
		}
		else if (t[1][1] > t[2][2])
		{
			// Column 1 :
			S = sqrt( 1.0f + t[1][1] - t[0][0] - t[2][2] ) * 2.f;
			X = (t[0][1] + t[1][0]) / S;
			Y = 0.25f * S;
			Z = (t[1][2] + t[2][1]) / S;
			W = (t[2][0] - t[0][2]) / S;
		}
		else
		{   // Column 2 :
			S = sqrt( 1.0f + t[1][1] - t[0][0] - t[1][1] ) * 2.f;
			X = (t[2][0] + t[0][2]) / S;
			Y = (t[1][2] + t[2][1]) / S;
			Z = 0.25f * S;
			W = (t[0][1] - t[1][0]) / S;
		}
	}
	q.w = W; q.x = -X; q.y = -Y; q.z = -Z;
	return q;
}

//----------------------------------------------------------------

glm::mat3x4 convertMatrix( glm::mat4 s )
{
	glm::mat3x4 t;
	t[0][0] = s[0][0];
	t[0][1] = s[0][1];
	t[0][2] = s[0][2];
	t[0][3] = s[0][3];

	t[1][0] = s[1][0];
	t[1][1] = s[1][1];
	t[1][2] = s[1][2];
	t[1][3] = s[1][3];

	t[2][0] = s[2][0];
	t[2][1] = s[2][1];
	t[2][2] = s[2][2];
	t[2][3] = s[2][3];

	return t;
}