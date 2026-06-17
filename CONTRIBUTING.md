# Contributing

Thanks for helping maintain **Clears Tracker** for Nexus. This addon is a port of the [BlishHUD Clears Tracker](https://github.com/a727891/BlishHud-Raid-Clears) module. Most encounter data is shared between both clients via static JSON hosted by Freesnöw.

## Development setup

### Requirements

- Linux with MinGW-w64 cross-compiler (or native Windows with MSVC/MinGW)
- CMake 3.20+, Ninja
- Git

Fedora/Nobara toolchain:

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

Output: `build/ClearsTracker.dll`

### Deploy locally

Copy `ClearsTracker.dll` to `<GW2>/addons/`.
### Project layout

| Path | Purpose |
|------|---------|
| `src/core/` | Settings, account registry, branding |
| `src/data/` | JSON parsers for static encounter data |
| `src/services/` | GW2 API, polling, clears persistence, map watchers |
| `src/ui/` | ImGui panels, options, tooltips, corner icon |
| `extern/nexus/` | Nexus addon API headers |
| `cmake/` | MinGW cross-compile toolchain |

---

## Maintaining encounter data

Most content updates **do not require a Nexus code change**. Raids, strikes,
fractals, daily bounties, and instability metadata are loaded from static JSON
at runtime. Dungeons are the main exception - they are hardcoded in C++.

### Repositories involved

| Repo | Role |
|------|------|
| [BlishHud-Raid-Clears](https://github.com/a727891/BlishHud-Raid-Clears) | Primary module; reference for behavior and API usage |
| `bhud-static/Soeed.RaidClears` branch | Static JSON and option textures (local clone often named `raidClearsStatic`) |
| **This repo** (`NexusRaidClears`) | Nexus port; consumes the same static files |

JSON data files are served from `.../static/v2/`. PNG textures and grid box masks
are served from `.../static/` (no `v2/` prefix), matching the BlishHUD module.

The addon caches them under `<GW2>/addons/ClearsTracker/clearsTracker/`.
HTTP requests to that host and to `assets.gw2dat.com` send
`User-Agent: ClearsTracker-Nexus/<version>`.

### Static data files

| File | What it drives |
|------|----------------|
| `raid_data.json` | Raid wings, encounters, tooltips, mentor IDs, bounty achievement IDs |
| `strike_data.json` | Strike missions, weekly achievement 9125 bit mapping, map-tracked strikes |
| `daily_bounties.json` | Daily raid bounty slot rotation |
| `fractal_maps.json` | Fractal names, scales, daily tier / rec tables, CM motes |
| `fractal_instabilities.json` | Instability names and icon asset IDs |
| `clears_tracker.json` | Version stamps, texture asset manifest, corner-icon MOTD |

Edit these under `static/v2/` in the static-data branch/repo.

### What needs a code change vs. static JSON only

| Content | Usually static JSON | Usually needs C++ |
|---------|--------------------|--------------------|
| New raid boss | Yes | Only if new JSON fields or API behavior |
| New strike | Yes | Map-tracked strikes need `mapIds` + `MapWatcherService` support (already generic) |
| Daily bounty rotation | Yes (`daily_bounties.json` + encounter IDs in `raid_data.json`) | No |
| New fractal | Yes | No |
| New dungeon path | **No** - edit `src/data/DungeonData.cpp` | Yes |
| New tooltip field | Maybe | If parser/UI does not already handle the field |
| New clear source (e.g. new achievement) | Maybe | Yes - new service logic in Blish + port to Nexus |

**Rule of thumb:** update static data first and test in BlishHUD. If Blish works
with JSON alone, Nexus usually picks it up on the next static refresh with no
DLL rebuild.

---

### Adding a new raid encounter

1. **Find the API id** - check [GW2 API `/v2/raids`](https://api.guildwars2.com/v2/raids) or existing entries in `raid_data.json`. The `api_id` must match what `/v2/account/raids` returns.

2. **Edit `raid_data.json`** - add an encounter object under the correct wing in `expansions[].wings[].encounters`:

   - `name`, `api_id`, `abbriviation` (sic - matches Blish schema)
   - `assetId` - GW2 dat icon id (tooltips / optional display)
   - `powerFavored`, `condiFavored`, `needsDefianceBreak` - tooltip flags
   - `mentor_achievement_id` / `mentor_achievement_max` - if the boss has a mentor achievement
   - `daily_bounty_achievement_id` - if the boss can appear as a daily raid bounty

3. **Bump `version`** at the top of `raid_data.json` (ISO timestamp).

4. **Update `clears_tracker.json`** - set `"raid_data"` to the same version string.

5. **If bounty-eligible** - ensure the `api_id` appears in the appropriate `bossSlots` in `daily_bounties.json`.

6. **Publish** static branch to hosting (see [Publishing static updates](#publishing-static-updates)).

7. **Verify** - refresh API in-game; new encounter should appear uncleared/cleared based on `/v2/account/raids`. Check mentor tooltip and non-weekly bounty highlight if applicable.

---

### Adding a new strike

1. **Edit `strike_data.json`** - add a mission under the right `expansions[].missions` entry:

   - `id` - stable string key used in settings and persistence
   - `name`, `abbriviation`, `assetId`, `mapIds`
   - `resets` - `"weekly"` (default) or `"daily"` for daily-reward strikes

2. **Weekly API-tracked strikes** - if the strike is part of achievement **9125** (Weekly Raid Encounters), append its `id` to `weekly_achievement_bit_strike_ids` **in bit order**. Index `i` in that array maps to bit `i` on the achievement.

3. **Map-tracked strikes** (e.g. Dragonstorm) - add the `id` to `map_tracked_strike_ids`, set `"resets": "daily"`, and include the strike's `mapIds`. Clears are recorded on map leave, not from the API.

4. **Bump `version`** in `strike_data.json` and update `clears_tracker.json` → `"strike_data"`.

5. **Publish and verify** - weekly strikes should sync from achievement 9125 on API refresh; map-tracked strikes should clear when leaving the map.

---

### Updating daily raid bounties

Daily bounties use achievement category **475**. The rotation is defined in
`daily_bounties.json` (`bossSlots` with encounter `api_id` lists), not computed
in code.

When ArenaNet changes the bounty pool or slot layout:

1. Update `bossSlots` in `daily_bounties.json`.
2. Ensure every referenced `api_id` exists in `raid_data.json` with a
   `daily_bounty_achievement_id` where needed.
3. Bump `daily_bounties.json` `version` and `clears_tracker.json` →
   `"daily_bounties"`.

---

### Adding or updating fractals

1. **Edit `fractal_maps.json`**:
   - Add/update an entry in `maps` (`api` label must match GW2 fractal API naming).
   - Set `scales`, `id` (map id), `label` / `short`.
   - Update `DailyTier` and/or `Recs` tables when daily/recommended rotations change.
   - Update `challengeMotes` if CM scale list changes.

2. **Instabilities** - if new instability names or icons are needed, edit
   `fractal_instabilities.json` and bump its version reference in
   `clears_tracker.json` → `"fractal_instabilities"`.

3. **Bump** `fractal_maps.json` version (via `clears_tracker.json` →
   `"fractal_map_data"`).

4. **Publish and verify** - fractal panel daily/rec rows and CM tooltips should
   reflect the new data after refresh.

---

### Dungeons (code change required)

Dungeon paths are **not** in static JSON. They live in
`src/data/DungeonData.cpp` as `DungeonGroupDef` entries. API path ids (e.g.
`hodgins`, `ac_story`) must match `/v2/account/dungeons`.

When a new path is added (rare):

1. Add the path to the correct group in `DungeonData.cpp`.
2. Rebuild and release the Nexus DLL (and update Blish if parity is required).

---

### Publishing static updates

1. Commit changes on the `bhud-static/Soeed.RaidClears` branch under `static/v2/`.
2. Deploy to `bhm.blishhud.com` (Freesnöw's hosting - coordinate with him if you do not publish yourself).
3. If `clears_tracker.json` lists new textures in `assets`, upload those PNGs too.
4. Users with cached files get updates on **Refresh API** / addon reload when the hosted version changes.

`clears_tracker.json` also carries `motd` / `motd_id` for the corner-icon message
and `cache_bust` for asset invalidation - update when you want users to notice a
release.

---

### Nexus release checklist

Use this when static data alone is not enough, or when shipping a Nexus-specific fix:

1. Implement and test the change; keep behavior aligned with Blish where possible.
2. Bump version in **both**:
   - `src/core/Branding.h` → `kVersion` and `kHttpUserAgent`
   - `src/entry.cpp` → `def.Version = {major, minor, patch, build}`
3. Add an entry to [CHANGELOG.md](CHANGELOG.md).
4. Build `ClearsTracker.dll` and smoke-test with the [README test checklist](README.md#manual-test-checklist).
5. Distribute the DLL to Nexus users (distribution method TBD for this repo).

---

### Porting changes from Blish

When the Blish module gains a feature or fix:

1. Find the equivalent area in this repo (`src/services/`, `src/ui/`, `src/data/`).
2. Port logic - Nexus uses ImGui + Nexus API instead of Blish HUD controls, but
   GW2 API and static JSON layers should stay parallel.
3. If Blish changed static schema, update parsers in `src/data/*.cpp` only if
   new fields are required in the UI or services.

Useful Blish references:

- `Features/Raids/Services/RaidData.cs`
- `Features/Strikes/Services/StrikeData.cs`
- `Features/Fractals/Services/FractalMapData.cs`
- `Features/Shared/Services/DailyBountyDataService.cs`

---

### API permissions and test tokens

The addon requires a GW2 API key with **`account`** and **`progression`**
permissions. Use `gw2.http` (local REST client file) for manual API probing -
never commit real tokens.

---

## Questions

- **Blish / static hosting:** Freesnöw (BlishHUD infrastructure)
- **Module logic / GW2 data:** open an issue or PR on the relevant repo
- **Nexus runtime:** [Raidcore Nexus](https://raidcore.gg/gw2/nexus)
