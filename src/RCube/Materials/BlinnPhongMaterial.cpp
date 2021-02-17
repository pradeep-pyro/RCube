#include "RCube/Materials/BlinnPhongMaterial.h"

namespace rcube
{

const std::string MatCapVertexShader =
    R"(
#version 450
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 3) in vec3 color;
layout (location = 5) in float wire;

out vec3 vert_position;
out vec3 vert_color;
flat out float vert_wire;

uniform mat4 model_matrix;
uniform mat3 normal_matrix;

void main()
{
    vec4 world_pos = model_matrix * vec4(position, 1.0);
    vert_position = world_pos.xyz;
    vert_normal = normal_matrix * normal;
    vert_color = color;
    vert_wire = wire;
    gl_Position = projection_matrix * view_matrix * world_pos;
}
)";

const static std::string MatCapGeometryShader =
    R"(
#version 450
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

layout (std140, binding=0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
};

in vec3 vert_position[];
out vec3 geom_position;
in vec3 vert_normal[];
out vec3 geom_normal;
in vec3 vert_color[];
out vec3 vert_color;
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
    geom_position = vert_position[0];
    geom_normal = vert_normal[0];
    geom_wire = vert_wire[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    // Emit vertex 2
    dist = vec3(0, hb, 0);
    geom_position = vert_position[1];
    geom_normal = vert_normal[1];
    geom_wire = vert_wire[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex();

    // Emit vertex 3
    dist = vec3(0, 0, hc);
    geom_position = vert_position[2];
    geom_normal = vert_normal[2];
    geom_wire = vert_wire[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex();
    EndPrimitive();
}
)";

const std::string MatCapFragmentShader =
    R"(
#version 450
in vec3 geom_position;
in vec3 geom_color;
in vec3 geom_normal;
in float geom_wire;

layout(binding=0) uniform sampler2D matcap_red;
layout(binding=1) uniform sampler2D matcap_green;
layout(binding=2) uniform sampler2D matcap_blue;
layout(binding=3) uniform sampler2D matcap_black;

uniform bool blend_rgb;

void main()
{
    vec3 eye = normalize(geom_position);
    vec3 ref = reflect( eye, vert_normal );
    float denom = 2.0 * sqrt(pow(ref.x, 2.0) + pow(ref.y, 2.0) + pow(ref.z + 1., 2.0));
    vec2 uv = 0.5 + r.xy / denom;
    vec3 frag_color = use_vertex_colors ? vert_color : color;
    
    if (blend_rgb)
    {
        float red = texture2D(matcap_red, uv).rgb;
        float green = texture2D(matcap_green, uv).rgb;
        float blue = texture2D(matcap_blue, uv).rgb;
        float black = texture2D(matcap_black, uv).rgb;
        out_color = vec4(frag_color.r * red + frag_color.g * green + frag_color.b * blue + (1.0 - frag_color.r - frag_color.g - frag_color.b) * black, 1.0);
    }
    else
    {
        out_color = vec4(texture2D(matcap_red, uv).rgb, 1.0);
    }
}
)";

MatCapMaterial::MatCapMaterial()
{
    shader_ = ShaderProgram::create(MatCapVertexShader, MatCapGeometryShader, MatCapFragmentShader, true);
}

void MatCapMaterial::updateUniforms()
{
}

} // namespace rcube