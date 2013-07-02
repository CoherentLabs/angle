// Copyright (c) 2013 Coherent Labs AD, Stoyan Nikolov. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <d3d9.h>
#include <d3d11.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#define WIDTH 640
#define HEIGHT 480

void CleanUpGLES();
bool InitializeGLES();

enum DeviceType
{
	DT_Dx9,
	DT_Dx9Ex,
	DT_Dx11
};

static const DeviceType g_DeviceType = DT_Dx9;

struct _Context {
	~_Context() {
		if(Device) Device->Release();
		if(D3D) D3D->Release();
		if(Context11) Context11->Release();
		if(Device11) Device11->Release();

		if(D3DModule) ::FreeLibrary(D3DModule);
	}
	HWND hWnd;
	HMODULE D3DModule;
	D3DPRESENT_PARAMETERS PresentParams;
	IDirect3D9* D3D;
	IDirect3DDevice9* Device;
	ID3D11Device* Device11;
	ID3D11DeviceContext* Context11;
	bool HasLostDevice;

	EGLDisplay Display;
	EGLSurface Surface;
	EGLContext Context;
	GLuint ProgramObject;

	PFNEGLGETDISPLAYANGLE eglGetDisplayANGLE;
	PFNEGLTRYRESTOREDEVICEANGLE eglTryRestoreDeviceANGLE;
	PFNEGLBEGINRENDERINGANGLE eglBeginRenderingANGLE;
	PFNEGLENDRENDERINGANGLE eglEndRenderingANGLE;
	PFNGLGETGRAPHICSRESETSTATUSEXTPROC glGetGraphicsResetStatusEXT;
} g_Context = {0};

///
// Create a shader object, load the shader source, and
// compile the shader.
//
GLuint LoadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = static_cast<char*>(malloc(sizeof(char) * infoLen ));

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         
         free ( infoLog );
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;
}

///
// Initialize the shader and program object
//
int Init ()
{
   g_Context.eglBeginRenderingANGLE(g_Context.Display);

   const char vShaderStr[] =  
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   
   const char fShaderStr[] =  
      "precision mediump float;\n"\
      "void main()                                  \n"
      "{                                            \n"
      "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                            \n";

   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = LoadShader ( GL_VERTEX_SHADER, vShaderStr );
   fragmentShader = LoadShader ( GL_FRAGMENT_SHADER, fShaderStr );

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
   {
      g_Context.eglEndRenderingANGLE(g_Context.Display);
      return 0;
   }

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Bind vPosition to attribute 0   
   glBindAttribLocation ( programObject, 0, "vPosition" );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = static_cast<char*>(malloc (sizeof(char) * infoLen ));

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         
         free ( infoLog );
      }

      glDeleteProgram ( programObject );
	  g_Context.eglEndRenderingANGLE(g_Context.Display);
      return FALSE;
   }

   // Store the program object
   g_Context.ProgramObject = programObject;

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

   g_Context.eglEndRenderingANGLE(g_Context.Display);

   return TRUE;
}

void ResetDevice()
{
	CleanUpGLES();
	
	g_Context.eglTryRestoreDeviceANGLE(g_Context.Display); // this call will fail but will clear all internal used resources

	if(g_Context.Device)
	{
		HRESULT result = g_Context.Device->TestCooperativeLevel();
	    while (result == D3DERR_DEVICELOST)
	    {
	       Sleep(100);
	       result = g_Context.Device->TestCooperativeLevel();
	    }
	    if (result == D3DERR_DEVICENOTRESET)
	    {
	        result = g_Context.Device->Reset(&g_Context.PresentParams);
	    }
		if(result != D3D_OK)
			return;
	}

	if(!g_Context.eglTryRestoreDeviceANGLE(g_Context.Display)) // this call must succeed and will restore the internal state in GLES
		return;

	if(!InitializeGLES())
	   return;
	if(!Init())
	   return;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw ( )
{
   if(g_Context.HasLostDevice)
   {
	   if(g_Context.glGetGraphicsResetStatusEXT() == GL_NO_ERROR)
	   {
		   ResetDevice();
	   }
	   g_Context.HasLostDevice = false;
   }
   
   if(g_Context.HasLostDevice)
	   return;

   g_Context.eglBeginRenderingANGLE(g_Context.Display);

   GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f, 
                           -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f, 0.0f };
      
   // Set the viewport
   glViewport ( 0, 0, WIDTH, HEIGHT );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( g_Context.ProgramObject );

   // Load the vertex data
   glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
   glEnableVertexAttribArray ( 0 );

   glDrawArrays ( GL_TRIANGLES, 0, 3 );

   g_Context.HasLostDevice = !eglSwapBuffers ( g_Context.Display, g_Context.Surface );
   
   g_Context.eglEndRenderingANGLE(g_Context.Display);
}

LRESULT WINAPI WindowProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) 
{
   LRESULT  lRet = 1; 

   switch (uMsg) 
   { 
      case WM_CREATE:
         break;
      case WM_PAINT:
         {
            Draw();
            ValidateRect(hWnd, NULL);
         }
         break;
      case WM_DESTROY:
         PostQuitMessage(0);             
         break; 
      default: 
         lRet = DefWindowProc (hWnd, uMsg, wParam, lParam); 
         break; 
   } 

   return lRet; 
}

HWND CreateWin(unsigned width, unsigned height, const char* title)
{
	WNDCLASS wndclass = {0}; 
	DWORD    wStyle   = 0;
	RECT     windowRect;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	
	wndclass.style         = CS_OWNDC;
	wndclass.lpfnWndProc   = (WNDPROC)WindowProc; 
	wndclass.hInstance     = hInstance; 
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); 
	wndclass.lpszClassName = TEXT("opengles2.0");
	
	if (!RegisterClass (&wndclass) ) 
	   return FALSE; 
	
	wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_SIZEBOX;
	
	// Adjust the window rectangle so that the client area has
	// the correct number of pixels
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.right = width;
	windowRect.bottom = height;
	
	AdjustWindowRect ( &windowRect, wStyle, FALSE );
	
	HWND hWnd = CreateWindow(
	                      TEXT("opengles2.0"),
	                      title,
	                      wStyle,
	                      0,
	                      0,
	                      windowRect.right - windowRect.left,
	                      windowRect.bottom - windowRect.top,
	                      NULL,
	                      NULL,
	                      hInstance,
	                      NULL);
	
	if(!hWnd)
		return NULL;

	ShowWindow(hWnd, TRUE);
	
	return hWnd;
}

void WinLoop ()
{
   MSG msg = { 0 };
   int done = 0;
   DWORD lastTime = GetTickCount();
   
   while (!done)
   {
      int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
      if ( gotMsg )
      {
         if (msg.message==WM_QUIT)
         {
             done=1; 
         }
         else
         {
             TranslateMessage(&msg); 
             DispatchMessage(&msg); 
         }
      }
      else {
         SendMessage( g_Context.hWnd, WM_PAINT, 0, 0 );
	  }
   }
}

bool InitializeDirectX9() {
	g_Context.D3DModule = LoadLibrary("d3d9.dll");
	if(!g_Context.D3DModule) 
		return false;

	typedef HRESULT (WINAPI *Direct3DCreate9ExFunc)(UINT, IDirect3D9Ex**);
	Direct3DCreate9ExFunc Direct3DCreate9ExPtr = reinterpret_cast<Direct3DCreate9ExFunc>(GetProcAddress(g_Context.D3DModule, "Direct3DCreate9Ex"));

	if(g_DeviceType == DT_Dx9Ex)
	{
		IDirect3D9Ex* d3d9ex = NULL;
		Direct3DCreate9ExPtr(D3D_SDK_VERSION, &d3d9ex);
		g_Context.D3D = d3d9ex;
	}
	else
	{
		g_Context.D3D = ::Direct3DCreate9(D3D_SDK_VERSION);
	}
	if(!g_Context.D3D)
		return false;

	// Initialize with a dummy window
	HWND window = GetShellWindow();
	g_Context.PresentParams.BackBufferWidth = 1;
	g_Context.PresentParams.BackBufferHeight = 1;
	g_Context.PresentParams.BackBufferCount = 1;
	g_Context.PresentParams.BackBufferFormat = D3DFMT_A8R8G8B8;
	g_Context.PresentParams.hDeviceWindow = window;
	g_Context.PresentParams.Windowed = TRUE;
	g_Context.PresentParams.Flags = 0;
	g_Context.PresentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	g_Context.PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

	HRESULT hr = S_OK;

	if(g_DeviceType == DT_Dx9Ex)
	{
		IDirect3DDevice9Ex* deviceEx = NULL;
		hr = static_cast<IDirect3D9Ex*>(g_Context.D3D)->CreateDeviceEx(
						D3DADAPTER_DEFAULT
						, D3DDEVTYPE_HAL
						, window
						, D3DCREATE_FPU_PRESERVE | D3DCREATE_HARDWARE_VERTEXPROCESSING
						, &g_Context.PresentParams
						, NULL
						, &deviceEx);
		g_Context.Device = deviceEx;
	}
	else
	{
		hr = g_Context.D3D->CreateDevice(
						D3DADAPTER_DEFAULT
						, D3DDEVTYPE_HAL
						, window
						, D3DCREATE_FPU_PRESERVE | D3DCREATE_HARDWARE_VERTEXPROCESSING
						, &g_Context.PresentParams
						, &g_Context.Device);
	}

	if(FAILED(hr))
		return false;
	
	return true;
}

bool InitializeDirectX11() {
	g_Context.D3DModule = LoadLibrary("d3d11.dll");
	if(!g_Context.D3DModule) 
		return false;

	PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(g_Context.D3DModule, "D3D11CreateDevice");
	if(!D3D11CreateDevice)
		return false;

	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	D3D_FEATURE_LEVEL resultFl;
	DWORD flags = 0;
	#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif
	HRESULT result = D3D11CreateDevice(NULL,
						D3D_DRIVER_TYPE_HARDWARE,
						NULL,
						flags,
						featureLevels,
						sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
						D3D11_SDK_VERSION,
						&g_Context.Device11,
						&resultFl,
						&g_Context.Context11);
	
	return SUCCEEDED(result);
}

bool InitializeGLES() {
	if(!g_Context.Display)
	{
		g_Context.eglGetDisplayANGLE = (PFNEGLGETDISPLAYANGLE) eglGetProcAddress("eglGetDisplayANGLE");
		g_Context.eglTryRestoreDeviceANGLE = (PFNEGLTRYRESTOREDEVICEANGLE) eglGetProcAddress("eglTryRestoreDeviceANGLE");
		g_Context.eglBeginRenderingANGLE = (PFNEGLBEGINRENDERINGANGLE) eglGetProcAddress("eglBeginRenderingANGLE");
		g_Context.eglEndRenderingANGLE = (PFNEGLENDRENDERINGANGLE) eglGetProcAddress("eglEndRenderingANGLE");
		g_Context.glGetGraphicsResetStatusEXT = (PFNGLGETGRAPHICSRESETSTATUSEXTPROC) eglGetProcAddress("glGetGraphicsResetStatusEXT");

		switch(g_DeviceType)
		{
		case DT_Dx9:
			g_Context.Display = g_Context.eglGetDisplayANGLE(EGL_DEFAULT_DISPLAY, EGL_ANGLE_D3D9_DISPLAY_DEVICE, g_Context.Device);
			break;
		case DT_Dx9Ex:
			g_Context.Display = g_Context.eglGetDisplayANGLE(EGL_DEFAULT_DISPLAY, EGL_ANGLE_D3D9EX_DISPLAY_DEVICE, g_Context.Device);
			break;
		case DT_Dx11:
			g_Context.Display = g_Context.eglGetDisplayANGLE(EGL_DEFAULT_DISPLAY, EGL_ANGLE_D3D11_DISPLAY_DEVICE, g_Context.Device11);
			break;
		default:
			return false;
		};
		
		if(!eglInitialize(g_Context.Display, NULL, NULL))
		   return false;
	}
	static const EGLint configAttribs[] = {
		EGL_BUFFER_SIZE, 32,
		EGL_ALPHA_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
		EGL_NONE
	};

	static EGLint surfaceAttribList[] =
	{
		EGL_POST_SUB_BUFFER_SUPPORTED_NV, EGL_TRUE,
		EGL_NONE, EGL_NONE
	};

	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

	EGLint configsCnt = 0;
	if (!eglChooseConfig(g_Context.Display,
	                     configAttribs,
	                     NULL,
	                     0,
	                     &configsCnt)) {
		return false;
	}
	
	if (configsCnt == 0) {
		return false;
	}
	
	EGLConfig config;
	EGLint numConfigs;
	if (!eglChooseConfig(g_Context.Display,
	                     configAttribs,
	                     &config,
	                     1,
	                     &numConfigs)) {
		return false;
	}

	g_Context.Surface = eglCreateWindowSurface(g_Context.Display, config, (EGLNativeWindowType)g_Context.hWnd, surfaceAttribList);
	if(g_Context.Surface == EGL_NO_SURFACE)
	   return false;
	// Create a GL context
	g_Context.Context = eglCreateContext(g_Context.Display, config, EGL_NO_CONTEXT, contextAttribs);
	if(g_Context.Context == EGL_NO_CONTEXT)
		return false;
	
	// Make the context current
	if (!eglMakeCurrent(g_Context.Display, g_Context.Surface, g_Context.Surface, g_Context.Context))
		return false;
	
	return true;
}

void CleanUpGLES()
{
	if(g_Context.Surface)
	{
		eglDestroySurface(g_Context.Display, g_Context.Surface);
		g_Context.Surface = 0;
	}
	if(g_Context.Context)
	{
		eglDestroyContext(g_Context.Display, g_Context.Context);
		g_Context.Context = 0;
	}
}

int main ( int argc, char *argv[] )
{
	g_Context.hWnd = CreateWin(640, 480, "Hello_Triangle_ExtDevice");
	if(!g_Context.hWnd)
		return 1;

	if(g_DeviceType == DT_Dx11)
	{
		if(!InitializeDirectX11())
			return 2;
	}
	else
	{
		if(!InitializeDirectX9())
			return 2;
	}

	if(!InitializeGLES())
		return 3;

	if(!Init())
		return 4;

	WinLoop();

	CleanUpGLES();
	if(!eglTerminate(g_Context.Display))
		return 5;

	return 0;   
}
