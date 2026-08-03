cmake_minimum_required(VERSION 3.24)

project(VesperImageGenerator
    VERSION 0.1.0
    DESCRIPTION "Offline local image generation"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(VESPER_DISPLAY_NAME "Vesper Image Generator Pro")
set(VESPER_ORGANIZATION "Vesper")
set(VESPER_DOMAIN "vesper.local")

option(VESPER_BUILD_TESTS "Build unit tests" ON)

if(APPLE)
    set(SD_METAL ON CACHE BOOL "" FORCE)
    enable_language(OBJCXX)
endif()

if(MSVC)
    add_compile_options(/bigobj)
endif()

if(SD_VULKAN)
    find_package(SPIRV-Headers CONFIG QUIET)
    if(NOT SPIRV-Headers_FOUND)
        include(FetchContent)
        FetchContent_Declare(SPIRV-Headers
            GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
            GIT_TAG vulkan-sdk-1.4.309.0
            GIT_SHALLOW TRUE
            OVERRIDE_FIND_PACKAGE
        )
        FetchContent_MakeAvailable(SPIRV-Headers)
    endif()
endif()

set(SD_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)
set(SD_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/stable-diffusion.cpp EXCLUDE_FROM_ALL)
set_target_properties(sd-cli PROPERTIES EXCLUDE_FROM_ALL FALSE)

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core Gui Quick Qml QuickControls2 Network Concurrent Svg
)

qt_standard_project_setup(REQUIRES 6.5)

set(VESPER_SOURCES
    src/main.cpp
    src/app/AppPaths.cpp
    src/app/AppPaths.h
    src/app/AppSettings.cpp
    src/app/AppSettings.h
    src/app/ByteFormat.cpp
    src/app/ByteFormat.h
    src/models/ModelManifest.cpp
    src/models/ModelManifest.h
    src/models/ModelCatalog.cpp
    src/models/ModelCatalog.h
    src/models/ModelDownloader.cpp
    src/models/ModelDownloader.h
    src/models/FileHasher.cpp
    src/models/FileHasher.h
    src/models/DiskSpace.cpp
    src/models/DiskSpace.h
    src/inference/GenerationRequest.h
    src/inference/SamplerCatalog.cpp
    src/inference/SamplerCatalog.h
    src/inference/SdCliLocator.cpp
    src/inference/SdCliLocator.h
    src/inference/ArgumentBuilder.cpp
    src/inference/ArgumentBuilder.h
    src/inference/ProgressParser.cpp
    src/inference/ProgressParser.h
    src/inference/InferenceBridge.cpp
    src/inference/InferenceBridge.h
    src/gallery/GenerationMetadata.cpp
    src/gallery/GenerationMetadata.h
    src/gallery/GalleryModel.cpp
    src/gallery/GalleryModel.h
    src/compositor/TextLayer.cpp
    src/compositor/TextLayer.h
    src/compositor/CompositionDocument.cpp
    src/compositor/CompositionDocument.h
    src/compositor/CompositionRenderer.cpp
    src/compositor/CompositionRenderer.h
    src/compositor/TemplateCatalog.cpp
    src/compositor/TemplateCatalog.h
    src/platform/WindowBackdrop.cpp
    src/platform/WindowBackdrop.h
)

if(WIN32)
    list(APPEND VESPER_SOURCES src/platform/WindowBackdrop_win.cpp)
elseif(APPLE)
    list(APPEND VESPER_SOURCES src/platform/WindowBackdrop_mac.mm)
else()
    list(APPEND VESPER_SOURCES src/platform/WindowBackdrop_linux.cpp)
endif()

qt_add_executable(VesperImageGenerator ${VESPER_SOURCES})

set(VESPER_QML_FILES
    qml/Main.qml
    qml/theme/Theme.qml
    qml/components/GlassPanel.qml
    qml/components/GlassButton.qml
    qml/components/GlassField.qml
    qml/components/GlassSlider.qml
    qml/components/GlassCombo.qml
    qml/components/TitleBar.qml
    qml/components/NavRail.qml
    qml/components/ConfirmDialog.qml
    qml/components/SectionLabel.qml
    qml/screens/GenerateScreen.qml
    qml/screens/ModelsScreen.qml
    qml/screens/GalleryScreen.qml
    qml/screens/ComposerScreen.qml
    qml/screens/SettingsScreen.qml
)

foreach(qml_file IN LISTS VESPER_QML_FILES)
    get_filename_component(qml_name "${qml_file}" NAME)
    set_source_files_properties("${qml_file}" PROPERTIES QT_RESOURCE_ALIAS "${qml_name}")
endforeach()

set_source_files_properties(qml/theme/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(VesperImageGenerator
    URI Vesper
    VERSION 1.0
    QML_FILES ${VESPER_QML_FILES}
    RESOURCES
        resources/models.json
        resources/templates/letter.json
        resources/templates/card.json
        resources/templates/poster.json
)

target_include_directories(VesperImageGenerator PRIVATE
    src
    src/app
    src/models
    src/inference
    src/gallery
    src/compositor
    src/platform
)

target_link_libraries(VesperImageGenerator PRIVATE
    Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml
    Qt6::QuickControls2 Qt6::Network Qt6::Concurrent Qt6::Svg
)

target_compile_definitions(VesperImageGenerator PRIVATE
    VESPER_VERSION="${PROJECT_VERSION}"
    VESPER_DISPLAY_NAME="${VESPER_DISPLAY_NAME}"
    VESPER_ORGANIZATION="${VESPER_ORGANIZATION}"
)

if(MSVC)
    target_compile_options(VesperImageGenerator PRIVATE /W4 /permissive-)
    target_link_libraries(VesperImageGenerator PRIVATE dwmapi)
else()
    target_compile_options(VesperImageGenerator PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(APPLE)
    target_link_libraries(VesperImageGenerator PRIVATE "-framework AppKit")
    set_target_properties(VesperImageGenerator PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${VESPER_DISPLAY_NAME}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "local.vesper.imagegenerator"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/Info.plist.in"
    )
endif()

if(WIN32)
    set_target_properties(VesperImageGenerator PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(APPLE)
    set(VESPER_SDCLI_DESTINATION "$<TARGET_BUNDLE_CONTENT_DIR:VesperImageGenerator>/MacOS")
else()
    set(VESPER_SDCLI_DESTINATION "$<TARGET_FILE_DIR:VesperImageGenerator>")
endif()

add_custom_command(TARGET VesperImageGenerator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:sd-cli>" "${VESPER_SDCLI_DESTINATION}"
)

include(GNUInstallDirs)

install(TARGETS VesperImageGenerator
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(APPLE)
    install(PROGRAMS "$<TARGET_FILE:sd-cli>"
        DESTINATION "${VESPER_DISPLAY_NAME}.app/Contents/MacOS")
else()
    install(PROGRAMS "$<TARGET_FILE:sd-cli>" DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(VESPER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
