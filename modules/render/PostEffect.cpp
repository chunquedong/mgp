
#include "PostEffect.h"

#include <random>

using namespace mgp;

///////////////////////////////////////////////////////////////////
// SSAO

static float lerp(float a, float b, float f)
{
    return a + f * (b - a);
}


SSAO::SSAO(RenderPath* renderPath) {
    RenderPass* ssao = new RenderPass();
    ssao->_renderPath = renderPath;
    ssao->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/ssao.frag");
    ssao->_drawType = -1;
    ssao->_inputTextureBuffers["u_texture"] = "main.1";
    ssao->_newDstBufferSize = 0.5;
    ssao->_newDstBufferFormat = Texture::RED;
    ssao->_dstBufferName = "ssao";
    _passGroup.push_back(ssao);

    RenderPass* post = new RenderPass();
    post->_renderPath = renderPath;
    post->_inputTextureBuffers["u_texture"] = "ssao.0";
    post->_newDstBufferSize = 0.5;
    post->_newDstBufferFormat = Texture::RED;
    post->_dstBufferName = "ssaoBlur";
    post->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/blurOne.frag");
    _passGroup.push_back(post);

    RenderPass* merge = new RenderPass();
    merge->_renderPath = renderPath;
    merge->_inputTextureBuffers["u_main"] = "main.0";
    merge->_inputTextureBuffers["u_ssao"] = "ssaoBlur.0";
    merge->_dstBufferName = "main";
    merge->_clearBuffer = 0;
    merge->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/ssaoMerge.frag");
    merge->_material->getStateBlock()->setDepthTest(false);
    _passGroup.push_back(merge);

    /*RenderPass* merge = new RenderPass();
    merge->_renderPath = renderPath;
    merge->_inputTextureBuffers["u_texture"] = "ssaoBlur.0";
    merge->_dstBufferName = "main";
    merge->_clearBuffer = 0;
    merge->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/depth.frag", "DEPTH_TO_ALPHA");
    merge->_material->getStateBlock()->setBlend(true);
    merge->_material->getStateBlock()->setBlendSrc(StateBlock::BLEND_ZERO);
    merge->_material->getStateBlock()->setBlendDst(StateBlock::BLEND_SRC_ALPHA);
    _passGroup.push_back(merge);*/


    // Sample kernel
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    std::vector<Vector3> ssaoKernel;
    for (int i = 0; i < 12; ++i)
    {
        Vector3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample.normalize();
        sample *= randomFloats(generator);
        float scale = float(i) / 12.0;

        // Scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // Noise texture
    std::vector<float> ssaoNoise;
    for (int i = 0; i < 16; i++)
    {
        Vector3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise.x);
        ssaoNoise.push_back(noise.y);
        ssaoNoise.push_back(noise.z);
    }
    UPtr<Texture> _noiseTexture = Texture::create(Texture::RGB16F, 4, 4, (const unsigned char*)&ssaoNoise[0]);
    /*GLuint noiseTexture; glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

    ssao->_material->getParameter("u_samples")->setVector3Array(ssaoKernel.data(), ssaoKernel.size(), true);
    ssao->_material->getParameter("u_texNoise")->setSampler(_noiseTexture.get());
    //SAFE_RELEASE(_noiseTexture);
}


///////////////////////////////////////////////////////////////////
// bloom

Bloom::Bloom(RenderPath* renderPath) {
    RenderPass* bloom = new RenderPass();
    bloom->_renderPath = renderPath;
    bloom->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/bright.frag");
    bloom->_drawType = -1;
    bloom->_inputTextureBuffers["u_texture"] = "main.0";
    bloom->_dstBufferName = "bloom";
    bloom->_newDstBufferSize = 0.5;
    bloom->_newDstBufferFormat = Texture::RGBA16F;
    bloom->_material->getParameter("u_brightLimit")->setFloat(1.0);
    _passGroup.push_back(bloom);

    //RenderPass* blur = new RenderPass();
    //blur->_renderPath = renderPath;
    //blur->_inputTextureBuffers["u_texture"] = "bloom.0";
    //blur->_dstBufferName = "bloom_blur";
    //blur->_newDstBufferSize = 0.5;
    //blur->_newDstBufferFormat = Texture::RGBA;
    //blur->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/gaussianBlur.frag");
    //blur->_material->getParameter("u_horizontal")->setValue(true);
    //_passGroup.push_back(blur);

    RenderPass* blur2 = new RenderPass();
    blur2->_renderPath = renderPath;
    blur2->_inputTextureBuffers["u_texture"] = "bloom.0";
    blur2->_dstBufferName = "bloom_blur2";
    blur2->_newDstBufferSize = 0.15;
    blur2->_newDstBufferFormat = Texture::RGBA16F;
    blur2->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/gaussianBlur.frag");
    //blur2->_material->getParameter("u_horizontal")->setValue(false);
    _passGroup.push_back(blur2);

    RenderPass* blur3 = new RenderPass();
    blur3->_renderPath = renderPath;
    blur3->_inputTextureBuffers["u_texture"] = "bloom_blur2.0";
    blur3->_dstBufferName = "bloom_blur3";
    blur3->_newDstBufferSize = 0.15;
    blur3->_newDstBufferFormat = Texture::RGBA16F;
    blur3->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/gaussianBlur.frag", "GAUSS_HORIZONTAL");
    //blur3->_material->getParameter("u_horizontal")->setValue(true);
    _passGroup.push_back(blur3);

    //pingpang blur
    for (int i = 0; i < 4; ++i) {

        RenderPass* blur4 = new RenderPass();
        blur4->_renderPath = renderPath;
        blur4->_inputTextureBuffers["u_texture"] = "bloom_blur3.0";
        blur4->_dstBufferName = "bloom_blur2";
        blur4->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/gaussianBlur.frag");
        //blur4->_material->getParameter("u_horizontal")->setValue(false);
        _passGroup.push_back(blur4);

        RenderPass* blur5 = new RenderPass();
        blur5->_renderPath = renderPath;
        blur5->_inputTextureBuffers["u_texture"] = "bloom_blur2.0";
        blur5->_dstBufferName = "bloom_blur3";
        blur5->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/gaussianBlur.frag", "GAUSS_HORIZONTAL");
        //blur5->_material->getParameter("u_horizontal")->setValue(true);
        _passGroup.push_back(blur5);
    }


    RenderPass* merge = new RenderPass();
    merge->_renderPath = renderPath;
    merge->_inputTextureBuffers["u_main"] = "main.0";
    merge->_inputTextureBuffers["u_texture"] = "bloom_blur3.0";
    merge->_dstBufferName = "main";
    merge->_clearBuffer = 0;
    merge->_material = Material::create("res/shaders/postEffect/fullQuad.vert", "res/shaders/postEffect/bloomMerge.frag");
    merge->_material->getStateBlock()->setDepthTest(false);
    _passGroup.push_back(merge);
}
