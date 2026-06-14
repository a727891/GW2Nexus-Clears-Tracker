cmake_minimum_required(VERSION 3.20)
project(NexusRaidClears LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(NEXUSRAIDCLEARS_CROSS_COMPILE "Cross-compile for Windows using MinGW" ON)

if(NEXUSRAIDCLEARS_CROSS_COMPILE)
    if(NOT CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mingw-w64-toolchain.cmake"
            CACHE FILEPATH "MinGW cross-compile toolchain")
    endif()
endif()

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.80
)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(nlohmann_json)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    )
    target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
endif()

file(GLOB LOGO_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/logos/*.png")
set(CORNER_ICON_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/raidIconDark.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/raidIconBright.png"
)
set(EMBEDDED_ASSETS_DIR "${CMAKE_BINARY_DIR}/generated")
set(EMBEDDED_LOGOS_CPP "${EMBEDDED_ASSETS_DIR}/EmbeddedLogos.cpp")
set(EMBEDDED_LOGOS_H "${EMBEDDED_ASSETS_DIR}/EmbeddedLogos.h")
set(EMBEDDED_CORNER_ICONS_CPP "${EMBEDDED_ASSETS_DIR}/EmbeddedCornerIcons.cpp")
set(EMBEDDED_CORNER_ICONS_H "${EMBEDDED_ASSETS_DIR}/EmbeddedCornerIcons.h")

add_custom_command(
    OUTPUT ${EMBEDDED_LOGOS_CPP} ${EMBEDDED_LOGOS_H}
           ${EMBEDDED_CORNER_ICONS_CPP} ${EMBEDDED_CORNER_ICONS_H}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${EMBEDDED_ASSETS_DIR}"
    COMMAND python3 "${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_assets.py"
            --logos-dir "${CMAKE_CURRENT_SOURCE_DIR}/assets/logos"
            --textures-dir "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures"
            --output-dir "${EMBEDDED_ASSETS_DIR}"
    DEPENDS ${LOGO_FILES} ${CORNER_ICON_FILES}
            "${CMAKE_CURRENT_SOURCE_DIR}/scripts/embed_assets.py"
    COMMENT "Embedding addon PNG assets into binary"
    VERBATIM
)

set(SRC_FILES
    src/entry.cpp
    src/core/AppState.cpp
    src/core/SettingsStore.cpp
    src/core/AccountRegistry.cpp
    src/core/MumbleIdentity.cpp
    src/core/Types.cpp
    src/core/PanelVisibility.cpp
    src/core/RaidVisibilityStore.cpp
    src/core/StrikeVisibilityStore.cpp
    src/core/EncounterVisibilityFilter.cpp
    src/data/StaticDataLoader.cpp
    src/data/RaidData.cpp
    src/data/StrikeData.cpp
    src/data/DailyBountyData.cpp
    src/data/FractalMapData.cpp
    src/data/InstabilitiesData.cpp
    src/data/DungeonData.cpp
    src/services/HttpClient.cpp
    src/services/Gw2ApiClient.cpp
    src/services/ApiPollService.cpp
    src/services/RaidClearsService.cpp
    src/services/StrikeClearsService.cpp
    src/services/DailyBountyService.cpp
    src/services/DailyBountyProgressService.cpp
    src/services/PriorityRotationService.cpp
    src/services/WeeklyBountyEncountersService.cpp
    src/services/WeeklyModifierService.cpp
    src/services/DatAssetIconService.cpp
    src/services/MentorAchievementProgressService.cpp
    src/services/StrikePersistance.cpp
    src/services/ResetsWatcher.cpp
    src/services/MapWatcherService.cpp
    src/services/FractalRotationService.cpp
    src/services/FractalPersistance.cpp
    src/services/FractalMapWatcherService.cpp
    src/services/DungeonsClearsService.cpp
    src/ui/GridLayout.cpp
    src/ui/GridRenderer.cpp
    src/ui/EncounterTooltip.cpp
    src/ui/FractalCmTooltip.cpp
    src/ui/OverlayPanel.cpp
    src/ui/GridMaskService.cpp
    src/ui/UiFontService.cpp
    src/ui/PanelAnchor.cpp
    src/ui/RaidPanel.cpp
    src/ui/StrikesPanel.cpp
    src/ui/OptionsPanel.cpp
    src/ui/OptionsUiKit.cpp
    src/ui/OptionsTextureService.cpp
    src/ui/ContentLogoService.cpp
    src/ui/OptionsCommon.cpp
    src/ui/OptionsGeneralTab.cpp
    src/ui/OptionsRaidsTab.cpp
    src/ui/OptionsStrikesTab.cpp
    src/ui/OptionsFractalsTab.cpp
    src/ui/OptionsDungeonsTab.cpp
    src/ui/EncounterSelectionPanel.cpp
    src/ui/LabelCustomizationPanel.cpp
    src/ui/FractalsPanel.cpp
    src/ui/DungeonsPanel.cpp
    src/ui/MentorProgressPopupService.cpp
    src/ui/QuickAccessService.cpp
    ${EMBEDDED_LOGOS_CPP}
    ${EMBEDDED_CORNER_ICONS_CPP}
)

add_custom_target(generate_embedded_assets
    DEPENDS ${EMBEDDED_LOGOS_CPP} ${EMBEDDED_LOGOS_H}
            ${EMBEDDED_CORNER_ICONS_CPP} ${EMBEDDED_CORNER_ICONS_H})

add_library(NexusRaidClears SHARED ${SRC_FILES})
add_dependencies(NexusRaidClears generate_embedded_assets)

target_include_directories(NexusRaidClears PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/nexus
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/mumble
    ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(NexusRaidClears PRIVATE
    imgui
    nlohmann_json::nlohmann_json
    winhttp
    ws2_32
    crypt32
    shell32
)

target_compile_definitions(NexusRaidClears PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# MinGW builds must statically link runtime DLLs or Nexus cannot LoadLibrary the addon on Windows.
if(CMAKE_CXX_COMPILER MATCHES "mingw" OR MINGW)
    target_link_options(NexusRaidClears PRIVATE
        -static-libgcc
        -static-libstdc++
        -static
    )
endif()

set_target_properties(NexusRaidClears PROPERTIES
    OUTPUT_NAME "NexusRaidClears"
    PREFIX ""
    SUFFIX ".dll"
)
