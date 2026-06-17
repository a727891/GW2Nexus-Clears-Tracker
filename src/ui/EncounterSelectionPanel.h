#pragma once

namespace rc {

class AppState;

namespace EncounterSelectionPanel {
void RenderRaidSelection(AppState& state);
void RenderStrikeSelection(AppState& state);
void RenderFractalSelection(AppState& state);
void RenderDungeonSelection(AppState& state);
}

}  // namespace rc
