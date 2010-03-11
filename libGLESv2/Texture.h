//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Texture.h: Defines the abstract gl::Texture class and its concrete derived
// classes Texture2D and TextureCubeMap. Implements GL texture objects and
// related functionality. [OpenGL ES 2.0.24] section 3.7 page 63.

#ifndef LIBGLESV2_TEXTURE_H_
#define LIBGLESV2_TEXTURE_H_

#include "Renderbuffer.h"

#define GL_APICALL
#include <GLES2/gl2.h>
#include <d3d9.h>

#include <vector>

namespace gl
{
enum
{
    MAX_TEXTURE_SIZE = 2048,
    MAX_CUBE_MAP_TEXTURE_SIZE = 2048,

    MAX_TEXTURE_LEVELS = 11   // log2 of MAX_TEXTURE_SIZE
};

class Texture : public Colorbuffer
{
  public:
    Texture();

    ~Texture();

    virtual GLenum getTarget() const = 0;

    bool setMinFilter(GLenum filter);
    bool setMagFilter(GLenum filter);
    bool setWrapS(GLenum wrap);
    bool setWrapT(GLenum wrap);

    GLenum getMinFilter();
    GLenum getMagFilter();
    GLenum getWrapS();
    GLenum getWrapT();

    virtual bool isComplete() = 0;
    IDirect3DBaseTexture9 *getTexture();

  protected:
    // Helper structure representing a single image layer
    struct Image
    {
        GLenum internalFormat;
        GLsizei width;
        GLsizei height;
        GLenum format;
        GLenum type;

        std::vector<unsigned char> pixels;
    };

    void copyImage(const D3DLOCKED_RECT &lock, D3DFORMAT format, Image *image);

    static D3DFORMAT selectFormat(const Image &image);
    static int pixelSize(const Image &image);

    GLenum mMinFilter;
    GLenum mMagFilter;
    GLenum mWrapS;
    GLenum mWrapT;

    void setImage(GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels, Image *img);

    // The pointer returned is weak and it is assumed the derived class will keep a strong pointer until the next createTexture() call.
    virtual IDirect3DBaseTexture9 *createTexture() = 0;

    bool mDirtyImageData; // FIXME: would be private but getRenderTarget is still implemented through the derived classes and they need it.

  private:
    DISALLOW_COPY_AND_ASSIGN(Texture);

    IDirect3DBaseTexture9 *mBaseTexture; // This is a weak pointer. The derived class is assumed to own a strong pointer.
};

class Texture2D : public Texture
{
  public:
    Texture2D();

    ~Texture2D();

    GLenum getTarget() const;

    void setImage(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

    bool setMinFilter(GLenum filter);
    bool setMagFilter(GLenum filter);
    bool setWrapS(GLenum wrap);
    bool setWrapT(GLenum wrap);

    GLenum getMinFilter();
    GLenum getMagFilter();
    GLenum getWrapS();
    GLenum getWrapT();

    bool isComplete();
    IDirect3DSurface9 *getRenderTarget();

  private:
    DISALLOW_COPY_AND_ASSIGN(Texture2D);

    virtual IDirect3DBaseTexture9 *createTexture();

    Image mImageArray[MAX_TEXTURE_LEVELS];

    IDirect3DTexture9 *mTexture;
};

class TextureCubeMap : public Texture
{
  public:
    TextureCubeMap();

    ~TextureCubeMap();

    GLenum getTarget() const;

    void setImagePosX(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void setImageNegX(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void setImagePosY(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void setImageNegY(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void setImagePosZ(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void setImageNegZ(GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

    bool isComplete();

  private:
    DISALLOW_COPY_AND_ASSIGN(TextureCubeMap);

    virtual IDirect3DBaseTexture9 *createTexture();

    void setImage(int face, GLint level, GLenum internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);

    Image mImageArray[6][MAX_TEXTURE_LEVELS];

    IDirect3DCubeTexture9 *mTexture;
};
}

#endif   // LIBGLESV2_TEXTURE_H_