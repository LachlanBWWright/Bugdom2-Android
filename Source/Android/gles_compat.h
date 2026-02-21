// GLES COMPATIBILITY BRIDGE
// Emulates OpenGL 1.x/2.x fixed-function pipeline on top of OpenGL ES 3.0
// Used for the Android port of Bugdom 2.

#pragma once

#ifdef __ANDROID__

#include <GLES3/gl3.h>
#include <stdbool.h>

// GLdouble is not defined by GLES3 headers; define it here so that callers
// that use glOrtho / glFrustum (which take double parameters on desktop GL)
// can compile without changes.
typedef double GLdouble;

// Function pointer types used in OGL_Support.c (not defined in GLES3 headers)
typedef void (*PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (*PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);

#ifdef __cplusplus
extern "C" {
#endif

// -------------------------------------------------------------------------
// Initialization / Shutdown
// -------------------------------------------------------------------------

void GLESBridge_Init(void);
void GLESBridge_Shutdown(void);

// -------------------------------------------------------------------------
// Matrix stack emulation
// -------------------------------------------------------------------------

#define GL_MODELVIEW   0x1700
#define GL_PROJECTION  0x1701
#define GL_TEXTURE     0x1702

void bridge_MatrixMode(GLenum mode);
void bridge_LoadIdentity(void);
void bridge_LoadMatrixf(const GLfloat *m);
void bridge_MultMatrixf(const GLfloat *m);
void bridge_PushMatrix(void);
void bridge_PopMatrix(void);
void bridge_Translatef(GLfloat x, GLfloat y, GLfloat z);
void bridge_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
void bridge_Scalef(GLfloat x, GLfloat y, GLfloat z);
void bridge_Ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
void bridge_Frustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);

// -------------------------------------------------------------------------
// Lighting emulation
// -------------------------------------------------------------------------

#define GL_LIGHT0       0x4000
#define GL_LIGHT1       0x4001
#define GL_LIGHT2       0x4002
#define GL_LIGHT3       0x4003
#define GL_LIGHT4       0x4004
#define GL_LIGHT5       0x4005
#define GL_LIGHT6       0x4006
#define GL_LIGHT7       0x4007

#define GL_AMBIENT      0x1200
#define GL_DIFFUSE      0x1201
#define GL_SPECULAR     0x1202
#define GL_POSITION     0x1203
#define GL_EMISSION     0x1600
#define GL_SHININESS    0x1601
#define GL_AMBIENT_AND_DIFFUSE  0x1602

#define GL_LIGHT_MODEL_AMBIENT  0x0B53

#define GL_FRONT_AND_BACK 0x0408
#define GL_FRONT          0x0404
#define GL_BACK           0x0405

void bridge_Lightfv(GLenum light, GLenum pname, const GLfloat *params);
void bridge_LightModelfv(GLenum pname, const GLfloat *params);
void bridge_Materialfv(GLenum face, GLenum pname, const GLfloat *params);
void bridge_ColorMaterial(GLenum face, GLenum mode);

// -------------------------------------------------------------------------
// Fog emulation
// -------------------------------------------------------------------------

#define GL_FOG           0x0B60
#define GL_FOG_MODE      0x0B65
#define GL_FOG_DENSITY   0x0B62
#define GL_FOG_START     0x0B63
#define GL_FOG_END       0x0B64
#define GL_FOG_COLOR     0x0B66
#define GL_FOG_HINT      0x0C54
#define GL_LINEAR        0x2601
#define GL_EXP           0x0800
#define GL_EXP2          0x0801
#define GL_NICEST        0x1102
#define GL_FASTEST       0x1101
#define GL_DONT_CARE     0x1100

void bridge_Fogf(GLenum pname, GLfloat param);
void bridge_Fogfv(GLenum pname, const GLfloat *params);
void bridge_Fogi(GLenum pname, GLint param);

// -------------------------------------------------------------------------
// Alpha test emulation
// -------------------------------------------------------------------------

#define GL_ALPHA_TEST     0x0BC0
#define GL_ALWAYS         0x0207
#define GL_NEVER          0x0200
#define GL_LESS           0x0201
#define GL_EQUAL          0x0202
#define GL_LEQUAL         0x0203
#define GL_GREATER        0x0204
#define GL_NOTEQUAL       0x0205
#define GL_GEQUAL         0x0206

void bridge_AlphaFunc(GLenum func, GLfloat ref);

// -------------------------------------------------------------------------
// Fixed-function state (stubbed or emulated)
// -------------------------------------------------------------------------

#define GL_LIGHTING        0x0B50
#define GL_COLOR_MATERIAL  0x0B57
#define GL_NORMALIZE       0x0BA1
#define GL_RESCALE_NORMAL  0x803A

// These are handled by the bridge's Enable/Disable
void bridge_Enable(GLenum cap);
void bridge_Disable(GLenum cap);
bool bridge_IsEnabled(GLenum cap);

// -------------------------------------------------------------------------
// Client state / vertex arrays
// -------------------------------------------------------------------------

#define GL_VERTEX_ARRAY         0x8074
#define GL_NORMAL_ARRAY         0x8075
#define GL_COLOR_ARRAY          0x8076
#define GL_TEXTURE_COORD_ARRAY  0x8078

void bridge_EnableClientState(GLenum array);
void bridge_DisableClientState(GLenum array);
void bridge_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void bridge_NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
void bridge_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
void bridge_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

// -------------------------------------------------------------------------
// Uniform current color
// -------------------------------------------------------------------------

void bridge_Color4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a);
void bridge_Color4fv(const GLfloat *v);

// -------------------------------------------------------------------------
// Draw calls (wrap to upload VBOs first)
// -------------------------------------------------------------------------

void bridge_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
void bridge_DrawArrays(GLenum mode, GLint first, GLsizei count);

// -------------------------------------------------------------------------
// Immediate mode (glBegin/glEnd) for debug rendering
// -------------------------------------------------------------------------

void bridge_Begin(GLenum mode);
void bridge_End(void);
void bridge_Vertex3f(GLfloat x, GLfloat y, GLfloat z);
void bridge_Vertex2f(GLfloat x, GLfloat y);
void bridge_Normal3f(GLfloat x, GLfloat y, GLfloat z);
void bridge_TexCoord2f(GLfloat s, GLfloat t);

// -------------------------------------------------------------------------
// Hint (stub)
// -------------------------------------------------------------------------

// GL_FOG_HINT etc. – no-op on GLES
void bridge_Hint(GLenum target, GLenum mode);

// -------------------------------------------------------------------------
// Polygon mode (stub – GLES has no wireframe)
// -------------------------------------------------------------------------

#define GL_POINT 0x1B00
#define GL_LINE  0x1B01
#define GL_FILL  0x1B02

void bridge_PolygonMode(GLenum face, GLenum mode);

// -------------------------------------------------------------------------
// Texture environment (stub – bridge always uses GL_MODULATE behavior)
// -------------------------------------------------------------------------

#define GL_TEXTURE_ENV         0x2300
#define GL_TEXTURE_ENV_MODE    0x2200
#define GL_MODULATE            0x2100
#define GL_DECAL               0x2101
#define GL_COMBINE             0x8570
#define GL_COMBINE_RGB         0x8571
#define GL_COMBINE_ALPHA       0x8572
#define GL_ADD                 0x0104

// glTexEnvi is not available in GLES3; stub it since bridge handles texturing
static inline void bridge_TexEnvi(GLenum target, GLenum pname, GLint param)
{
    (void)target; (void)pname; (void)param;
}

// -------------------------------------------------------------------------
// State query wrappers
// (GL_CURRENT_COLOR, GL_PROJECTION_MATRIX, GL_BLEND_SRC/DST etc. are not
//  queryable in GLES3; bridge_Get* intercepts them and falls through to the
//  real glGet* for all other pnames.)
// -------------------------------------------------------------------------

#define GL_CURRENT_COLOR        0x0B00
#define GL_PROJECTION_MATRIX    0x0BA7
#define GL_MODELVIEW_MATRIX     0x0BA6
#define GL_BLEND_SRC            0x0BE1
#define GL_BLEND_DST            0x0BE0

void bridge_GetFloatv(GLenum pname, GLfloat *params);
void bridge_GetIntegerv(GLenum pname, GLint *params);

#define glGetFloatv     bridge_GetFloatv
#define glGetIntegerv   bridge_GetIntegerv

// Intercept glBlendFunc to track blend state for bridge_GetIntegerv
void bridge_BlendFunc(GLenum sfactor, GLenum dfactor);
#define glBlendFunc bridge_BlendFunc

// -------------------------------------------------------------------------
// Dirty-flag: call before render to flush shader state
// -------------------------------------------------------------------------

void bridge_FlushState(void);

#ifdef __cplusplus
}
#endif

// =========================================================================
// Macro redirects
// =========================================================================

// Matrix stack
#define glMatrixMode        bridge_MatrixMode
#define glLoadIdentity      bridge_LoadIdentity
#define glLoadMatrixf       bridge_LoadMatrixf
#define glMultMatrixf       bridge_MultMatrixf
#define glPushMatrix        bridge_PushMatrix
#define glPopMatrix         bridge_PopMatrix
#define glTranslatef        bridge_Translatef
#define glRotatef           bridge_Rotatef
#define glScalef            bridge_Scalef
#define glOrtho             bridge_Ortho
#define glFrustum           bridge_Frustum

// Lighting
#define glLightfv           bridge_Lightfv
#define glLightModelfv      bridge_LightModelfv
#define glMaterialfv        bridge_Materialfv
#define glColorMaterial     bridge_ColorMaterial

// Fog
#define glFogf              bridge_Fogf
#define glFogfv             bridge_Fogfv
#define glFogi              bridge_Fogi
#define glHint              bridge_Hint

// Alpha test
#define glAlphaFunc         bridge_AlphaFunc

// State
#define glEnable            bridge_Enable
#define glDisable           bridge_Disable
#define glIsEnabled         bridge_IsEnabled

// Client state / arrays
#define glEnableClientState     bridge_EnableClientState
#define glDisableClientState    bridge_DisableClientState
#define glVertexPointer         bridge_VertexPointer
#define glNormalPointer         bridge_NormalPointer
#define glTexCoordPointer       bridge_TexCoordPointer
#define glColorPointer          bridge_ColorPointer

// Current color
#define glColor4f           bridge_Color4f
#define glColor4fv          bridge_Color4fv

// Draw calls
#define glDrawElements      bridge_DrawElements
#define glDrawArrays        bridge_DrawArrays

// Immediate mode
#define glBegin             bridge_Begin
#define glEnd               bridge_End
#define glVertex3f          bridge_Vertex3f
#define glVertex2f          bridge_Vertex2f
#define glNormal3f          bridge_Normal3f
#define glTexCoord2f        bridge_TexCoord2f

// Polygon mode (stub)
#define glPolygonMode       bridge_PolygonMode

// Texture environment (stub)
#define glTexEnvi           bridge_TexEnvi

// Texture generation (stub – sphere mapping not available in GLES3)
#ifndef GL_S
#define GL_S                    0x2000
#endif
#ifndef GL_T
#define GL_T                    0x2001
#endif
#ifndef GL_TEXTURE_GEN_MODE
#define GL_TEXTURE_GEN_MODE     0x2500
#endif
#ifndef GL_SPHERE_MAP
#define GL_SPHERE_MAP           0x2402
#endif
#ifndef GL_TEXTURE_GEN_S
#define GL_TEXTURE_GEN_S        0x0C60
#endif
#ifndef GL_TEXTURE_GEN_T
#define GL_TEXTURE_GEN_T        0x0C61
#endif
static inline void bridge_TexGeni(GLenum coord, GLenum pname, GLint param)
{
    (void)coord; (void)pname; (void)param;
}
#define glTexGeni           bridge_TexGeni

// Redirect unsupported enums / formats
// These desktop-GL / extension constants are not defined by GLES3/gl3.h.
#ifndef GL_BGRA
#define GL_BGRA                         0x80E1
#endif
#ifndef GL_BGR
#define GL_BGR                          0x80E0
#endif
#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT                     0x80E1
#endif
#ifndef GL_UNSIGNED_INT_8_8_8_8_REV
#define GL_UNSIGNED_INT_8_8_8_8_REV     0x8367
#endif
#ifndef GL_UNSIGNED_SHORT_1_5_5_5_REV
#define GL_UNSIGNED_SHORT_1_5_5_5_REV   0x8366
#endif

// Multi-texture ARB aliases (GLES3 has the base versions)
#define GL_TEXTURE0_ARB     GL_TEXTURE0
#define GL_TEXTURE1_ARB     GL_TEXTURE1

// GL_QUADS is not in GLES3; define the desktop value so bridge_End can detect it.
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif

// GL_UNPACK_ROW_LENGTH is supported in GLES3 (already defined in gl3.h)
#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH    0x0CF2
#endif

#endif // __ANDROID__
