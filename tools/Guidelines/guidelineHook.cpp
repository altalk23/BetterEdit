#include "../../BetterEdit.hpp"
#include "EnterBPMLayer.hpp"

class CreateGuidelinesLayer_CB : public CreateGuidelinesLayer {
    public:
        void onEnterBPM(CCObject*) {
            EnterBPMLayer::create()->setTarget(this)->show();
        }
};


bool  CreateGuidelinesLayer_init(CreateGuidelinesLayer* self,  LevelSettingsObject* pObj) {
    if (!matdash::orig<&CreateGuidelinesLayer_init>(self,  pObj))
        return false;
    
    // auto info = CCLabelBMFont::create(
    //     "Warning: Entering BPM may CRASH!\nI'm looking into a fix, "
    //     "but for now,\nproceed with caution. -HJfod",
    //     "bigFont.fnt"
    // );
    // info->setScale(.3f);
    // info->setColor(cc3x(0xf00));
    // info->setPosition(120.0f, -20.0f);
    // self->m_pButtonMenu->addChild(info);

    auto bpmBtn = CCMenuItemSpriteExtra::create(
        CCNodeConstructor<ButtonSprite*>()
            .fromButtonSprite("BPM", "GJ_button_04.png", "goldFont.fnt")
            .scale(.8f)
            .done(),
        self,
        (SEL_MenuHandler)&CreateGuidelinesLayer_CB::onEnterBPM
    );
    bpmBtn->setPosition(120.0f, -50.0f);
    self->m_pButtonMenu->addChild(bpmBtn);

    return true;
} MAT_GDMAKE_HOOK(0x4c190, CreateGuidelinesLayer_init);
