#include "Scene.h"
#include "components/Camera.h"
#include "components/Transform.h"
#include "components/Drawable.h"
#include "components/DirectionalLight.h"
#include "components/PointLight.h"
#include "systems/CameraSystem.h"
#include "systems/TransformSystem.h"
#include "systems/RenderSystem.h"

namespace rcube {

Scene::Scene() {
    addSystem(std::make_unique<TransformSystem>());
    addSystem(std::make_unique<CameraSystem>());
    auto rs = std::make_unique<RenderSystem>();
    addSystem(std::move(rs));
}

EntityHandle Scene::createCamera() {
    auto ent = createEntity();
    ent.add<Camera>(Camera());
    ent.add<Transform>(Transform());
    return ent;
}

EntityHandle Scene::createDrawable() {
    auto ent = createEntity();
    auto dr = Drawable();
    ent.add<Drawable>(dr);
    ent.add<Transform>(Transform());
    return ent;
}

EntityHandle Scene::createDirectionalLight() {
    auto ent = createEntity();
    ent.add(DirectionalLight());
    ent.add(Transform());
    return ent;
}

EntityHandle Scene::createPointLight() {
    auto ent = createEntity();
    ent.add(PointLight());
    ent.add(Transform());
    return ent;
}

} // namespace rcube
