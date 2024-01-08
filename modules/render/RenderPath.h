/*
 * Copyright (c) 2023, chunquedong
 *
 * This file is part of MGP project
 * Licensed under the GNU LESSER GENERAL PUBLIC LICENSE
 *
 */
#ifndef RENDERPATH_H_
#define RENDERPATH_H_

#include "base/Base.h"
#include "scene/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Drawable.h"
#include "RenderQueue.h"

namespace mgp {
	class Shadow;
	struct RenderStage;
	class FrameBuffer;
	class RenderPath;

	/**
	* Render pipeline
	*/
	class RenderPath : public Refable
	{
	protected:
		RenderInfo _renderView;

		Renderer* _renderer;
		RenderQueue _renderQueue;

		static Model* _quadModel;

		FrameBuffer* _frameBuffer;
		FrameBuffer* _previousFrameBuffer;

		int _width;
		int _height;
		std::vector<RenderStage*> _renderStages;

		std::map<Light*, Shadow*> _shadowMapCache;

		std::map<std::string, FrameBuffer*> _frameBufferPool;
		std::map<std::string, Texture*> _texturePool;
	public:
		bool _use_ssao;
		bool _use_bloom;
		bool _use_prez;
		bool _use_shadow;
		bool _use_fxaa;
		bool _use_hdr;
		bool _blend;
	public:
		RenderPath(Renderer* renderer);
		~RenderPath();
		RenderInfo* getRenderView();
		void applyDraw(RenderInfo* view);
		
		static Model* fullscreenQuadModel();
		static void releaseStatic();

		std::vector<RenderStage*>& getRenderStages() { return _renderStages; }
		FrameBuffer* getFrameBuffer() { return _frameBuffer; }
		RenderQueue* getRenderQueue() { return &_renderQueue; }

		Renderer* getRenderer() { return _renderer; }
		void render(Scene* scene, Camera *camera, Rectangle *viewport);
		void renderDrawables(std::vector<Drawable*>& drawables, Camera* camera, Rectangle* viewport);

		void onResize(int w, int h);

		virtual void finalize();

		virtual void initDeferred();
		virtual void initForward();

		void bindShadow(std::vector<Light*>* lights, DrawCall* drawCall);

		void addFrameBuffer(FrameBuffer* frameBuffer);
		FrameBuffer* getFrameBuffer(const std::string& name);
		Texture* getTexture(const std::string& name);
	protected:

		virtual void addPostProcess();
		
		void createFramebuffer();
		void updateShadowMap(Scene* scene, Camera* camera);
		void clearStages();
		void clearBuffer();
		void resetViewport(Rectangle* viewport);
	};

}
#endif