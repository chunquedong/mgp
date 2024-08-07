#include "StateBlock.h"
#include "../scene/Renderer.h"

#include <algorithm>

using namespace mgp;

#define RS_ALL_ONES 0xFFFFFFFF

static bool parseBoolean(const char* value)
{
    GP_ASSERT(value);

    if (strlen(value) == 4)
    {
        return (
            tolower(value[0]) == 't' &&
            tolower(value[1]) == 'r' &&
            tolower(value[2]) == 'u' &&
            tolower(value[3]) == 'e');
    }

    return false;
}

static int parseInt(const char* value)
{
    GP_ASSERT(value);

    int rValue;
    int scanned = sscanf(value, "%d", &rValue);
    if (scanned != 1)
    {
        GP_ERROR("Error attempting to parse int '%s'. (Will default to 0 if errors are treated as warnings)", value);
        return 0;
    }
    return rValue;
}

static unsigned int parseUInt(const char* value)
{
    GP_ASSERT(value);

    unsigned int rValue;
    int scanned = sscanf(value, "%u", &rValue);
    if (scanned != 1)
    {
        GP_ERROR("Error attempting to parse unsigned int '%s'. (Will default to 0 if errors are treated as warnings)", value);
        return 0;
    }
    return rValue;
}

static StateBlock::Blend parseBlend(const char* value)
{
    GP_ASSERT(value);

    // Convert the string to uppercase for comparison.
    std::string upper(value);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
    if (upper == "ZERO")
        return StateBlock::BLEND_ZERO;
    else if (upper == "ONE")
        return StateBlock::BLEND_ONE;
    else if (upper == "SRC_COLOR")
        return StateBlock::BLEND_SRC_COLOR;
    else if (upper == "ONE_MINUS_SRC_COLOR")
        return StateBlock::BLEND_ONE_MINUS_SRC_COLOR;
    else if (upper == "DST_COLOR")
        return StateBlock::BLEND_DST_COLOR;
    else if (upper == "ONE_MINUS_DST_COLOR")
        return StateBlock::BLEND_ONE_MINUS_DST_COLOR;
    else if (upper == "SRC_ALPHA")
        return StateBlock::BLEND_SRC_ALPHA;
    else if (upper == "ONE_MINUS_SRC_ALPHA")
        return StateBlock::BLEND_ONE_MINUS_SRC_ALPHA;
    else if (upper == "DST_ALPHA")
        return StateBlock::BLEND_DST_ALPHA;
    else if (upper == "ONE_MINUS_DST_ALPHA")
        return StateBlock::BLEND_ONE_MINUS_DST_ALPHA;
    else if (upper == "CONSTANT_ALPHA")
        return StateBlock::BLEND_CONSTANT_ALPHA;
    else if (upper == "ONE_MINUS_CONSTANT_ALPHA")
        return StateBlock::BLEND_ONE_MINUS_CONSTANT_ALPHA;
    else if (upper == "SRC_ALPHA_SATURATE")
        return StateBlock::BLEND_SRC_ALPHA_SATURATE;
    else
    {
        GP_ERROR("Unsupported blend value (%s). (Will default to BLEND_ONE if errors are treated as warnings)", value);
        return StateBlock::BLEND_ONE;
    }
}

static StateBlock::DepthFunction parseDepthFunc(const char* value)
{
    GP_ASSERT(value);

    // Convert string to uppercase for comparison
    std::string upper(value);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
    if (upper == "NEVER")
        return StateBlock::DEPTH_NEVER;
    else if (upper == "LESS")
        return StateBlock::DEPTH_LESS;
    else if (upper == "EQUAL")
        return StateBlock::DEPTH_EQUAL;
    else if (upper == "LEQUAL")
        return StateBlock::DEPTH_LEQUAL;
    else if (upper == "GREATER")
        return StateBlock::DEPTH_GREATER;
    else if (upper == "NOTEQUAL")
        return StateBlock::DEPTH_NOTEQUAL;
    else if (upper == "GEQUAL")
        return StateBlock::DEPTH_GEQUAL;
    else if (upper == "ALWAYS")
        return StateBlock::DEPTH_ALWAYS;
    else
    {
        GP_ERROR("Unsupported depth function value (%s). Will default to DEPTH_LESS if errors are treated as warnings)", value);
        return StateBlock::DEPTH_LESS;
    }
}

static StateBlock::CullFaceSide parseCullFaceSide(const char* value)
{
    GP_ASSERT(value);

    // Convert string to uppercase for comparison
    std::string upper(value);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
    if (upper == "BACK")
        return StateBlock::CULL_FACE_SIDE_BACK;
    else if (upper == "FRONT")
        return StateBlock::CULL_FACE_SIDE_FRONT;
    else if (upper == "FRONT_AND_BACK")
        return StateBlock::CULL_FACE_SIDE_FRONT_AND_BACK;
    else
    {
        GP_ERROR("Unsupported cull face side value (%s). Will default to BACK if errors are treated as warnings.", value);
        return StateBlock::CULL_FACE_SIDE_BACK;
    }
}

static StateBlock::FrontFace parseFrontFace(const char* value)
{
    GP_ASSERT(value);

    // Convert string to uppercase for comparison
    std::string upper(value);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
    if (upper == "CCW")
        return StateBlock::FRONT_FACE_CCW;
    else if (upper == "CW")
        return StateBlock::FRONT_FACE_CW;
    else
    {
        GP_ERROR("Unsupported front face side value (%s). Will default to CCW if errors are treated as warnings.", value);
        return StateBlock::FRONT_FACE_CCW;
    }
}

//static StateBlock::StencilFunction parseStencilFunc(const char* value)
//{
//    GP_ASSERT(value);
//
//    // Convert string to uppercase for comparison
//    std::string upper(value);
//    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
//    if (upper == "NEVER")
//        return StateBlock::STENCIL_NEVER;
//    else if (upper == "LESS")
//        return StateBlock::STENCIL_LESS;
//    else if (upper == "EQUAL")
//        return StateBlock::STENCIL_EQUAL;
//    else if (upper == "LEQUAL")
//        return StateBlock::STENCIL_LEQUAL;
//    else if (upper == "GREATER")
//        return StateBlock::STENCIL_GREATER;
//    else if (upper == "NOTEQUAL")
//        return StateBlock::STENCIL_NOTEQUAL;
//    else if (upper == "GEQUAL")
//        return StateBlock::STENCIL_GEQUAL;
//    else if (upper == "ALWAYS")
//        return StateBlock::STENCIL_ALWAYS;
//    else
//    {
//        GP_ERROR("Unsupported stencil function value (%s). Will default to STENCIL_ALWAYS if errors are treated as warnings)", value);
//        return StateBlock::STENCIL_ALWAYS;
//    }
//}

static StateBlock::StencilOperation parseStencilOp(const char* value)
{
    GP_ASSERT(value);

    // Convert string to uppercase for comparison
    std::string upper(value);
    std::transform(upper.begin(), upper.end(), upper.begin(), (int(*)(int))toupper);
    if (upper == "KEEP")
        return StateBlock::STENCIL_OP_KEEP;
    else if (upper == "ZERO")
        return StateBlock::STENCIL_OP_ZERO;
    else if (upper == "REPLACE")
        return StateBlock::STENCIL_OP_REPLACE;
    else if (upper == "INCR")
        return StateBlock::STENCIL_OP_INCR;
    else if (upper == "DECR")
        return StateBlock::STENCIL_OP_DECR;
    else if (upper == "INVERT")
        return StateBlock::STENCIL_OP_INVERT;
    else if (upper == "INCR_WRAP")
        return StateBlock::STENCIL_OP_INCR_WRAP;
    else if (upper == "DECR_WRAP")
        return StateBlock::STENCIL_OP_DECR_WRAP;
    else
    {
        GP_ERROR("Unsupported stencil operation value (%s). Will default to STENCIL_OP_KEEP if errors are treated as warnings)", value);
        return StateBlock::STENCIL_OP_KEEP;
    }
}

void StateBlock::setState(const char* name, const char* value)
{
    GP_ASSERT(name);

    if (strcmp(name, "blend") == 0)
    {
        setBlend(parseBoolean(value));
    }
    else if (strcmp(name, "blendSrc") == 0 || strcmp(name, "srcBlend") == 0)   // Leaving srcBlend for backward compat.
    {
        setBlendSrc(parseBlend(value));
    }
    else if (strcmp(name, "blendDst") == 0 || strcmp(name, "dstBlend") == 0)    // // Leaving dstBlend for backward compat.
    {
        setBlendDst(parseBlend(value));
    }
    else if (strcmp(name, "cullFace") == 0)
    {
        setCullFace(parseBoolean(value));
    }
    else if (strcmp(name, "cullFaceSide") == 0)
    {
        setCullFaceSide(parseCullFaceSide(value));
    }
    else if (strcmp(name, "frontFace") == 0)
    {
        setFrontFace(parseFrontFace(value));
    }
    else if (strcmp(name, "depthTest") == 0)
    {
        setDepthTest(parseBoolean(value));
    }
    else if (strcmp(name, "depthWrite") == 0)
    {
        setDepthWrite(parseBoolean(value));
    }
    else if (strcmp(name, "depthFunc") == 0)
    {
        setDepthFunction(parseDepthFunc(value));
    }
    else if (strcmp(name, "stencilTest") == 0)
    {
        setStencilTest(parseBoolean(value));
    }
    else if (strcmp(name, "stencilWrite") == 0)
    {
        setStencilWrite(parseUInt(value));
    }
    else if (strcmp(name, "stencilFunc") == 0)
    {
        setStencilFunction(parseDepthFunc(value), _stencilFunctionRef, _stencilFunctionMask);
    }
    else if (strcmp(name, "stencilFuncRef") == 0)
    {
        setStencilFunction(_stencilFunction, parseInt(value), _stencilFunctionMask);
    }
    else if (strcmp(name, "stencilFuncMask") == 0)
    {
        setStencilFunction(_stencilFunction, _stencilFunctionRef, parseUInt(value));
    }
    else if (strcmp(name, "stencilOpSfail") == 0)
    {
        setStencilOperation(parseStencilOp(value), _stencilOpDpfail, _stencilOpDppass);
    }
    else if (strcmp(name, "stencilOpDpfail") == 0)
    {
        setStencilOperation(_stencilOpSfail, parseStencilOp(value), _stencilOpDppass);
    }
    else if (strcmp(name, "stencilOpDppass") == 0)
    {
        setStencilOperation(_stencilOpSfail, _stencilOpDpfail, parseStencilOp(value));
    }
    else
    {
        GP_ERROR("Unsupported render state string '%s'.", name);
    }
}

void StateBlock::setBlend(bool enabled)
{
    _blendEnabled = enabled;
    //_bits |= RS_BLEND;
}

void StateBlock::setBlendSrc(Blend blend)
{
    _blendSrc = blend;
    //_bits |= RS_BLEND_FUNC;
}

void StateBlock::setBlendDst(Blend blend)
{
    _blendDst = blend;
    //_bits |= RS_BLEND_FUNC;
}

void StateBlock::setBlendSrcAlpha(Blend blend)
{
    _blendSrcAlpha = blend;
    //_bits |= RS_BLEND_FUNC;
}

void StateBlock::setBlendDstAlpha(Blend blend)
{
    _blendDstAlpha = blend;
    //_bits |= RS_BLEND_FUNC;
}

void StateBlock::setCullFace(bool enabled)
{
    _cullFaceEnabled = enabled;
    //_bits |= RS_CULL_FACE;
}

bool StateBlock::cullFaceEnabled() {
    return _cullFaceEnabled;
}

void StateBlock::setCullFaceSide(CullFaceSide side)
{
    _cullFaceSide = side;
    //_bits |= RS_CULL_FACE_SIDE;
}

void StateBlock::setFrontFace(FrontFace winding)
{
    _frontFace = winding;
    //_bits |= RS_FRONT_FACE;
}

void StateBlock::setDepthTest(bool enabled)
{
    _depthTestEnabled = enabled;
    //_bits |= RS_DEPTH_TEST;
}

void StateBlock::setDepthWrite(bool enabled)
{
    _depthWriteEnabled = enabled;
    //_bits |= RS_DEPTH_WRITE;
}

void StateBlock::setDepthFunction(DepthFunction func)
{
    _depthFunction = func;
    //_bits |= RS_DEPTH_FUNC;
}

void StateBlock::setStencilTest(bool enabled)
{
    _stencilTestEnabled = enabled;
    //_bits |= RS_STENCIL_TEST;
}

void StateBlock::setStencilWrite(unsigned int mask)
{
    _stencilWrite = mask;
    //_bits |= RS_STENCIL_WRITE;
}

void StateBlock::setStencilFunction(DepthFunction func, int ref, unsigned int mask)
{
    _stencilFunction = func;
    _stencilFunctionRef = ref;
    _stencilFunctionMask = mask;
    //_bits |= RS_STENCIL_FUNC;
}

void StateBlock::setStencilOperation(StencilOperation sfail, StencilOperation dpfail, StencilOperation dppass)
{
    _stencilOpSfail = sfail;
    _stencilOpDpfail = dpfail;
    _stencilOpDppass = dppass;
    //_bits |= RS_STENCIL_OP;
}

void StateBlock::setPolygonOffset(bool enable, float factor, float units)
{
    _polygonOffset = enable;
    _offsetFactor = factor;
    _offsetUnits = units;
    //_bits |= RS_POLYGON_OFFSET;
}

StateBlock::StateBlock()
    : _cullFaceEnabled(true), _depthTestEnabled(true), _depthWriteEnabled(true), _depthFunction(StateBlock::DEPTH_LESS),
    _blendEnabled(false), _blendSrc(StateBlock::BLEND_SRC_ALPHA), _blendDst(BLEND_ONE_MINUS_SRC_ALPHA), _blendSrcAlpha(BLEND_ONE), _blendDstAlpha(BLEND_ONE_MINUS_SRC_ALPHA),
    _cullFaceSide(CULL_FACE_SIDE_BACK), _frontFace(FRONT_FACE_CCW), _stencilTestEnabled(false), _stencilWrite(RS_ALL_ONES),
    _stencilFunction(StateBlock::DEPTH_ALWAYS), _stencilFunctionRef(0), _stencilFunctionMask(RS_ALL_ONES),
    _stencilOpSfail(StateBlock::STENCIL_OP_KEEP), _stencilOpDpfail(StateBlock::STENCIL_OP_KEEP), _stencilOpDppass(StateBlock::STENCIL_OP_KEEP), //_bits(0L),
    _polygonOffset(false), _offsetFactor(0), _offsetUnits(0)
{
}

StateBlock::StateBlock(const StateBlock& copy)
{
    // Hidden
}

StateBlock::~StateBlock()
{
}

void StateBlock::bind(int force)
{
    Renderer::cur()->updateState(this, force);
}

void StateBlock::cloneInto(StateBlock* state) const
{
    GP_ASSERT(state);

    state->_cullFaceEnabled = _cullFaceEnabled;
    state->_depthTestEnabled = _depthTestEnabled;
    state->_depthWriteEnabled = _depthWriteEnabled;
    state->_depthFunction = _depthFunction;
    state->_blendEnabled = _blendEnabled;
    state->_blendSrc = _blendSrc;
    state->_blendDst = _blendDst;
    state->_blendSrcAlpha = _blendSrcAlpha;
    state->_blendDstAlpha = _blendDstAlpha;
    state->_cullFaceSide = _cullFaceSide;
    state->_frontFace = _frontFace;
    state->_stencilTestEnabled = _stencilTestEnabled;
    state->_stencilWrite = _stencilWrite;
    state->_stencilFunction = _stencilFunction;
    state->_stencilFunctionRef = _stencilFunctionRef;
    state->_stencilFunctionMask = _stencilFunctionMask;
    state->_stencilOpSfail = _stencilOpSfail;
    state->_stencilOpDpfail = _stencilOpDpfail;
    state->_stencilOpDppass = _stencilOpDppass;
    state->_polygonOffset = _polygonOffset;
    state->_offsetFactor = _offsetFactor;
    state->_offsetUnits = _offsetUnits;
    //state->_bits = _bits;
}

std::string StateBlock::enumToString(const std::string& enumName, int value) {
    if (enumName == "mgp::StateBlock::DepthFunction") {
        switch (value)
        {
        case StateBlock::DEPTH_NEVER:
            return "NEVER";
        case StateBlock::DEPTH_LESS:
            return "LESS";
        case StateBlock::DEPTH_EQUAL:
            return "EQUAL";
        case StateBlock::DEPTH_LEQUAL:
            return "LEQUAL";
        case StateBlock::DEPTH_GREATER:
            return "GREATER";
        case StateBlock::DEPTH_NOTEQUAL:
            return "NOTEQUAL";
        case StateBlock::DEPTH_GEQUAL:
            return "GEQUAL";
        case StateBlock::DEPTH_ALWAYS:
            return "ALWAYS";
        default:
            return "LESS";
        }
    }
    else if (enumName == "mgp::StateBlock::Blend") {
        switch (value)
        {
        case StateBlock::BLEND_ZERO:
            return "ZERO";
        case StateBlock::BLEND_ONE:
            return "ONE";
        case StateBlock::BLEND_SRC_COLOR:
            return "SRC_COLOR";
        case StateBlock::BLEND_ONE_MINUS_SRC_COLOR:
            return "ONE_MINUS_SRC_COLOR";
        case StateBlock::BLEND_DST_COLOR:
            return "DST_COLOR";
        case StateBlock::BLEND_ONE_MINUS_DST_COLOR:
            return "ONE_MINUS_DST_COLOR";
        case StateBlock::BLEND_SRC_ALPHA:
            return "SRC_ALPHA";
        case StateBlock::BLEND_ONE_MINUS_SRC_ALPHA:
            return "ONE_MINUS_SRC_ALPHA";
        case StateBlock::BLEND_DST_ALPHA:
            return "DST_ALPHA";
        case StateBlock::BLEND_ONE_MINUS_DST_ALPHA:
            return "ONE_MINUS_DST_ALPHA";
        case StateBlock::BLEND_CONSTANT_ALPHA:
            return "CONSTANT_ALPHA";
        case StateBlock::BLEND_ONE_MINUS_CONSTANT_ALPHA:
            return "ONE_MINUS_CONSTANT_ALPHA";
        case StateBlock::BLEND_SRC_ALPHA_SATURATE:
            return "SRC_ALPHA_SATURATE";
        default:
            return "SRC_ALPHA";
        }
    }
    else if (enumName == "mgp::StateBlock::CullFaceSide") {
        switch (value)
        {
        case CullFaceSide::CULL_FACE_SIDE_BACK:
            return "BACK";
        case CullFaceSide::CULL_FACE_SIDE_FRONT:
            return "FRONT";
        case CullFaceSide::CULL_FACE_SIDE_FRONT_AND_BACK:
            return "FRONT_AND_BACK";
        default:
            return "BACK";
        }
    }
    else if (enumName == "mgp::StateBlock::FrontFace") {
        switch (value)
        {
        case FrontFace::FRONT_FACE_CW:
            return "CW";
        case FrontFace::FRONT_FACE_CCW:
            return "CCW";
        default:
            return "CCW";
        }
    }
    else if (enumName == "mgp::StateBlock::StencilOperation") {
        switch (value)
        {
        case StateBlock::STENCIL_OP_KEEP:
            return "KEEP";
        case StateBlock::STENCIL_OP_ZERO:
            return "ZERO";
        case StateBlock::STENCIL_OP_REPLACE:
            return "REPLACE";
        case StateBlock::STENCIL_OP_INCR:
            return "INCR";
        case StateBlock::STENCIL_OP_DECR:
            return "DECR";
        case StateBlock::STENCIL_OP_INVERT:
            return "INVERT";
        case StateBlock::STENCIL_OP_INCR_WRAP:
            return "INCR_WRAP";
        case StateBlock::STENCIL_OP_DECR_WRAP:
            return "DECR_WRAP";
        default:
            return "KEEP";
        }
    }
    return "";
}

int StateBlock::enumParse(const std::string& enumName, const std::string& str) {
    if (enumName == "mgp::StateBlock::DepthFunction") {
        return parseDepthFunc(str.c_str());
    }
    else if (enumName == "mgp::StateBlock::Blend") {
        return parseBlend(str.c_str());
    }
    else if (enumName == "mgp::StateBlock::CullFaceSide") {
        return parseCullFaceSide(str.c_str());
    }
    else if (enumName == "mgp::StateBlock::FrontFace") {
        return parseFrontFace(str.c_str());
    }
    else if (enumName == "mgp::StateBlock::StencilOperation") {
        return parseStencilOp(str.c_str());
    }
    return 0;
}

void StateBlock::onSerialize(Serializer* serializer) {
    serializer->writeBool("cullFaceEnabled", _cullFaceEnabled, true);
    serializer->writeBool("depthTestEnabled", _depthTestEnabled, true);
    serializer->writeBool("depthWriteEnabled", _depthWriteEnabled, true);
    serializer->writeEnum("depthFunction", "mgp::StateBlock::DepthFunction", _depthFunction, -1);
    serializer->writeBool("blendEnabled", _blendEnabled, false);
    serializer->writeEnum("blendSrc", "mgp::StateBlock::Blend", _blendSrc, -1);
    serializer->writeEnum("blendDst", "mgp::StateBlock::Blend", _blendDst, -1);
    serializer->writeEnum("blendSrcAlpha", "mgp::StateBlock::Blend", _blendSrcAlpha, -1);
    serializer->writeEnum("blendDstAlpha", "mgp::StateBlock::Blend", _blendDstAlpha, -1);
    serializer->writeEnum("cullFaceSide", "mgp::StateBlock::CullFaceSide", _cullFaceSide, CULL_FACE_SIDE_BACK);
    serializer->writeEnum("frontFace", "mgp::StateBlock::FrontFace", _frontFace, FRONT_FACE_CCW);

    serializer->writeBool("stencilTestEnabled", _stencilTestEnabled, false);
    serializer->writeInt("stencilWrite", _stencilWrite, RS_ALL_ONES);
    serializer->writeEnum("stencilFunction", "mgp::StateBlock::DepthFunction", _stencilFunction, StateBlock::DEPTH_ALWAYS);
    serializer->writeInt("stencilFunctionRef", _stencilFunctionRef, 0);
    serializer->writeInt("stencilFunctionMask", _stencilFunctionMask, RS_ALL_ONES);
    serializer->writeEnum("stencilOpSfail", "mgp::StateBlock::StencilOperation", _stencilOpSfail, StateBlock::STENCIL_OP_KEEP);
    serializer->writeEnum("stencilOpDpfail", "mgp::StateBlock::StencilOperation", _stencilOpDpfail, StateBlock::STENCIL_OP_KEEP);
    serializer->writeEnum("stencilOpDppass", "mgp::StateBlock::StencilOperation", _stencilOpDppass, StateBlock::STENCIL_OP_KEEP);

    serializer->writeBool("polygonOffset", _polygonOffset, false);
    serializer->writeInt("offsetFactor", _offsetFactor, 0);
    serializer->writeInt("offsetUnits", _offsetUnits, 0);
}

void StateBlock::onDeserialize(Serializer* serializer) {
    _cullFaceEnabled = serializer->readBool("cullFaceEnabled", true);
    _depthTestEnabled = serializer->readBool("depthTestEnabled", true);
    _depthWriteEnabled = serializer->readBool("depthWriteEnabled", true);
    _depthFunction = (StateBlock::DepthFunction)serializer->readEnum("depthFunction", "mgp::StateBlock::DepthFunction", -1);
    _blendEnabled = serializer->readBool("blendEnabled", false);
    _blendSrc = (StateBlock::Blend)serializer->readEnum("blendSrc", "mgp::StateBlock::Blend", -1);
    _blendDst = (StateBlock::Blend)serializer->readEnum("blendDst", "mgp::StateBlock::Blend", -1);
    _blendSrcAlpha = (StateBlock::Blend)serializer->readEnum("blendSrcAlpha", "mgp::StateBlock::Blend", -1);
    _blendDstAlpha = (StateBlock::Blend)serializer->readEnum("blendDstAlpha", "mgp::StateBlock::Blend", -1);
    _cullFaceSide = (StateBlock::CullFaceSide)serializer->readEnum("cullFaceSide", "mgp::StateBlock::CullFaceSide", CULL_FACE_SIDE_BACK);
    _frontFace = (StateBlock::FrontFace)serializer->readEnum("frontFace", "mgp::StateBlock::FrontFace", FRONT_FACE_CCW);

    _stencilTestEnabled = serializer->readBool("stencilTestEnabled", false);
    _stencilWrite = serializer->readInt("stencilWrite", RS_ALL_ONES);
    _stencilFunction = (StateBlock::DepthFunction)serializer->readEnum("stencilFunction", "mgp::StateBlock::DepthFunction", StateBlock::DEPTH_ALWAYS);
    _stencilFunctionRef = serializer->readInt("stencilFunctionRef", 0);
    _stencilFunctionMask = serializer->readInt("stencilFunctionMask", RS_ALL_ONES);
    _stencilOpSfail = (StateBlock::StencilOperation)serializer->readEnum("stencilOpSfail", "mgp::StateBlock::StencilOperation", StateBlock::STENCIL_OP_KEEP);
    _stencilOpDpfail = (StateBlock::StencilOperation)serializer->readEnum("stencilOpDpfail", "mgp::StateBlock::StencilOperation", StateBlock::STENCIL_OP_KEEP);
    _stencilOpDppass = (StateBlock::StencilOperation)serializer->readEnum("stencilOpDppass", "mgp::StateBlock::StencilOperation", StateBlock::STENCIL_OP_KEEP);

    _polygonOffset = serializer->readBool("polygonOffset", false);
    _offsetFactor = serializer->readInt("offsetFactor", 0);
    _offsetUnits = serializer->readInt("offsetUnits", 0);
}