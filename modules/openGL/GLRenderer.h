/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef GLRENDERER_H_
#define GLRENDERER_H_

#include "base/Base.h"
#include "scene/Renderer.h"

namespace mgp
{
class FrameBuffer;

class GLRenderer : public Renderer {
	uint64_t __currentShaderProgram = 0;
	StateBlock stateBlock;
	int _drawCallCount = 0;
public:
	GLRenderer();
    ~GLRenderer();

	void clear(ClearFlags flags, const Vector4& color = Vector4::zero(), float clearDepth = 1.0, int clearStencil = 0.0) override;
	void setViewport(int x, int y, int w, int h) override;

	uint64_t createBuffer(int type) override;
    void setBufferData(uint64_t buffer, int type, size_t startOffset, const char* data, size_t len, int usage) override;
    void deleteBuffer(uint64_t buffer) override;
    void draw(DrawCall* drawCall) override;

	
	void updateState(StateBlock* state, int force = 1) override;

	void updateTexture(Texture* texture) override;
	void deleteTexture(Texture* texture) override;
	void bindTextureSampler(Texture* texture) override;

	ShaderProgram* createProgram(ProgramSrc* src) override;
	void deleteProgram(ShaderProgram*effect) override;
	void bindProgram(ShaderProgram* effect) override;
	bool bindUniform(MaterialParameter* value, Uniform* uniform, ShaderProgram* effect) override;

	void bindVertexAttributeObj(VertexAttributeObject* vertextAttribute);
	void unbindVertexAttributeObj(VertexAttributeObject* vertextAttribute);
	void deleteVertexAttributeObj(VertexAttributeObject* vertextAttribute);

    UPtr<FrameBuffer> createFrameBuffer(const char* id, unsigned int width, unsigned int height, Texture::Format format = Texture::RGBA) override;
    FrameBuffer* getCurrentFrameBuffer() override;

	int drawCallCount() override;
private:

	void enableDepthWrite();

};

}
#endif
