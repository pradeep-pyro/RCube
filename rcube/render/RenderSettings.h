#ifndef RENDERSETTINGS_H
#define RENDERSETTINGS_H

#include "glad/glad.h"

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


struct RenderSettings {
    bool culling = false;
    Cull cull_mode = Cull::Back;
    bool depth_test = true;
    bool depth_write = true;
    bool blending = false;
    BlendFunc blendfunc_src, blendfunc_dst;
};

#endif // RENDERSETTINGS_H
