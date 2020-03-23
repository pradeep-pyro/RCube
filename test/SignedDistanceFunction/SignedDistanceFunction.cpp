#include "RCube/Core/Graphics/Effects/GammaCorrectionEffect.h"
#include "RCube/Core/Graphics/Materials/SDFGridMaterial.h"
#include "RCubeViewer/RCubeViewer.h"

rcube::GridData makeSphereSDF(size_t nx, size_t ny, size_t nz, float radius)
{
    rcube::GridData sdf;
    sdf.nx = nx;
    sdf.ny = ny;
    sdf.nz = nz;
    sdf.data.resize(nx * ny * nz, 0.0);
    const size_t nxy = nx * ny;
    for (size_t z = 0; z < nz; ++z)
    {
        for (size_t y = 0; y < ny; ++y)
        {
            for (size_t x = 0; x < nx; ++x)
            {
                float dz = float(z) - float(nz / 2);
                float dy = float(y) - float(ny / 2); 
                float dx = float(x) - float(nx / 2); 
                sdf.data[z * nxy + y * nx + x] = std::sqrt(dx * dx + dy * dy + dz * dz) - radius;
            }
        }
    }
    return sdf;
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
    EntityHandle sphere = viewer.addSphereVolume("sphere", 1.0);
    sphere.get<Volume>()->grid = DenseGrid::create(64, 64, 64);
    sphere.get<Volume>()->grid->data = makeSphereSDF(64, 64, 64, 10);
    sphere.get<Volume>()->material = makeSDFVolumeMaterial();

    // Show viewer
    viewer.execute();
    return 0;
}