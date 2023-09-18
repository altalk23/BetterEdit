#pragma once

#include "../../BetterEdit.hpp"

class KeybindingsLayer_CB : public KeybindingsLayer {
    public:
        void reloadList();
        void detachInput();
        void onResetAll(CCObject*);
        void onGlobalSettings(CCObject*);
        void onKeymap(CCObject*);
        void onFinishSelect(CCObject*);
        void setSelectMode(bool, Keybind const&);
};

class KeybindingsLayerDelegate :
    public CCNode,
    public TextInputDelegate,
    public FLAlertLayerProtocol
{
    public:
        KeybindingsLayer_CB* m_mainLayer;
        static std::map<std::string, bool> m_mFoldedCategories;

        void textChanged(CCTextInputNode* input) override;
        void FLAlert_Clicked(FLAlertLayer*, bool btn2) override;
        static KeybindingsLayerDelegate* create(KeybindingsLayer_CB* layer);
};
