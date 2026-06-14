# Clears Tracker (Nexus)

Nexus addon port of the [BlishHUD Clears Tracker](https://github.com/a727891/BlishHud-Raid-Clears) module.

Track daily and weekly PvE clears — raids, strikes, fractals, and dungeons — in overlay panels backed by the Guild Wars 2 API.

## Features

- **Raids** — wing/encounter grid from `/v2/account/raids`; daily raid bounty rows; mentor progress tooltips and popups
- **Strikes** — weekly clears from achievement 9125; daily bounty and tomorrow rows; map-tracked daily strikes (e.g. Dragonstorm)
- **Fractals** — recommended and daily tiers, CM selection, instability tooltips
- **Dungeons** — path clear status from `/v2/account/dungeons`
- **Customization** — layout, colors, labels, per-encounter visibility, screen clamp, stylized grid boxes
- **Quick access** — corner icon and `NRC_TOGGLE_PANELS` keybind (default `ALT+SHIFT+R`)

## Requirements

- [Nexus](https://raidcore.gg/gw2/nexus) in Guild Wars 2
- GW2 API key with `**account`** and `**progression**` permissions
- Windows x64 at runtime (DLL is cross-compiled from Linux)

## Installation

1. Copy `NexusRaidClears.dll` to `<GW2>/addons/` (directly in `addons/`, not a subfolder)
2. Launch GW2 with Nexus enabled
3. Enable **Clears Tracker** in Nexus addon settings
4. Enter your API key in options and click **Save API Key**
5. Enable the panels you want (Raids, Strikes, Fractals, Dungeons)

On first run the addon downloads encounter metadata into
`<GW2>/addons/NexusRaidClears/clearsTracker/`. Pre-seeding that folder avoids a
background download during load (see [Build & deploy](#build--deploy)).

## Build & deploy

### Toolchain (Fedora/Nobara)

```bash
sudo dnf install mingw64-gcc-c++ mingw64-winpthreads-static cmake ninja git
```

### Build

```bash
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build/NexusRaidClears.dll`

### Deploy to local GW2

```bash
./scripts/deploy-to-gw2.sh           # DLL + static JSON + corner icon textures
./scripts/deploy-to-gw2.sh --ftue    # DLL only — test first-load downloads
```

Grid box masks are downloaded at runtime from the static host (not pre-seeded by
the deploy script). Pre-seeding `NexusRaidClears/clearsTracker/` JSON avoids
blocking on first load for encounter metadata.

A local deploy helper script may exist on your machine; it is not required for
end users.

### Nexus does not list the DLL

Rebuild with the provided `CMakeLists.txt` (static MinGW runtime). The DLL should
only import Windows system libraries:

```bash
x86_64-w64-mingw32-objdump -p build/NexusRaidClears.dll | rg "DLL Name"
```

Expect `KERNEL32.dll`, `USER32.dll`, `WS2_32.dll`, `msvcrt.dll` — **not**
`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, or `libwinpthread-1.dll`.

Also confirm:

- Filename is `NexusRaidClears.dll` (not `libNexusRaidClears.dll`)
- File is x64 and sits directly in `addons/`

## Manual test checklist

- [ ] Addon loads without Nexus errors
- [ ] API key validates (`account` + `progression`)
- [ ] Raids panel shows wings/encounters with correct cleared/uncleared colors
- [ ] Strikes panel reflects weekly achievement 9125 clears
- [ ] Daily bounty row shows encounters with correct clear state
- [ ] Tomorrow row shows next-day rotation
- [ ] Fractals panel shows daily/rec tiers and CM tooltips
- [ ] Dungeons panel shows path clears
- [ ] Panels hide on world map; reappear in gameplay
- [ ] Settings and window positions persist after reload
- [ ] Manual refresh updates clears
- [ ] Keybind / corner icon toggle panels

## Static data

Encounter layout and metadata are fetched from freesnow's Blish static host:

`https://bhm.blishhud.com/Soeed.RaidClears/static/v2/`

Icons may be loaded from `https://assets.gw2dat.com/`. Both hosts receive an
identifiable `User-Agent: ClearsTracker-Nexus/<version>`.

To add or update encounters, see **[CONTRIBUTING.md](CONTRIBUTING.md)** — most
changes are JSON-only in the static-data branch; dungeons require a code change.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build notes, project layout, and a
step-by-step guide for maintaining raids, strikes, fractals, bounties, and
dungeons when ArenaNet adds content.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).

## Credits

- **Soeed** — Clears Tracker (BlishHUD) and this Nexus port
- **Abbadon** — BlishHUD contributor
- **freesnow** — static data and icon hosting (`bhm.blishhud.com`, `assets.gw2dat.com`)
- **Raidcore** — Nexus addon platform

Inspired by the raid feature in Gw2TaCO.

## Links

- [BlishHUD Clears Tracker](https://github.com/a727891/BlishHud-Raid-Clears)
- [BlishHUD module page](https://blishhud.com/modules/?module=Soeed.RaidClears)
- [Patch notes](https://pkgs.blishhud.com/Soeed.RaidClears.html)
- [Nexus](https://raidcore.gg/gw2/nexus)

## License

MIT — see [LICENSE](LICENSE). Nexus API headers: Raidcore.GG MIT license.