//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Program.h: Defines the gl::Program class. Implements GL program objects
// and related functionality. [OpenGL ES 2.0.24] section 2.10.3 page 28.

#ifndef LIBGLESV2_PROGRAM_H_
#define LIBGLESV2_PROGRAM_H_

#include "Context.h"

#include <d3dx9.h>
#include <string>
#include <vector>

namespace gl
{
class FragmentShader;
class VertexShader;

enum UniformType
{
    UNIFORM_1FV,
    UNIFORM_2FV,
    UNIFORM_3FV,
    UNIFORM_4FV,
    UNIFORM_MATRIX_2FV,
    UNIFORM_MATRIX_3FV,
    UNIFORM_MATRIX_4FV,
    UNIFORM_1IV
};

// Helper struct representing a single shader uniform
struct Uniform
{
    Uniform(UniformType type, const std::string &name, unsigned int bytes);

    ~Uniform();

    const UniformType type;
    const std::string name;
    const unsigned int bytes;
    unsigned char *data;
};

class Program
{
  public:
    Program();

    ~Program();

    bool attachShader(Shader *shader);
    bool detachShader(Shader *shader);

    IDirect3DPixelShader9 *getPixelShader();
    IDirect3DVertexShader9 *getVertexShader();

    void bindAttributeLocation(GLuint index, const char *name);
    GLuint getAttributeLocation(const char *name);
    bool isActiveAttribute(int attributeIndex);
    int getInputMapping(int attributeIndex);

    GLint getSamplerMapping(unsigned int samplerIndex);

    GLint getUniformLocation(const char *name);
    bool setUniform1fv(GLint location, GLsizei count, const GLfloat *v);
    bool setUniform2fv(GLint location, GLsizei count, const GLfloat *v);
    bool setUniform3fv(GLint location, GLsizei count, const GLfloat *v);
    bool setUniform4fv(GLint location, GLsizei count, const GLfloat *v);
    bool setUniformMatrix2fv(GLint location, GLsizei count, const GLfloat *value);
    bool setUniformMatrix3fv(GLint location, GLsizei count, const GLfloat *value);
    bool setUniformMatrix4fv(GLint location, GLsizei count, const GLfloat *value);
    bool setUniform1iv(GLint location, GLsizei count, const GLint *v);

    void applyUniforms();

    void link();
    bool isLinked();

    void flagForDeletion();
    bool isFlaggedForDeletion() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(Program);

    ID3DXBuffer *compileToBinary(const char *hlsl, const char *profile, ID3DXConstantTable **constantTable);
    void unlink(bool destroy = false);

    bool linkAttributes();
    bool linkUniforms(ID3DXConstantTable *constantTable);
    bool defineUniform(const D3DXHANDLE &constantHandle, const D3DXCONSTANT_DESC &constantDescription, std::string name = "");
    bool defineUniform(const D3DXCONSTANT_DESC &constantDescription, std::string &name);
    Uniform *createUniform(const D3DXCONSTANT_DESC &constantDescription, std::string &name);
    bool applyUniform1fv(GLint location, GLsizei count, const GLfloat *v);
    bool applyUniform2fv(GLint location, GLsizei count, const GLfloat *v);
    bool applyUniform3fv(GLint location, GLsizei count, const GLfloat *v);
    bool applyUniform4fv(GLint location, GLsizei count, const GLfloat *v);
    bool applyUniformMatrix2fv(GLint location, GLsizei count, const GLfloat *value);
    bool applyUniformMatrix3fv(GLint location, GLsizei count, const GLfloat *value);
    bool applyUniformMatrix4fv(GLint location, GLsizei count, const GLfloat *value);
    bool applyUniform1iv(GLint location, GLsizei count, const GLint *v);

    FragmentShader *mFragmentShader;
    VertexShader *mVertexShader;

    IDirect3DPixelShader9 *mPixelExecutable;
    IDirect3DVertexShader9 *mVertexExecutable;
    ID3DXConstantTable *mConstantTablePS;
    ID3DXConstantTable *mConstantTableVS;

    char *mAttributeName[MAX_VERTEX_ATTRIBS];
    int mInputMapping[MAX_VERTEX_ATTRIBS];

    GLint mSamplerMapping[MAX_TEXTURE_IMAGE_UNITS];

    typedef std::vector<Uniform*> UniformArray;
    UniformArray mUniforms;

    bool mLinked;
    bool mDeleteStatus;   // Flag to indicate that the program can be deleted when no longer in use
};
}

#endif   // LIBGLESV2_PROGRAM_H_