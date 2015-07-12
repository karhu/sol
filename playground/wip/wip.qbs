import qbs

CppApplication {
    type: "application" // To suppress bundle generation on Mac
    consoleApplication: true
    files: [
        "core.hpp",
        "events.cpp",
        "events.hpp",
        "main.cpp",
        "context.hpp",
        "context.cpp",
        "result.hpp",
        "window.cpp",
        "window.hpp",
    ]
    
    cpp.includePaths: ['../../dependencies/win/include/']
    cpp.libraryPaths: ['../../dependencies/win/lib/x86/']
    cpp.staticLibraries : ['SDL2Main','SDL2','gdi32']
    
    
    cpp.cppFlags: "-std=c++14"

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}

