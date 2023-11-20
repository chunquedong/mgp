#ifndef RENDERSTATEBLOCK_H_
#define RENDERSTATEBLOCK_H_

#include "base/Base.h"
#include "base/Ref.h"

namespace mgp
{

    /**
     * Defines a block of fixed-function render states that can be applied to a
     * RenderState object.
     */
    class StateBlock
    {
    public:
        /**
         * Defines blend constants supported by the blend function.
         */
        enum Blend
        {
            BLEND_ZERO = 0,
            BLEND_ONE = 1,
            BLEND_SRC_COLOR = 0x0300,
            BLEND_ONE_MINUS_SRC_COLOR = 0x0301,
            BLEND_DST_COLOR = 0x0306,
            BLEND_ONE_MINUS_DST_COLOR = 0x0307,
            BLEND_SRC_ALPHA = 0x0302,
            BLEND_ONE_MINUS_SRC_ALPHA = 0x0303,
            BLEND_DST_ALPHA = 0x0304,
            BLEND_ONE_MINUS_DST_ALPHA = 0x0305,
            BLEND_CONSTANT_ALPHA = 0x8003,
            BLEND_ONE_MINUS_CONSTANT_ALPHA = 0x8004,
            BLEND_SRC_ALPHA_SATURATE = 0x0308
        };

        /**
         * Defines the supported depth compare functions.
         *
         * Depth compare functions specify the comparison that takes place between the
         * incoming pixel's depth value and the depth value already in the depth buffer.
         * If the compare function passes, the new pixel will be drawn.
         *
         * The intial depth compare function is DEPTH_LESS.
         */
        enum DepthFunction
        {
            DEPTH_NEVER = 0x0200,
            DEPTH_LESS = 0x0201,
            DEPTH_EQUAL = 0x0202,
            DEPTH_LEQUAL = 0x0203,
            DEPTH_GREATER = 0x0204,
            DEPTH_NOTEQUAL = 0x0205,
            DEPTH_GEQUAL = 0x0206,
            DEPTH_ALWAYS = 0x0207
        };

        /**
         * Defines culling criteria for front-facing, back-facing and both-side
         * facets.
         */
        enum CullFaceSide
        {
            CULL_FACE_SIDE_BACK = 0x0405,
            CULL_FACE_SIDE_FRONT = 0x0404,
            CULL_FACE_SIDE_FRONT_AND_BACK = 0x0408
        };

        /**
         * Defines the winding of vertices in faces that are considered front facing.
         *
         * The initial front face mode is set to FRONT_FACE_CCW.
         */
        enum FrontFace
        {
            FRONT_FACE_CW = 0x0900,
            FRONT_FACE_CCW = 0x0901
        };

        /**
         * Defines the supported stencil compare functions.
         *
         * Stencil compare functions determine if a new pixel will be drawn.
         *
         * The initial stencil compare function is STENCIL_ALWAYS.
         */
        enum StencilFunction
        {
            STENCIL_NEVER = 0x0200,
            STENCIL_ALWAYS = 0x0207,
            STENCIL_LESS = 0x0201,
            STENCIL_LEQUAL = 0x0203,
            STENCIL_EQUAL = 0x0202,
            STENCIL_GREATER = 0x0204,
            STENCIL_GEQUAL = 0x0206,
            STENCIL_NOTEQUAL = 0x0205
        };

        /**
         * Defines the supported stencil operations to perform.
         *
         * Stencil operations determine what should happen to the pixel if the
         * stencil test fails, passes, or passes but fails the depth test.
         *
         * The initial stencil operation is STENCIL_OP_KEEP.
         */
        enum StencilOperation
        {
            STENCIL_OP_KEEP = 0x1E00,
            STENCIL_OP_ZERO = 0,
            STENCIL_OP_REPLACE = 0x1E01,
            STENCIL_OP_INCR = 0x1E02,
            STENCIL_OP_DECR = 0x1E03,
            STENCIL_OP_INVERT = 0x150A,
            STENCIL_OP_INCR_WRAP = 0x8507,
            STENCIL_OP_DECR_WRAP = 0x8508
        };


    public:

        /**
         * Binds the state in this StateBlock to the renderer.
         *
         * This method handles both setting and restoring of render states to ensure that
         * only the state explicitly defined by this StateBlock is applied to the renderer.
         */
        void bind(int force = 1);

        /**
         * Toggles blending.
         *
          * @param enabled true to enable, false to disable.
         */
        void setBlend(bool enabled);

        /**
         * Explicitly sets the source used in the blend function for this render state.
         *
         * Note that the blend function is only applied when blending is enabled.
         *
         * @param blend Specifies how the source blending factors are computed.
         */
        void setBlendSrc(Blend blend);
        void setBlendSrcAlpha(Blend blend);

        /**
         * Explicitly sets the source used in the blend function for this render state.
         *
         * Note that the blend function is only applied when blending is enabled.
         *
         * @param blend Specifies how the destination blending factors are computed.
         */
        void setBlendDst(Blend blend);
        void setBlendDstAlpha(Blend blend);

        /**
         * Explicitly enables or disables backface culling.
         *
         * @param enabled true to enable, false to disable.
         */
        void setCullFace(bool enabled);

        /**
         * Sets the side of the facets to cull.
         *
         * When not explicitly set, the default is to cull back-facing facets.
         *
         * @param side The side to cull.
         */
        void setCullFaceSide(CullFaceSide side);

        /**
         * Sets the winding for front facing polygons.
         *
         * By default, counter-clockwise wound polygons are considered front facing.
         *
         * @param winding The winding for front facing polygons.
         */
        void setFrontFace(FrontFace winding);

        /**
         * Toggles depth testing.
         *
         * By default, depth testing is disabled.
         *
         * @param enabled true to enable, false to disable.
         */
        void setDepthTest(bool enabled);

        /**
         * Toggles depth writing.
         *
         * @param enabled true to enable, false to disable.
         */
        void setDepthWrite(bool enabled);

        /**
         * Sets the depth function to use when depth testing is enabled.
         *
         * When not explicitly set and when depth testing is enabled, the default
         * depth function is DEPTH_LESS.
         *
         * @param func The depth function.
         */
        void setDepthFunction(DepthFunction func);

        /**
         * Toggles stencil testing.
         *
         * By default, stencil testing is disabled.
         *
         * @param enabled true to enable, false to disable.
         */
        void setStencilTest(bool enabled);

        /**
         * Sets the stencil writing mask.
         *
         * By default, the stencil writing mask is all 1's.
         *
         * @param mask Bit mask controlling writing to individual stencil planes.
         */
        void setStencilWrite(unsigned int mask);

        /**
         * Sets the stencil function.
         *
         * By default, the function is set to STENCIL_ALWAYS, the reference value is 0, and the mask is all 1's.
         *
         * @param func The stencil function.
         * @param ref The stencil reference value.
         * @param mask The stencil mask.
         */
        void setStencilFunction(StencilFunction func, int ref, unsigned int mask);

        /**
         * Sets the stencil operation.
         *
         * By default, stencil fail, stencil pass/depth fail, and stencil and depth pass are set to STENCIL_OP_KEEP.
         *
         * @param sfail The stencil operation if the stencil test fails.
         * @param dpfail The stencil operation if the stencil test passes, but the depth test fails.
         * @param dppass The stencil operation if both the stencil test and depth test pass.
         */
        void setStencilOperation(StencilOperation sfail, StencilOperation dpfail, StencilOperation dppass);

        void setPolygonOffset(bool enable, float factor, float units);

        /**
         * Sets a render state from the given name and value strings.
         *
         * This method attempts to interpret the passed in strings as render state
         * name and value. This is normally used when loading render states from
         * material files.
         *
         * @param name Name of the render state to set.
         * @param value Value of the specified render state.
         */
        void setState(const char* name, const char* value);

    //private:

        /**
         * Constructor.
         */
        StateBlock();

        /**
         * Copy constructor.
         */
        StateBlock(const StateBlock& copy);

        /**
         * Destructor.
         */
        ~StateBlock();
    public:
        void cloneInto(StateBlock* state) const;
    private:
        friend class GLRenderer;
        // States
        bool _cullFaceEnabled;
        bool _depthTestEnabled;
        bool _depthWriteEnabled;
        DepthFunction _depthFunction;
        bool _blendEnabled;
        Blend _blendSrc;
        Blend _blendDst;
        Blend _blendSrcAlpha;
        Blend _blendDstAlpha;
        CullFaceSide _cullFaceSide;
        FrontFace _frontFace;
        bool _stencilTestEnabled;
        unsigned int _stencilWrite;
        StencilFunction _stencilFunction;
        int _stencilFunctionRef;
        unsigned int _stencilFunctionMask;
        StencilOperation _stencilOpSfail;
        StencilOperation _stencilOpDpfail;
        StencilOperation _stencilOpDppass;
        
        bool _polygonOffset;
        float _offsetFactor;
        float _offsetUnits;

// Render state override bits
#define RS_BLEND 1
#define RS_BLEND_FUNC 2
#define RS_CULL_FACE 4
#define RS_DEPTH_TEST 8
#define RS_DEPTH_WRITE 16
#define RS_DEPTH_FUNC 32
#define RS_CULL_FACE_SIDE 64
#define RS_STENCIL_TEST 128
#define RS_STENCIL_WRITE 256
#define RS_STENCIL_FUNC 512
#define RS_STENCIL_OP 1024
#define RS_FRONT_FACE 2048
#define RS_POLYGON_OFFSET 4096

#define RS_ALL_ONES 0xFFFFFFFF
        long _bits;
    };
}

#endif