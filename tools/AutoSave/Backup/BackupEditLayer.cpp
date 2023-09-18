#include "BackupEditLayer.hpp"

CCSprite* BackupEditLayer::createBtnSprite(const char* spr, const char* text, ccColor3B color) {
    auto parent = CCSprite::create();

    parent->setContentSize({ 80.0f, 60.0f });

    auto bg = CCScale9Sprite::create(
        "square02c_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
    );

    bg->setContentSize({ 160.0f, 120.0f });
    bg->setPosition(
        parent->getContentSize() / 2
    );
    bg->setOpacity(30);
    bg->setScale(.5f);

    parent->addChild(bg);

    auto pSpr = createBESprite(spr, spr);
    if (pSpr) {
        pSpr->setPosition({
            parent->getContentSize().width / 2,
            40.0f
        });
        parent->addChild(pSpr);
    }

    auto label = CCLabelBMFont::create(text, "bigFont.fnt", 160.0f, kCCTextAlignmentCenter);
    label->setColor(color);
    label->setScale(.3f);
    label->setPosition(
        parent->getContentSize().width / 2,
        15.0f
    );
    parent->addChild(label);

    return parent;
}

void BackupEditLayer::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->m_pNameInput = InputNode::create(220.0f, "Backup Name", "bigFont.fnt", inputf_Default, 50);
    
    this->m_pNameInput->setPosition(winSize.width / 2, winSize.height / 2 + 25.0f);
    this->m_pNameInput->setString(this->m_pBackup->name.c_str());
    this->m_pNameInput->getInputNode()->setDelegate(this);

    this->m_mainLayer->addChild(this->m_pNameInput);

    auto deleteBtn = CCMenuItemSpriteExtra::create(
        createBtnSprite("edit_delBtn_001.png", "Delete\nBackup", cc3x(0xfa3)), //  { 255, 115, 95 }
        this,
        menu_selector(BackupEditLayer::onDelete)
    );
    this->m_buttonMenu->addChild(deleteBtn);

    auto viewBtn = CCMenuItemSpriteExtra::create(
        createBtnSprite("BE_eye-on.png", "View\nBackup"), // { 255, 255, 80 }
        this,
        menu_selector(BackupEditLayer::onView)
    );
    this->m_buttonMenu->addChild(viewBtn);

    auto loadBtn = CCMenuItemSpriteExtra::create(
        createBtnSprite("GJ_downloadsIcon_001.png", "Load\nBackup"), // { 80, 255, 95 }
        this,
        menu_selector(BackupEditLayer::onApply)
    );
    this->m_buttonMenu->addChild(loadBtn);

    this->m_buttonMenu->alignItemsHorizontallyWithPadding(10.0f);

    deleteBtn->setPositionY(-35.0f);
    loadBtn->setPositionY(-35.0f);
    viewBtn->setPositionY(-35.0f);

    this->m_buttonMenu->addChild(
        CCNodeConstructor<CCMenuItemSpriteExtra*>()
            .fromNode(CCMenuItemSpriteExtra::create(
                CCNodeConstructor()
                    .fromFrameName("GJ_resetBtn_001.png")
                    .scale(.8f)
                    .done(),
                this,
                (SEL_MenuHandler)&BackupEditLayer::onResetName
            ))
            .move(this->m_pLrSize.width / 2 - 25.0f, 25.0f)
            .done()
    );
}

void BackupEditLayer::onDelete(CCObject*) {
    LevelBackupManager::get()->removeBackupForLevel(
        this->m_pBackupLayer->m_pLevel, this->m_pBackup
    );

    this->m_pBackupLayer->reloadList();
    this->onClose(nullptr);
}

void BackupEditLayer::onView(CCObject*) {
    auto level = GJGameLevel::create();

    level->setLevelData(this->m_pBackup->data);

    BetterEdit::setEditorViewOnlyMode(true);

    auto scene = CCScene::create();

    auto lel = LevelEditorLayer::create(level);
    lel->m_nCurrentLayer = -1;
    scene->addChild(lel);
    CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(
        0.5f, scene
    ));
}

void BackupEditLayer::onApply(CCObject*) {
    FLAlertLayer::create(
        this,
        "Confirm Apply",
        "Apply", "Backup & Apply",
        "Would you like to <co>back up</c> your current progress "
        "before <cl>applying</c>? (<cr>Otherwise progress will be "
        "lost</c>)"
    )->show();
}

void BackupEditLayer::applyBackup() {
    this->m_pBackupLayer->m_pLevel->setLevelData(this->m_pBackup->data);

    auto alert = FLAlertLayer::create(
        nullptr,
        "Backup Applied",
        "OK", nullptr,
        "\n\n"
    );

    auto checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
    checkmark->setScale(1.1f);
    checkmark->setPosition(
        CCDirector::sharedDirector()->getWinSize() / 2
    );
    alert->m_mainLayer->addChild(checkmark, 10);
    alert->show();

    this->m_pBackupLayer->reloadList();

    this->onClose(nullptr);
}

void BackupEditLayer::FLAlert_Clicked(FLAlertLayer*, bool btn2) {
    if (btn2) {
        LevelBackupManager::get()->createBackupForLevel(
            this->m_pBackupLayer->m_pLevel
        );
    }

    this->applyBackup();
}

void BackupEditLayer::onResetName(CCObject*) {
    this->m_pNameInput->setString("");
    this->m_pNameInput->getInputNode()->m_delegate->textChanged(
        this->m_pNameInput->getInputNode()
    );
}

void BackupEditLayer::textChanged(CCTextInputNode* input) {
    this->m_pBackup->name = input->getString();
    this->m_pCell->updateTitle(input->getString());
}

BackupEditLayer* BackupEditLayer::create(
    BackupViewLayer* layer,
    BackupCell* cell,
    LevelBackup* backup
) {
    auto pRet = new BackupEditLayer;

    if (
        pRet &&
        (pRet->m_pBackup = backup) &&
        (pRet->m_pCell = cell) &&
        (pRet->m_pBackupLayer = layer) &&
        pRet->init(300.0f, 190.0f, "GJ_square01.png", "Edit Backup")
    ) {
        pRet->autorelease();
        return pRet;
    }

    CC_SAFE_DELETE(pRet);
    return nullptr;
}
