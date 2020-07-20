#include "RCube/Core/Graphics/ImageBasedLighting/IBLSpecularSplitSum.h"
#include "RCube/Core/Graphics/MeshGen/Box.h"
#include "RCube/Core/Graphics/OpenGL/CheckGLError.h"
#include <iostream>

namespace rcube
{
#define RCUBE_GLSL_VERSION "420"

#define RCUBE_GLSL(source) "#version " RCUBE_GLSL_VERSION "\n" #source

const static std::string SpecularPrefilterVertexShader =
    R"(
    #version 420
    layout(location = 0) in vec3 position;

    out vec3 direction;

    layout(std140, binding = 0) uniform Camera {
    mat4 view_matrix;
    mat4 projection_matrix;
    mat4 viewport_matrix;
    vec3 eye_pos;
                                         };

                                         void main() {
    direction = position;
    gl_Position = projection_matrix * view_matrix * vec4(position, 1);
}
)";

const static std::string SpecularPrefilterFragmentShader =
    R"(
        #version 420
        layout(binding=0) uniform samplerCube env_map;

        out vec4 frag_color;

        in vec3 direction;

        uniform int num_samples;

        uniform float roughness;

        const float PI = 3.14159265358979323846;

        uint reverseBits32(uint n) {
            n = (n << 16) | (n >> 16); // Swap first and last 16 bits
            n = ((n & 0x00ff00ff) << 8) |
                ((n & 0xff00ff00) >> 8); // Swap consecutive 8 bits in the first half & second half
            n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4); // Continue similarly
            n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
            n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
            return n;
        }

        float radicalInverse(uint n) {
            n = reverseBits32(n);
            // Divide by 2^32
            return float(n) * 2.3283064365386963e-10;
        }

        // Generate low-discrepancy sequence using the Hammersley method
        vec2 hammersleySample(uint i, int N) {
            return vec2(float(i) / float(N), radicalInverse(i));
        }

        vec3 importanceSampleGgx(vec2 u, vec3 N, float roughness) {
            float a = roughness * roughness;
            float phi = 2.0 * PI * u.x;
            float cos_theta = sqrt((1.0 - u.y) / (1.0 + (a * a - 1.0) * u.y));
            float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            // Spherical to cartesian coordinates
            float x = cos(phi) * sin_theta;
            float y = sin(phi) * sin_theta;
            float z = cos_theta;

            // Tangent-space to world-space sample vector
            vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
            vec3 tangent = normalize(cross(up, N));
            vec3 bitangent = cross(N, tangent);

            vec3 sample_vec = tangent * x + bitangent * y + N * z;
            return normalize(sample_vec);
        }

        void main() {
            vec3 N = normalize(direction);
            // Approximation introduced by Epic Games: assume view direction
            // is equal to reflected light direction (\omega_o)
            vec3 R = N;
            vec3 V = R;

            vec3 prefiltered_color = vec3(0.0);
            float weight = 0.0;

            for (uint i = 0; i < num_samples; ++i)
            {
                vec2 u = hammersleySample(i, num_samples);
                vec3 H = importanceSampleGgx(u, N, roughness);
                vec3 L = normalize(2.0 * dot(V, H) * H - V);
                float LdotN = max(dot(L, N), 0.0);
                if (LdotN > 0.0)
                {
                    prefiltered_color += texture(env_map, L).rgb * LdotN;
                    weight += LdotN;
                }
            }
            prefiltered_color /= weight;
            frag_color = vec4(prefiltered_color, 1.0);
        })";

///////////////////////////////////////////////////////////////////////////////

const static std::string SpecularBrdfVertexShader = R"(
#version 420
layout (location = 0) in vec3 vertex;
layout (location = 2) in vec2 texcoord;
out vec2 v_texcoord;

void main() {
    v_texcoord = texcoord;
    gl_Position = vec4(vertex, 1.0);
}
)";

const static std::string SpecularBrdfFragmentShader = R"(
    #version 420
    in vec2 v_texcoord;

    out vec2 out_color;

    const float PI = 3.14159265358979323846;

    uint reverseBits32(uint n) {
        n = (n << 16) | (n >> 16); // Swap first and last 16 bits
        n = ((n & 0x00ff00ff) << 8) |
            ((n & 0xff00ff00) >> 8); // Swap consecutive 8 bits in the first half & second half
        n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4); // Continue similarly
        n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
        n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
        return n;
    }

    float radicalInverse(uint n) {
        n = reverseBits32(n);
        // Divide by 2^32
        return float(n) * 2.3283064365386963e-10;
    }

    // Generate low-discrepancy sequence using the Hammersley method
    vec2 hammersleySample(uint i, uint N) {
        return vec2(float(i) / float(N), radicalInverse(i));
    }

    vec3 importanceSampleGgx(vec2 u, vec3 N, float roughness) {
        float a = roughness * roughness;
        float phi = 2.0 * PI * u.x;
        float cos_theta = sqrt((1.0 - u.y) / (1.0 + (a * a - 1.0) * u.y));
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        // Spherical to cartesian coordinates
        float x = cos(phi) * sin_theta;
        float y = sin(phi) * sin_theta;
        float z = cos_theta;

        // Tangent-space to world-space sample vector
        vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
        vec3 tangent = normalize(cross(up, N));
        vec3 bitangent = cross(N, tangent);

        vec3 sample_vec = tangent * x + bitangent * y + N * z;
        return normalize(sample_vec);
    }

    float GSchlickGgx(float NdotV, float roughness) {
        float r = 1.0 + roughness;
        float k = (r * r) / 8.0;

        float numerator = NdotV;
        float denominator = NdotV * (1.0 - k) + k;

        return numerator / denominator;
    }

    float GSmith(float NdotV, float LdotN, float roughness) {
        NdotV = max(NdotV, 0.0);
        LdotN = max(LdotN, 0.0);
        float ggx1 = GSchlickGgx(LdotN, roughness);
        float ggx2 = GSchlickGgx(NdotV, roughness);
        return ggx1 * ggx2;
    }

    vec2 integrateBRDF(float NdotV, float roughness) {
        vec3 V;
        V.x = sqrt(1.0 - NdotV * NdotV);
        V.y = 0.0;
        V.z = NdotV;

        float A = 0.0;
        float B = 0.0;

        vec3 N = vec3(0.0, 0.0, 1.0);

        const uint SAMPLE_COUNT = 1024u;
        for (uint i = 0u; i < SAMPLE_COUNT; ++i)
        {
            vec2 Xi = hammersleySample(i, SAMPLE_COUNT);
            vec3 H = importanceSampleGgx(Xi, N, roughness);
            vec3 L = normalize(2.0 * dot(V, H) * H - V);

            float LdotN = max(L.z, 0.0);
            float NdotH = max(H.z, 0.0);
            float VdotH = max(dot(V, H), 0.0);

            if (LdotN > 0.0)
            {
                float G = GSmith(NdotV, LdotN, roughness);
                float G_Vis = (G * VdotH) / (NdotH * NdotV);
                float Fc = pow(1.0 - VdotH, 5.0);

                A += (1.0 - Fc) * G_Vis;
                B += Fc * G_Vis;
            }
        }
        A /= float(SAMPLE_COUNT);
        B /= float(SAMPLE_COUNT);
        return vec2(A, B);
    }

    void main() {
        out_color = integrateBRDF(v_texcoord.x, v_texcoord.y);
    }
)";

///////////////////////////////////////////////////////////////////////////////

IBLSpecularSplitSum::IBLSpecularSplitSum(unsigned int resolution, int num_samples)
    : resolution_(resolution), num_samples_(num_samples)
{
    // Create a unit cube in clip space
    cube_ = Mesh::create(box(2.0, 2.0, 2.0, 1, 1, 1));
    cube_->uploadToGPU();

    // Compile the prefilter shader
    shader_ =
        ShaderProgram::create(SpecularPrefilterVertexShader, SpecularPrefilterFragmentShader, true);

    // Compile the brdf shader
    shader_brdf_ =
        ShaderProgram::create(SpecularBrdfVertexShader, SpecularBrdfFragmentShader, true);

    // Create framebuffer to hold result
    fbo_ = Framebuffer::create(resolution, resolution);
    fbo_->setColorAttachment(0, TextureInternalFormat::RGB16F);
    fbo_->setDepthAttachment(TextureInternalFormat::Depth24Stencil8);

    // Matrices for rendering the cubemap from cameras set pointing at the
    // cube faces
    projection_ = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
    views_ = {glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
              glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))};

    // Create renderer
    rdr_.initialize();
}

IBLSpecularSplitSum::~IBLSpecularSplitSum()
{
    cube_->release();
    rdr_.cleanup();
    fbo_->release();
    shader_->release();
    shader_brdf_->release();
}

int IBLSpecularSplitSum::numSamples() const
{
    return num_samples_;
}

void IBLSpecularSplitSum::setNumSamples(int num_samples)
{
    num_samples_ = num_samples;
}

std::shared_ptr<TextureCubemap>
IBLSpecularSplitSum::prefilter(std::shared_ptr<TextureCubemap> env_map)
{
    const int num_mipmaps = 5;
    auto prefiltered_map = TextureCubemap::create(resolution_, resolution_, num_mipmaps, true,
                                                  TextureInternalFormat::RGB16F);
    prefiltered_map->setFilterModeMin(TextureFilterMode::Trilinear); // enable trilinear filtering
    prefiltered_map->setFilterModeMag(TextureFilterMode::Linear);
    prefiltered_map->setWrapMode(TextureWrapMode::ClampToEdge);
    shader_->uniform("num_samples").set(num_samples_);
    rdr_.resize(0, 0, resolution_, resolution_);

    glm::mat4 eye(1.0);
    const glm::vec3 eye_pos(0., 0., 0.);
    for (unsigned int mip = 0; mip < num_mipmaps; ++mip)
    {
        // Resize framebuffer and viewport according to mipmap size
        unsigned int mip_width = static_cast<unsigned int>(resolution_ * std::pow(0.5, mip));
        unsigned int mip_height = static_cast<unsigned int>(resolution_ * std::pow(0.5, mip));
        fbo_->resize(mip_width, mip_height);
        rdr_.resize(0, 0, mip_width, mip_height);
        fbo_->use();

        float roughness = static_cast<float>(mip) / static_cast<float>(num_mipmaps - 1);
        shader_->uniform("roughness").set(roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            rdr_.clear();
            rdr_.setCamera(eye_pos, views_[i], projection_, eye);
            env_map->use(0);
            rdr_.render(cube_.get(), shader_.get(), eye);
            prefiltered_map->use();
            glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, 0, 0, 0, 0, mip_width,
                                mip_height);
        }
        fbo_->done();
    }

    return prefiltered_map;
}

std::shared_ptr<Texture2D> IBLSpecularSplitSum::integrateBRDF(unsigned int resolution)
{
    std::shared_ptr<Texture2D> brdf_lut =
        Texture2D::create(resolution, resolution, 1, TextureInternalFormat::RG16F);
    brdf_lut->setWrapMode(TextureWrapMode::ClampToEdge);
    brdf_lut->setFilterMode(TextureFilterMode::Linear);

    auto fbo = Framebuffer::create(resolution, resolution);
    fbo->setColorAttachment(0, brdf_lut);
    fbo->setDrawBuffers({0});
    fbo->setDepthAttachment(TextureInternalFormat::Depth32F);
    fbo->use();
    rdr_.resize(0, 0, resolution, resolution);
    rdr_.clear();
    rdr_.renderFullscreenQuad(shader_brdf_.get(), fbo.get());
    return brdf_lut;
}

} // namespace rcube
