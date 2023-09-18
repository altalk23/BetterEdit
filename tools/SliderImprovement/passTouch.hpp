#pragma once

#include <GDMake.h>

inline cocos2d::CCSprite* createLockSprite(const char* bg, const char* spr) {
    auto lockSpr = cocos2d::CCSprite::create(bg);

    auto lockSpr_lock = cocos2d::CCSprite::createWithSpriteFrameName(spr);
    lockSpr_lock->setPosition(lockSpr->getContentSize() / 2);

    lockSpr->addChild(lockSpr_lock);
    lockSpr->setScale(.45f);
    lockSpr->setContentSize(lockSpr->getScaledContentSize());

    return lockSpr;
}

bool pointIntersectsScaleControls(EditorUI*, cocos2d::CCTouch*, cocos2d::CCEvent*);
bool pointIntersectsRotateControls(EditorUI*, cocos2d::CCTouch*, cocos2d::CCEvent*);

inline bool pointIntersectsControls(EditorUI* ui, cocos2d::CCTouch* touch, cocos2d::CCEvent* event) {
    if (pointIntersectsScaleControls(ui, touch, event))
        return true;
    if (pointIntersectsRotateControls(ui, touch, event))
        return true;

    return false;
}

inline bool g_bHoldingDownTouch = false;
