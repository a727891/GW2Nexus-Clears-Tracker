#!/usr/bin/env python3
"""Generate C++ sources that embed addon PNG assets into the DLL."""

from __future__ import annotations

import argparse
from pathlib import Path

LOGO_FILENAME_TO_EXPANSION = {
    "heart_of_thorns.png": "hot",
    "path_of_fire.png": "pof",
    "janthir_wilds.png": "janthir_wilds",
    "ice_brood_saga.png": "ibs",
    "end_of_dragons.png": "eod",
    "secrets_of_the_obscure.png": "soto",
    "visions_of_eternity.png": "voe",
    "guild_war_2.png": "core",
}

CORNER_ICON_FILES = {
    "raidIconDark.png": "NRC_ICON",
    "raidIconBright.png": "NRC_ICON_HOVER",
}


def sanitize_symbol(name: str) -> str:
    return "".join(ch if ch.isalnum() else "_" for ch in Path(name).stem)


def format_bytes(data: bytes) -> str:
    lines = []
    row = []
    for byte in data:
        row.append(f"0x{byte:02x}")
        if len(row) == 16:
            lines.append("    " + ", ".join(row) + ",")
            row = []
    if row:
        lines.append("    " + ", ".join(row) + ",")
    return "\n".join(lines)


def write_logos(logos_dir: Path, out_cpp: Path, out_h: Path) -> None:
    logo_files = sorted(logos_dir.glob("*.png"))
    if not logo_files:
        raise SystemExit(f"no PNG logos found in {logos_dir}")

    assets = []
    blob_sections = []

    for logo_path in logo_files:
        expansion_id = LOGO_FILENAME_TO_EXPANSION.get(logo_path.name)
        if not expansion_id:
            raise SystemExit(f"no expansion id mapping for {logo_path.name}")

        data = logo_path.read_bytes()
        symbol = sanitize_symbol(logo_path.name)
        blob_sections.append(
            f"alignas(4) const unsigned char kLogoBlob_{symbol}[] = {{\n"
            f"{format_bytes(data)}\n"
            f"}};\n"
            f"const size_t kLogoBlobSize_{symbol} = {len(data)};\n"
        )
        assets.append(
            f'    {{"{expansion_id}", "NRC_LOGO_{symbol}", '
            f"kLogoBlob_{symbol}, kLogoBlobSize_{symbol}}},"
        )

    out_h.write_text(
        """#pragma once

#include <cstddef>

namespace rc {
namespace EmbeddedLogos {

struct Asset {
    const char* expansionId;
    const char* identifier;
    const unsigned char* data;
    std::size_t size;
};

const Asset* Find(const char* expansionId);

}  // namespace EmbeddedLogos
}  // namespace rc
""",
        encoding="utf-8",
    )

    cpp = """#include "EmbeddedLogos.h"

#include <cstring>

namespace rc {
namespace EmbeddedLogos {
namespace {

"""
    cpp += "\n".join(blob_sections)
    cpp += "\nconst Asset kAssets[] = {\n"
    cpp += "\n".join(assets)
    cpp += """
};

}  // namespace

const Asset* Find(const char* expansionId) {
    if (!expansionId) return nullptr;
    for (const auto& asset : kAssets) {
        if (std::strcmp(asset.expansionId, expansionId) == 0) {
            return &asset;
        }
    }
    return nullptr;
}

}  // namespace EmbeddedLogos
}  // namespace rc
"""
    out_cpp.write_text(cpp, encoding="utf-8")


def write_corner_icons(textures_dir: Path, out_cpp: Path, out_h: Path) -> None:
    assets = []
    blob_sections = []

    for filename, identifier in CORNER_ICON_FILES.items():
        icon_path = textures_dir / filename
        if not icon_path.exists():
            raise SystemExit(f"missing corner icon: {icon_path}")

        data = icon_path.read_bytes()
        symbol = sanitize_symbol(filename)
        blob_sections.append(
            f"alignas(4) const unsigned char kCornerIconBlob_{symbol}[] = {{\n"
            f"{format_bytes(data)}\n"
            f"}};\n"
            f"const size_t kCornerIconBlobSize_{symbol} = {len(data)};\n"
        )
        assets.append(
            f'    {{"{identifier}", kCornerIconBlob_{symbol}, kCornerIconBlobSize_{symbol}}},'
        )

    out_h.write_text(
        """#pragma once

#include <cstddef>

namespace rc {
namespace EmbeddedCornerIcons {

struct Asset {
    const char* identifier;
    const unsigned char* data;
    std::size_t size;
};

const Asset* Find(const char* identifier);

}  // namespace EmbeddedCornerIcons
}  // namespace rc
""",
        encoding="utf-8",
    )

    cpp = """#include "EmbeddedCornerIcons.h"

#include <cstring>

namespace rc {
namespace EmbeddedCornerIcons {
namespace {

"""
    cpp += "\n".join(blob_sections)
    cpp += "\nconst Asset kAssets[] = {\n"
    cpp += "\n".join(assets)
    cpp += """
};

}  // namespace

const Asset* Find(const char* identifier) {
    if (!identifier) return nullptr;
    for (const auto& asset : kAssets) {
        if (std::strcmp(asset.identifier, identifier) == 0) {
            return &asset;
        }
    }
    return nullptr;
}

}  // namespace EmbeddedCornerIcons
}  // namespace rc
"""
    out_cpp.write_text(cpp, encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--logos-dir", required=True, type=Path)
    parser.add_argument("--textures-dir", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    write_logos(args.logos_dir, args.output_dir / "EmbeddedLogos.cpp", args.output_dir / "EmbeddedLogos.h")
    write_corner_icons(
        args.textures_dir,
        args.output_dir / "EmbeddedCornerIcons.cpp",
        args.output_dir / "EmbeddedCornerIcons.h",
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
