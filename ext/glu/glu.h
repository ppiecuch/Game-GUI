#ifndef __GLU_H_
#define __GLU_H_

#if defined(QT_GUI_LIB)
# include <QtOpenGL/qgl.h>
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#else
# include "glew.h"
#endif

const char *glErrorString( GLenum err );

void glPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);

GLint glUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
             const GLfloat modelMatrix[16], const GLfloat projMatrix[16],
             const GLint viewport[4],
             GLfloat* objx, GLfloat* objy, GLfloat* objz);

GLint glUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
             const GLdouble modelMatrix[16], const GLdouble projMatrix[16],
             const GLint viewport[4],
             GLdouble* objx, GLdouble* objy, GLdouble* objz);

GLint glBuild2DMipmaps(GLenum _targetfmt,
  GLenum _internalfmt, 
  GLint _width, 
  GLint _height,
  GLenum _format,
  GLenum _type, 
  const void* _data);

#endif // __GLU_H_
