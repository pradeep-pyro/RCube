#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCubeViewer/RCubeViewer.h"

std::shared_ptr<rcube::ShaderProgram> makeShader()
{
    using namespace rcube;
    std::shared_ptr<rcube::ShaderProgram> shader_scalarfield_ =
        ShaderProgram::create(SCALARFIELD_VERTEX_SHADER, SCALARFIELD_FRAGMENT_SHADER, true);
    shader_scalarfield_->renderState().depth_test = true;
    shader_scalarfield_->renderState().depth_write = true;
    shader_scalarfield_->renderState().blending = false;
    shader_scalarfield_->renderState().culling = false;
    shader_scalarfield_->renderPriority() = RenderPriority::Opaque;
    return shader_scalarfield_;
}

int main()
{
    using namespace rcube;

    // Properties to configure the viewer
    viewer::RCubeViewerProps props;
    props.resolution = glm::vec2(1280 /*4096*/, 720 /*2160*/); // 720p
    // props.MSAA = 2;                                            // turn on 2x multisampling

    // Create a viewer
    viewer::RCubeViewer viewer(props);

    // Add a subdivided icosahedron surface to viewer
    EntityHandle sphere = viewer.addIcoSphereSurface("sphere", 1.0, 4);

    // Create a scalar field
    std::vector<float> height_field;
    Mesh *sphere_mesh = sphere.get<Drawable>()->mesh.get();
    height_field.reserve(sphere_mesh->data.vertices.size());
    for (auto &v : sphere_mesh->data.vertices)
    {
        height_field.push_back(v.y);
    }
    sphere_mesh->addCustomAttribute("scalarfield", ScalarField::ATTRIBUTE_LOCATION, GLDataType::Float);
    sphere_mesh->customAttribute("scalarfield").data->setData(height_field);
    sphere.add<ScalarField>();
    sphere.get<ScalarField>()->vmin = -1.f;
    sphere.get<ScalarField>()->vmax = +1.f;
    sphere.get<ScalarField>()->colormap = ScalarField::Colormap::Viridis;

    // Show viewer
    viewer.execute();
    return 0;
}