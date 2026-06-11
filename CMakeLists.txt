cmake_minimum_required(VERSION 3.20)
project(SkyFirePatrol CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(SkyFirePatrol ${SOURCES})
target_include_directories(SkyFirePatrol PRIVATE src)

if(EMSCRIPTEN)
    # Path to the SDL3_mixer static library built from source for Emscripten.
    # (sdl3_mixer is not yet available as an Emscripten port.)
    # Build it once with: bash build_sdl3mixer_em.sh
    set(EM_MIXER_BUILD "C:/emsdk/SDL_mixer-build-em")
    set(EM_MIXER_SRC   "C:/emsdk/SDL_mixer-git")

    # SDL3 and SDL3_ttf via Emscripten's port system
    set(EM_PORTS
        "SHELL:--use-port=sdl3"
        "SHELL:--use-port=sdl3_ttf"
    )

    target_include_directories(SkyFirePatrol PRIVATE
        "${EM_MIXER_SRC}/include"
    )

    target_compile_options(SkyFirePatrol PRIVATE ${EM_PORTS})

    target_link_options(SkyFirePatrol PRIVATE
        ${EM_PORTS}
        -sALLOW_MEMORY_GROWTH=1
        -sMAXIMUM_MEMORY=256MB
        "SHELL:-sEXPORTED_RUNTIME_METHODS=['FS']"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
        -O2
    )

    # Link the prebuilt SDL3_mixer and its bundled codec libs
    target_link_libraries(SkyFirePatrol PRIVATE
        "${EM_MIXER_BUILD}/libSDL3_mixer.a"
        "${EM_MIXER_BUILD}/external/ogg-build/libogg.a"
        "${EM_MIXER_BUILD}/external/vorbis-build/lib/libvorbisfile.a"
        "${EM_MIXER_BUILD}/external/vorbis-build/lib/libvorbis.a"
    )

    set_target_properties(SkyFirePatrol PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )
else()
    # Native Windows build via vcpkg — mirrors the Shooter.vcxproj deps.
    # Run cmake with -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
    find_package(SDL3       REQUIRED CONFIG)
    find_package(SDL3_image REQUIRED CONFIG)
    find_package(SDL3_mixer REQUIRED CONFIG)
    find_package(SDL3_ttf   REQUIRED CONFIG)

    target_link_libraries(SkyFirePatrol PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        SDL3_ttf::SDL3_ttf
    )
endif()
