#ifndef RENDERSETTINGS_H
#define RENDERSETTINGS_H

#include "glad/glad.h"

namespace rcube {

enum RenderPriority {
    Opaque = 0,
    Background = 10,
    Transparent = 20,
    Overlay = 30
};

enum class BlendFunc {
    SrcAlpha = GL_SRC_ALPHA,
    OneMinusSrcAlpha = GL_ONE_MINUS_SRC_ALPHA,
    Zero = GL_ZERO,
    One = GL_ONE,
    SrcColor = GL_SRC_COLOR,
    OneMinusSrcColor = GL_ONE_MINUS_SRC_COLOR,
    DstColor = GL_DST_COLOR,
    OneMinusDstColor = GL_ONE_MINUS_DST_COLOR
};

enum class Cull {
    Back = GL_BACK,
    Front = GL_FRONT,
    Both = GL_FRONT_AND_BACK
};

enum class DepthFunc {
    Less = GL_LESS,   /// Passes if the incoming depth value is less than the stored depth value.
    Equal = GL_EQUAL, /// Passes if the incoming depth value is equal to the stored depth value.
    LessOrEqual = GL_LEQUAL, ///Passes if the incoming depth value is less than or equal to the stored depth value.
    Greater = GL_GREATER, /// Passes if the incoming depth value is greater than the stored depth value.
    NotEqual = GL_NOTEQUAL, ///Passes if the incoming depth value is not equal to the stored depth value.
    GreaterOrEqual = GL_GEQUAL, /// Passes if the incoming depth value is greater than or equal to the stored depth value.
    Never = GL_NEVER, /// Never passes
    Always = GL_ALWAYS, /// Always passes
};

struct RenderSettings {
    bool culling = false;
    Cull cull_mode = Cull::Back;
    bool depth_test = true;
    bool depth_write = true;
    bool blending = false;
    BlendFunc blendfunc_src, blendfunc_dst;
    DepthFunc depthfunc = DepthFunc::Less;
};

} // namespace rcube

#endif // RENDERSETTINGS_H
