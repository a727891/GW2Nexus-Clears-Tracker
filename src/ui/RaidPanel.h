#pragma once

namespace rc {

class AppState;

namespace RaidPanel {
    void Render(AppState& state);
}

namespace StrikesPanel {
    void Render(AppState& state);
}

namespace FractalsPanel {
    void Render(AppState& state);
}

namespace DungeonsPanel {
    void Render(AppState& state);
}

}  // namespace rc
