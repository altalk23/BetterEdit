#pragma once

#include "../BetterEdit.hpp"

class EditorPauseLayer_CB : public EditorPauseLayer {
    public:
        void onBESettings(cocos2d::CCObject* pSender);
        void onRotateSaws(CCObject* pSender);
        void onShowKeybinds(CCObject* pSender);
        void onCustomizeUI(CCObject*);
};

void setupRotateSaws();
int countLDMObjects(LevelEditorLayer*);
