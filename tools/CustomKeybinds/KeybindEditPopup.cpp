#include "KeybindEditPopup.hpp"

void KeybindEditPopup::setup() {
    if (m_pStoreItem)
        m_obTypedBind = m_pStoreItem->m_obBind;
    
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    this->m_bNoElasticity = true;
    this->setSuperMouseHitSize(this->m_pLrSize);
    this->setSuperMouseHitOffset(winSize / 2);

    auto subTitle = CCLabelBMFont::create(m_pCell->m_pBind->name.c_str(), "bigFont.fnt");
    subTitle->setColor(m_pCell->m_pBind->modifier ? cc3x(0x8fa) : cc3x(0x3af));
    subTitle->setPosition(
        winSize.width / 2,
        winSize.height / 2 + this->m_pLrSize.height / 2 - 50.0f
    );
    subTitle->limitLabelWidth(this->m_pLrSize.width - 40.0f, .5f, .1f);
    this->m_mainLayer->addChild(subTitle);

    m_pPreLabel = CCLabelBMFont::create("Press Keys...", "bigFont.fnt");
    m_pPreLabel->limitLabelWidth(this->m_pLrSize.width - 40.0f, .6f, .2f);
    m_pPreLabel->setOpacity(80);
    m_pPreLabel->setPosition(winSize / 2);
    this->m_mainLayer->addChild(m_pPreLabel);

    m_pTypeLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_pTypeLabel->setPosition(winSize / 2);
    m_pTypeLabel->setVisible(false);
    this->m_mainLayer->addChild(m_pTypeLabel);

    m_pInfoLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_pInfoLabel->setPosition(
        winSize.width / 2,
        winSize.height / 2 - this->m_pLrSize.height / 2 + 55.0f
    );
    m_pInfoLabel->setColor({ 200, 255, 70 });
    m_pInfoLabel->setScale(.5f);
    m_pInfoLabel->setVisible(false);
    this->m_mainLayer->addChild(m_pInfoLabel);

    auto setBtn = CCMenuItemSpriteExtra::create(
        CCNodeConstructor<ButtonSprite*>()
            .fromButtonSprite(
                m_pStoreItem ? "Set" : "Add", "GJ_button_01.png", "bigFont.fnt"
            )
            .scale(.6f)
            .done(),
        this,
        menu_selector(KeybindEditPopup::onSet)
    );
    this->m_buttonMenu->addChild(setBtn);

    CCMenuItemSpriteExtra* removeBtn = nullptr;
    if (m_pStoreItem) {
        removeBtn = CCMenuItemSpriteExtra::create(
            CCNodeConstructor<ButtonSprite*>()
                .fromButtonSprite(
                    "Remove", "GJ_button_06.png", "bigFont.fnt"
                )
                .scale(.6f)
                .done(),
            this,
            menu_selector(KeybindEditPopup::onRemove)
        );
        this->m_buttonMenu->addChild(removeBtn);
    }

    this->m_buttonMenu->alignItemsHorizontallyWithPadding(5.0f);

    setBtn->setPositionY(-this->m_pLrSize.height / 2 + 30.0f);
    if (removeBtn) removeBtn->setPositionY(-this->m_pLrSize.height / 2 + 30.0f);

    // if (this->m_pCell->m_pBind->repeatable) {
    //     auto repeatLabel = CCLabelBMFont::create("Repeat", "bigFont.fnt");
    //     repeatLabel->setPosition(
    //         winSize.width / 2 + this->m_pLrSize.width / 2 - 30.0f,
    //         winSize.height / 2 - this->m_pLrSize.height / 2 + 35.0f
    //     );
    //     repeatLabel->setScale(.3f);
    //     this->m_mainLayer->addChild(repeatLabel);

    //     auto toggle = CCMenuItemToggler::createWithStandardSprites(
    //         this, (SEL_MenuHandler)&KeybindEditPopup::onRepeat, .5f
    //     );
    //     toggle->setPosition(
    //         this->m_pLrSize.width / 2 - 30.0f,
    //         - this->m_pLrSize.height / 2 + 20.0f
    //     );
    //     toggle->toggle(this->m_pCell->m_pBind->repeat);
    //     this->m_buttonMenu->addChild(toggle);
    // }
}

void KeybindEditPopup::onRepeat(CCObject* pSender) {
    auto t = as<CCMenuItemToggler*>(pSender);

    this->m_pCell->m_pBind->repeat = !t->isToggled();
}

void KeybindEditPopup::updateLabel() {
    m_pPreLabel->setVisible(false);
    m_pTypeLabel->setVisible(true);

    m_pTypeLabel->setString(m_obTypedBind.toString().c_str());
    m_pTypeLabel->limitLabelWidth(this->m_pLrSize.width - 40.0f, .8f, .2f);

    auto binds = KeybindManager::get()->getCallbacksForKeybind(
        this->m_pCell->m_pItem->type,
        this->m_obTypedBind
    );

    if (binds.size()) {
        m_pInfoLabel->setVisible(true);

        std::string bs = "Also assigned to ";
        for (auto const& bind : binds)
            bs += bind->name + ", ";
        bs = bs.substr(0, bs.size() - 2);
        bs += "!";

        m_pInfoLabel->setString(bs.c_str());
        m_pInfoLabel->limitLabelWidth(this->m_pLrSize.width - 40.0f, .4f, .1f);
    } else {
        m_pInfoLabel->setVisible(false);
    }
}

void KeybindEditPopup::keyDown(enumKeyCodes key) {
    this->m_obTypedBind = Keybind(key);
    this->updateLabel();
}

bool KeybindEditPopup::keyDownSuper(enumKeyCodes key) {
    if (this->m_pCell->m_pBind->modifier)
        this->keyDown(key);
    return false;
}

void KeybindEditPopup::onRemove(CCObject*) {
    KeybindManager::get()->removeKeybind(
        this->m_pCell->m_pItem->type,
        this->m_pCell->m_pBind,
        this->m_pStoreItem->m_obBind
    );

    this->onClose(nullptr);
}

void KeybindEditPopup::onClose(CCObject*) {
    m_pCell->updateMenu();

    BrownAlertDelegate::onClose(nullptr);
}

void KeybindEditPopup::onSet(CCObject*) {
    if (this->m_pCell->m_pBind->modifier) {
        if (
            !this->m_obTypedBind.modifiers &&
            this->m_obTypedBind.key == KEY_None &&
            this->m_obTypedBind.mouse == kMouseButtonNone
        ) return;
    } else if (
        this->m_obTypedBind.key == KEY_None &&
        this->m_obTypedBind.mouse == kMouseButtonNone
    ) return;

    if (this->m_pStoreItem)
        KeybindManager::get()->editKeybind(
            this->m_pCell->m_pItem->type,
            this->m_pCell->m_pBind,
            this->m_pStoreItem->m_obBind,
            this->m_obTypedBind
        );
    else
        KeybindManager::get()->addKeybind(
            this->m_pCell->m_pItem->type,
            this->m_pCell->m_pBind,
            this->m_obTypedBind
        );

    this->onClose(nullptr);
}

bool KeybindEditPopup::mouseDownSuper(MouseButton btn, CCPoint const& pos) {
    if (!KeybindManager::get()->isAllowedMouseButton(btn))
        return false;
    
    this->m_obTypedBind = Keybind(btn);
    this->updateLabel();

    return true;
}

KeybindEditPopup* KeybindEditPopup::create(KeybindCell* cell, KeybindStoreItem* item) {
    auto ret = new KeybindEditPopup;

    if (
        ret &&
        ((ret->m_pCell = cell) || true) &&
        ((ret->m_pStoreItem = item) || true) &&
        ret->init(220.0f, 160.0f, "GJ_square01.png", item ? "Edit Keybind" : "Add Keybind")
    ) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}
