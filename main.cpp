#include <Windows.h>

#include "glcorearb.h"
#include "wglext.h"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#pragma comment(lib, "opengl32.lib")

void APIENTRY DebugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    fprintf(stderr, "DebugCallbackGL: %s\n", message);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
        ExitProcess(0);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int main()
{
    // Register window class
    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("WindowClass");
    bool ok = RegisterClass(&wc) != NULL;
    assert(ok);

    // Create window that will be used to create a GL context
    HWND gl_hWnd = CreateWindow(TEXT("WindowClass"), 0, 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
    assert(gl_hWnd != NULL);

    HDC gl_hDC = GetDC(gl_hWnd);
    assert(gl_hDC != NULL);

    // set pixelformat for window that supports OpenGL
    PIXELFORMATDESCRIPTOR gl_pfd = {};
    gl_pfd.nSize = sizeof(gl_pfd);
    gl_pfd.nVersion = 1;
    gl_pfd.dwFlags = PFD_SUPPORT_OPENGL;

    int chosenPixelFormat = ChoosePixelFormat(gl_hDC, &gl_pfd);
    ok = SetPixelFormat(gl_hDC, chosenPixelFormat, &gl_pfd) != FALSE;
    assert(ok);

    // Create dummy GL context that will be used to create the real context
    HGLRC dummy_hGLRC = wglCreateContext(gl_hDC);
    assert(dummy_hGLRC != NULL);

    // Use the dummy context to get function to create a better context
    ok = wglMakeCurrent(gl_hDC, dummy_hGLRC) != FALSE;
    assert(ok);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    int contextFlagsGL = 0;
#ifdef _DEBUG
    contextFlagsGL |= WGL_CONTEXT_DEBUG_BIT_ARB;
#endif

    int contextAttribsGL[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 4,
        WGL_CONTEXT_FLAGS_ARB, contextFlagsGL,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    // Create better GL context
    HGLRC hGLRC = wglCreateContextAttribsARB(gl_hDC, NULL, contextAttribsGL);
    assert(hGLRC != NULL);

    // Switch to the new context and ditch the old one
    ok = wglMakeCurrent(gl_hDC, hGLRC) != FALSE;
    assert(ok);
    ok = wglDeleteContext(dummy_hGLRC) != FALSE;
    assert(ok);

    auto GetProcGL = [](const char* name) {
        void* proc = wglGetProcAddress(name);
        if (!proc)
        {
            // Fall back to GetProcAddress to get GL 1 functions. wglGetProcAddress returns NULL on those.
            static HMODULE hOpenGL32 = LoadLibrary(TEXT("OpenGL32.dll"));
            proc = GetProcAddress(hOpenGL32, name);
        }
        return proc;
    };

    // Grab OpenGL functions
    PFNGLGETSTRINGPROC glGetString = (PFNGLGETSTRINGPROC)GetProcGL("glGetString");
    PFNGLENABLEPROC glEnable = (PFNGLENABLEPROC)GetProcGL("glEnable");

    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC)GetProcGL("glGenBuffers");
    PFNGLBINDBUFFERPROC glBindBuffer = (PFNGLBINDBUFFERPROC)GetProcGL("glBindBuffer");
    PFNGLBINDBUFFERSBASEPROC glBindBuffersBase = (PFNGLBINDBUFFERSBASEPROC)GetProcGL("glBindBuffersBase");
    PFNGLBUFFERSTORAGEPROC glBufferStorage = (PFNGLBUFFERSTORAGEPROC)GetProcGL("glBufferStorage");
    PFNGLMAPBUFFERRANGEPROC glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)GetProcGL("glMapBufferRange");
    PFNGLUNMAPBUFFERPROC glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)GetProcGL("glUnmapBuffer");
    
    PFNGLGENTEXTURESPROC glGenTextures = (PFNGLGENTEXTURESPROC)GetProcGL("glGenTextures");
    PFNGLBINDTEXTUREPROC glBindTexture = (PFNGLBINDTEXTUREPROC)GetProcGL("glBindTexture");
    PFNGLBINDTEXTURESPROC glBindTextures = (PFNGLBINDTEXTURESPROC)GetProcGL("glBindTextures");
    PFNGLTEXSTORAGE1DPROC glTexStorage1D = (PFNGLTEXSTORAGE1DPROC)GetProcGL("glTexStorage1D");
    PFNGLTEXSUBIMAGE1DPROC glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC)GetProcGL("glTexSubImage1D");
    PFNGLTEXTUREVIEWPROC glTextureView = (PFNGLTEXTUREVIEWPROC)GetProcGL("glTextureView");

    PFNGLCREATESHADERPROGRAMVPROC glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC)GetProcGL("glCreateShaderProgramv");
    PFNGLGETPROGRAMIVPROC glGetProgramiv = (PFNGLGETPROGRAMIVPROC)GetProcGL("glGetProgramiv");
    PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)GetProcGL("glGetProgramInfoLog");
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC)GetProcGL("glUseProgram");

    PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)GetProcGL("glDispatchCompute");
    PFNGLMEMORYBARRIERPROC glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)GetProcGL("glMemoryBarrier");

    // Enable OpenGL debugging
#ifdef _DEBUG
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_DEBUG_OUTPUT);
    PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
    glDebugMessageCallback(DebugCallbackGL, 0);
#endif

    printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
    printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    printf("\n");

    unsigned char all8bitpixels[256 * 4];
    for (int i = 0; i < 256; i++)
    {
        all8bitpixels[i * 4 + 0] = i;
        all8bitpixels[i * 4 + 1] = i;
        all8bitpixels[i * 4 + 2] = i;
        all8bitpixels[i * 4 + 3] = i;
    }

    // Initialize the texture with raw 8-bit data in it.
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_1D, texture);
    glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGBA8UI, 256);
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, all8bitpixels);
    glBindTexture(GL_TEXTURE_1D, 0);

    // Create a UNORM view of the 8-bit data
    GLuint unormView;
    glGenTextures(1, &unormView);
    glTextureView(unormView, GL_TEXTURE_1D, texture, GL_RGBA8, 0, 1, 0, 1);

    // Create a SNORM view of the 8-bit data
    GLuint snormView;
    glGenTextures(1, &snormView);
    glTextureView(snormView, GL_TEXTURE_1D, texture, GL_RGBA8_SNORM, 0, 1, 0, 1);

    // Create a SRGB view of the 8-bit data
    GLuint srgbView;
    glGenTextures(1, &srgbView);
    glTextureView(srgbView, GL_TEXTURE_1D, texture, GL_SRGB8_ALPHA8, 0, 1, 0, 1);

    // Create a buffer to store the results of the test
    GLuint resultsBuf;
    glGenBuffers(1, &resultsBuf);
    glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
    glBufferStorage(GL_ARRAY_BUFFER, sizeof(GLfloat) * 256, NULL, GL_MAP_READ_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Compute shader that just converts every 8-bit value into a float and stores the result in a buffer.
    const char* srcs[] = {
R"GLSL(
#version 440 core

layout(binding = 0)
uniform sampler1D all8bitpixels;

layout(std430, binding = 0)
buffer ResultsBuffer { float Results[]; };

layout(local_size_x = 1) in;
void main()
{
    for (int i = 0; i < 256; i++)
    {
        Results[i] = texelFetch(all8bitpixels, i, 0).r;
    }
}
)GLSL"
    };

    GLuint program = glCreateShaderProgramv(GL_COMPUTE_SHADER, sizeof(srcs) / sizeof(*srcs), srcs);
    GLint logLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar* infolog = (GLchar*)malloc(logLength);
        glGetProgramInfoLog(program, logLength, NULL, infolog);
        fprintf(stderr, "%s", infolog);
        free(infolog);
    }

    // Run the test once for each view
    GLuint views[] = { unormView, snormView, srgbView };
    const char* viewNames[] = { "GL_RGBA8 (= GL_RGBA8_UNORM)", "GL_RGBA8_SNORM", "GL_SRGB8_ALPHA8" };

    for (int viewIdx = 0; viewIdx < sizeof(views) / sizeof(*views); viewIdx++)
    {
        GLuint view = views[viewIdx];
        const char* viewName = viewNames[viewIdx];

        // Bind shader resources
        glBindTextures(0, 1, &view);
        glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, &resultsBuf);

        // Run the test
        glUseProgram(program);
        glDispatchCompute(1, 1, 1);
        glUseProgram(0);

        // Unbind shader resources
        glBindTextures(0, 1, NULL);
        glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, NULL);

        // Make sure all writes to the buffer are done before reading them.
        glMemoryBarrier(GL_ALL_BARRIER_BITS);

        // Map the buffer to be able to read its data.
        glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
        GLfloat* results = (GLfloat*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 256, GL_MAP_READ_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Output the table of values
        printf("%s:\n", viewName);
        int nCols = 4;
        int nRows = 256 / nCols;
        assert(nCols * nRows == 256);
        for (int row = 0; row < nRows; row++)
        {
            printf("| ");
            for (int col = 0; col < nCols; col++)
            {
                int i = row + col * nRows;

                if (view == unormView || view == srgbView || (view == snormView && i <= 127))
                {
                    printf("%3d -> %2.3f | ", i, results[i]);
                }
                else
                {
                    printf("%3d (%4d) -> %2.3f | ", i, (signed char)i, results[i]);
                }
            }
            printf("\n");
        }
        printf("\n");

        // Unmap the buffer
        glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}
