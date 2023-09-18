#include "../../BetterEdit.hpp"
#include "PasteLayer.hpp"

using namespace gdmake;

void  EditorUI_onPasteState(EditorUI* self,  cocos2d::CCObject* pSender) {
    if (BetterEdit::sharedState()->getPasteStateEnabled())
        PasteLayer::create()->show();
    else
        matdash::orig<&EditorUI_onPasteState>(self,  pSender);
} MAT_GDMAKE_HOOK(0x884c0, EditorUI_onPasteState);

void  GameObject_duplicateAttributes(GameObject* dest,  GameObject *src) {
    if (!PasteLayer::wantsToPasteState())
        return matdash::orig<&GameObject_duplicateAttributes>(dest,  src);

    auto states = PasteLayer::getStates();

    dest->m_bGroupDisabled = false;
    dest->m_unk414 = 0;

    if (states->count(PasteLayer::ObjID))           dest->m_nObjectID = src->m_nObjectID;
    if (states->count(PasteLayer::EditorLayer))     dest->m_nEditorLayer = src->m_nEditorLayer;
    if (states->count(PasteLayer::EditorLayer2))    dest->m_nEditorLayer2 = src->m_nEditorLayer2;
    if (states->count(PasteLayer::DontFade))        dest->m_bIsDontFade = src->m_bIsDontFade;
    if (states->count(PasteLayer::DontEnter))       dest->m_bIsDontEnter = src->m_bIsDontEnter;
    if (states->count(PasteLayer::PositionX))       dest->setPositionX(src->getPositionX());
    if (states->count(PasteLayer::PositionY))       dest->setPositionY(src->getPositionY());
    if (states->count(PasteLayer::Rotation))        dest->setRotation(src->getRotation());
    if (states->count(PasteLayer::Scale))           dest->updateCustomScale(src->getScale());
    if (states->count(PasteLayer::ZOrder)) {
        auto z = src->m_nGameZOrder;
        if (!z) z = src->m_nDefaultZOrder;
        dest->m_nGameZOrder = z;
    }
    if (states->count(PasteLayer::ZLayer)) {
        auto z = src->m_nZLayer;
        if (!z) z = src->m_nDefaultZLayer;
        dest->m_nZLayer = z;
    }
    if (states->count(PasteLayer::Groups)) {
        dest->m_nGroupCount = 0;
        if (src->m_nGroupCount && src->m_pGroups)
            for (auto ix = 0; ix < src->m_nGroupCount && ix < 10; ix++) 
                dest->addToGroup(src->m_pGroups[ix]);
    }

    LevelEditorLayer::get()->getEditorUI()->updateSpecialUIElements();
    LevelEditorLayer::get()->getEditorUI()->updateObjectInfoLabel();
} MAT_GDMAKE_HOOK(0xef6b0, GameObject_duplicateAttributes);

void  LevelEditorLayer_copyObjectState(LevelEditorLayer* self,  GameObject* obj) {
    matdash::orig<&LevelEditorLayer_copyObjectState>(self,  obj);

    if (obj != nullptr) {
        self->m_pCopyStateObject->m_nObjectID = obj->m_nObjectID;
        self->m_pCopyStateObject->setPosition(obj->getPosition());
        self->m_pCopyStateObject->setRotation(obj->getRotation());
        self->m_pCopyStateObject->updateCustomScale(obj->getScale());
    }
} MAT_GDMAKE_HOOK(0x16b600, LevelEditorLayer_copyObjectState);
