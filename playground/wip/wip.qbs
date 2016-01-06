import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "../../dependencies/win/src/gl3w.c",
        "../../dependencies/win/src/nanovg.c",
        "Canvas.cpp",
        "Canvas.hpp",
        "CanvasView.cpp",
        "CanvasView.hpp",
        "NetworkConnection.cpp",
        "NetworkConnection.hpp",
        "Transform2.cpp",
        "Transform2.hpp",
        "capnp/test.capnp.c++",
        "capnp/test.capnp.h",
        "common.hpp",
        "core.hpp",
        "delegate.hpp",
        "events.cpp",
        "events.hpp",
        "keyboard.hpp",
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
        "miro/Client.cpp",
        "miro/Client.hpp",
        "miro/MessageHeader.hpp",
        "miro/Server.cpp",
        "miro/Server.hpp",
        "miro/action/ActionBuffer.cpp",
        "miro/action/ActionBuffer.hpp",
        "miro/action/ActionDefinitions.hpp",
        "miro/action/ActionForwarder.cpp",
        "miro/action/ActionForwarder.hpp",
        "miro/action/ActionHeader.hpp",
        "miro/action/ActionRange.cpp",
        "miro/action/ActionRange.hpp",
        "miro/action/ActionReference.hpp",
        "miro/action/ActionSender.hpp",
        "miro/action/ActionType.hpp",
        "miro/action/ActionWriter.hpp",
        "miro/action/BufferingActionSink.cpp",
        "miro/action/BufferingActionSink.hpp",
        "miro/action/ConcurrentActionForwarder.cpp",
        "miro/action/ConcurrentActionForwarder.hpp",
        "miro/action/ConcurrentBufferingActionSink.cpp",
        "miro/action/ConcurrentBufferingActionSink.hpp",
        "miro/action/IActionSink.cpp",
        "miro/action/IActionSink.hpp",
        "miro/action/IActionSource.cpp",
        "miro/action/IActionSource.hpp",
        "miro/action/NotifyingActionBuffer.cpp",
        "miro/action/NotifyingActionBuffer.hpp",
        "miro/action/RelativeMemoryRange16.hpp",
        "miro/action/StringRef.cpp",
        "miro/action/StringRef.hpp",
        "miro/action/connect.cpp",
        "miro/action/connect.hpp",
        "miro_client.cpp",
        "miro_client.hpp",
        "networking/Connection.cpp",
        "networking/Connection.hpp",
        "networking/Listener.cpp",
        "networking/Listener.hpp",
        "networking/Scheduler.cpp",
        "networking/Scheduler.hpp",
        "networking/Socket.cpp",
        "networking/Socket.hpp",
        "render_context.cpp",
        "render_context.hpp",
        "result.hpp",
        "sol/MemoryRange.hpp",
        "sol/StringView.hpp",
        "sol/meta/type_name.hpp",
        "util.hpp",
        "vec2.hpp",
        "window.cpp",
        "window.hpp",
    ]
    
    cpp.includePaths: [
        '../../dependencies/win/include/',
        '../../dependencies/win/include/nanovg/',
        '.'
    ]
    cpp.libraryPaths: ['../../dependencies/win/lib/x86/']
    cpp.staticLibraries : [
        'SDL2Main','SDL2','gdi32',
        'avformat', 'avcodec', 'avutil',
        'opengl32', 'glew32', 'glu32',
        'ws2_32', 'wsock32',            // asio
        //'capnp', 'kj', // capnproto
    ]
    
    
    cpp.cppFlags: "-std=c++14"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}

