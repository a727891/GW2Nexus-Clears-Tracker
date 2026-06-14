#pragma once

namespace rc {

class AppState;

namespace OptionsPanel {
void Render(AppState& state);
}

namespace OptionsGeneralTab {
void Render(AppState& state, int& section);
}

namespace OptionsRaidsTab {
void Render(AppState& state, int& section);
}

namespace OptionsStrikesTab {
void Render(AppState& state, int& section);
}

namespace OptionsFractalsTab {
void Render(AppState& state, int& section);
}

namespace OptionsDungeonsTab {
void Render(AppState& state, int& section);
}

}  // namespace rc
