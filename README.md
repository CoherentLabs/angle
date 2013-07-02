ANGLE project - Coherent Labs AD extensions
=====

ANGLE is a GLES2/EGL wrapper implemented via DirectX 9 & 11. For more information visit the project's official repo - http://code.google.com/p/angleproject/.

Motivation
=====

This repo clone primarily addresses the issue that it is not possible to use a pre-created DirectX rendering device in ANGLE. By adding such a feature users can now 
freely mix DirectX and OpenGL ES code backed by the same device and hence sharing the same resources. No special handling is needed in order to access content rendered via the OpenGL API 
from DirectX as the same device owns it all.

Changes
=====

This clone is sponsored by Coherent Labs AD and adds the following features:

 - The ability to use client-supplied rendering devices in ANGLE. This allows you to mix DirectX & Open GL ES 2 rendering
 APIs and still use the same rendering device. 
 The feature is implemented via the EGL_ANGLE_direct3d_device_existing extension. A full documentation for the extension is planned but not ready yet.
 The new functions in the extension are:
  - EGLDisplay eglGetDisplayANGLE(EGLNativeDisplayType displayId, EGLint type, void* device) - creates a display from an existing Dx9, Dx9
Ex or Dx11 device.
  - EGLBoolean eglTryRestoreDeviceANGLE(EGLDisplay display) - frees all rendering resources held by ANGLE so that the device can be restored by the client. If the device is already restored it reinits all internal resources.
  - void eglBeginRenderingANGLE(EGLDisplay display) - marks the begginning of OpenGL rendering. Captures all th device state before the call and applies the last saved ANGLE device state.
  - void eglEndRenderingANGLE(EGLDisplay display) - saves the ANGLE device state and re-applies the lasst saved client state.
 All OpenGL rendering when using a client device must happen in a eglBeginRenderingANGLE / eglEndRenderingANGLE block. These calls prevent device state to leak between the client code and ANGLE. Failing to do so will result in undefined rendering behavior.
  - void eglForceDisableSharedTexturesANGLE(EGLBoolean disable) - Forces ANGLE not support shared textures even on devices that could support it (like Dx9Ex and Dx11).
 
 - Some APIs have been moved to higher abstract interfaces to make embedding in certain applications easier and accessing some internal resources faster.
 
 - New sample - "Hello_Triangle_ExtDevice". This new sample shows how you can use an externally created DirectX device to power ANGLE. It also show how to properly reset a device and keep rendering states separate between the OpenGL emulation and the client code.

Future
=====

We plan to update this feature-set and enhance it. The new features and extensions are not part of the official ANGLE project and repository.
In the future we might try to push them to the official repo after consultation with the authors of the ANGLE project.

License
=====
This code is governed by the same permissive license that is used in ANGLE. Please refer to the LICENSE file for details.
