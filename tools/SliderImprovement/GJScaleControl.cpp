#include "passTouch.hpp"
#include "ScaleTextDelegate.hpp"

bool g_bLockPosEnabled;
bool g_bUnlockScaleEnabled;

bool LockButton::init(
    CCSprite* sprite,
    CCNode* target,
    SEL_MenuHandler cb,
    LockType type
) {
    if (!CCMenuItemSprite::initWithNormalSprite(
        sprite, nullptr, nullptr,
        target, cb
    )) return false;
    if (!CCMenuItemSpriteExtra::init(sprite))
        return false;
    
    this->m_eType = type;

    return true;
}

void LockButton::mouseLeaveSuper(CCPoint const&) {
    this->unselected();
}

bool LockButton::mouseDownSuper(MouseButton, CCPoint const&) {
    this->selected();
    return true;
}

bool LockButton::mouseUpSuper(MouseButton, CCPoint const&) {
    this->activate();
    return true;
}

LockButton* LockButton::create(
    CCSprite* sprite,
    CCNode* target,
    SEL_MenuHandler cb,
    LockType type
) {
    auto ret = new LockButton;

    if (ret && ret->init(sprite, target, cb, type)) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}


void ScaleTextDelegate::textChanged(CCTextInputNode* input) {
    float val = 1.0f;

    if (input->getString() && strlen(input->getString()))
        val = static_cast<float>(std::atof(input->getString()));

    m_pControl->m_fValue = val;
    m_pControl->m_pSlider->setValue((val - .5f) / 1.5f);

    auto ui = GameManager::sharedState()->getEditorLayer()->getEditorUI();

    if (ui)
        ui->scaleChanged(val);
}

ScaleTextDelegate* ScaleTextDelegate::create(GJScaleControl* c) {
    auto ret = new ScaleTextDelegate();

    if (ret && ret->init()) {
        ret->m_pControl = c;
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

class GJScaleControl_CB : public GJScaleControl {
    ScaleTextDelegate* getSTD() {
        return reinterpret_cast<ScaleTextDelegate*>(this->getChildByTag(7777));
    }
    
    public:
        void onLockPosition(CCObject* pSender) {
            auto std = getSTD();

            std->m_bLockPosEnabled = !std->m_bLockPosEnabled;
            g_bLockPosEnabled = std->m_bLockPosEnabled;

            reinterpret_cast<CCMenuItemSpriteExtra*>(pSender)
                ->setNormalImage(createLockSprite(
                    std->m_bLockPosEnabled ? "GJ_button_02.png" : "GJ_button_04.png",
                    std->m_bLockPosEnabled ? "GJ_lock_001.png" : "GJ_lock_open_001.png"
                ));
            
            if (std->m_bLockPosEnabled)
                patch(0x8f2f9, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
            else
                unpatch(0x8f2f9);
        }

        void onUnlockScale(CCObject* pSender) {
            auto std = getSTD();

            std->m_bUnlockScaleEnabled = !std->m_bUnlockScaleEnabled;
            g_bUnlockScaleEnabled = std->m_bUnlockScaleEnabled;

            reinterpret_cast<CCMenuItemSpriteExtra*>(pSender)
                ->setNormalImage(createLockSprite(
                    std->m_bUnlockScaleEnabled ? "GJ_button_02.png" : "GJ_button_01.png",
                    std->m_bUnlockScaleEnabled ? "GJ_lock_open_001.png" : "GJ_lock_001.png"
                ));
        }
};

bool mouseIsHoveringNode(CCNode* node, CCPoint const& mpos) {
    auto pos = node->getParent()->convertToWorldSpace(node->getPosition());
    auto size = node->getScaledContentSize();

    auto rect = CCRect {
        pos.x - size.width / 2,
        pos.y - size.height / 2,
        size.width,
        size.height
    };

    return rect.containsPoint(mpos);
}

bool pointIntersectsScaleControls(EditorUI* self, CCTouch* touch, CCEvent* event) {
    auto std = reinterpret_cast<ScaleTextDelegate*>(
        self->m_pScaleControl->getChildByTag(7777)
    );

    if (!self->m_pScaleControl->isVisible()) {
        if (std)
            reinterpret_cast<ScaleTextDelegate*>(std)->m_pInputNode->getTextField()->detachWithIME();
        
        return false;
    }

    if (std) {
        auto inputNodeSize = std->m_pInputNode->getScaledContentSize();
        inputNodeSize.width *= 2;
        inputNodeSize.height *= 1.5f;

        auto inputRect = CCRect {
            self->m_pScaleControl->getPositionX() + std->m_pInputNode->getPositionX() - inputNodeSize.width / 2,
            self->m_pScaleControl->getPositionY() + std->m_pInputNode->getPositionY() - inputNodeSize.height / 2,
            inputNodeSize.width,
            inputNodeSize.height
        };

        auto pos = GameManager::sharedState()
            ->getEditorLayer()
            ->getObjectLayer()
            ->convertTouchToNodeSpace(touch);
        
        if (inputRect.containsPoint(pos))
            std->m_pInputNode->getTextField()->attachWithIME();
        else
            std->m_pInputNode->getTextField()->detachWithIME();
        
        if (mouseIsHoveringNode(std->m_pLockPosBtn, touch->getLocation())) {
            std->m_pLockPosBtn->selected();
            return true;
        }
        
        if (mouseIsHoveringNode(std->m_pLockScaleBtn, touch->getLocation())) {
            std->m_pLockScaleBtn->selected();
            return true;
        }

        return inputRect.containsPoint(pos);
    }

    return false;
}

void  GJScaleControl_ccTouchMoved(GJScaleControl* self_,  CCTouch* touch, CCEvent* event) {
    auto self = reinterpret_cast<GJScaleControl*>(reinterpret_cast<uintptr_t>(self_) - 0xEC);

    if (self->m_nTouchID == touch->getID()) {
        self->m_pSlider->ccTouchMoved(touch, event);

        float val = roundf((self->m_pSlider->getValue() * 1.5f + .5f) * 100) / 100;

        auto std = reinterpret_cast<ScaleTextDelegate*>(self->getChildByTag(7777));
        auto snap = BetterEdit::sharedState()->getScaleSnap();
        if (snap == 0) snap = 1;

        if (std && !std->m_bUnlockScaleEnabled) {
            val = roundf(val / snap) * snap;
        
            self->m_pSlider->setValue((val - .5f) / 1.5f);
        }

        if (self->m_pDelegate)
            self->m_pDelegate->scaleChanged(val);

        self->m_fValue = val;
        
        self->updateLabel(val);
    }
} MAT_GDMAKE_HOOK(0x94840, GJScaleControl_ccTouchMoved);

void  GJScaleControl_updateLabel(GJScaleControl* self) {
    // note: there's a float in xmm0

    matdash::orig<&GJScaleControl_updateLabel>(self);

    auto t = self->getChildByTag(6978);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << self->m_fValue;
    std::string s = stream.str();

    if (t)
        reinterpret_cast<CCTextInputNode*>(t)->setString(s.c_str());
} MAT_GDMAKE_HOOK(0x94990, GJScaleControl_updateLabel);

void  GJScaleControl_loadValues(GJScaleControl* self,  GameObject* obj, CCArray* objs) {
    matdash::orig<&GJScaleControl_loadValues>(self,  obj, objs);

    auto scale = 1.5f * self->m_pSlider->getValue() + .5f;

    auto t = self->getChildByTag(6978);

    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << scale;
    std::string s = stream.str();

    if (t)
        reinterpret_cast<CCTextInputNode*>(t)->setString(s.c_str());
} MAT_GDMAKE_HOOK(0x94590, GJScaleControl_loadValues);

bool  GJScaleControl_init(GJScaleControl* self) {
    if (!matdash::orig<&GJScaleControl_init>(self))
        return false;

    BetterEdit::saveGlobalBool("scale-lock-pos",  &g_bLockPosEnabled);
    BetterEdit::saveGlobalBool("scale-dont-snap", &g_bUnlockScaleEnabled);

    auto ed = ScaleTextDelegate::create(self);

    ed->m_bLockPosEnabled = g_bLockPosEnabled;
    ed->m_bUnlockScaleEnabled = g_bUnlockScaleEnabled;

    self->m_pLabel->setVisible(false);

    auto spr = extension::CCScale9Sprite::create(
        "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );

    spr->setScale(.3f);
    spr->setColor({ 0, 0, 0 });
    spr->setOpacity(100);
    spr->setContentSize({ 115.0f, 75.0f });
    spr->setPosition(self->m_pLabel->getPosition());

    auto eLayerInput = CCTextInputNode::create("1.00", ed, "bigFont.fnt", 40.0f, 30.0f);

    eLayerInput->setPosition(self->m_pLabel->getPosition());
    eLayerInput->setLabelPlaceholderColor({ 120, 120, 120 });
    eLayerInput->setAllowedChars(".0123456789");
    eLayerInput->setAnchorPoint({ 0, 0 });
    eLayerInput->setScale(.7f);
    eLayerInput->setDelegate(ed);
    eLayerInput->setTag(6978);
    eLayerInput->setString("1.00");

    self->addChild(spr);
    self->addChild(eLayerInput, 999);
    self->addChild(ed);

    auto menu = CCMenu::create();

    /////////////////////////////////

    auto unlockScaleBtn = LockButton::create(
        CCSprite::create(
            "GJ_button_01.png"
        ),
        self,
        (SEL_MenuHandler)&GJScaleControl_CB::onUnlockScale,
        kLockTypeSlider
    );

    unlockScaleBtn->setNormalImage(
        createLockSprite(
            ed->m_bUnlockScaleEnabled ? "GJ_button_02.png" : "GJ_button_01.png",
            "GJ_lock_001.png"
        )
    );
    unlockScaleBtn->setPosition({ 30.0f, 0.0f });

    menu->addChild(unlockScaleBtn);

    /////////////////////////////////

    auto lockPositionBtn = LockButton::create(
        CCSprite::create("GJ_button_01.png"),
        self,
        (SEL_MenuHandler)&GJScaleControl_CB::onLockPosition,
        kLockTypeAbsolute
    );

    lockPositionBtn->setNormalImage(
        createLockSprite(
            ed->m_bLockPosEnabled ? "GJ_button_02.png" : "GJ_button_04.png",
            "GJ_lock_open_001.png"
        )
    );
    lockPositionBtn->setPosition({ -30.0f, 0.0f });

    menu->addChild(lockPositionBtn);

    /////////////////////////////////

    menu->setPosition(self->m_pLabel->getPosition());
    self->addChild(menu);

    ed->m_pInputNode = eLayerInput;
    ed->m_pLockPosBtn = lockPositionBtn;
    ed->m_pLockScaleBtn = unlockScaleBtn;
    ed->setTag(7777);

    return true;
} MAT_GDMAKE_HOOK(0x94490, GJScaleControl_init);

void  EditorUI_activateScaleControl(EditorUI* self,  CCObject* pSender) {
    auto fixPos =
        BetterEdit::getFixScaleSliderPosition() &&
        self->m_pEditorLayer->m_pObjectLayer->getScale() >= 2.f;

    if (fixPos) {
        patch(0x88b64, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
    } else {
        unpatch(0x88b64);
    }

    matdash::orig<&EditorUI_activateScaleControl>(self,  pSender);

    if (fixPos) {
        auto pos = CCDirector::sharedDirector()->getWinSize() / 2;
        pos.height += 50.0f;
        pos = self->m_pEditorLayer->m_pObjectLayer->convertToNodeSpace(pos);
        self->m_pScaleControl->setPosition(pos);
    }
} MAT_GDMAKE_HOOK(0x889b0, EditorUI_activateScaleControl);

void  EditorUI_updateSpecialUIElements(EditorUI* self) {
    matdash::orig<&EditorUI_updateSpecialUIElements>(self);

    CATCH_NULL(reinterpret_cast<CCTextInputNode*>(self->getChildByTag(6978)))->detachWithIME();
} MAT_GDMAKE_HOOK(0x87030, EditorUI_updateSpecialUIElements);
