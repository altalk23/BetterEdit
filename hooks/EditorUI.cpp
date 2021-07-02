#include <GDMake.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include "../utils.hpp"
#include "passTouch.hpp"
#include "../tools/GroupIDFilter/groupfilter.hpp"
#include "../tools/GridSize/gridButton.hpp"
#include "../BetterEdit.hpp"
#include <thread>

using namespace gdmake;

#define CATCH_NULL(x) if (x) x

char* g_clipboard;

void showErrorMessages() {
    std::cout << "thread!\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::cout << "showy\n";

    for (auto errMsg : BetterEdit::sharedState()->getErrors())
        gd::FLAlertLayer::create(
            nullptr,
            "Error",
            "OK", nullptr,
            errMsg
        )->show();

    std::cout << "bye\n";
    
    std::terminate();
}

class EUITextDelegate : public cocos2d::CCNode, public gd::TextInputDelegate {
    public:
        gd::EditorUI* m_pEditorUI;

        virtual void textChanged(gd::CCTextInputNode* input) override {
            if (input->getString() && strlen(input->getString()))
                this->m_pEditorUI->m_pEditorLayer->setCurrentLayer(strToInt(input->getString()));
            else
                this->m_pEditorUI->m_pEditorLayer->setCurrentLayer(-1);
        }

        static EUITextDelegate* create(gd::EditorUI* ui) {
            auto ret = new EUITextDelegate();

            if (ret && ret->init()) {
                ret->m_pEditorUI = ui;
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
            return nullptr;
        }
};

GDMAKE_HOOK(0x886b0)
void __fastcall EditorUI_onGoToLayer(gd::EditorUI* self, edx_t edx, cocos2d::CCObject* pSender) {
    GDMAKE_ORIG_V(self, edx, pSender);

    auto i = self->getChildByTag(6978);

    self->m_pCurrentLayerLabel->setVisible(false);

    if (i)
        reinterpret_cast<gd::CCTextInputNode*>(i)->setString(
            self->m_pCurrentLayerLabel->getString()
        );
}

GDMAKE_HOOK(0x907b0)
bool __fastcall EditorUI_ccTouchBegan(gd::EditorUI* self, edx_t edx, cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    auto self_ = reinterpret_cast<gd::EditorUI*>(reinterpret_cast<uintptr_t>(self) - 0xEC);
    auto i = self_->getChildByTag(6978);

    g_bHoldingDownTouch = true;
    
    if (i) {
        auto inp = reinterpret_cast<gd::CCTextInputNode*>(i);
        auto isize = inp->getScaledContentSize();

        auto rect = cocos2d::CCRect {
            inp->getPositionX() - isize.width / 2,
            inp->getPositionY() - isize.height / 2,
            isize.width,
            isize.height
        };

        if (!rect.containsPoint(touch->getLocation()))
            reinterpret_cast<gd::CCTextInputNode*>(i)->getTextField()->detachWithIME();
        else
            return true;
    }
    
    if (pointIntersectsControls(self_, touch, event))
        return true;
    
    if (BetterEdit::sharedState()->m_bHookConflictFound)
        BetterEdit::showHookConflictMessage();

    return GDMAKE_ORIG(self, edx, touch, event);
}

GDMAKE_HOOK(0x911a0)
void __fastcall EditorUI_ccTouchEnded(gd::EditorUI* self, edx_t edx, cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    g_bHoldingDownTouch = false;

    GDMAKE_ORIG_V(self, edx, touch, event);
}

GDMAKE_HOOK(0x8d7e0)
void __fastcall EditorUI_onGroupDown(gd::EditorUI* self, edx_t edx, cocos2d::CCObject* pSender) {
    GDMAKE_ORIG(self, edx, pSender);

    auto i = self->getChildByTag(6978);

    self->m_pCurrentLayerLabel->setVisible(false);

    if (i)
        reinterpret_cast<gd::CCTextInputNode*>(i)->setString(
            self->m_pCurrentLayerLabel->getString()
        );
}

GDMAKE_HOOK(0x8d780)
void __fastcall EditorUI_onGroupUp(gd::EditorUI* self, edx_t edx, cocos2d::CCObject* pSender) {
    GDMAKE_ORIG(self, edx, pSender);

    auto i = self->getChildByTag(6978);

    self->m_pCurrentLayerLabel->setVisible(false);

    if (i)
        reinterpret_cast<gd::CCTextInputNode*>(i)->setString(
            self->m_pCurrentLayerLabel->getString()
        );
}

GDMAKE_HOOK(0x88790)
void __fastcall EditorUI_onGoToBaseLayer(gd::EditorUI* self, edx_t edx, cocos2d::CCObject* pSender) {
    GDMAKE_ORIG(self, edx, pSender);

    auto i = self->getChildByTag(6978);

    self->m_pCurrentLayerLabel->setVisible(false);

    if (i)
        reinterpret_cast<gd::CCTextInputNode*>(i)->setString(
            self->m_pCurrentLayerLabel->getString()
        );
}

GDMAKE_HOOK(0x76090)
void __fastcall EditorUI_destructorHook(gd::EditorUI* self) {
    auto addr = reinterpret_cast<uintptr_t>(self) + 0x2D0;
    auto str_len = *reinterpret_cast<size_t*>(addr + 16);
    if (str_len) {
        char* str_buf;
        if (str_len < 16) {
            // string is small enough to be directly here
            str_buf = reinterpret_cast<char*>(addr);
        } else {
            str_buf = *reinterpret_cast<char**>(addr);
        }
        g_clipboard = reinterpret_cast<char*>(realloc(g_clipboard, str_len + 1));
        memcpy(g_clipboard, str_buf, str_len + 1);
    }

    return GDMAKE_ORIG_V(self);
}

GDMAKE_HOOK(EditorUI::init)
bool __fastcall EditorUI_init(gd::EditorUI* self, edx_t edx, gd::GJGameLevel* lvl) {
    if (!GDMAKE_ORIG(self, edx, lvl))
        return false;

    auto ed = EUITextDelegate::create(self);

    self->m_pCurrentLayerLabel->setVisible(false);

    auto spr = cocos2d::extension::CCScale9Sprite::create(
        "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );

    spr->setScale(.3f);
    spr->setColor({ 0, 0, 0 });
    spr->setOpacity(100);
    spr->setContentSize({ 115.0f, 75.0f });
    spr->setTag(6977);
    spr->setPosition(self->m_pCurrentLayerLabel->getPosition());

    auto eLayerInput = gd::CCTextInputNode::create("All", ed, "bigFont.fnt", 40.0f, 30.0f);

    eLayerInput->setPosition(self->m_pCurrentLayerLabel->getPosition());
    eLayerInput->setLabelPlaceholderColor({ 120, 120, 120 });
    eLayerInput->setAllowedChars("0123456789");
    eLayerInput->setAnchorPoint({ 0, 0 });
    eLayerInput->setScale(.7f);
    eLayerInput->setDelegate(ed);
    eLayerInput->setTag(6978);
    eLayerInput->setString(self->m_pCurrentLayerLabel->getString());

    self->addChild(spr);
    self->addChild(eLayerInput);

    self->addChild(ed);

    setupGroupFilterButton(self);
    loadGridButtons(self);

    if (g_clipboard && g_clipboard[0]) {
        auto clipboard = reinterpret_cast<uintptr_t>(self) + 0x2D0;
        auto len = strlen(g_clipboard);
        *reinterpret_cast<size_t*>(clipboard + 16) = len; // size
        *reinterpret_cast<size_t*>(clipboard + 20) = max(len, 15); // capacity
        if (len <= 15) {
            memcpy(reinterpret_cast<char*>(clipboard), g_clipboard, len + 1);
        } else {
            void* newb = malloc(len + 1);
            memcpy(newb, g_clipboard, len + 1);
            *reinterpret_cast<void**>(clipboard) = newb;
        }
        self->updateButtons();
    }

    BetterEdit::sharedState()->m_bHookConflictFound = false;

    std::thread(showErrorMessages);

    return true;
}

GDMAKE_HOOK(0x87180)
void __fastcall EditorUI_showUI(gd::EditorUI* self, edx_t edx, bool show) {
    GDMAKE_ORIG_V(self, edx, show);

    self->m_pCurrentLayerLabel->setVisible(false);

    CATCH_NULL(self->getChildByTag(6978))->setVisible(show);
    CATCH_NULL(self->getChildByTag(6977))->setVisible(show);
    CATCH_NULL(self->m_pCopyBtn->getParent()->getChildByTag(7777))->setVisible(show);
}
