#include "RCube/Materials/MatCapMaterial.h"
#include "RCube/Core/Graphics/ShaderManager.h"
#include "RCube/Materials/images/black.png.h"
#include "RCube/Materials/images/blue.png.h"
#include "RCube/Materials/images/green.png.h"
#include "RCube/Materials/images/red.png.h"
#include "imgui.h"

namespace rcube
{

const std::string MatCapVertexShader =
    R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec3 color;
layout (location = 5) in float wire;

invariant gl_Position;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

out vec3 vert_color;
out vec3 vert_normal;
flat out float vert_wire;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_normal = vec3(view_matrix * vec4(normal_matrix * normal, 0.0));
    vert_color = color;
    vert_wire = wire;
    gl_Position = projection_matrix * view_matrix * world_pos;
}
)";

const static std::string MatCapGeometryShader =
    R"(
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

in vec3 vert_normal[];
out vec3 geom_normal;
in vec3 vert_color[];
out vec3 geom_color;
flat in float vert_wire[];
out float geom_wire;

noperspective out vec3 dist;

void main() {
    // Transform each vertex into viewport space
    vec3 p0 = vec3(viewport_matrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w));
    vec3 p1 = vec3(viewport_matrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w));
    vec3 p2 = vec3(viewport_matrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

    float a = length(p1 - p2);
    float b = length(p2 - p0);
    float c = length(p0 - p1);

    // Interior angles
    float alpha = acos((b*b + c*c - a*a) / (2.0 * b * c));
    float beta = acos((a*a + c*c - b*b) / (2.0 * a * c));

    // Distance from vertex to opposite side using law of cosines
    float ha = c * sin(beta);
    float hb = c * sin(alpha);
    float hc = b * sin(alpha);

    // Emit vertex 1
    dist = vec3(ha, 0, 0);
    geom_color = vert_color[0];
    geom_normal = vert_normal[0];
    geom_wire = vert_wire[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Emit vertex 2
    dist = vec3(0, hb, 0);
    geom_color = vert_color[1];
    geom_normal = vert_normal[1];
    geom_wire = vert_wire[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Emit vertex 3
    dist = vec3(0, 0, hc);
    geom_color = vert_color[2];
    geom_normal = vert_normal[2];
    geom_wire = vert_wire[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
)";

const std::string MatCapFragmentShader =
    R"(
in vec3 geom_color;
in vec3 geom_normal;
in float geom_wire;
noperspective in vec3 dist;
out vec4 out_color;

layout(binding=0) uniform sampler2D matcap;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform vec3 color;

struct Wireframe {
    bool show;
    vec3 color;
    float thickness;
};
uniform Wireframe wireframe;
uniform bool show_wireframe;

uniform bool valid_texture;

const vec3 PINK = pow(vec3(255.0 / 255.0, 20.0 / 255.0, 147.0 / 255.0), vec3(2.2));
const vec3 PURPLE = pow(vec3(138.0 / 255.0, 43.0 / 255.0, 226.0 / 255.0), vec3(2.2));

void main()
{
    int edge_flag = int(round(geom_wire));
    vec3 col = color;
    // edge_flag == 0: wireframe is not rendered
    // edge_flag == 1: wireframe is rendered
    // edge_flag == 2: wireframe is rendered in PURPLE
    // edge_flag == 3: wireframe is rendered in PINK
    // Draw a wireframe if it's set as visible globally or if the edge is set as visible
    if (edge_flag > 0) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float thickness = edge_flag > 1 ? 2.0 * wireframe.thickness : wireframe.thickness;
        if (d < thickness)
        {
            float mix_val = smoothstep(thickness - 1, thickness + 1, d);
            vec3 wcolor = edge_flag == 3 ? PINK : (edge_flag == 2 ? PURPLE : (wireframe.show ? wireframe.color : color));
            col = mix(wcolor, col, mix_val);
        }
    }

    vec2 uv = 0.498 * vec2(normalize(geom_normal)) + vec2(0.5, 0.5);
    uv.y = 1.0 - uv.y;
    if (valid_texture)
    {
        col *= texture2D(matcap, uv).rgb;
    }
    out_color = vec4(col, 1.0);
}
)";

const std::string MatCapRGBFragmentShader =
    R"(
in vec3 geom_color;
in vec3 geom_normal;
in float geom_wire;
noperspective in vec3 dist;

#if RCUBE_RENDERPASS == 0
out vec4 out_color;
#elif RCUBE_RENDERPASS == 1
layout (location = 0) out vec4 accum;
layout (location = 1) out float reveal;
#endif

layout(binding=0) uniform sampler2D matcap_red;
layout(binding=1) uniform sampler2D matcap_green;
layout(binding=2) uniform sampler2D matcap_blue;
layout(binding=3) uniform sampler2D matcap_black;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

uniform vec3 color;
uniform vec3 emissive_color;
uniform float opacity;

struct Wireframe {
    bool show;
    vec3 color;
    float thickness;
};
uniform Wireframe wireframe;
uniform bool show_wireframe;

const vec3 PINK = pow(vec3(255.0 / 255.0, 20.0 / 255.0, 147.0 / 255.0), vec3(2.2));
const vec3 PURPLE = pow(vec3(138.0 / 255.0, 43.0 / 255.0, 226.0 / 255.0), vec3(2.2));

void main()
{
    vec3 frag_color = geom_color * color + emissive_color;

    int edge_flag = int(round(geom_wire));
    // edge_flag == 0: wireframe is not rendered
    // edge_flag == 1: wireframe is rendered
    // edge_flag == 2: wireframe is rendered in PURPLE
    // edge_flag == 3: wireframe is rendered in PINK
    // Draw a wireframe if it's set as visible globally or if the edge is set as visible
    if (edge_flag > 0) {
        // Find the smallest distance
        float d = min(dist.x, dist.y);
        d = min(d, dist.z);
        float thickness = edge_flag > 1 ? 2.0 * wireframe.thickness : wireframe.thickness;
        if (d < thickness)
        {
            float mix_val = smoothstep(thickness - 1, thickness + 1, d);
            vec3 wcolor = edge_flag == 3 ? PINK : (edge_flag == 2 ? PURPLE : (wireframe.show ? wireframe.color : frag_color));
            frag_color = mix(wcolor, frag_color, mix_val);
        }
    }

    vec2 uv = 0.498 * vec2(normalize(geom_normal)) + vec2(0.5, 0.5);
    uv.y = 1.0 - uv.y;
    vec3 red = texture2D(matcap_red, uv).rgb;
    vec3 green = texture2D(matcap_green, uv).rgb;
    vec3 blue = texture2D(matcap_blue, uv).rgb;
    vec3 black = texture2D(matcap_black, uv).rgb;
    vec4 final_color = vec4(frag_color.r * red + frag_color.g * green + frag_color.b * blue + (1.0 - frag_color.r - frag_color.g - frag_color.b) * black, opacity);

#if RCUBE_RENDERPASS == 0
    out_color = final_color;
#elif RCUBE_RENDERPASS == 1
    // Based on https://learnopengl.com/Guest-Articles/2020/OIT/Weighted-Blended
    // and http://casual-effects.blogspot.com/2015/03/implemented-weighted-blended-order.html
    float a = min(1.0, final_color.a) * 8.0 + 0.01;
    float b = -gl_FragCoord.z * 0.95 + 1.0;
    float weight = clamp(a * a * a * 1e8 * b * b * b, 1e-2, 3e2);
    accum = vec4(final_color.rgb * final_color.a, final_color.a) * weight;
    reveal = final_color.a;
#endif
}
)";

MatCapRGBMaterial::MatCapRGBMaterial() : ShaderMaterial("MatCapRGBMaterial")
{
    ForwardRenderSystemShaderManager::instance().create("MatCapRGBMaterial", MatCapVertexShader,
                                                        MatCapGeometryShader,
                                                        MatCapRGBFragmentShader, true);
    textures_.reserve(4);
    red_ = Texture2D::create(256, 256, 1, TextureInternalFormat::sRGB8);
    red_->setData(Image::fromMemory(red_png_start, red_png_size, 3));
    green_ = Texture2D::create(256, 256, 1, TextureInternalFormat::sRGB8);
    green_->setData(Image::fromMemory(green_png_start, green_png_size, 3));
    blue_ = Texture2D::create(256, 256, 1, TextureInternalFormat::sRGB8);
    blue_->setData(Image::fromMemory(blue_png_start, blue_png_size, 3));
    black_ = Texture2D::create(256, 256, 1, TextureInternalFormat::sRGB8);
    black_->setData(Image::fromMemory(black_png_start, black_png_size, 3));
    textures_.push_back({red_->id(), 0});
    textures_.push_back({green_->id(), 1});
    textures_.push_back({blue_->id(), 2});
    textures_.push_back({black_->id(), 3});
}

void MatCapRGBMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
{
    shader->uniform("color").set(glm::pow(color, glm::vec3(2.2f)));
    shader->uniform("emissive_color").set(glm::pow(emissive_color, glm::vec3(2.2f)));
    shader->uniform("wireframe.show").set(wireframe);
    shader->uniform("wireframe.color").set(glm::pow(wireframe_color, glm::vec3(2.2f)));
    shader->uniform("wireframe.thickness").set(wireframe_thickness);
    ShaderMaterial::updateUniforms(shader);
}

const std::vector<DrawCall::Texture2DInfo> MatCapRGBMaterial::textureSlots()
{
    return textures_;
}

void MatCapRGBMaterial::drawGUI()
{
    ImGui::Text("MatCapRGBMaterial");
    ImGui::Text("This material renders the object with \nblended RGB material capture textures.\n"
                "Lighting is baked into the material.");
    ImGui::Separator();
    ImGui::ColorEdit3("Color", glm::value_ptr(color));
    ImGui::ColorEdit3("Emissive color", glm::value_ptr(emissive_color));
    ShaderMaterial::drawGUI();
    ImGui::Separator();
    ImGui::Text("Wireframe");
    ImGui::Checkbox("Show", &wireframe);
    ImGui::InputFloat("Thickness###wireframe.thickness", &wireframe_thickness);
    ImGui::ColorEdit3("Color###wireframe.color", glm::value_ptr(wireframe_color));
}

MatCapMaterial::MatCapMaterial() : ShaderMaterial("MatCapMaterial")
{
    ForwardRenderSystemShaderManager::instance().create(
        "MatCapMaterial", MatCapVertexShader, MatCapGeometryShader, MatCapFragmentShader, true);
    textures_.reserve(1);
}

void MatCapMaterial::updateUniforms(std::shared_ptr<ShaderProgram> shader)
{
    shader->uniform("color").set(glm::pow(color, glm::vec3(2.2f)));
    shader->uniform("wireframe.show").set(wireframe);
    shader->uniform("wireframe.color").set(glm::pow(wireframe_color, glm::vec3(2.2f)));
    shader->uniform("wireframe.thickness").set(wireframe_thickness);
    shader->uniform("valid_texture").set(matcap != nullptr);
}

const std::vector<DrawCall::Texture2DInfo> MatCapMaterial::textureSlots()
{
    textures_.clear();
    if (matcap != nullptr)
    {
        textures_.push_back({matcap->id(), 0});
    }
    return textures_;
}

void MatCapMaterial::drawGUI()
{
    ImGui::Text("MatCapRGBMaterial");
    ImGui::Text("This material renders the object with \na material capture texture.\n"
                "Lighting is baked into the material.");
    ImGui::Separator();
    ImGui::ColorEdit3("Color###MatCapMaterial.color", glm::value_ptr(color));
    ImGui::Separator();
    ImGui::Text("Wireframe###MatCapMaterial.wireframe");
    ImGui::Checkbox("Show###MatCapMaterial.show", &wireframe);
    ImGui::InputFloat("Thickness###MatCapMaterial.wireframe.thickness", &wireframe_thickness);
    ImGui::ColorEdit3("Color###MatCapMaterial.wireframe.color", glm::value_ptr(wireframe_color));
}

} // namespace rcube