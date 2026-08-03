find_package(Qt6 REQUIRED COMPONENTS Test Gui)

set(VESPER_TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/src/app/ByteFormat.cpp
    ${CMAKE_SOURCE_DIR}/src/models/ModelManifest.cpp
    ${CMAKE_SOURCE_DIR}/src/inference/ArgumentBuilder.cpp
    ${CMAKE_SOURCE_DIR}/src/inference/ProgressParser.cpp
    ${CMAKE_SOURCE_DIR}/src/inference/SamplerCatalog.cpp
    ${CMAKE_SOURCE_DIR}/src/compositor/TextLayer.cpp
    ${CMAKE_SOURCE_DIR}/src/compositor/CompositionRenderer.cpp
)

function(vesper_add_test name)
    qt_add_executable(${name} ${name}.cpp ${VESPER_TEST_SOURCES})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE Qt6::Test Qt6::Gui)
    target_compile_definitions(${name} PRIVATE VESPER_VERSION="${PROJECT_VERSION}")
    add_test(NAME ${name} COMMAND ${name})
endfunction()

vesper_add_test(tst_argumentbuilder)
vesper_add_test(tst_progressparser)
vesper_add_test(tst_modelmanifest)
vesper_add_test(tst_composition)
