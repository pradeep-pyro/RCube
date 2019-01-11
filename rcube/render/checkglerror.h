#ifndef CHECKGLERROR_H
#define CHECKGLERROR_H

namespace rcube {

void _checkGLError(const char *file, int line);

#define checkGLError() _checkGLError(__FILE__,__LINE__)

} // namespace rcube

#endif // CHECKGLERROR_H
