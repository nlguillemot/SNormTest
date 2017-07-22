#include <Windows.h>
#include <comdef.h>

#include "glcorearb.h"
#include "wglext.h"

#include <cstdlib>

#pragma comment(lib, "opengl32.lib")

bool CheckHR(HRESULT hr)
{
    if (SUCCEEDED(hr))
    {
        return true;
    }

    _com_error err(hr);

    int result = MessageBoxW(NULL, err.ErrorMessage(), L"Error", MB_ABORTRETRYIGNORE);
    if (result == IDABORT)
    {
        ExitProcess(-1);
    }
    else if (result == IDRETRY)
    {
        DebugBreak();
    }

    return false;
}

bool CheckWin32(BOOL okay)
{
    if (okay)
    {
        return true;
    }

    return CheckHR(HRESULT_FROM_WIN32(GetLastError()));
}

void APIENTRY DebugCallbackGL(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    OutputDebugStringA("DebugCallbackGL: ");
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
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
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.lpszClassName = TEXT("WindowClass");
    CheckWin32(RegisterClassExW(&wc) != NULL);

    // Create window that will be used to create a GL context
    HWND gl_hWnd = CreateWindowEx(0, TEXT("WindowClass"), 0, 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);
    CheckWin32(gl_hWnd != NULL);

    HDC gl_hDC = GetDC(gl_hWnd);
    CheckWin32(gl_hDC != NULL);

    // set pixelformat for window that supports OpenGL
    PIXELFORMATDESCRIPTOR gl_pfd = {};
    gl_pfd.nSize = sizeof(gl_pfd);
    gl_pfd.nVersion = 1;
    gl_pfd.dwFlags = PFD_SUPPORT_OPENGL;

    int chosenPixelFormat = ChoosePixelFormat(gl_hDC, &gl_pfd);
    CheckWin32(SetPixelFormat(gl_hDC, chosenPixelFormat, &gl_pfd) != FALSE);

    // Create dummy GL context that will be used to create the real context
    HGLRC dummy_hGLRC = wglCreateContext(gl_hDC);
    CheckWin32(dummy_hGLRC != NULL);

    // Use the dummy context to get function to create a better context
    CheckWin32(wglMakeCurrent(gl_hDC, dummy_hGLRC) != FALSE);

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

    HGLRC hGLRC = wglCreateContextAttribsARB(gl_hDC, NULL, contextAttribsGL);
    CheckWin32(hGLRC != NULL);

    // Switch to the new context and ditch the old one
    CheckWin32(wglMakeCurrent(gl_hDC, hGLRC) != FALSE);
    CheckWin32(wglDeleteContext(dummy_hGLRC) != FALSE);

    HMODULE hOpenGL32 = LoadLibrary(TEXT("OpenGL32.dll"));

    auto GetProcGL = [&](const char* name) {
        void* proc = wglGetProcAddress(name);
        if (!proc)
        {
            // Fall back to GetProcAddress to get GL 1 functions. wglGetProcAddress returns NULL on those.
            proc = GetProcAddress(hOpenGL32, name);
        }
        return proc;
    };

    // Grab OpenGL functions
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

    // Test contents
    {
        unsigned char all8BitValues[256];
        for (int i = 0; i < 256; i++)
        {
            all8BitValues[i] = i;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_1D, texture);
        glTexStorage1D(GL_TEXTURE_1D, 1, GL_R8UI, 256);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RED_INTEGER, GL_UNSIGNED_BYTE, all8BitValues);
        glBindTexture(GL_TEXTURE_1D, 0);

        GLuint unormView;
        glGenTextures(1, &unormView);
        glTextureView(unormView, GL_TEXTURE_1D, texture, GL_R8, 0, 1, 0, 1);

        GLuint snormView;
        glGenTextures(1, &snormView);
        glTextureView(snormView, GL_TEXTURE_1D, texture, GL_R8_SNORM, 0, 1, 0, 1);

        // buffer to store the results of the test
        GLuint resultsBuf;
        glGenBuffers(1, &resultsBuf);
        glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
        glBufferStorage(GL_ARRAY_BUFFER, sizeof(GLfloat) * 256, NULL, GL_MAP_READ_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        const char* srcs[] = {
R"GLSL(
#version 440 core

layout(local_size_x = 1) in;

layout(binding = 0)
uniform sampler1D all8BitValues;

layout(std430, binding = 0)
buffer ResultsBuffer { float Results[]; };

void main()
{
    for (int i = 0; i < 256; i++)
    {
        Results[i] = texelFetch(all8BitValues, i, 0).x;
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
        }

        GLuint views[] = { unormView, snormView };
        const char* viewNames[] = { "GL_R8 (= GL_R8_UNORM)", "GL_R8_SNORM" };
        for (int viewIdx = 0; viewIdx < sizeof(views) / sizeof(*views); viewIdx++)
        {
            GLuint view = views[viewIdx];
            const char* viewName = viewNames[viewIdx];

            glBindTextures(0, 1, &view);
            glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, &resultsBuf);

            glUseProgram(program);
            glDispatchCompute(1, 1, 1);
            glUseProgram(0);

            glBindTextures(0, 1, NULL);
            glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 0, 1, NULL);

            glMemoryBarrier(GL_ALL_BARRIER_BITS);

            glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
            GLfloat* results = (GLfloat*)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * 256, GL_MAP_READ_BIT);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

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

                    if (view == unormView || (view == snormView && i <= 127))
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

            glBindBuffer(GL_ARRAY_BUFFER, resultsBuf);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }
}
