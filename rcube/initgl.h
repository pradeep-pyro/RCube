#ifndef SETUPGL_H
#define SETUPGL_H

#include <stdexcept>
#include "glad/glad.h"

namespace rcube {

typedef void (*procAddress)(const char *);

void initGL(procAddress p=nullptr);

} // namespace rcube

#endif // SETUPGL_H
