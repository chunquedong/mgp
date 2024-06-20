/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef GLTFLOADER_H_
#define GLTFLOADER_H_

#include "base/Base.h"
#include "scene/Mesh.h"
#include "scene/Scene.h"
#include "scene/Node.h"

namespace mgp
{
/**
 * Loading GLTF format from file or buffer.
 * support skin animation and morph animation
 */
class GltfLoader {
public:
	enum LightingType {
		Pbr = 1, //physic base lighting
		NoSpecular = 2, //no specular lighting
		Ldr = 4,  //convert result from HDR to LDR
	};

	//LightingType: Pbr | NoSpecular | Ldr
	int lighting = 0;

	UPtr<Scene> load(const std::string &file);
	UPtr<Scene> loadFromBuf(const char* file_data, size_t file_size);
	std::vector<SPtr<MeshSkin> > loadSkins(const std::string& file);
};
}

#endif //GLTFLOADER_H_