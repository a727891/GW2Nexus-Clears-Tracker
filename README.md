# Nexus Raid Clears

Nexus addon port of the BlishHUD Raid Clears module. Shows raid and strike overlay panels with weekly clear indicators from the GW2 API, plus daily raid bounty rows on the Strikes panel.

## Features (MVP)

- **Raids panel** — wing/encounter grid colored by `/v2/account/raids`
- **Strikes panel** — weekly strike clears from achievement 9125, plus Daily Bounty and Tomorrow rows
- **Daily raid bounties** — today's bounty clear state from achievement category 475
- **Map-tracked strike** — Dragonstorm clear on map leave via Mumble link
- **Settings** — API key, colors, poll interval, panel visibility
- **Keybind** — `NRC_TOGGLE_PANELS` (default `ALT+SHIFT+R`). Choose which panels respond in options via **Toggle raids/strikes on keybind / corner icon click**. Corner icon left-click uses the same shortcut.

## Requirements

- [Nexus](https://raidcore.gg/gw2/nexus) installed in Guild Wars 2
- GW2 API key with `account` and `progression` permissions
- Windows x64 for runtime

## Build (Linux cross-compile → Windows DLL)

Install MinGW toolchain (Fedora/Nobara):

```bash
sudo dnf install mingw64-gcc-c++ mingw64-winpthreads-static cmake ninja git
```

Configure and build:

```bash
cd NexusRaidClears
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Output: `build/NexusRaidClears.dll`

### Deploy to local GW2 (Steam/Proton)

After building, run:

```bash
./scripts/deploy-to-gw2.sh
```

This copies:
- `build/NexusRaidClears.dll` → GW2 `addons/`
- `raid_data.json`, `strike_data.json`, `daily_bounties.json` → `addons/NexusRaidClears/clearsTracker/`

Pre-seeding the static cache avoids a background download on first load and prevents load-time freezes.

## Deploy

1. Copy `build/NexusRaidClears.dll` to `<GW2>/addons/` (the `addons` subfolder, not the game root)
2. Launch GW2 with Nexus
3. Open Nexus options → find **Nexus Raid Clears** in the addon list
4. Enable the addon, then enter your GW2 API key and click **Save API Key**
5. Enable Raids / Strikes panels

Static encounter data is cached under `<GW2>/addons/NexusRaidClears/clearsTracker/`.

### Nexus does not list the DLL

The most common cause for MinGW-built addons is missing runtime DLLs. Rebuild with the current `CMakeLists.txt` (static MinGW runtime linking). After rebuilding, verify the DLL only imports Windows system libraries:

```bash
x86_64-w64-mingw32-objdump -p build/NexusRaidClears.dll | rg "DLL Name"
```

You should see only entries like `KERNEL32.dll`, `USER32.dll`, `WS2_32.dll`, `msvcrt.dll` — **not** `libstdc++-6.dll`, `libgcc_s_seh-1.dll`, or `libwinpthread-1.dll`.

Also confirm:
- File is named `NexusRaidClears.dll` (not `libNexusRaidClears.dll`)
- File is directly in `addons/`, not a subfolder
- DLL is x64 (matches GW2 / Nexus)

## Manual test checklist

- [ ] Addon loads without Nexus errors
- [ ] API key validates (`account` + `progression`)
- [ ] Raids panel shows wings/encounters with correct cleared/uncleared colors
- [ ] Strikes panel shows weekly clears from achievement 9125
- [ ] Daily Bounty row shows 4 encounters with correct clear state
- [ ] Tomorrow row shows next-day rotation (neutral coloring)
- [ ] Panels hide when world map is open
- [ ] Panels reappear in gameplay
- [ ] Window positions and settings persist after reload
- [ ] Manual refresh updates clears
- [ ] Keybinds toggle panels

## Static data source

Encounter layout JSON is fetched from:

`https://bhm.blishhud.com/Soeed.RaidClears/static/v2/`

Files: `raid_data.json`, `strike_data.json`, `daily_bounties.json`

## License

MIT (Nexus API headers: Raidcore.GG MIT license)
