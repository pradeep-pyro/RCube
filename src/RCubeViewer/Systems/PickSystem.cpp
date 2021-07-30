#include "RCubeViewer/Systems/PickSystem.h"
#include "RCube/Components/Camera.h"
#include "RCube/Components/Drawable.h"
#include "RCube/Components/Transform.h"
#include "RCube/Systems/ForwardRenderSystem.h"
#include "RCubeViewer/Components/CameraController.h"
#include "RCubeViewer/Components/Pickable.h"
#include "RCubeViewer/Pointcloud.h"
#include "imgui.h"

namespace rcube
{
namespace viewer
{

glm::vec2 screenToNDC(double xpos, double ypos, double width, double height)
{
    const float x = static_cast<float>(xpos);
    const float y = static_cast<float>(ypos);
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    float ndc_x = (2.0f * x) / w - 1.0f;
    float ndc_y = 1.0f - 2.0f * y / h;
    return glm::vec2(ndc_x, ndc_y);
}

PickSystem::PickSystem()
{
    addFilter({Camera::family(), CameraController::family()});
    addFilter({Drawable::family(), Transform::family(), Pickable::family()});
}

void PickSystem::update(bool)
{
    if (ImGui::GetIO().WantCaptureMouse)
    {
        return;
    }
    // Get the render system if we don't already have it
    if (render_system_ == nullptr)
    {
        render_system_ =
            dynamic_cast<ForwardRenderSystem *>(world_->getSystem("ForwardRenderSystem"));
    }
    // Can't pick without a render system
    if (render_system_ == nullptr)
    {
        return;
    }
    // Get the texture holding the picking information
    std::shared_ptr<Texture2D> objPrimID = render_system_->objPrimIDTexture();
    if (objPrimID != nullptr)
    {
        glm::dvec2 xy = InputState::instance().mousePos();
        const glm::dvec2 window_size = InputState::instance().windowSize();

        // Initialize double-buffered PBOs if not already available
        if (pbos_.first() == nullptr)
        {
            pbos_.first() = PixelPackBuffer::create(size_t(window_size[0]) * size_t(window_size[1]),
                                                    GL_STREAM_READ);
        }
        if (pbos_.second() == nullptr)
        {
            pbos_.second() = PixelPackBuffer::create(
                size_t(window_size[0]) * size_t(window_size[1]), GL_STREAM_READ);
        }
        // Transform mouse coordinate to texture image space
        xy /= window_size;
        xy[1] = 1.0 - xy[1];
        xy[0] *= objPrimID->width();
        xy[1] *= objPrimID->height();
        // Return if mouse is outside window
        if (xy[0] < 0 || xy[1] < 0 || xy[0] >= objPrimID->width() || xy[1] >= objPrimID->height())
        {
            return;
        }
        // Copy a single pixel from the texture asynchronously
        pbos_.first()->use();
        objPrimID->getSubImage(int(xy.x), int(xy.y), 1, 1, (uint32_t *)NULL, 2);
        pbos_.second()->use();
        GLuint *ptr = (GLuint *)pbos_.second()->map();
        int entity_id = -1;
        int primitive_id = -1;
        if (ptr != nullptr)
        {
            entity_id = ptr[0];    // Entity ID
            primitive_id = ptr[1]; // Primitive ID
            pbos_.second()->unmap();
        }
        pbos_.second()->done();
        // Swap the buffers
        pbos_.increment();

        // Assign to Pickable components
        for (Entity ent :
             getFilteredEntities({Drawable::family(), Transform::family(), Pickable::family()}))
        {
            Pickable *p = world_->getComponent<Pickable>(ent);
            if (p != nullptr)
            {
                p->picked = p->active && (ent.id() == entity_id);
                if (p->picked)
                {
                    EntityHandle ent_handle;
                    ent_handle.entity = ent;
                    ent_handle.world = world_;
                    p->picked_entity = ent_handle;
                    p->picked_primitive = primitive_id;
                    p->picked_xy = xy;
                }
            }
        }
    }
}

unsigned int PickSystem::priority() const
{
    return 9000;
}

const std::string PickSystem::name() const
{
    return "PickSystem";
}

} // namespace viewer
} // namespace rcube