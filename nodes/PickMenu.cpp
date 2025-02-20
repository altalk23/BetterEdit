#include "PickMenu.hpp"

using namespace gdmake;
using namespace gdmake::extra;

void PickMenu::setup() {
    auto menu = cocos2d::CCMenu::create();
    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    size_t ix = 0;
    for (auto const& [key, value] : this->m_mPicks) {
        auto but = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(
                key.c_str(),
                static_cast<int>(this->m_pLrSize.width - 40.0f), true,
                "goldFont.fnt", "GJ_button_01.png",
                0, .8f
            ),
            this,
            (cocos2d::SEL_MenuHandler)&PickMenu::onSelect
        );

        but->setUserData(as<void*>(ix++));

        menu->addChild(but);
    }
    menu->alignItemsVerticallyWithPadding(5.0f);
    menu->setPosition(.0f, .0f);

    this->m_buttonMenu->addChild(menu);
    
    this->registerWithTouchDispatcher();
    cocos2d::CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);
}

void PickMenu::onSelect(cocos2d::CCObject* pSender) {
    auto index = as<size_t>(as<CCMenuItemSpriteExtra*>(pSender)->getUserData());

    auto cb = this->m_mPicks.at(index).second;

    (this->m_pThis->*cb)(pSender);

    this->onClose(nullptr);
}

bool PickMenu::initPicks(const char* _title, cocos2d::CCObject* _obj, PickMenu::picks_t const& _picks) {
    this->m_mPicks = _picks;
    this->m_pThis = _obj;

    return PickMenu::init(250.0f, this->m_mPicks.size() * 40.0f + 20.0f, "GJ_square01.png", _title);
}

PickMenu* PickMenu::create(const char* _title, cocos2d::CCObject* _obj, PickMenu::picks_t const& _picks) {
    auto pRet = new PickMenu();

    if (pRet && pRet->initPicks(_title, _obj, _picks)) {
        pRet->autorelease();
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}
