
#include "glu.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
# include <malloc.h>
#endif

#ifdef QT_GUI_LIB
# include <QDebug>
# include <QImage>
#endif

static const struct {
   GLuint Token;
   const char *String;
} Errors[] = {
   { GL_NO_ERROR, "no error" },
   { GL_INVALID_ENUM, "invalid enumerant" },
   { GL_INVALID_VALUE, "invalid value" },
   { GL_INVALID_OPERATION, "invalid operation" },
   { GL_STACK_OVERFLOW, "stack overflow" },
   { GL_STACK_UNDERFLOW, "stack underflow" },
   { GL_OUT_OF_MEMORY, "out of memory" },
   /*{ GL_TABLE_TOO_LARGE, "table too large" },*/
   { 0x0506, "invalid framebuffer operation" },
   /* GLU */
   { 100900, "invalid enumerant" },
   { 100901, "invalid value" },
   { 100902, "out of memory" },
   { 100903, "incompatible gl version" },
   { 100904, "invalid operation" },
   { static_cast<GLuint>(~0), NULL } /* end of list indicator */
};

const char *glErrorString( )
{
    GLenum err = glGetError();
	int i; for (i = 0; Errors[i].String; i++) { if (Errors[i].Token == err) break; }
    return Errors[i].String?Errors[i].String:"<unknown>";
}

const char *glErrorString( GLenum err )
{
	int i; for (i = 0; Errors[i].String; i++) { if (Errors[i].Token == err) break; }
    return Errors[i].String?Errors[i].String:"<unknown>";
}

/*
** Make m an identity matrix
*/

static void __glMakeIdentityf(GLfloat m[16])
{
    m[0+4*0] = 1; m[0+4*1] = 0; m[0+4*2] = 0; m[0+4*3] = 0;
    m[1+4*0] = 0; m[1+4*1] = 1; m[1+4*2] = 0; m[1+4*3] = 0;
    m[2+4*0] = 0; m[2+4*1] = 0; m[2+4*2] = 1; m[2+4*3] = 0;
    m[3+4*0] = 0; m[3+4*1] = 0; m[3+4*2] = 0; m[3+4*3] = 1;
}

#define __glPi 3.14159265358979323846

void glPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    GLfloat m[4][4];
    GLfloat sine, cotangent, deltaZ;
    GLfloat radians=(GLfloat)(fovy/2.0f*__glPi/180.0f);

    deltaZ=zFar-zNear;
    sine=(GLfloat)sin(radians);
    if ((deltaZ==0.0f) || (sine==0.0f) || (aspect==0.0f))
        return;

    cotangent=(GLfloat)(cos(radians)/sine);

    __glMakeIdentityf(&m[0][0]);
    m[0][0] = cotangent / aspect;
    m[1][1] = cotangent;
    m[2][2] = -(zFar + zNear) / deltaZ;
    m[2][3] = -1.0f;
    m[3][2] = -2.0f * zNear * zFar / deltaZ;
    m[3][3] = 0;
    glMultMatrixf(&m[0][0]);
}

static void normalize(GLfloat v[3])
{
    GLfloat r = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

    if (r==0.0f)
        return;

    v[0]/=r;
    v[1]/=r;
    v[2]/=r;
}

static void cross(GLfloat v1[3], GLfloat v2[3], GLfloat result[3])
{
    result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = v1[2]*v2[0] - v1[0]*v2[2];
    result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void glLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx,
          GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy,
          GLfloat upz)
{
    GLfloat forward[3], side[3], up[3];
    GLfloat m[4][4];

    forward[0] = centerx - eyex;
    forward[1] = centery - eyey;
    forward[2] = centerz - eyez;

    up[0] = upx;
    up[1] = upy;
    up[2] = upz;

    normalize(forward);

    /* Side = forward x up */
    cross(forward, up, side);
    normalize(side);

    /* Recompute up as: up = side x forward */
    cross(side, forward, up);

    __glMakeIdentityf(&m[0][0]);
    m[0][0] = side[0];
    m[1][0] = side[1];
    m[2][0] = side[2];

    m[0][1] = up[0];
    m[1][1] = up[1];
    m[2][1] = up[2];

    m[0][2] = -forward[0];
    m[1][2] = -forward[1];
    m[2][2] = -forward[2];

    glMultMatrixf(&m[0][0]);
    glTranslatef(-eyex, -eyey, -eyez);
}

template<typename T>
void __glMultMatrixVec(const T matrix[16], const T in[4],
                                T out[4])
{
    for (int i=0; i<4; i++)
    {
        out[i] = in[0] * matrix[0*4+i] +
                 in[1] * matrix[1*4+i] +
                 in[2] * matrix[2*4+i] +
                 in[3] * matrix[3*4+i];
    }
}

/*
** Invert 4x4 matrix.
** Contributed by David Moore (See Mesa bug #6748)
*/
template<typename T>
int __glInvertMatrix(const T m[16], T invOut[16])
{
    T inv[16], det;

    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
             + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
             - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
             + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
             - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
             - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
             + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
             - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
             + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
             + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
             - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
             + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
             - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
             - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
             + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
             - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
             + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;

    det=1.0f/det;

    for (int i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return GL_TRUE;
}

template<typename T>
void __glMultMatrices(const T a[16], const T b[16], T r[16])
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            r[i*4+j] = a[i*4+0]*b[0*4+j] +
                       a[i*4+1]*b[1*4+j] +
                       a[i*4+2]*b[2*4+j] +
                       a[i*4+3]*b[3*4+j];
        }
    }
}

template <typename T>
GLint __glProject(T objx, T objy, T objz, 
           const T modelMatrix[16], 
           const T projMatrix[16],
           const GLint viewport[4],
           T* winx, T* winy, T* winz)
{
    T in[4];
    T out[4];

    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __glMultMatrixVec<T>(modelMatrix, in, out);
    __glMultMatrixVec<T>(projMatrix, out, in);
    if (in[3] == 0.0)
        return(GL_FALSE);

    in[0]/=in[3];
    in[1]/=in[3];
    in[2]/=in[3];
    /* Map x, y and z to range 0-1 */
    in[0]=in[0]*0.5f+0.5f;
    in[1]=in[1]*0.5f+0.5f;
    in[2]=in[2]*0.5f+0.5f;

    /* Map x,y to viewport */
    in[0]=in[0] * viewport[2] + viewport[0];
    in[1]=in[1] * viewport[3] + viewport[1];

    *winx=in[0];
    *winy=in[1];
    *winz=in[2];

    return(GL_TRUE);
}

GLint glProject(GLfloat objx, GLfloat objy, GLfloat objz, 
           const GLfloat modelMatrix[16], 
           const GLfloat projMatrix[16],
           const GLint viewport[4],
           GLfloat* winx, GLfloat* winy, GLfloat* winz)
{
	return __glProject<GLfloat>(objx, objy, objz, modelMatrix, projMatrix, viewport, winx, winy, winz);
}

GLint glProject(GLdouble objx, GLdouble objy, GLdouble objz, 
           const GLdouble modelMatrix[16], 
           const GLdouble projMatrix[16],
           const GLint viewport[4],
           GLdouble* winx, GLdouble* winy, GLdouble* winz)
{
	return __glProject<GLdouble>(objx, objy, objz, modelMatrix, projMatrix, viewport, winx, winy, winz);
}

template <typename T>
GLint __glUnProject(T winx, T winy, T winz,
             const T modelMatrix[16], 
             const T projMatrix[16],
             const GLint viewport[4],
             T* objx, T* objy, T* objz)
{
    T finalMatrix[16];
    T in[4];
    T out[4];

    __glMultMatrices<T>(modelMatrix, projMatrix, finalMatrix);
    if (!__glInvertMatrix<T>(finalMatrix, finalMatrix))
        return(GL_FALSE);

    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;

    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    __glMultMatrixVec<T>(finalMatrix, in, out);
    if (out[3] == 0.0)
        return(GL_FALSE);

    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];

    return(GL_TRUE);
}

GLint glUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
             const GLfloat modelMatrix[16], 
             const GLfloat projMatrix[16],
             const GLint viewport[4],
             GLfloat* objx, GLfloat* objy, GLfloat* objz)
{
	return __glUnProject<GLfloat>(winx, winy, winz, modelMatrix, projMatrix, viewport, objx, objy, objz);
}

GLint glUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
             const GLdouble modelMatrix[16], 
             const GLdouble projMatrix[16],
             const GLint viewport[4],
             GLdouble* objx, GLdouble* objy, GLdouble* objz)
{
	return __glUnProject<GLdouble>(winx, winy, winz, modelMatrix, projMatrix, viewport, objx, objy, objz);
}

#include <stdio.h>

#undef  POINT_SAMPLE
#define CEILING( A, B )  ( (A) % (B) == 0 ? (A)/(B) : (A)/(B)+1 )

static void dummy(GLuint j, GLuint k) { }

static GLint round2( GLint n )
{
  GLint m;

  for (m=1; m<n; m*=2)
    ;

  /* m>=n */
  if (m-n <= n-m/2) {
    return m;
  }
  else {
    return m/2;
  }
}

static GLint bytes_per_pixel( GLenum format, GLenum type )
{
  GLint n, m;

  switch (format) {
  /*
  case GL_COLOR_INDEX:
  case GL_STENCIL_INDEX:
  case GL_DEPTH_COMPONENT:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE: */
  case GL_ALPHA:
  case GL_LUMINANCE:
    n = 1;
    break;
  case GL_LUMINANCE_ALPHA:
    n = 2;
    break;
  case GL_RGB:
    n = 3;
    break;
  case GL_RGBA:
    n = 4;
    break;
  default:
    n = 0;
  }

  switch (type) {
  case GL_UNSIGNED_BYTE:	m = sizeof(GLubyte);	break;
  case GL_BYTE:		m = sizeof(GLbyte);	break;
//  case GL_BITMAP:		m = 1;			break;
  case GL_UNSIGNED_SHORT:	m = sizeof(GLushort);	break;
  case GL_SHORT:		m = sizeof(GLshort);	break;
//  case GL_UNSIGNED_INT:	m = sizeof(GLuint);	break;
//  case GL_INT:		m = sizeof(GLint);	break;
  case GL_FLOAT:		m = sizeof(GLfloat);	break;
  default:			m = 0;
  }

  return n * m;
}



static GLint glScaleImage( GLenum format,
		     GLint widthin, GLint heightin,
		     GLenum typein, const void *datain,
		     GLint widthout, GLint heightout,
		     GLenum typeout, void *dataout )
{
  //printf("glumScaleImg win=%d hin=%d typein=%d wout=%d hout=%d typeout=%d\n"
  //	, widthin, heightin, typein
  //	, widthout, heightout, typeout);

  GLint components, i, j, k;
  GLfloat *tempin, *tempout;
  GLfloat sx, sy;
  GLint unpackrowlength, unpackalignment, unpackskiprows, unpackskippixels;
  GLint packrowlength, packalignment, packskiprows, packskippixels;
  GLint sizein, sizeout;
  GLint rowstride, rowlen;


  /* Determine number of components per pixel */
  switch (format) {
/*  
  case GL_COLOR_INDEX:
  case GL_STENCIL_INDEX:
  case GL_DEPTH_COMPONENT:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE: 
*/
  case GL_ALPHA:
  case GL_LUMINANCE:
    components = 1;
    break;
  case GL_LUMINANCE_ALPHA:
    components = 2;
    break;
  case GL_RGB:
    components = 3;
    break;
  case GL_RGBA:
    components = 4;
    break;
  default:
    return GL_INVALID_ENUM;
  }

  /* Determine bytes per input datum */
  switch (typein) {
  case GL_UNSIGNED_BYTE:	sizein = sizeof(GLubyte);	break;
  case GL_BYTE:		sizein = sizeof(GLbyte);	break;
  case GL_UNSIGNED_SHORT:	sizein = sizeof(GLushort);	break;
  case GL_SHORT:		sizein = sizeof(GLshort);	break;
  // case GL_UNSIGNED_INT:	sizein = sizeof(GLuint);	break;
  // case GL_INT:		sizein = sizeof(GLint);		break;
  case GL_FLOAT:		sizein = sizeof(GLfloat);	break;
  // case GL_BITMAP:
  /* not implemented yet */
  default:
    return GL_INVALID_ENUM;
  }

  /* Determine bytes per output datum */
  switch (typeout) {
  case GL_UNSIGNED_BYTE:	sizeout = sizeof(GLubyte);	break;
  case GL_BYTE:		sizeout = sizeof(GLbyte);	break;
  case GL_UNSIGNED_SHORT:	sizeout = sizeof(GLushort);	break;
  case GL_SHORT:		sizeout = sizeof(GLshort);	break;
  // case GL_UNSIGNED_INT:	sizeout = sizeof(GLuint);	break;
  // case GL_INT:		sizeout = sizeof(GLint);	break;
  case GL_FLOAT:		sizeout = sizeof(GLfloat);	break;
  // case GL_BITMAP:
  /* not implemented yet */
  default:
    return GL_INVALID_ENUM;
  }

  /* Get glPixelStore state */
  unpackrowlength = 0;  //  glGetIntegerv( GL_UNPACK_ROW_LENGTH, &unpackrowlength );
  unpackalignment = 1;  // RETURNS GARBAGE glGetIntegerv( GL_UNPACK_ALIGNMENT, &unpackalignment );
  unpackskiprows = 0;   //  glGetIntegerv( GL_UNPACK_SKIP_ROWS, &unpackskiprows );
  unpackskippixels = 0; //  glGetIntegerv( GL_UNPACK_SKIP_PIXELS, &unpackskippixels );
  packrowlength = 0;    //  glGetIntegerv( GL_PACK_ROW_LENGTH, &packrowlength );
  packalignment = 1;    // RETURNS GARBAGE glGetIntegerv( GL_PACK_ALIGNMENT, &packalignment );
  packskiprows = 0;     //  glGetIntegerv( GL_PACK_SKIP_ROWS, &packskiprows );
  packskippixels = 0;   //  glGetIntegerv( GL_PACK_SKIP_PIXELS, &packskippixels );

  //printf("glumScaleImg components=%d sizein=%d sizeout=%d win=%d hin=%d unpackalignment=%d packalignment=%d\n",
  //	components, sizein, sizeout, widthin, heightin, unpackalignment, packalignment);

  /* Allocate storage for intermediate images */
  tempin = (GLfloat *) malloc( widthin * heightin
			       * components * sizeof(GLfloat) );
  if (!tempin) {
    return GL_OUT_OF_MEMORY;
  }
  tempout = (GLfloat *) malloc( widthout * heightout
				* components * sizeof(GLfloat) );
  if (!tempout) {
    free( tempin );
    return GL_OUT_OF_MEMORY;
  }


  /*
   * Unpack the pixel data and convert to floating point
   */

  if (unpackrowlength>0) {
    rowlen = unpackrowlength;
  }
  else {
    rowlen = widthin;
  }
  if (sizein >= unpackalignment) {
    rowstride = components * rowlen;
  }
  else {
    rowstride = unpackalignment/sizein
      * CEILING( components * rowlen * sizein, unpackalignment );
  }

  //printf("glumScaleImg Unpacking the pixel data and convert to floating point rowlen=%d rowstride=%d\n"
  //	, rowlen, rowstride);
  
  switch (typein) {
  case GL_UNSIGNED_BYTE:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLubyte *ubptr = (GLubyte *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *ubptr++;
      }
    }
    break;
  case GL_BYTE:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLbyte *bptr = (GLbyte *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *bptr++;
      }
    }
    break;
  case GL_UNSIGNED_SHORT:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLushort *usptr = (GLushort *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *usptr++;
      }
    }
    break;
  case GL_SHORT:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLshort *sptr = (GLshort *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *sptr++;
      }
    }
    break;
/*	
  case GL_UNSIGNED_INT:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLuint *uiptr = (GLuint *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *uiptr++;
      }
    }
    break; 
  case GL_INT:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLint *iptr = (GLint *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = (GLfloat) *iptr++;
      }
    }
    break;
*/	
  case GL_FLOAT:
    k = 0;
    for (i=0;i<heightin;i++) {
      GLfloat *fptr = (GLfloat *) datain
	+ i * rowstride
	+ unpackskiprows * rowstride
	+ unpackskippixels * components;
      for (j=0;j<widthin*components;j++) {
	dummy(j, k);
	tempin[k++] = *fptr++;
      }
    }
    break;
  default:
    return GL_INVALID_ENUM;
  }


  /*
   * Scale the image!
   */

  if (widthout > 1)
    sx = (GLfloat) (widthin-1) / (GLfloat) (widthout-1);
  else
    sx = (GLfloat) (widthin-1);
  if (heightout > 1)
    sy = (GLfloat) (heightin-1) / (GLfloat) (heightout-1);
  else
    sy = (GLfloat) (heightin-1);

#ifdef POINT_SAMPLE
  for (i=0;i<heightout;i++) {
    GLint ii = i * sy;
    for (j=0;j<widthout;j++) {
      GLint jj = j * sx;

      GLfloat *src = tempin + (ii * widthin + jj) * components;
      GLfloat *dst = tempout + (i * widthout + j) * components;

      for (k=0;k<components;k++) {
	*dst++ = *src++;
      }
    }
  }
#else
  if (sx<1.0 && sy<1.0) {
    /* magnify both width and height:  use weighted sample of 4 pixels */
    GLint i0, i1, j0, j1;
    GLfloat alpha, beta;
    GLfloat *src00, *src01, *src10, *src11;
    GLfloat s1, s2;
    GLfloat *dst;

    for (i=0;i<heightout;i++) {
      i0 = (int) i * (int) sy;
      i1 = i0 + 1;
      if (i1 >= heightin) i1 = heightin-1;
      /*	 i1 = (i+1) * sy - EPSILON;*/
      alpha = i*sy - i0;
      for (j=0;j<widthout;j++) {
	j0 = (int) j * (int) sx;
	j1 = j0 + 1;
	if (j1 >= widthin) j1 = widthin-1;
	/*	    j1 = (j+1) * sx - EPSILON; */
	beta = j*sx - j0;

	/* compute weighted average of pixels in rect (i0,j0)-(i1,j1) */
	src00 = tempin + (i0 * widthin + j0) * components;
	src01 = tempin + (i0 * widthin + j1) * components;
	src10 = tempin + (i1 * widthin + j0) * components;
	src11 = tempin + (i1 * widthin + j1) * components;

	dst = tempout + (i * widthout + j) * components;

	for (k=0;k<components;k++) {
	  s1 = *src00++ * (1.0f-beta) + *src01++ * beta;
	  s2 = *src10++ * (1.0f-beta) + *src11++ * beta;
	  *dst++ = s1 * (1.0f-alpha) + s2 * alpha;
	}
      }
    }
  }
  else {
    /* shrink width and/or height:  use an unweighted box filter */
    GLint i0, i1;
    GLint j0, j1;
    GLint ii, jj;
    GLfloat sum, *dst;

    for (i=0;i<heightout;i++) {
      i0 = (int) i * (int) sy;
      i1 = i0 + 1;
      if (i1 >= heightin) i1 = heightin-1; 
      /*	 i1 = (i+1) * sy - EPSILON; */
      for (j=0;j<widthout;j++) {
	j0 = (int) j * (int) sx;
	j1 = j0 + 1;
	if (j1 >= widthin) j1 = widthin-1;
	/*	    j1 = (j+1) * sx - EPSILON; */

	dst = tempout + (i * widthout + j) * components;

	/* compute average of pixels in the rectangle (i0,j0)-(i1,j1) */
	for (k=0;k<components;k++) {
	  sum = 0.0;
	  for (ii=i0;ii<=i1;ii++) {
	    for (jj=j0;jj<=j1;jj++) {
	      sum += *(tempin + (ii * widthin + jj) * components + k);
	    }
	  }
	  sum /= (j1-j0+1) * (i1-i0+1);
	  *dst++ = sum;
	}
      }
    }
  }
#endif


  /*
   * Return output image
   */

  if (packrowlength>0) {
    rowlen = packrowlength;
  }
  else {
    rowlen = widthout;
  }
  if (sizeout >= packalignment) {
    rowstride = components * rowlen;
  }
  else {
    rowstride = packalignment/sizeout
      * CEILING( components * rowlen * sizeout, packalignment );
  }

  switch (typeout) {
  case GL_UNSIGNED_BYTE:
    k = 0;

    for (i=0;i<heightout;i++) {
      GLubyte *ubptr = (GLubyte *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;

      for (j=0;j<widthout*components;j++) {
		dummy(j, k+i);
		*ubptr++ = (GLubyte) tempout[k++];
      }
    }

    break;
  case GL_BYTE:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLbyte *bptr = (GLbyte *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*bptr++ = (GLbyte) tempout[k++];
      }
    }
    break;
  case GL_UNSIGNED_SHORT:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLushort *usptr = (GLushort *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*usptr++ = (GLushort) tempout[k++];
      }
    }
    break;
  case GL_SHORT:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLshort *sptr = (GLshort *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*sptr++ = (GLshort) tempout[k++];
      }
    }
    break;
#ifdef GL_UNSIGNED_INT
  case GL_UNSIGNED_INT:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLuint *uiptr = (GLuint *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*uiptr++ = (GLuint) tempout[k++];
      }
    }
    break;
#endif
#ifdef GL_INT
  case GL_INT:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLint *iptr = (GLint *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*iptr++ = (GLint) tempout[k++];
      }
    }
    break;
#endif
  case GL_FLOAT:
    k = 0;
    for (i=0;i<heightout;i++) {
      GLfloat *fptr = (GLfloat *) dataout
	+ i * rowstride
	+ packskiprows * rowstride
	+ packskippixels * components;
      for (j=0;j<widthout*components;j++) {
	dummy(j, k+i);
	*fptr++ = tempout[k++];
      }
    }
    break;
  default:
    return GL_INVALID_ENUM;
  }

  /* free temporary image storage */
  free( tempin );
  free( tempout );

  return 0;
}



GLint glBuild2DMipmaps( GLenum target, GLenum components,
			 GLint width, GLint height, GLenum format,
			 GLenum type, const void *data )
{
  GLint w, h, maxsize;
  void *image;
  GLint neww, newh, level, bpp;
  int error;

  if (width < 1 || height < 1)
    return GL_INVALID_VALUE;

  glGetIntegerv( GL_MAX_TEXTURE_SIZE, &maxsize );

  w = round2( width );
  h = round2( height );

  while (width < maxsize)
    {
      maxsize = maxsize/2;
    }

  while (height < maxsize)
    {
      maxsize = maxsize/2;
    }

  if (w>maxsize) w = maxsize;
  if (h>maxsize) h = maxsize;

  bpp = bytes_per_pixel( format, type );
  
  // printf("gluBuild2DMipmaps bpp=%d w=%d h=%d fmt=%d type=%d\n", bpp, width, height, format, type);
  
  if (bpp==0) {
    /* probably a bad format or type enum */
    return GL_INVALID_ENUM;
  }

  if (w!=width || h!=height) {
    /* must rescale image to get "top" mipmap texture image */
    image = malloc( (w+4) * h * bpp );
    if (!image) {
      return GL_OUT_OF_MEMORY;
    }
    error = glScaleImage( format, width, height, type, data,
			   w, h, type, image );
    if (error) {
      return error;
    }
  }
  else {
    image = (void *) data;
  }

  level = 0;
  while (1) {
	components = format; // Vladimir

    glTexImage2D( target, level, components, w, h, 0, format, type, image );
    if (1) {
    #ifdef QT_GUI_LIB
    	QImage( (const uchar*)image, w, h, QImage::Format_Alpha8).save(QString("mip%1.png").arg(level));
    	qDebug() << "*** saving" << QString("mip%1.png").arg(level);
    #endif
    }

    if (w==1 && h==1)  break;

    neww = (w<2) ? 1 : w/2;
    newh = (h<2) ? 1 : h/2;

    error =  glScaleImage( format, w, h, type, data, neww, newh, type, image );
    if (error) {
      return error;
    }

    w = neww;
    h = newh;
    level++;
  }

  if (image!=data) {
    free( image );
  }

  return 0;
}
