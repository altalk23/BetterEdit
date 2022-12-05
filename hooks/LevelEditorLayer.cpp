#include "../BetterEdit.hpp"
#include "../tools/LevelPercent/levelPercent.hpp"
#include "../tools/RotateSaws/rotateSaws.hpp"
#include "../tools/AutoSave/autoSave.hpp"
#include "../tools/other/teleportScaleFix.hpp"
#include "../tools/other/dashOrbLine.hpp"

GDMAKE_HOOK(0x15ee00, "_ZN16LevelEditorLayer4initEP11GJGameLevel")
bool __fastcall LevelEditorLayer_init(LevelEditorLayer* self, edx_t edx, GJGameLevel* level) {
    if (!GDMAKE_ORIG(self, edx, level))
        return false;

    BetterEdit::setEditorInitialized(true);

    updatePercentLabelPosition(self->m_pEditorUI);
    // getAutoSaveTimer(self->m_pEditorUI)->resetTimer();

    return true;
}

GDMAKE_HOOK(0x15e8d0, "_ZN16LevelEditorLayerD2Ev")
void __fastcall LevelEditorLayer_destructorHook(LevelEditorLayer* self) {
    BetterEdit::setEditorInitialized(false);
    BetterEdit::setEditorViewOnlyMode(false);
    clearDashOrbLines();

    return GDMAKE_ORIG_V(self);
}

GDMAKE_HOOK(0x162650, "_ZN16LevelEditorLayer10addSpecialEP10GameObject")
void __fastcall LevelEditorLayer_addSpecial(
    LevelEditorLayer* self,
    edx_t edx,
    GameObject* obj
) {
    GDMAKE_ORIG_V(self, edx, obj);

    handleObjectAddForSlider(self, obj);

    registerDashOrb(obj);
    
    if (shouldRotateSaw() && objectIsSaw(obj))
        beginRotateSaw(obj);

    fixPortalScale(obj);
}

GDMAKE_HOOK(0x161cb0, "_ZN16LevelEditorLayer12removeObjectEP10GameObjectb")
void __fastcall LevelEditorLayer_removeObject(
    LevelEditorLayer* self,
    edx_t edx,
    GameObject* obj,
    bool idk
) {
    GDMAKE_ORIG_V(self, edx, obj, idk);

    unregisterDashOrb(obj);

    handleObjectAddForSlider(self, obj);
    
    if (shouldRotateSaw() && objectIsSaw(obj))
        stopRotateSaw(obj);
}
