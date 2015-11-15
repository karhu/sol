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
        "NetworkConnection.cpp",
        "NetworkConnection.hpp",
        "Transform2.cpp",
        "Transform2.hpp",
        "common.hpp",
        "core.hpp",
        "delegate.hpp",
        "events.cpp",
        "events.hpp",
        "main.cpp",
        "context.hpp",
        "context.cpp",
        "memory.hpp",
        "memory/AAllocator.cpp",
        "memory/AAllocator.hpp",
        "memory/Mallocator.cpp",
        "memory/Mallocator.hpp",
        "memory/rich_ptr.cpp",
        "memory/rich_ptr.hpp",
        "memory/util.hpp",
        "miro_client.cpp",
        "miro_client.hpp",
        "networking/Connection.cpp",
        "networking/Connection.hpp",
        "networking/Listener.cpp",
        "networking/Listener.hpp",
        "networking/Scheduler.cpp",
        "networking/Scheduler.hpp",
        "networking/Session.cpp",
        "networking/Session.hpp",
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
        'ws2_32', 'wsock32'  // asio
    ]
    
    
    cpp.cppFlags: "-std=c++14"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}

