#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCube/Systems/RenderSystem.h"
#include "RCubeViewer/Components/ScalarField.h"
#include <string>

namespace rcube
{

    const std::string vert_str =
    R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 5) in float scalar;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

out vec3 v_vertex;
out vec3 v_normal;
out float v_scalar;

void main() {
    vec4 world_vertex = model_matrix * vec4(vertex, 1.0);
    gl_Position = projection_matrix * view_matrix * world_vertex;
    v_vertex = world_vertex.xyz;
    v_normal = normalize(normal_matrix * normal);
    v_scalar = scalar;
}
)";

const std::string frag_str =
    R"(
#version 420

#define MAX_LIGHTS 99

// Interpolated input from vertex shader
in vec3 v_vertex;
in vec3 v_normal;
in float v_scalar;

// Fragment shader output
out vec4 out_color;

// --------------------------------
// Camera Uniform Block
// --------------------------------
layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

// --------------------------------
// Light Uniform Block
// --------------------------------
struct Light {
    vec4 position;
    vec4 direction_radius;
    vec4 color_coneangle;
};

layout (std140, binding=2) uniform Lights {
    Light lights[MAX_LIGHTS];
    int num_lights;
};

uniform float vmin;
uniform float vmax;
uniform int colormap_style;

// Returns the attenuation factor that is multiplied with the light's color
float attenuation(float dist, float radius) {
    return 1.0 / (1.0 + (dist * dist) / (radius * radius));
}

bool close(float a, float b) {
    return abs(a - b) < 0.00001;
}

// Taken from https://github.com/kbinani/colormap-shaders/
vec3 jet(float v) {
    vec3 rgb = vec3(0.0);
    float x = (v - vmin) / (vmax - vmin);
    // x = round(t / 0.2) * 0.2;
    if (x < 0.7) {
        rgb.r = 4.0 * x - 1.5;
    } else {
        rgb.r = -4.0 * x + 4.5;
    }
    if (x < 0.5) {
        rgb.g = 4.0 * x - 0.5;
    } else {
        rgb.g = -4.0 * x + 3.5;
    }
    if (x < 0.3) {
       rgb.b = 4.0 * x + 0.5;
    } else {
       rgb.b = -4.0 * x + 2.5;
    }
    return rgb;
}

// Taken from: https://www.shadertoy.com/view/WlfXRN
vec3 viridis(float v) {
    float t = (v - vmin) / (vmax - vmin);
    //t = round(t / 0.2) * 0.2;
    const vec3 c0 = vec3(0.2777273272234177, 0.005407344544966578, 0.3340998053353061);
    const vec3 c1 = vec3(0.1050930431085774, 1.404613529898575, 1.384590162594685);
    const vec3 c2 = vec3(-0.3308618287255563, 0.214847559468213, 0.09509516302823659);
    const vec3 c3 = vec3(-4.634230498983486, -5.799100973351585, -19.33244095627987);
    const vec3 c4 = vec3(6.228269936347081, 14.17993336680509, 56.69055260068105);
    const vec3 c5 = vec3(4.776384997670288, -13.74514537774601, -65.35303263337234);
    const vec3 c6 = vec3(-5.435455855934631, 4.645852612178535, 26.3124352495832);

    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

// Taken from: https://www.shadertoy.com/view/WlfXRN
vec3 plasma(float v) {
    float t = (v - vmin) / (vmax - vmin);
    //t = round(t / 0.2) * 0.2;
    const vec3 c0 = vec3(0.05873234392399702, 0.02333670892565664, 0.5433401826748754);
    const vec3 c1 = vec3(2.176514634195958, 0.2383834171260182, 0.7539604599784036);
    const vec3 c2 = vec3(-2.689460476458034, -7.455851135738909, 3.110799939717086);
    const vec3 c3 = vec3(6.130348345893603, 42.3461881477227, -28.51885465332158);
    const vec3 c4 = vec3(-11.10743619062271, -82.66631109428045, 60.13984767418263);
    const vec3 c5 = vec3(10.02306557647065, 71.41361770095349, -54.07218655560067);
    const vec3 c6 = vec3(-3.658713842777788, -22.93153465461149, 18.19190778539828);

    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

vec3 magma(float v) {
    float t = (v - vmin) / (vmax - vmin);
    //t = round(t / 0.2) * 0.2;
    const vec3 c0 = vec3(-0.002136485053939582, -0.000749655052795221, -0.005386127855323933);
    const vec3 c1 = vec3(0.2516605407371642, 0.6775232436837668, 2.494026599312351);
    const vec3 c2 = vec3(8.353717279216625, -3.577719514958484, 0.3144679030132573);
    const vec3 c3 = vec3(-27.66873308576866, 14.26473078096533, -13.64921318813922);
    const vec3 c4 = vec3(52.17613981234068, -27.94360607168351, 12.94416944238394);
    const vec3 c5 = vec3(-50.76852536473588, 29.04658282127291, 4.23415299384598);
    const vec3 c6 = vec3(18.65570506591883, -11.48977351997711, -5.601961508734096);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

vec3 inferno(float v) {
    float t = (v - vmin) / (vmax - vmin);
    //t = round(t / 0.2) * 0.2;
    const vec3 c0 = vec3(0.0002189403691192265, 0.001651004631001012, -0.01948089843709184);
    const vec3 c1 = vec3(0.1065134194856116, 0.5639564367884091, 3.932712388889277);
    const vec3 c2 = vec3(11.60249308247187, -3.972853965665698, -15.9423941062914);
    const vec3 c3 = vec3(-41.70399613139459, 17.43639888205313, 44.35414519872813);
    const vec3 c4 = vec3(77.162935699427, -33.40235894210092, -81.80730925738993);
    const vec3 c5 = vec3(-71.31942824499214, 32.62606426397723, 73.20951985803202);
    const vec3 c6 = vec3(25.13112622477341, -12.24266895238567, -23.07032500287172);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

vec3 colormap(float x) {
    vec3 rgb;
    if (colormap_style == 0) {
        rgb = viridis(x);
    }
    else if (colormap_style == 1)
    {
        rgb = plasma(x);
    }
    else if (colormap_style == 2)
    {
        rgb = magma(x);
    }
    else if (colormap_style == 3)
    {
        rgb = inferno(x);
    }
    else {
        rgb = jet(x);
    }
    rgb = clamp(rgb, vec3(0.0), vec3(1.0));
    return rgb;
}

void main() {
    vec3 result = vec3(0.0);
    // Diffuse component
    vec3 material_diffuse = colormap(v_scalar);
    // Surface normal
    vec3 N = v_normal;
    N = normalize(N);
    // Surface to eye
    vec3 V = normalize(vec3(eye_pos - v_vertex)); // Surface to eye

    for (int i = 0; i < min(num_lights, MAX_LIGHTS); ++i)
    {
        vec3 L;          // Surface to light
        float att = 1.0; // Light attenuation

        if (close(lights[i].position.w, 0.0)) { // is directional?
            L = lights[i].position.xyz;
        }
        else {
            L = lights[i].position.xyz - v_vertex;
            // att = attenuation(length(L), lights[i].radius);
            att = attenuation(length(L), lights[i].direction_radius.w);
        }
        L = normalize(L);

        // Accumulate output color using each light source
        vec3 H = normalize(L + V);  // Halfway vector
        float LdotN = dot(L, N);
        float diff_contrib = max(LdotN, 0.0);
        /*float spec_contrib = 0.0;
        if (LdotN > 0.0) {
            spec_contrib = pow(max(0, dot(N, H)), 4.0);
        }*/
        vec3 light_color = att * lights[i].color_coneangle.xyz;
        result += light_color * (diff_contrib * material_diffuse); // + spec_contrib * vec3(1.0));
    }

    // Output
    out_color = vec4(result, 1.0);
}
)";

const static VertexShader SCALARFIELD_VERTEX_SHADER = {
    /*attributes: */
    {ShaderAttributeDesc("vertex", GLDataType::Vec3f),
     ShaderAttributeDesc("scalar", GLDataType::Float)},
    /*uniforms: */
    {{"model_matrix", GLDataType::Mat4f}, {"normal_matrix", GLDataType::Mat3f}},
    vert_str};

const static FragmentShader SCALARFIELD_FRAGMENT_SHADER = {
    /*uniforms: */
    {ShaderUniformDesc{"vmin", GLDataType::Float}, ShaderUniformDesc{"vmax", GLDataType::Float},
     ShaderUniformDesc{"colormap_style", GLDataType::Int}},
    /*textures: */
    {},
    /*cubemaps: */
    {},
    "out_color",
    frag_str};

class ViewerRenderSystem : public RenderSystem
{
    std::shared_ptr<ShaderProgram> shader_scalarfield_;

  public:
    ViewerRenderSystem(glm::ivec2 resolution, unsigned int msaa) : RenderSystem(resolution, msaa)
    {
        shader_scalarfield_ =
            ShaderProgram::create(SCALARFIELD_VERTEX_SHADER, SCALARFIELD_FRAGMENT_SHADER, true);
        shader_scalarfield_->renderState().depth_test = true;
        shader_scalarfield_->renderState().depth_write = true;
        shader_scalarfield_->renderState().blending = false;
        shader_scalarfield_->renderState().culling = false;
        shader_scalarfield_->renderPriority() = RenderPriority::Opaque;
    }

    /*void ViewerRenderSystem::initialize() override
    {
        RenderSystem::initialize();
        shader_scalarfield_ =
            ShaderProgram::create(SCALARFIELD_VERTEX_SHADER, SCALARFIELD_FRAGMENT_SHADER, true);
        shader_scalarfield_->renderState().depth_test = true;
        shader_scalarfield_->renderState().depth_write = true;
        shader_scalarfield_->renderState().blending = false;
        shader_scalarfield_->renderState().culling = false;
        shader_scalarfield_->renderPriority() = RenderPriority::Opaque;
    }
*/
  protected:
    virtual void drawEntity(Entity ent) override
    {
        Drawable *dr = world_->getComponent<Drawable>(ent);
        if (!dr->visible)
        {
            return;
        }
        ScalarField *sf = world_->getComponentUnsafe<ScalarField>(ent);
        if (sf != nullptr && sf->show)
        {
            //// Only draw if scalar field is 1D float
            // Attribute attr;
            // try
            //{
            //    attr = dr->mesh->customAttribute(sf->attribute_name);
            //}
            // catch (const std::exception &)
            //{
            //    return;
            //}
            // if (attr.type == GLDataType::Float)
            //{
            //    // Draw the surface using scalar field shader
            //    Transform *tr = world_->getComponent<Transform>(ent);
            //    renderer.render(dr->mesh.get(), shader_scalarfield_.get(), tr->worldTransform());
            //}
            shader_scalarfield_->uniform("vmin").set(sf->vmin);
            shader_scalarfield_->uniform("vmax").set(sf->vmax);
            shader_scalarfield_->uniform("colormap_style").set(sf->colormap);
            Transform *tr = world_->getComponent<Transform>(ent);
            renderer.render(dr->mesh.get(), shader_scalarfield_.get(), tr->worldTransform());
            checkGLError();
        }
        else
        {
            // Draw as usual
            RenderSystem::drawEntity(ent);
        }
    }
};

} // namespace rcube
