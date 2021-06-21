#pragma once

#include <string>
#include <GDMake.h>

template<class T = cocos2d::CCSprite*>
class CCNodeConstructor {
    protected:
        T node;

    public:
        inline CCNodeConstructor<cocos2d::CCSprite*> & fromFrameName(const char* fname) {
            this->node = cocos2d::CCSprite::createWithSpriteFrameName(fname);
            return *this;
        }
        inline CCNodeConstructor<cocos2d::CCSprite*> & fromFile(const char* fname) {
            this->node = cocos2d::CCSprite::create(fname);
            return *this;
        }
        inline CCNodeConstructor<cocos2d::CCLabelBMFont*> & fromText(const char* text, const char* font) {
            this->node = cocos2d::CCLabelBMFont::create(text, font);
            return *this;
        }
        inline CCNodeConstructor<cocos2d::CCMenu*> & fromMenu() {
            this->node = cocos2d::CCMenu::create();
            return *this;
        }
        inline CCNodeConstructor<T> & fromNode(T node) {
            this->node = node;
            return *this;
        }
        inline CCNodeConstructor<T> & flipX(bool flip = true) {
            node->setFlipX(flip);
            return *this;
        }
        inline CCNodeConstructor<T> & flipY(bool flip = true) {
            node->setFlipY(flip);
            return *this;
        }
        inline CCNodeConstructor<T> & rotate(float rot) {
            node->setRotation(rot);
            return *this;
        }
        inline CCNodeConstructor<T> & scale(float scale) {
            node->setScale(scale);
            return *this;
        }
        inline CCNodeConstructor<T> & move(float x, float y) {
            node->setPosition(x, y);
            return *this;
        }
        inline CCNodeConstructor<T> & move(cocos2d::CCPoint const& pos) {
            node->setPosition(pos);
            return *this;
        }
        inline CCNodeConstructor<T> & color(cocos2d::ccColor3B color) {
            node->setColor(color);
            return *this;
        }
        inline CCNodeConstructor<T> & add(cocos2d::CCNode* anode) {
            node->addChild(anode);
            return *this;
        }
        inline CCNodeConstructor<T> & add(std::function<cocos2d::CCNode*(T)> func) {
            node->addChild(func(node));
            return *this;
        }
        inline CCNodeConstructor<T> & exec(std::function<void(T)> func) {
            func(node);
            return *this;
        }
        inline CCNodeConstructor<T> & uobj(cocos2d::CCObject* obj) {
            node->setUserObject(obj);
            return *this;
        }
        inline CCNodeConstructor<T> & save(T* another) {
            *another = node;
            return *this;
        }
        inline T done() {
            return node;
        }
};

inline int strToInt(const char* str, bool* is = nullptr) {
    bool isStr = true;
    for (auto i = 0u; i < strlen(str); i++)
        if (!isdigit(str[i]) && str[i] != '-')
            isStr = false;
        
    if (is) *is = isStr;

    return isStr ? std::atoi(str) : -1;
}
