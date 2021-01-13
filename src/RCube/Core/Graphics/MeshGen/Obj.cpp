#ifndef OBJ_H
#define OBJ_H

#include "RCube/Core/Graphics/OpenGL/Mesh.h"
#include <algorithm>
#include <iterator>
#include <locale>
#include <sstream>
#include <string>

constexpr int MAX_LINE_LENGTH = 2048;

namespace rcube
{
/**
 * Creates mesh data from OBJ file
 * @param file_name file name of the input mesh
 */
TriangleMeshData loadOBJ(const std::string &file_name)
{

    auto getFileExtension = [](const std::string &FileName) -> std::string {
        // https://stackoverflow.com/a/4505931/1608232
        if (FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".") + 1);
        return "";
    };
    std::string file_extension = getFileExtension(file_name);

    std::for_each(file_extension.begin(), file_extension.end(),
                  [](char &c) { c = std::tolower(c); });
    if (file_extension != "obj")
    {
        throw std::runtime_error("loadOBJ:: only read OBJ files. Input file extension is \n" +
                                 file_extension + " and file name is " + file_name);
    }

    FILE *Objfile = fopen(file_name.c_str(), "r");
    if (NULL == Objfile)
    {
        throw std::runtime_error("loadOBJ:: can not open\n" + file_name);
    }

    TriangleMeshData mesh_data;
    mesh_data.indexed = true;
    mesh_data.clear();

    char line[MAX_LINE_LENGTH];
    uint32_t lineNum = 1;
    while (fgets(line, MAX_LINE_LENGTH, Objfile) != NULL)
    {
        char type[MAX_LINE_LENGTH];

        if (sscanf(line, "%s", type) == 1)
        {
            // read only the first letter of the line

            char *l = &line[strlen(type)]; // next thing after the type
            if (strcmp(type, "v") == 0)
            {
                // vertex
                std::istringstream ls(&line[1]);
                std::vector<float> vert_vec{std::istream_iterator<float>(ls),
                                            std::istream_iterator<float>()};
                if (vert_vec.size() < 3)
                {
                    fclose(Objfile);
                    throw std::runtime_error(
                        "loadOBJ:: vertex has less than 3 coordinates at Line[" +
                        std::to_string(lineNum) + "]\n");
                }
                else
                {
                    // read the vertex coordinates
                    glm::vec3 vert(0);
                    vert.x = vert_vec[0];
                    vert.y = vert_vec[1];
                    vert.z = vert_vec[2];
                    mesh_data.vertices.push_back(vert);
                }

                if (vert_vec.size() == 6)
                {
                    // read the vertex color
                    glm::vec3 color(0);
                    color.x = vert_vec[3];
                    color.y = vert_vec[4];
                    color.z = vert_vec[5];
                    mesh_data.colors.push_back(color);
                }
            }
            else if (strcmp(type, "vn") == 0)
            {
                // normal
                glm::vec3 norm;
                uint32_t count = sscanf(l, "%f %f %f\n", &norm.x, &norm.y, &norm.z);

                if (count != 3)
                {
                    fclose(Objfile);
                    throw std::runtime_error(
                        "loadOBJ:: normal has less than 3 coordinates at Line[" +
                        std::to_string(lineNum) + "]\n");
                }
                mesh_data.normals.push_back(norm);
            }
            else if (strcmp(type, "vt") == 0)
            {
                // texture
                glm::vec2 tex;
                float w;
                uint32_t count = sscanf(l, "%f %f %f\n", &tex.x, &tex.y, &w);

                if (count != 2 && count != 3)
                {
                    fclose(Objfile);
                    throw std::runtime_error(
                        "loadOBJ:: texture coordinates are not 2 or 3 coordinates at Line[" +
                        std::to_string(lineNum) + "]\n");
                }
                mesh_data.texcoords.push_back(tex);
            }
            else if (strcmp(type, "f") == 0)
            {
                // face (read vert id, norm id, tex id)

                std::vector<unsigned int> f;
                std::vector<unsigned int> ft;
                std::vector<unsigned int> fn;
                char word[MAX_LINE_LENGTH];
                uint32_t offset;
                while (sscanf(l, "%s%n", word, &offset) == 1)
                {
                    l += offset;
                    unsigned int i, it, in;
                    if (sscanf(word, "%ld/%ld/%ld", &i, &it, &in) == 3)
                    {
                        // face, norm, tex
                        f.push_back(unsigned int(i < 0 ? i + mesh_data.vertices.size() : i - 1));
                        ft.push_back(unsigned int(i < 0 ? i + mesh_data.texcoords.size() : i - 1));
                        fn.push_back(unsigned int(i < 0 ? i + mesh_data.normals.size() : i - 1));
                    }
                    else if (sscanf(word, "%ld/%ld", &i, &it) == 2)
                    {
                        // face, tex
                        f.push_back(unsigned int(i < 0 ? i + mesh_data.vertices.size() : i - 1));
                        ft.push_back(unsigned int(i < 0 ? i + mesh_data.texcoords.size() : i - 1));
                    }
                    else if (sscanf(word, "%ld//%ld", &i, &it) == 3)
                    {
                        // face, norm
                        f.push_back(unsigned int(i < 0 ? i + mesh_data.vertices.size() : i - 1));
                        fn.push_back(unsigned int(i < 0 ? i + mesh_data.normals.size() : i - 1));
                    }
                    else if (sscanf(word, "%ld", &i) == 1)
                    {
                        // face
                        f.push_back(unsigned int(i < 0 ? i + mesh_data.vertices.size() : i - 1));
                    }
                    else
                    {
                        fclose(Objfile);
                        throw std::runtime_error("loadOBJ:: face has wrong format at Line[" +
                                                 std::to_string(lineNum) + "]\n");
                    }
                }

                if ((f.size() > 0 && fn.size() == 0 && ft.size() == 0) || // face, no norm, no tex
                    (f.size() > 0 && fn.size() == f.size() &&
                     ft.size() == 0) || // face, norm, no tex
                    (f.size() > 0 && fn.size() == 0 &&
                     ft.size() == f.size()) || // face, no norm, tex
                    (f.size() > 0 && fn.size() == f.size() &&
                     ft.size() == f.size())) // face, norm, tex
                {
                    for (size_t fidx = 0; fidx < f.size(); fidx += 3)
                    {
                        mesh_data.indices.push_back({f[fidx], f[fidx + 1], f[fidx + 2]});
                    }
                    // TODO when we support face texture and face norm, we can
                    // read if from here
                }
                else
                {
                    fclose(Objfile);
                    throw std::runtime_error("loadOBJ:: face has wrong format at Line[" +
                                             std::to_string(lineNum) + "]\n");
                }
            }
            else if (strlen(type) >= 1 &&
                     (type[0] == '#' || type[0] == 'g' || type[0] == 'l' || type[0] == 's' ||
                      strcmp("usemtl", type) == 0 || strcmp("mtllib", type) == 0))
            {
                // materials, comments, groups, shading, or lines -> do nothing
            }
            else
            {
                // others
                fclose(Objfile);
                throw std::runtime_error("loadOBJ:: invalid line at Line[" +
                                         std::to_string(lineNum) + "]\n");
            }
        }
        else
        {
            // empty line
        }
        lineNum++;
    }
    fclose(Objfile);

    return mesh_data;
}

} // namespace rcube

#endif // OBJ_H