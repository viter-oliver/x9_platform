//========================================================================
// GLFW 3.3 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2016 Google Inc.
// Copyright (c) 2006-2016 Camilla Löwy <elmindreda@glfw.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <dlfcn.h>
#include <stdlib.h>
//#include <EGL/eglext.h>
//#include <fbdev_window.h>
//#define _GLFW_EGL_NATIVE_WINDOW         ((EGLNativeWindowType) &window->fb_window)
#define _GLFW_EGL_NATIVE_DISPLAY        ((EGLNativeDisplayType) _glfw.egl.display)

#define _GLFW_PLATFORM_WINDOW_STATE //fbdev_window fb_window

#define _GLFW_PLATFORM_CONTEXT_STATE 
#define _GLFW_PLATFORM_MONITOR_STATE
#define _GLFW_PLATFORM_CURSOR_STATE
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE 
#define _GLFW_PLATFORM_LIBRARY_CONTEXT_STATE
#define _GLFW_PLATFORM_JOYSTICK_STATE
#define _GLFW_PLATFORM_LIBRARY_JOYSTICK_STATE
//#define _GLFW_EGL_CONTEXT_STATE
//#define _GLFW_EGL_LIBRARY_CONTEXT_STATE _GLFWlibraryEGL egl

#include "posix_time.h"
#include "linux_joystick.h"
#include "posix_thread.h"
#include "egl_context.h"

#if defined(_GLFW_WIN32)
 #define _glfw_dlopen(name) LoadLibraryA(name)
 #define _glfw_dlclose(handle) FreeLibrary((HMODULE) handle)
 #define _glfw_dlsym(handle, name) GetProcAddress((HMODULE) handle, name)
#else
 #define _glfw_dlopen(name) dlopen(name, RTLD_LAZY | RTLD_LOCAL)
 #define _glfw_dlclose(handle) dlclose(handle)
 #define _glfw_dlsym(handle, name) dlsym(handle, name)
#endif


