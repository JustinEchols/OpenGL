internal texture_type_t
assimp_texture_type_convert(aiTextureType texture_type)
{
	texture_type_t Result;

	// TODO(Justin): More cases
	if(texture_type == aiTextureType_DIFFUSE)
	{
		Result = TEXTURE_TYPE_DIFFUSE;
	}
	else
	{
		Result = TEXTURE_TYPE_SPECULAR;
	}
	return(Result);
}

// For each face, loop through the indices and copy them. When using GL_TRIANGULATE
// the number of indices is always 3.
internal mesh_indices_t
mesh_process_indices(aiMesh *Mesh)
{
	mesh_indices_t MeshIndices = {};

	size_t index_size = sizeof(u32);


	// NOTE(Justin): This assignmenet of indices count only works for meshes
	// composed of triangles only.
	u32 indices_count = Mesh->mNumFaces * 3;
	u32 index_buffer_memory_size = index_size * indices_count;

	MeshIndices.indices = (u32 *)calloc((size_t)indices_count, index_size);
	MeshIndices.indices_count = indices_count;

	u32* DestMeshIndex = MeshIndices.indices;
	for (u32 i = 0; i < Mesh->mNumFaces; i++)
	{
		aiFace *MeshFace = &Mesh->mFaces[i];
		u32* SrcMeshIndex = MeshFace->mIndices;
		for (u32 j = 0; j < MeshFace->mNumIndices; j++)
		{
			*DestMeshIndex++ = *SrcMeshIndex++;
		}
	}
	return(MeshIndices);
}

internal mesh_vertices_t 
mesh_process_vertices(aiMesh *Mesh)
{
	mesh_vertices_t MeshVertices = {};

	u32 vertices_count = Mesh->mNumVertices;
	size_t vertex_size = sizeof(mesh_vertex_t);
	u32 vertex_buffer_memory_size = vertex_size * vertices_count;

	// Allocate an array of mesh_vertex_t vertices (postion, normal, and texture coordinates)
	MeshVertices.Vertices = (mesh_vertex_t*)calloc((size_t)vertices_count, vertex_size);
	MeshVertices.vertices_count = vertices_count;

	mesh_vertex_t *MeshVertex = MeshVertices.Vertices;
	for (u32 i = 0; i < vertices_count; i++)
	{
		MeshVertex->Position.x = Mesh->mVertices[i].x;
		MeshVertex->Position.y = Mesh->mVertices[i].y;
		MeshVertex->Position.z = Mesh->mVertices[i].z;

		MeshVertex->Normal.x = Mesh->mNormals[i].x;
		MeshVertex->Normal.y = Mesh->mNormals[i].y;
		MeshVertex->Normal.z = Mesh->mNormals[i].z;

		if (Mesh->mTextureCoords[0])
		{
			// TODO(Justin): This check every loop is completley redundant?
			MeshVertex->TexCoord.x = Mesh->mTextureCoords[0][i].x;
			MeshVertex->TexCoord.y = Mesh->mTextureCoords[0][i].y;
		}
		else
		{
			MeshVertex->TexCoord = glm::vec2(0.0f, 0.0f);
		}
		MeshVertex++;
	}
	return(MeshVertices);
}


internal void
mesh_process_texture_map(app_state_t *AppState, aiMaterial *MeshMaterial,
		mesh_textures_t *MeshTextures, u32 texture_count, aiTextureType texture_type,
		aiString path_to_texture)
{
	texture_t* Texture = &MeshTextures->Textures[MeshTextures->texture_count];
	for(u32 i = 0; i < texture_count; i++)
	{
		aiString texture_filename;
		MeshMaterial->GetTexture(texture_type, i, &texture_filename);
		path_to_texture.Append(texture_filename.C_Str());

		b32 texture_is_loaded = false;
		for(u32 j = 0; j < AppState->loaded_texture_count; j++)
		{
			const char * loaded_texture_path = AppState->LoadedTextures[j].path;
			if(loaded_texture_path)
			{
				if(strcmp(loaded_texture_path, path_to_texture.C_Str()) == 0)
				{
					// Texture already previously loaded do not load memory again, only copy data from
					// previously loaded state.
					texture_is_loaded = true;
					*Texture = AppState->LoadedTextures[j];
					MeshTextures->texture_count++;
					break;
				}
			}
		}
		if(!texture_is_loaded)
		{
			// Texture does not exist, load the texture into the mesh array and also into the loaded 
			// textures of the app state.
			texture_type_t TextureType = assimp_texture_type_convert(texture_type);
			*Texture = texture_simple_init(path_to_texture.C_Str(), TextureType);
			MeshTextures->texture_count++;
			AppState->LoadedTextures[AppState->loaded_texture_count] = *Texture;
			AppState->loaded_texture_count++;
		}
		Texture++;
	}
}

internal mesh_textures_t
mesh_process_material(app_state_t *AppState, aiMaterial *MeshMaterial)
{
	mesh_textures_t MeshTextures = {};

	u32 texture_diffuse_count = MeshMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	u32 texture_specular_count = MeshMaterial->GetTextureCount(aiTextureType_SPECULAR);

	size_t texture_size = sizeof(texture_t);
	u32 texture_count = texture_diffuse_count + texture_specular_count;
	u32 texture_memory_size = texture_count * texture_size;

	MeshTextures.Textures = (texture_t*)calloc((size_t)texture_count, texture_size);

	aiString path_to_texture = aiString("models/backpack/");

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_diffuse_count, aiTextureType_DIFFUSE, 
			path_to_texture);

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_specular_count, aiTextureType_SPECULAR,
			path_to_texture);

	return(MeshTextures);
}

// TODO(Justin): Need to remove hardcoded inputs.
internal void
node_process(app_state_t *AppState, const aiScene *Scene, aiNode *Node, model_t *Model,
		const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	mesh_t* ModelMesh = &Model->Meshes[Model->mesh_count];
	for (u32 i = 0; i < Node->mNumMeshes; i++)
	{
		aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[i]];

		mesh_vertices_t MeshVertices = mesh_process_vertices(Mesh);
		mesh_indices_t MeshIndices = mesh_process_indices(Mesh);

		GLuint MeshVAO, MeshVBO, MeshEBO;
		glGenVertexArrays(1, &MeshVAO);
		glGenBuffers(1, &MeshVBO);
		glGenBuffers(1, &MeshEBO);

		glBindVertexArray(MeshVAO);
		glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);

		glBufferData(GL_ARRAY_BUFFER, MeshVertices.vertices_count * sizeof(mesh_vertex_t), MeshVertices.Vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MeshIndices.indices_count * sizeof(u32), MeshIndices.indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)OffsetOfMember(mesh_vertex_t, Normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)OffsetOfMember(mesh_vertex_t, TexCoord));

		glBindVertexArray(0);

		//
		// NOTE(Justin): Materials
		//

		mesh_textures_t MeshTextures = {};
		if (Mesh->mMaterialIndex >= 0)
		{
			aiMaterial* MeshMaterial = Scene->mMaterials[Mesh->mMaterialIndex];
			MeshTextures = mesh_process_material(AppState, MeshMaterial);
		}

		ModelMesh->MeshVBO = MeshVBO;
		ModelMesh->MeshEBO = MeshEBO;
		ModelMesh->MeshVAO = MeshVAO;

		ModelMesh->MeshVertices = MeshVertices;
		ModelMesh->MeshIndices = MeshIndices;
		ModelMesh->MeshTextures = MeshTextures;

		shader_program_t MeshShader;
		MeshShader.vertex_shader_filename = vertex_shader_filename;
		MeshShader.fragment_shader_filename = fragment_shader_filename;
		MeshShader = shader_program_create_from_files(MeshShader.vertex_shader_filename,
													  MeshShader.fragment_shader_filename);
		gl_log_shader_info(&MeshShader);

		ModelMesh->MeshShader = MeshShader;
		
		Model->mesh_count++;

		// free memory allocated?
	}
	for(u32 i = 0; i < Node->mNumChildren; i++)
	{
		node_process(AppState, Scene, Node->mChildren[i], Model, vertex_shader_filename, fragment_shader_filename);
	}
}

// NOTE(Justin): Should this return a model?
// NOTE(Justin): The meshes of a model may require different shaders?
internal void
model_process(app_state_t *AppState, const char *model_filename, const char* vertex_shader_filename,
																 const char* fragment_shader_filename)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(model_filename, ASSIMP_LOAD_FLAGS);

	model_t Result;

	u32 mesh_count = Scene->mNumMeshes;
	size_t mesh_size = sizeof(mesh_t);
	Result.Meshes = (mesh_t *)calloc((size_t)mesh_count, mesh_size);
	Result.mesh_count = 0;
	
	aiNode *Node = Scene->mRootNode;

	node_process(AppState, Scene, Node, &Result, vertex_shader_filename, fragment_shader_filename);
	if(mesh_count == Result.mesh_count)
	{
		AppState->Models[AppState->model_count] = Result;
		AppState->model_count++;
	}
	else
	{
	}
}

internal void
mesh_draw(app_state_t *AppState, mesh_t *Mesh, glm::mat4 ModelTransform, glm::mat4 MapToCamera, glm::mat4 MapToPersp, glm::vec3 LightPosition)
{
	shader_program_t Shader = Mesh->MeshShader;
	glUseProgram(Shader.id);

	// TODO(Justin): Can we get the program information for the mesh shader and
	// then set the uniforms we need to set? Otherwise will need a different
	// draw call for each mesh that uses a different shader with differen inputs
	// and outputs... :(
	uniform_set_mat4f(Shader.id, "ModelTransform", ModelTransform);
	uniform_set_mat4f(Shader.id, "MapToCamera", MapToCamera);
	uniform_set_mat4f(Shader.id,"MapToPersp", MapToPersp);

	uniform_set_vec3f(Shader.id, "u_CameraPos", AppState->Camera.Pos);
	uniform_set_f32(Shader.id, "u_Material.shininess", 32.0f);

	LightPosition.x = 5.0f * cos(glfwGetTime());
	LightPosition.y = 0.0f;
	LightPosition.z = -5.0f * sin(glfwGetTime());

	uniform_set_vec3f(Shader.id, "u_LightPoint.Pos", LightPosition);
	uniform_set_vec3f(Shader.id, "u_LightPoint.Ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	uniform_set_vec3f(Shader.id, "u_LightPoint.Diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	uniform_set_vec3f(Shader.id, "u_LightPoint.Specular", glm::vec3(1.0f, 1.0f, 1.0f));

	uniform_set_f32(Shader.id, "u_LightPoint.atten_constant", 1.0f);
	uniform_set_f32(Shader.id, "u_LightPoint.atten_linear", 0.09f);
	uniform_set_f32(Shader.id, "u_LightPoint.atten_quadratic", 0.032f);

	// TODO(Justin): Loop through all the textures for each mesh and set them.
	if (Mesh->MeshTextures.texture_count)
	{
		for(u32 texture_index = 0; texture_index < Mesh->MeshTextures.texture_count; texture_index++)
		{
			texture_t* MeshTexture = Mesh->MeshTextures.Textures + texture_index;
			// TODO(Justin): What type of texture are we setting? Is it implicit
			// in MeshTexture. It is being set as GL_TEXTURE_2D for each one.
			// The type of texture should probably be store in MeshTexturee.
			texture_set_active_and_bind(texture_index, MeshTexture);
			//MeshTexture++;
			//texture_set_active_and_bind(1, MeshTexture);
		}
	}
	glBindVertexArray(Mesh->MeshVAO);
	glDrawElements(GL_TRIANGLES, Mesh->MeshIndices.indices_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}
