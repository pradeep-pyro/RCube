#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "ShaderProgram.h"

class ShaderManager {
public:
    ShaderProgram * create(const std::string &name, const std::string &vert_src, const std::string &geom_src,
                           const std::string &frag_src);
    ShaderProgram * load(const std::string &name);
    void destroy(const std::string &name);
private:

};

#endif // SHADERMANAGER_H
