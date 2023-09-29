#include "passTouch.hpp"
#include "ScaleTextDelegate.hpp"

bool g_bLockPosEnabled = false;
bool g_bUnlockScaleEnabled = true;

// bool LockButton::init(
//     CCSprite* sprite,
//     CCNode* target,
//     SEL_MenuHandler cb,
//     LockType type
// ) {
//     if (!CCMenuItemSprite::initWithNormalSprite(
//         sprite, nullptr, nullptr,
//         target, cb
//     )) return false;
//     if (!CCMenuItemSpriteExtra::init(sprite, nullptr, nullptr, cb))
//         return false;
    
//     this->m_eType = type;

//     return true;
// }

// void LockButton::mouseLeaveSuper(CCPoint const&) {
//     this->unselected();
// }

// bool LockButton::mouseDownSuper(MouseButton, CCPoint const&) {
//     this->selected();
//     return true;
// }

// bool LockButton::mouseUpSuper(MouseButton, CCPoint const&) {
//     this->activate();
//     return true;
// }

// LockButton* LockButton::create(
//     CCSprite* sprite,
//     CCNode* target,
//     SEL_MenuHandler cb,
//     LockType type
// ) {
//     auto ret = new LockButton;

//     if (ret && ret->init(sprite, target, cb, type)) {
//         ret->autorelease();
//         return ret;
//     }

//     CC_SAFE_DELETE(ret);
//     return nullptr;
// }


void ScaleTextDelegate::textChanged(CCTextInputNode* input) {
    float val = 1.0f;

    if (input->getString() && strlen(input->getString()))
        val = static_cast<float>(std::atof(input->getString()));

    m_pControl->m_value = val;
    m_pControl->m_slider->setValue((val - .5f) / 1.5f);

    auto ui = GameManager::sharedState()->getEditorLayer()->m_editorUI;

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
        self->m_scaleControl->getChildByTag(7777)
    );

    if (!self->m_scaleControl->isVisible()) {
        if (std)
            reinterpret_cast<ScaleTextDelegate*>(std)->m_pInputNode->getTextField()->detachWithIME();
        
        return false;
    }

    if (std) {
        auto inputNodeSize = std->m_pInputNode->getScaledContentSize();
        inputNodeSize.width *= 2;
        inputNodeSize.height *= 1.5f;

        auto inputRect = CCRect {
            self->m_scaleControl->getPositionX() + std->m_pInputNode->getPositionX() - inputNodeSize.width / 2,
            self->m_scaleControl->getPositionY() + std->m_pInputNode->getPositionY() - inputNodeSize.height / 2,
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

#include <Geode/modify/GJScaleControl.hpp>

class $modify(GJScaleControl) {
    void ccTouchMoved(CCTouch* touch, CCEvent* event) {
        if (m_touchID == touch->getID()) {
            m_slider->ccTouchMoved(touch, event);

            float val = roundf((m_slider->getValue() * 1.5f + .5f) * 100) / 100;

            auto std = reinterpret_cast<ScaleTextDelegate*>(this->getChildByTag(7777));
            auto snap = BetterEdit::sharedState()->getScaleSnap();
            if (snap == 0) snap = 1;

            if (std && !std->m_bUnlockScaleEnabled) {
                val = roundf(val / snap) * snap;
            
                m_slider->setValue((val - .5f) / 1.5f);
            }

            if (m_delegate) {
                // geode::log::debug("func {}", (*((void***)m_delegate))[2]);
                m_delegate->scaleChanged(val);
            }
                

            m_value = val;
            
            GJScaleControl::updateLabel(val);
        }
    }

    void updateLabel(float value) {
        GJScaleControl::updateLabel(value);

        auto t = this->getChildByTag(6978);

        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << m_value;
        std::string s = stream.str();

        if (t)
            reinterpret_cast<CCTextInputNode*>(t)->setString(s.c_str());
    }

    void loadValues(GameObject* obj, CCArray* objs) {
        GJScaleControl::loadValues(obj, objs);

        GJScaleControl::updateLabel(1.5f * m_slider->getValue() + .5f);
    }

    bool init() {
        if (!GJScaleControl::init())
            return false;

        // BetterEdit::saveGlobalBool("scale-lock-pos",  &g_bLockPosEnabled);
        // BetterEdit::saveGlobalBool("scale-dont-snap", &g_bUnlockScaleEnabled);

        auto ed = ScaleTextDelegate::create(this);

        ed->m_bLockPosEnabled = g_bLockPosEnabled;
        ed->m_bUnlockScaleEnabled = g_bUnlockScaleEnabled;

        m_label->setVisible(false);

        auto spr = extension::CCScale9Sprite::create(
            "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
        );

        spr->setScale(.3f);
        spr->setColor({ 0, 0, 0 });
        spr->setOpacity(100);
        spr->setContentSize({ 115.0f, 75.0f });
        spr->setPosition(m_label->getPosition());

        auto eLayerInput = CCTextInputNode::create(40.0f, 30.0f, "1.00", 24, "bigFont.fnt");

        eLayerInput->setPosition(m_label->getPosition());
        eLayerInput->setLabelPlaceholderColor({ 120, 120, 120 });
        eLayerInput->setAllowedChars(".0123456789");
        eLayerInput->setAnchorPoint({ 0, 0 });
        eLayerInput->setScale(.7f);
        eLayerInput->setDelegate(ed);
        eLayerInput->setTag(6978);
        eLayerInput->setString("1.00");

        this->addChild(spr);
        this->addChild(eLayerInput, 999);
        this->addChild(ed);

        auto menu = CCMenu::create();

        /////////////////////////////////

        // TODO: readd?
        // auto unlockScaleBtn = LockButton::create(
        //     CCSprite::create(
        //         "GJ_button_01.png"
        //     ),
        //     this,
        //     (SEL_MenuHandler)&GJScaleControl_CB::onUnlockScale,
        //     kLockTypeSlider
        // );

        // unlockScaleBtn->setNormalImage(
        //     createLockSprite(
        //         ed->m_bUnlockScaleEnabled ? "GJ_button_02.png" : "GJ_button_01.png",
        //         "GJ_lock_001.png"
        //     )
        // );
        // unlockScaleBtn->setPosition({ 30.0f, 0.0f });

        // menu->addChild(unlockScaleBtn);

        // /////////////////////////////////

        // auto lockPositionBtn = LockButton::create(
        //     CCSprite::create("GJ_button_01.png"),
        //     this,
        //     (SEL_MenuHandler)&GJScaleControl_CB::onLockPosition,
        //     kLockTypeAbsolute
        // );

        // lockPositionBtn->setNormalImage(
        //     createLockSprite(
        //         ed->m_bLockPosEnabled ? "GJ_button_02.png" : "GJ_button_04.png",
        //         "GJ_lock_open_001.png"
        //     )
        // );
        // lockPositionBtn->setPosition({ -30.0f, 0.0f });

        // menu->addChild(lockPositionBtn);

        /////////////////////////////////

        menu->setPosition(m_label->getPosition());
        this->addChild(menu);

        ed->m_pInputNode = eLayerInput;
        // ed->m_pLockPosBtn = lockPositionBtn;
        // ed->m_pLockScaleBtn = unlockScaleBtn;
        ed->setTag(7777);

        return true;
    }
};

#include <Geode/modify/EditorUI.hpp>

class $modify(EditorUI) {
    void activateScaleControl(CCObject* pSender) {
        auto fixPos =
            BetterEdit::getFixScaleSliderPosition() &&
            m_editorLayer->m_objectLayer->getScale() >= 2.f;

        // TODO: patch
        // if (fixPos) {
        //     patch(0x88b64, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
        // } else {
        //     unpatch(0x88b64);
        // }

        EditorUI::activateScaleControl(pSender);

        if (fixPos) {
            auto pos = CCDirector::sharedDirector()->getWinSize() / 2;
            pos.height += 50.0f;
            pos = m_editorLayer->m_objectLayer->convertToNodeSpace(pos);
            m_scaleControl->setPosition(pos);
        }
    }

    void updateSpecialUIElements() {
        EditorUI::updateSpecialUIElements();

        CATCH_NULL(reinterpret_cast<CCTextInputNode*>(this->getChildByTag(6978)))->detachWithIME();
    }
};
