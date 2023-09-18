#pragma once

#include "../../BetterEdit.hpp"
#include <BrownAlertDelegate.hpp>
#include <InputNode.hpp>

class ColorTriggerPopup : public BrownAlertDelegate {
    protected:
        InputNode* m_pRangeStartInput;
        InputNode* m_pRangeEndInput;
        EditorPauseLayer* m_pPauseLayer;

        void setup() override;

        void onCreate(cocos2d::CCObject*);
        void onInfo(cocos2d::CCObject*);

    public:
        static ColorTriggerPopup* create(EditorPauseLayer* = nullptr);
};
