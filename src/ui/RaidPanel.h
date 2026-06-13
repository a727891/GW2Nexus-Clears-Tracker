#pragma once

namespace rc {

class AppState;

namespace RaidPanel {
    void Render(AppState& state);
}

namespace StrikesPanel {
    void Render(AppState& state);
}

namespace OptionsPanel {
    void Render(AppState& state);
}

}  // namespace rc
