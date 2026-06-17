#include "ui/OptionsPanelAppearance.h"

#include "ui/OptionsUiKit.h"

namespace rc {
namespace OptionsPanelAppearance {

bool RenderFields(PanelAppearance& appearance,
                  const SettingsStore& settings,
                  bool showPreview) {
    using namespace OptionsUiKit;

    bool changed = false;

    const char* layoutLabels[] = {"Vertical", "Horizontal"};
    int layoutIndex = appearance.panelLayout == PanelLayout::Horizontal ? 1 : 0;
    if (SettingCombo("Panel layout", &layoutIndex, layoutLabels, 2,
                     "Arrange encounter cells vertically or horizontally.")) {
        appearance.panelLayout =
            layoutIndex == 1 ? PanelLayout::Horizontal : PanelLayout::Vertical;
        changed = true;
    }

    const char* groupLabelLabels[] = {"Show", "Hidden"};
    int groupLabelIndex = appearance.groupLabelDisplay == GroupLabelDisplay::Hidden ? 1 : 0;
    if (SettingCombo("Group label display", &groupLabelIndex, groupLabelLabels, 2,
                     "Controls the category label column on overlay panels.\n"
                     "Show: custom or default short names.\n"
                     "Hidden: hide category labels and reclaim the space.")) {
        appearance.groupLabelDisplay =
            groupLabelIndex == 1 ? GroupLabelDisplay::Hidden : GroupLabelDisplay::Abbreviation;
        changed = true;
    }

    if (SettingSliderFloat("Panel scale", &appearance.panelScale, 0.5f, 2.0f, "%.2f",
                           "Scale this overlay panel.")) {
        changed = true;
    }

    if (SettingSliderFloat("Label opacity", &appearance.labelOpacity, 0.1f, 1.0f, "%.2f",
                           "Opacity for wing and group label cells.")) {
        changed = true;
    }

    if (SettingSliderFloat("Grid opacity", &appearance.gridOpacity, 0.1f, 1.0f, "%.2f",
                           "Opacity for encounter cells.")) {
        changed = true;
    }

    if (SettingSliderFloat("Grid background opacity", &appearance.panelBackgroundOpacity, 0.0f,
                           1.0f, "%.2f", "Opacity for the panel background behind the grid.")) {
        changed = true;
    }

    if (showPreview) {
        ImGui::Spacing();
        RenderGridPreview(settings, appearance);
    }

    return changed;
}

}  // namespace OptionsPanelAppearance
}  // namespace rc
