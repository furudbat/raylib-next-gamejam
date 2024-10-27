# add raylib (3rd-party)
# https://github.com/raysan5/raylib

set(RAYLIB_SUPPORT_MODULE_RAUDIO OFF)
set(RAYLIB_SUPPORT_TRACELOG OFF)
if("${CMAKE_BUILD_TYPE}" MATCHES "Debug" OR "${CMAKE_BUILD_TYPE}" MATCHES "RelWithDebInfo")
    set(RAYLIB_SUPPORT_TRACELOG ON)
endif()

cpmaddpackage(
        NAME
        raylib
        GITHUB_REPOSITORY
        raysan5/raylib
        GIT_TAG
        #5.5
        master # use up-to-date branch
        OPTIONS
        "BUILD_EXAMPLES OFF"
        "CUSTOMIZE_BUILD ON"
        "ENABLE_ASAN OFF"
        "ENABLE_UBSAN OFF"
        "ENABLE_MSAN OFF"
        "WITH_PIC OFF"
        "BUILD_SHARED_LIBS OFF"
        # raylib modules included
        "SUPPORT_MODULE_RSHAPES ON"
        "SUPPORT_MODULE_RTEXTURES ON"
        "SUPPORT_MODULE_RTEXT ON"
        "SUPPORT_MODULE_RMODELS OFF"
        "SUPPORT_MODULE_RAUDIO ${RAYLIB_SUPPORT_MODULE_RAUDIO}"
        # rcore.c
        "SUPPORT_CAMERA_SYSTEM OFF"
        "SUPPORT_GESTURES_SYSTEM ON"
        "SUPPORT_RPRAND_GENERATOR ON"
        "SUPPORT_MOUSE_GESTURES ON"
        "SUPPORT_SSH_KEYBOARD_RPI OFF"
        "SUPPORT_WINMM_HIGHRES_TIMER ON"
        "SUPPORT_PARTIALBUSY_WAIT_LOOP ON"
        "SUPPORT_SCREEN_CAPTURE OFF"
        "SUPPORT_GIF_RECORDING OFF"
        "SUPPORT_COMPRESSION_API OFF"
        "SUPPORT_AUTOMATION_EVENTS ON"
        # rshapes.c
        "SUPPORT_QUADS_DRAW_MODE ON"
        # rtextures.c
        "SUPPORT_FILEFORMAT_PNG ON"
        "SUPPORT_FILEFORMAT_HDR OFF"
        "SUPPORT_FILEFORMAT_PIC OFF"
        "SUPPORT_FILEFORMAT_PNM OFF"
        "SUPPORT_FILEFORMAT_KTX OFF"
        "SUPPORT_FILEFORMAT_ASTC OFF"
        "SUPPORT_FILEFORMAT_BMP OFF"
        "SUPPORT_FILEFORMAT_TGA OFF"
        "SUPPORT_FILEFORMAT_JPG OFF"
        "SUPPORT_FILEFORMAT_GIF OFF"
        "SUPPORT_FILEFORMAT_QOI OFF"
        "SUPPORT_FILEFORMAT_PSD OFF"
        "SUPPORT_FILEFORMAT_PKM OFF"
        "SUPPORT_FILEFORMAT_PVR OFF"
        "SUPPORT_FILEFORMAT_SVG OFF"
        "SUPPORT_IMAGE_EXPORT OFF"
        "SUPPORT_IMAGE_GENERATION ON"
        "SUPPORT_IMAGE_MANIPULATION ON"
        # rtext.c
        "SUPPORT_DEFAULT_FONT ON"
        "SUPPORT_FILEFORMAT_FNT OFF"
        "SUPPORT_FILEFORMAT_TTF OFF"
        "SUPPORT_TEXT_MANIPULATION ON"
        "SUPPORT_FONT_ATLAS_WHITE_REC ON"
        # rmodels.c
        "SUPPORT_FILEFORMAT_OBJ OFF"
        "SUPPORT_FILEFORMAT_MTL OFF"
        "SUPPORT_FILEFORMAT_IQM OFF"
        "SUPPORT_FILEFORMAT_GLTF OFF"
        "SUPPORT_FILEFORMAT_VOX OFF"
        "SUPPORT_FILEFORMAT_M3D OFF"
        "SUPPORT_MESH_GENERATION OFF"
        # raudio.c
        "SUPPORT_FILEFORMAT_WAV OFF"
        "SUPPORT_FILEFORMAT_OGG ${RAYLIB_SUPPORT_MODULE_RAUDIO}"
        "SUPPORT_FILEFORMAT_MOD OFF"
        "SUPPORT_FILEFORMAT_MP3 ${RAYLIB_SUPPORT_MODULE_RAUDIO}"
        "SUPPORT_FILEFORMAT_QOA OFF"
        "SUPPORT_FILEFORMAT_FLAC OFF"
        # utils.c
        "SUPPORT_STANDARD_FILEIO ON"
        "SUPPORT_TRACELOG ${RAYLIB_SUPPORT_TRACELOG}"
)
target_compile_options(raylib PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:-Wno-error=implicit-function-declaration>)
target_compile_options(raylib PRIVATE $<$<C_COMPILER_ID:GNU,Clang>:-Wno-unused-result>)
target_compile_options(raylib PRIVATE $<$<C_COMPILER_ID:GNU>:-Wno-stringop-overflow>)
target_compile_options(raylib PRIVATE $<$<C_COMPILER_ID:Clang>:-Wno-implicit-const-int-float-conversion>)
target_compile_features(raylib PRIVATE c_std_99)

if("${PLATFORM}" STREQUAL "Desktop")
    target_compile_features(glfw PRIVATE c_std_99)
endif()