import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "../../dependencies/win/src/gl3w.c",
        "../../dependencies/win/src/nanovg.c",
        "Action.cpp",
        "Action.hpp",
        "Canvas.cpp",
        "Canvas.hpp",
        "CanvasView.cpp",
        "CanvasView.hpp",
        "common.hpp",
        "core.hpp",
        "events.cpp",
        "events.hpp",
        "main.cpp",
        "context.hpp",
        "context.cpp",
        "render_context.cpp",
        "render_context.hpp",
        "result.hpp",
        "util.hpp",
        "vec2.hpp",
        "window.cpp",
        "window.hpp",
    ]
    
    cpp.includePaths: [
        '../../dependencies/win/include/',
        '../../dependencies/win/include/nanovg/'

    ]
    cpp.libraryPaths: ['../../dependencies/win/lib/x86/']
    cpp.staticLibraries : [
        'SDL2Main','SDL2','gdi32',
        'avformat', 'avcodec', 'avutil',
        'opengl32', 'glew32', 'glu32',
    ]
    
    
    cpp.cppFlags: "-std=c++14"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}

