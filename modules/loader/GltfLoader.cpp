/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#include "GLtfLoader.h"

#ifdef GLTFIO_DRACO_SUPPORTED
	#include "DracoCache.h"
#endif

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "material/MaterialParameter.h"
#include "base/FileSystem.h"
#include "material/Image.h"



using namespace mgp;

#ifdef GLTFIO_DRACO_SUPPORTED

using namespace filament_gltfio;

static void decodeDracoMeshes(DracoCache* dracoCache, cgltf_data* _gltf_data) {

	// For a given primitive and attribute, find the corresponding accessor.
	auto findAccessor = [](const cgltf_primitive* prim, cgltf_attribute_type type, cgltf_int idx) {
		for (cgltf_size i = 0; i < prim->attributes_count; i++) {
			const cgltf_attribute& attr = prim->attributes[i];
			if (attr.type == type && attr.index == idx) {
				return attr.data;
			}
		}
		return (cgltf_accessor*) nullptr;
		};

	// Go through every primitive and check if it has a Draco mesh.
	for (int i = 0; i < _gltf_data->meshes_count; ++i) {
		auto mesh = _gltf_data->meshes+i;
		for (int j = 0; j < mesh->primitives_count; ++j) {
			auto prim = mesh->primitives+j;
			if (!prim->has_draco_mesh_compression) {
				continue;
			}

			const cgltf_draco_mesh_compression& draco = prim->draco_mesh_compression;

			// If an error occurs, we can simply set the primitive's associated VertexBuffer to null.
			// This does not cause a leak because it is a weak reference.

			// Check if we have already decoded this mesh.
			DracoMesh* mesh = dracoCache->findOrCreateMesh(draco.buffer_view);
			if (!mesh) {
				printf("Cannot decompress mesh, Draco decoding error.\n");
				//vertexBuffer = nullptr;
				continue;
			}

			// Copy over the decompressed data, converting the data type if necessary.
			if (prim->indices && !mesh->getFaceIndices(prim->indices)) {
				//vertexBuffer = nullptr;
				continue;
			}

			// Go through each attribute in the decompressed mesh.
			for (cgltf_size i = 0; i < draco.attributes_count; i++) {

				// In cgltf, each Draco attribute's data pointer is an attribute id, not an accessor.
				const uint32_t id = draco.attributes[i].data - _gltf_data->accessors;

				// Find the destination accessor; this contains the desired component type, etc.
				const cgltf_attribute_type type = draco.attributes[i].type;
				const cgltf_int index = draco.attributes[i].index;
				cgltf_accessor* accessor = findAccessor(prim, type, index);
				if (!accessor) {
					printf("Cannot find matching accessor for Draco id %d\n", id);
					continue;
				}

				// Copy over the decompressed data, converting the data type if necessary.
				if (!mesh->getVertexAttributes(id, accessor)) {
					//vertexBuffer = nullptr;
					break;
				}
			}
		}
	}
}
#endif

class GltfLoaderImp {
	std::map<cgltf_node*, Node*> nodeMap;
	std::map<cgltf_mesh*, Model*> meshRes;
	std::string baseDir;
	cgltf_data* _gltf_data;

public:
	std::map<cgltf_skin*, SPtr<MeshSkin> > _skins;

private:
#ifdef GLTFIO_DRACO_SUPPORTED
	DracoCache* _dracoCache = NULL;
#endif

public:
	~GltfLoaderImp() {
#ifdef GLTFIO_DRACO_SUPPORTED
		delete _dracoCache;
#endif
	}
	bool lighting = false;
	UPtr<Scene> load(const char* file) {
		baseDir = FileSystem::getDirectoryName(file);

		cgltf_options options = {};
		cgltf_data* data = NULL;
		cgltf_result result = cgltf_parse_file(&options, file, &data);
		if (result == cgltf_result_success)
		{
			cgltf_load_buffers(&options, data, file);

			_gltf_data = data;
			UPtr<Scene> scene = loadScene(data);
			_gltf_data = NULL;

			cgltf_free(data);

			return scene;
		}
		else {
			GP_WARN("load fail: %s", file);
		}
		return UPtr<Scene>(NULL);
	}

	UPtr<Scene> loadFromBuf(const char* file_data, size_t file_size) {
		cgltf_options options = {};
		cgltf_data* data = NULL;

		cgltf_result result = cgltf_parse(&options, file_data, file_size, &data);
		if (result == cgltf_result_success)
		{
			cgltf_load_buffers(&options, data, NULL);

			_gltf_data = data;
			UPtr<Scene> scene = loadScene(data);
			_gltf_data = NULL;

			cgltf_free(data);

			return scene;
		}
		else {
			//GP_WARN("load fail: %s", file);
		}
		return UPtr<Scene>(NULL);
	}
private:
	UPtr<Texture> loadTexture(cgltf_texture* texture) {
		if (texture->image->uri == NULL && texture->image->buffer_view) {
			cgltf_buffer_view* buf = texture->image->buffer_view;
			UPtr<Image> image = Image::createFromBuf((const char*)buf->buffer->data + buf->offset, buf->size, false);
			UPtr<Texture> t = Texture::create(image.get(), false);
			//SAFE_RELEASE(image);
			return t;
		}
		std::string uri = baseDir + texture->image->uri;
		UPtr<Texture> t = Texture::create(uri.c_str(), true);
		return t;
	}

	UPtr<Material> loadPbrMaterial(cgltf_primitive* primitive, cgltf_material* cmaterial) {
		std::string define = "PBR;LDR";
		if (cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
			define += ";METALLIC_ROUGHNESS_MAP";
		}
		if (cmaterial->normal_texture.texture) {
			bool hasTangent = false;
			for (int i = 0; i < primitive->attributes_count; ++i) {
				if (primitive->attributes[i].type == cgltf_attribute_type_tangent) {
					hasTangent = true;
					break;
				}
			}
			if (hasTangent) {
				define += ";BUMPED";
			}
			else {
				define += ";SIMPLE_BUMPED";
			}
		}
		if (cmaterial->emissive_texture.texture) {
			define += ";EMISSIVE_MAP";
		}
		if (cmaterial->occlusion_texture.texture) {
			define += ";OCCLUSION_MAP";
		}

		cgltf_texture* ctexture = cmaterial->pbr_metallic_roughness.base_color_texture.texture;
		UPtr<Material> material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag", define.c_str());
		UPtr<Texture> texture = loadTexture(ctexture);
		material->getParameter("u_diffuseTexture")->setSampler(texture.get());

		float* color = cmaterial->pbr_metallic_roughness.base_color_factor;
		material->getParameter("u_albedo")->setVector3(Vector3(color[0], color[1], color[2]));

		material->getParameter("u_metallic")->setFloat(cmaterial->pbr_metallic_roughness.metallic_factor);
		material->getParameter("u_roughness")->setFloat(cmaterial->pbr_metallic_roughness.roughness_factor);
		material->getParameter("u_ao")->setFloat(1.0);

		float* emissive = cmaterial->emissive_factor;
		material->getParameter("u_emissive")->setVector3(Vector3(emissive[0], emissive[1], emissive[2]));

		if (cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture) {
			UPtr<Texture> metallic_roughness_texture = loadTexture(cmaterial->pbr_metallic_roughness.metallic_roughness_texture.texture);
			material->getParameter("u_metallic_roughness_map")->setSampler(metallic_roughness_texture.get());
		}

		if (cmaterial->normal_texture.texture) {
			UPtr<Texture> normal_texture = loadTexture(cmaterial->normal_texture.texture);
			material->getParameter("u_normalmapTexture")->setSampler(normal_texture.get());
		}

		if (cmaterial->emissive_texture.texture) {
			UPtr<Texture> emissive_texture = loadTexture(cmaterial->emissive_texture.texture);
			material->getParameter("u_emissive_map")->setSampler(emissive_texture.get());
		}

		if (cmaterial->occlusion_texture.texture) {
			UPtr<Texture> occlusion_texture = loadTexture(cmaterial->occlusion_texture.texture);
			material->getParameter("u_occlusion_texture")->setSampler(occlusion_texture.get());
		}
		return material;
	}

	void loadCommonMatrialProperty(cgltf_material* cmaterial, Material* material, Model* model) {
		if (cmaterial->double_sided) {
			material->getStateBlock()->setCullFace(false);
		}

		if (cmaterial->alpha_mode == cgltf_alpha_mode_blend) {
			material->getStateBlock()->setBlend(true);
			model->setRenderLayer(Drawable::Transparent);
			material->setShaderDefines(material->getShaderDefines() + ";TEXTURE_DISCARD_ALPHA");
			material->getParameter("u_alphaCutoff")->setFloat(0.1);
		}
		else if (cmaterial->alpha_mode == cgltf_alpha_mode_mask) {
			material->setShaderDefines(material->getShaderDefines()+";TEXTURE_DISCARD_ALPHA");
			material->getParameter("u_alphaCutoff")->setFloat(cmaterial->alpha_cutoff);
		}
	}

	UPtr<Material> loadMaterial(cgltf_primitive* primitive, cgltf_material* cmaterial, Model* model) {
		if (cmaterial->has_pbr_metallic_roughness) {
			cgltf_texture* ctexture = cmaterial->pbr_metallic_roughness.base_color_texture.texture;
			if (ctexture) {
				if (lighting) {
					UPtr<Material> material = loadPbrMaterial(primitive, cmaterial);
					loadCommonMatrialProperty(cmaterial, material.get(), model);
					return material;
				}
				else {
					UPtr<Material> material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag");
					UPtr<Texture> texture = loadTexture(ctexture);
					material->getParameter("u_diffuseTexture")->setSampler(texture.get());
					loadCommonMatrialProperty(cmaterial, material.get(), model);
					return material;
				}
			}
			else {
				if (lighting) {
					UPtr<Material> material = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag", "PBR;LDR");
					float* color = cmaterial->pbr_metallic_roughness.base_color_factor;
					material->getParameter("u_diffuseColor")->setVector4(Vector4(color[0], color[1], color[2], color[3]));

					material->getParameter("u_albedo")->setVector3(Vector3(color[0], color[1], color[2]));
					material->getParameter("u_metallic")->setFloat(cmaterial->pbr_metallic_roughness.metallic_factor);
					material->getParameter("u_roughness")->setFloat(cmaterial->pbr_metallic_roughness.roughness_factor);
					material->getParameter("u_ao")->setFloat(1.0);

					float* emissive = cmaterial->emissive_factor;
					material->getParameter("u_emissive")->setVector3(Vector3(emissive[0], emissive[1], emissive[2]));
					loadCommonMatrialProperty(cmaterial, material.get(), model);
					return material;
				}
				else {
					UPtr<Material> material = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag");
					float* color = cmaterial->pbr_metallic_roughness.base_color_factor;
					material->getParameter("u_diffuseColor")->setVector4(Vector4(color[0], color[1], color[2], color[3]));
					loadCommonMatrialProperty(cmaterial, material.get(), model);
					return material;
				}
			}
		}
		else if (cmaterial->extensions_count) {
			for (int i = 0; i < cmaterial->extensions_count; ++i) {
				if (strcmp(cmaterial->extensions[i].name, "KHR_techniques_webgl") == 0 || strcmp(cmaterial->extensions[i].name, "KHR_technique_webgl") == 0) {
					//{"technique":0,"values":{"diffuse":0}}
					char* p = strstr(cmaterial->extensions[i].data, "\"diffuse\"");
					p += 10;
					long diffuse = strtol(p, NULL, 10);

					if (_gltf_data->textures_count > diffuse && _gltf_data->textures) {
						cgltf_texture* ctexture = _gltf_data->textures+diffuse;
						UPtr<Texture> texture = loadTexture(ctexture);
						UPtr<Material> material = Material::create("res/shaders/textured.vert", "res/shaders/textured.frag");
						material->getParameter("u_diffuseTexture")->setSampler(texture.get());
						//SAFE_RELEASE(texture);
						loadCommonMatrialProperty(cmaterial, material.get(), model);
						return material;
					}
					break;
				}
			}
		}

		//fallback
		UPtr<Material> mat = Material::create("res/shaders/colored.vert", "res/shaders/colored.frag");
		mat->getParameter("u_diffuseColor")->setVector4(Vector4(1.0, 0.0, 0.0, 1.0));
		loadCommonMatrialProperty(cmaterial, mat.get(), model);
		return mat;
	}

	void loadPrimitive(cgltf_primitive* primitive, Model* model) {
		Mesh* mesh = model->getMesh();
		int partIndex = -1;
		if (primitive->indices) {
			Mesh::PrimitiveType type;
			switch (primitive->type)
			{
			case cgltf_primitive_type_points:
				type = Mesh::PrimitiveType::POINTS;
				break;
			case cgltf_primitive_type_lines:
				type = Mesh::PrimitiveType::LINES;
				break;
			case cgltf_primitive_type_line_loop:
				type = Mesh::PrimitiveType::LINE_LOOP;
				break;
			case cgltf_primitive_type_line_strip:
				type = Mesh::PrimitiveType::LINE_STRIP;
				break;
			case cgltf_primitive_type_triangles:
				type = Mesh::PrimitiveType::TRIANGLES;
				break;
			case cgltf_primitive_type_triangle_strip:
				type = Mesh::PrimitiveType::TRIANGLE_STRIP;
				break;
			case cgltf_primitive_type_triangle_fan:
				type = Mesh::PrimitiveType::TRIANGLE_FAN;
				break;
			default:
				type = Mesh::PrimitiveType::LINES;
			}
			int indexCount = primitive->indices->count;
			partIndex = mesh->getPartCount();
			Mesh::MeshPart* part = mesh->addPart(type, indexCount);

			int32_t* data = (int32_t*)malloc(indexCount * 4);
			for (int j = 0; j < indexCount; ++j) {
				int v = cgltf_accessor_read_index(primitive->indices, j);
				data[j] = v;
			}
			int bufferOffset = mesh->getIndexBuffer()->addData((char*)data, indexCount * 4);
			part->_bufferOffset = bufferOffset;
			free(data);
		}

		bool hasMaterial = false;
		if (primitive->material) {
			UPtr<Material> m = loadMaterial(primitive, primitive->material, model);
			if (m.get()) {
				model->setMaterial(std::move(m), partIndex);
				hasMaterial = true;
			}
			//SAFE_RELEASE(m);
		}
		
		if (!hasMaterial) {
			Material* mat = model->setMaterial("res/shaders/colored.vert", "res/shaders/colored.frag");
			mat->getParameter("u_diffuseColor")->setVector4(Vector4(1.0, 0.0, 0.0, 1.0));
		}
	}

	UPtr<Mesh> loadMeshVertices(std::vector<cgltf_attribute*>& attrs, cgltf_primitive* primitive) {
		int vertexCount = 0;
		std::vector<cgltf_accessor*> accessors;
		std::vector<VertexFormat::Element> vertexElemets;
		for (int j = 0; j < attrs.size(); ++j) {
			cgltf_attribute* attr = attrs[j];
			cgltf_accessor* accessor = attr->data;
			VertexFormat::Element element;
			element.size = cgltf_num_components(accessor->type);

			std::string name = attr->name;
			if (attr->type == cgltf_attribute_type_position) {
				element.usage = VertexFormat::POSITION;
			}
			else if (attr->type == cgltf_attribute_type_normal) {
				element.usage = VertexFormat::NORMAL;
			}
			else if (attr->type == cgltf_attribute_type_tangent) {
				element.usage = VertexFormat::TANGENT;
			}
			else if (attr->type == cgltf_attribute_type_texcoord) {
				element.usage = VertexFormat::TEXCOORD0;
			}
			else if (attr->type == cgltf_attribute_type_color) {
				element.usage = VertexFormat::COLOR;
			}
			else if (attr->type == cgltf_attribute_type_joints) {
				element.usage = VertexFormat::BLENDINDICES;
			}
			else if (attr->type == cgltf_attribute_type_weights) {
				element.usage = VertexFormat::BLENDWEIGHTS;
			}
			else if (name == "TEXCOORD_0") {
				element.usage = VertexFormat::TEXCOORD0;
			}
			else if (name == "TEXCOORD_1") {
				element.usage = VertexFormat::TEXCOORD1;
			}
			else if (name == "TEXCOORD_2") {
				element.usage = VertexFormat::TEXCOORD2;
			}
			else if (name == "TEXCOORD_3") {
				element.usage = VertexFormat::TEXCOORD3;
			}
			else if (name == "TEXCOORD_4") {
				element.usage = VertexFormat::TEXCOORD4;
			}
			else if (name == "TEXCOORD_5") {
				element.usage = VertexFormat::TEXCOORD5;
			}
			else if (name == "TEXCOORD_6") {
				element.usage = VertexFormat::TEXCOORD6;
			}
			else if (name == "TEXCOORD_7") {
				element.usage = VertexFormat::TEXCOORD7;
			}
			else {
				element.usage = VertexFormat::CUSTEM;
				element.name = name;
				//printf("unsupport attribute type: %s\n", name.c_str());
				//continue;
			}
			vertexElemets.push_back(element);
			accessors.push_back(accessor);
			if (vertexCount < attr->data->count) {
				vertexCount = attr->data->count;
			}
		}
		
		if (primitive) {
			for (int j = 0; j < primitive->targets_count; ++j) {
				if (j >= VertexFormat::MAX_MORPH_TARGET) break;
				cgltf_morph_target* target = primitive->targets + j;
				for (int i = 0; i < target->attributes_count; ++i) {
					cgltf_attribute* attr = target->attributes + i;
					cgltf_accessor* accessor = attr->data;
					VertexFormat::Element element;
					element.size = cgltf_num_components(accessor->type);
					std::string name = attr->name;
					if (attr->type == cgltf_attribute_type_position) {
						element.usage = (VertexFormat::Usage)(VertexFormat::MORPHTARGET0 + j);
					}
					else if (attr->type == cgltf_attribute_type_normal) {
						element.usage = (VertexFormat::Usage)(VertexFormat::MORPHNORMAL0 + j);
					}
					else if (attr->type == cgltf_attribute_type_tangent) {
						element.usage = (VertexFormat::Usage)(VertexFormat::MORPHTANGENT0 + j);
					}
					else {
						element.usage = VertexFormat::CUSTEM;
						element.name = name;
					}
					vertexElemets.push_back(element);
					accessors.push_back(accessor);
				}
			}
		}

		VertexFormat format(vertexElemets.data(), vertexElemets.size());
		UPtr<Mesh> mesh = Mesh::createMesh(format, vertexCount, Mesh::INDEX32);
		int vertexSize = format.getVertexSize();
		int bufsize = vertexSize * vertexCount;
		char* data = (char*)malloc(bufsize);

		for (int i = 0; i < vertexCount; ++i) {
			int offset = 0;
			for (int j = 0; j < vertexElemets.size(); ++j) {
				cgltf_accessor* accessor = accessors[j];
				float* out = (float*)(data + (vertexSize * i) + offset);
				cgltf_accessor_read_float(accessor, i, out, cgltf_num_components(accessor->type));
				offset += vertexElemets[j].size * sizeof(float);
			}
		}

		mesh->getVertexBuffer()->setData(data, bufsize, false);
		return mesh;
	}

	UPtr<Model> loadPrimitiveAsMesh(cgltf_primitive* primitive) {
		int vertexCount = 0;
		std::vector<cgltf_attribute*> attrs;
		attrs.resize(primitive->attributes_count);
		for (int j = 0; j < primitive->attributes_count; ++j) {
			cgltf_attribute* attr = primitive->attributes + j;
			attrs[j] = attr;
		}

		UPtr<Mesh> mesh = loadMeshVertices(attrs, primitive);
		UPtr<Model> model = Model::create(std::move(mesh));
		loadPrimitive(primitive, model.get());
		return model;
	}

	Model* loadMesh(cgltf_mesh *cmesh, Node* node) {

		bool sharedVertexBuf = true;
		std::map<std::string, cgltf_accessor*> attributeUnique;
		std::vector<cgltf_attribute*> attrs;
		for (int i = 0; i < cmesh->primitives_count; ++i) {
			cgltf_primitive* primitive = cmesh->primitives + i;
			if (primitive->targets_count > 0) {
				sharedVertexBuf = false;
				goto label1;
			}
			for (int j = 0; j < primitive->attributes_count; ++j) {
				cgltf_attribute* attr = primitive->attributes + j;
				cgltf_accessor* old = attributeUnique[attr->name];
				if (!old) {
					attributeUnique[attr->name] = attr->data;
					attrs.push_back(attr);
				}
				else if (old != attr->data) {
					sharedVertexBuf = false;
					goto label1;
				}
			}
		}

	label1:

		if (!sharedVertexBuf) {
			Model* res = NULL;
			UPtr<DrawableGroup> group(new DrawableGroup());
			for (int i = 0; i < cmesh->primitives_count; ++i) {
				cgltf_primitive* primitive = cmesh->primitives + i;
				UPtr<Model> model = loadPrimitiveAsMesh(primitive);
				if (!res) res = model.get();
				if (lighting) {
					model->setLightMask(1);
				}
				group->getDrawables().push_back(std::move(model));
			}
			for (int i = 0; i < cmesh->weights_count; ++i) {
				node->getWeights().push_back(cmesh->weights[i]);
			}
			node->addComponent(std::move(group));
			return res;
		}
		else {
			UPtr<Mesh> mesh = loadMeshVertices(attrs, NULL);
			UPtr<Model> model = Model::create(std::move(mesh));
			for (int i = 0; i < cmesh->weights_count; ++i) {
				node->getWeights().push_back(cmesh->weights[i]);
			}
			for (int i = 0; i < cmesh->primitives_count; ++i) {
				cgltf_primitive* primitive = cmesh->primitives + i;
				loadPrimitive(primitive, model.get());
			}
			Model* res = model.get();
			node->addComponent(std::move(model));

			if (lighting) {
				res->setLightMask(1);
			}
			return res;
		}
	}

	Node* getJointNode(cgltf_node* cnode) {
		auto it = nodeMap.find(cnode);
		if (it != nodeMap.end()) {
			return (it->second);
		}
		
		return NULL;
	}

	UPtr<MeshSkin> loadSkin(cgltf_skin* cskin) {
		auto it = _skins.find(cskin);
		if (it != _skins.end()) {
			return uniqueFromInstant(it->second.get());
		}

		MeshSkin* skin = new MeshSkin();
		int num_comp = cgltf_num_components(cskin->inverse_bind_matrices->type);
		float* matrix = (float*)malloc(cskin->inverse_bind_matrices->count * num_comp * sizeof(float));
		for (int i = 0; i < cskin->inverse_bind_matrices->count; ++i) {
			float* out = matrix + (num_comp * i);
			cgltf_accessor_read_float(cskin->inverse_bind_matrices, i, out, num_comp);
		}

		skin->setJointCount(cskin->joints_count);
		for (int i = 0; i < cskin->joints_count; ++i) {
			Node* joint = getJointNode(cskin->joints[i]);
			if (!joint) continue;
			Matrix m(matrix+(i*16));
			BoneJoint* bone = skin->getJoint(i);
			bone->_node = joint;
			bone->_name = cskin->joints[i]->name;
			bone->_bindPose = m;
		}

		if (cskin->skeleton) {
			Node* skeleton = getJointNode(cskin->skeleton);
			assert(skeleton);
			skin->setRootJoint(skeleton);
		}

		SPtr<MeshSkin> sskin; sskin = skin;
		_skins[cskin] = sskin;
		return UPtr<MeshSkin>(skin);
	}

	UPtr<Animation> loadAnimation(cgltf_animation* canimation) {
		Animation* animation = new Animation(canimation->name == NULL ? "" : canimation->name);
		for (int i = 0; i < canimation->channels_count; ++i) {
			cgltf_animation_channel* cchannel = canimation->channels+i;
			cgltf_animation_sampler* csampler = cchannel->sampler;

			AnimationTarget* target = nodeMap[cchannel->target_node];
			if (!target) continue;

			unsigned int propertyId = -1;
			switch (cchannel->target_path)
			{
			case cgltf_animation_path_type_invalid:
				break;
			case cgltf_animation_path_type_translation:
				propertyId = Transform::ANIMATE_TRANSLATE;
				break;
			case cgltf_animation_path_type_rotation:
				propertyId = Transform::ANIMATE_ROTATE;
				break;
			case cgltf_animation_path_type_scale:
				propertyId = Transform::ANIMATE_SCALE;
				break;
			case cgltf_animation_path_type_weights:
				propertyId = Transform::ANIMATE_WEIGHTS;
				break;
			case cgltf_animation_path_type_max_enum:
				break;
			default:
				break;
			}
			if (propertyId == -1) continue;

			unsigned int keyCount = cchannel->sampler->input->count;
			if (keyCount > cchannel->sampler->output->count) {
				printf("ERROR: keyCount != valueCount\n");
				continue;
			}
			//unsigned int num_comp = cgltf_num_components(cchannel->sampler->input->type);
			//float* fKeyTimes = (float*)malloc(num_comp * sizeof(float));
			unsigned int* keyTimes = (unsigned int*)malloc(keyCount*sizeof(int));
			float minTime = cchannel->sampler->input->min[0];
			float maxTime = cchannel->sampler->input->max[0];
			for (int i = 0; i < keyCount; ++i) {
				float out[16];
				cgltf_accessor_read_float(cchannel->sampler->input, i, out, 1);
				//keyTimes[i] = ((out[0] - minTime) / (maxTime-minTime))*1000;
				keyTimes[i] = out[0] * 1000;
			}

			unsigned int interpolationType = -1;
			switch (cchannel->sampler->interpolation)
			{
			case cgltf_interpolation_type_linear:
				interpolationType = Curve::LINEAR;
				break;
			case cgltf_interpolation_type_step:
				interpolationType = Curve::STEP;
				break;
			case cgltf_interpolation_type_cubic_spline:
				interpolationType = Curve::BSPLINE;
				break;
			case cgltf_interpolation_type_max_enum:
			default:
				break;
			}
			if (interpolationType == -1) break;

			int num_comp = cgltf_num_components(cchannel->sampler->output->type);
			float valueCount = cchannel->sampler->output->count;
			float* keyValues = (float*)malloc(valueCount *num_comp*sizeof(float));
			for (int i = 0; i < valueCount; ++i) {
				float* out = keyValues+(num_comp*i);
				cgltf_accessor_read_float(cchannel->sampler->output, i, out, num_comp);
			}

			animation->createChannel(target, propertyId, keyCount, keyTimes, keyValues, interpolationType);
		}
		return UPtr<Animation>(animation);
	}

	UPtr<Node> loadNode(cgltf_node* cnode) {
		Node* node = NULL;
		auto it = nodeMap.find(cnode);
		if (it != nodeMap.end()) {
			node = it->second;
			//node->addRef();
		}
		else {
			node = Node::create(cnode->name).take();
		}

		Matrix m;
		float matrix[16];
		cgltf_node_transform_local(cnode, matrix);
		m.set(matrix);
		node->setMatrix(m);

		
		Model* tempModel = NULL;
		if (cnode->mesh) {
			auto found = meshRes.find(cnode->mesh);
			if (found != meshRes.end()) {
				tempModel =  found->second;
				NodeCloneContext context;
				node->setDrawable(tempModel->clone(context));
			}
			else {
				tempModel = loadMesh(cnode->mesh, node);
				meshRes[cnode->mesh] = tempModel;
			}
		}
		if (cnode->skin) {
			UPtr<Model> model;
			UPtr<MeshSkin> skin = loadSkin(cnode->skin);
			if (!tempModel) {
				model = UPtr<Model>(new Model());
				tempModel = model.get();
				node->addComponent(std::move(model));
			}
			tempModel->setSkin(std::move(skin));
			//SAFE_RELEASE(skin);
		}

		//SAFE_RELEASE(model);

		for (int i = 0; i < cnode->children_count; ++i) {
			UPtr<Node> child = loadNode(cnode->children[i]);
			node->addChild(std::move(child));
			//SAFE_RELEASE(child);
		}

		nodeMap[cnode] = node;
		return UPtr<Node>(node);
	}

	UPtr<Scene> loadScene(cgltf_data* data) {

		for (cgltf_size i = 0; i < data->extensions_required_count; i++) {
			if (!strcmp(data->extensions_required[i], "KHR_draco_mesh_compression")) {
				#ifdef GLTFIO_DRACO_SUPPORTED
					_dracoCache = new DracoCache();
					decodeDracoMeshes(_dracoCache, data);
				#else
					printf("KHR_draco_mesh_compression is not supported.\n");
					return UPtr<Scene>(NULL);
				#endif
				break;
			}
		}

		if (!data->scenes_count) {
			return UPtr<Scene>(NULL);
		}
		cgltf_scene *cscene = data->scenes;
		UPtr<Scene> scene = Scene::create(cscene->name);

		for (int i = 0; i < data->skins_count; ++i) {
			cgltf_skin* skin = data->skins + i;
			for (int j = 0; j < skin->joints_count; ++j) {
				cgltf_node* joint = skin->joints[j];
				Node* node = Node::create(joint->name).take();
				node->setRecursiveUpdate(false);
				node->setBoneJoint(true);
				nodeMap[joint] = node;
			}
			loadSkin(skin);
		}

		for (int i = 0; i < cscene->nodes_count; ++i) {
			cgltf_node* cnode = cscene->nodes[i];
			UPtr<Node> node = loadNode(cnode);
			scene->addNode(std::move(node));
			//SAFE_RELEASE(node);
		}

		for (auto it = _skins.begin(); it != _skins.end(); ++it) {
			Node* rootJoint = (it->second)->getRootJoint();
			if (!rootJoint && it->second->getJointCount() > 0) {
				rootJoint = it->second->getJoint(0)->_node.get();
				while (rootJoint && rootJoint->getParent() && rootJoint->getParent()->isBoneJoint()) {
					rootJoint = rootJoint->getParent();
				}
				it->second->setRootJoint(rootJoint);
			}
		}

		for (int i = 0; i < data->animations_count; ++i) {
			cgltf_animation* ca = data->animations + i;
			UPtr<Animation> a = loadAnimation(ca);
			//a->release();
		}

		return scene;
	}
};

std::vector<SPtr<MeshSkin> > GltfLoader::loadSkins(const std::string& file) {
	GltfLoaderImp imp;
	imp.load(file.c_str());
	std::vector<SPtr<MeshSkin> > skins;
	for (auto it = imp._skins.begin(); it != imp._skins.end(); ++it) {
		skins.push_back(it->second);
	}
	return skins;
}

UPtr<Scene> GltfLoader::load(const std::string& file) {
	GltfLoaderImp imp;
	imp.lighting = lighting;
	return imp.load(file.c_str());
}

UPtr<Scene> GltfLoader::loadFromBuf(const char* file_data, size_t file_size) {
	GltfLoaderImp imp;
	imp.lighting = lighting;
	return imp.loadFromBuf(file_data, file_size);
}
