#include "settings.hpp"

bool isAstroMiniPc (void)
{
    static bool initialized = false, val = false;
    if (!initialized)
    {
        initialized = true;
        val = QSettings(QSettings::SystemScope, "Pliskin707", "AstroBoxGUI").value("isAstroMiniPc", false).toBool();
    }

    return val;
}
