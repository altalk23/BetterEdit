#include "GroupSummaryPopup.hpp"
#include "MoreTriggersPopup.hpp"
#include "GroupListView.hpp"
#include <InputPrompt.hpp>
#include "../../utils/moveGameLayer.hpp"

static constexpr const cocos2d::ccColor3B listBGLight { 142, 68, 28 };
static constexpr const cocos2d::ccColor3B listBGDark  { 114, 55, 22 };

ButtonSprite* GroupSummaryPopup::createFilterSpr(const char* sprName, const char* bg) {
    auto spr = ButtonSprite::create(
        createBESprite(sprName),
        20, 1, 1.f, 0, bg, true, 0x20 
    );
    spr->m_pSubSprite->setScale(1.0f);
    spr->setScale(.55f);
    return spr;
}

CCSprite* GroupSummaryPopup::createSpriteForTrigger(EffectGameObject* trigger, int group) {
    auto sprName = ObjectToolbox::sharedState()->intKeyToFrame(trigger->m_nObjectID);
    auto spr = CCSprite::createWithSpriteFrameName(sprName);
    spr->setScale(.65f);

    if (trigger->m_nObjectID == 1049) {
        auto toggleSpr = CCSprite::createWithSpriteFrameName("edit_eToggleBtn2_001.png");
        toggleSpr->setPosition({
            spr->getContentSize().width / 2,
            spr->getContentSize().height / 2
        });
        if (trigger->m_bActivateGroup) {
            toggleSpr->setColor({ 0, 255, 127 });
        } else {
            toggleSpr->setColor({ 255, 63, 63 });
        }
        spr->addChild(toggleSpr);
    }

    std::string text = "";
    if (trigger->m_nTargetGroupID == group) {
        text = "T";
    }
    if (trigger->m_nCenterGroupID == group) {
        if (text.size()) {
            text = "T+C";
        } else {
            text = "C";
        }
    }
    
    auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
    label->setScale(.5f);
    label->setPosition(
        spr->getContentSize().width / 2,
        spr->getContentSize().height / 2 - 5.f
    );
    spr->addChild(label);

    return spr;
}

void GroupSummaryPopup::setup() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->m_bNoElasticity = true;

    {
        auto topItem = CCSprite::createWithSpriteFrameName("GJ_commentTop_001.png");
        topItem->setPosition({
            winSize.width / 2,
            winSize.height / 2 + 85.0f
        });
        topItem->setZOrder(500);
        this->m_mainLayer->addChild(topItem);

        auto bottomItem = CCSprite::createWithSpriteFrameName("GJ_commentTop_001.png");
        bottomItem->setPosition({
            winSize.width / 2,
            winSize.height / 2 - 115.0f
        });
        bottomItem->setZOrder(500);
        bottomItem->setFlipY(true);
        this->m_mainLayer->addChild(bottomItem);

        auto sideItem = CCSprite::createWithSpriteFrameName("GJ_commentSide_001.png");
        sideItem->setPosition({
            winSize.width / 2 - 173.5f,
            winSize.height / 2 - 14.0f
        });
        sideItem->setZOrder(500);
        sideItem->setScaleY(6.f);
        this->m_mainLayer->addChild(sideItem);

        auto sideItemRight = CCSprite::createWithSpriteFrameName("GJ_commentSide_001.png");
        sideItemRight->setPosition({
            winSize.width / 2 + 173.5f,
            winSize.height / 2 - 14.0f
        });
        sideItemRight->setZOrder(500);
        sideItemRight->setScaleY(6.f);
        sideItemRight->setFlipX(true);
        this->m_mainLayer->addChild(sideItemRight);
    }

    {
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemSpriteExtra*>()
            .fromNode(CCMenuItemSpriteExtra::create(
                CCNodeConstructor()
                    .fromFrameName("GJ_arrow_01_001.png")
                    .scale(.8f)
                    .done(),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onPage
            ))
            .udata(-1)
            .move(- m_fItemWidth / 2 - 25.0f, 0.0f)
            .done()
        );
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemSpriteExtra*>()
            .fromNode(CCMenuItemSpriteExtra::create(
                CCNodeConstructor()
                    .fromFrameName("GJ_arrow_01_001.png")
                    .scale(.8f)
                    .flipX()
                    .done(),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onPage
            ))
            .udata(1)
            .move(m_fItemWidth / 2 + 25.0f, 0.0f)
            .done()
        );
    }

    {
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemToggler*>()
            .fromNode(CCMenuItemToggler::create(
                this->createFilterSpr("BE_all.png", "GJ_button_01.png"),
                this->createFilterSpr("BE_all.png", "GJ_button_02.png"),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onShow
            ))
            .tag(kShowAll)
            .move(- this->m_pLrSize.width / 2 + 40.f, this->m_pLrSize.height / 2 - 20.f)
            .exec([this](auto t) -> void { m_vShowBtns.push_back(t); })
            .done()
        );
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemToggler*>()
            .fromNode(CCMenuItemToggler::create(
                this->createFilterSpr("BE_0_objs.png", "GJ_button_01.png"),
                this->createFilterSpr("BE_0_objs.png", "GJ_button_02.png"),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onShow
            ))
            .tag(kShowOnlyOnesWithSomething)
            .move(- this->m_pLrSize.width / 2 + 65.f, this->m_pLrSize.height / 2 - 20.f)
            .exec([this](auto t) -> void { m_vShowBtns.push_back(t); })
            .done()
        );
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemToggler*>()
            .fromNode(CCMenuItemToggler::create(
                this->createFilterSpr("BE_trigger_only.png", "GJ_button_01.png"),
                this->createFilterSpr("BE_trigger_only.png", "GJ_button_02.png"),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onShow
            ))
            .tag(kShowOnesWithTriggersButNoObjects)
            .move(- this->m_pLrSize.width / 2 + 90.f, this->m_pLrSize.height / 2 - 20.f)
            .exec([this](auto t) -> void { m_vShowBtns.push_back(t); })
            .done()
        );
        this->m_buttonMenu->addChild(CCNodeConstructor<CCMenuItemToggler*>()
            .fromNode(CCMenuItemToggler::create(
                this->createFilterSpr("BE_obj_only.png", "GJ_button_01.png"),
                this->createFilterSpr("BE_obj_only.png", "GJ_button_02.png"),
                this,
                (SEL_MenuHandler)&GroupSummaryPopup::onShow
            ))
            .tag(kShowOnesWithObjectsButNoTriggers)
            .move(- this->m_pLrSize.width / 2 + 115.f, this->m_pLrSize.height / 2 - 20.f)
            .exec([this](auto t) -> void { m_vShowBtns.push_back(t); })
            .done()
        );

        this->m_pGroupCount = CCLabelBMFont::create("", "goldFont.fnt");
        this->m_pGroupCount->setScale(.3f);
        this->m_pGroupCount->setPosition(
            winSize.width / 2 - this->m_pLrSize.width / 2 + 75.f,
            winSize.height / 2 + this->m_pLrSize.height / 2 - 37.5f
        );
        this->m_mainLayer->addChild(this->m_pGroupCount);
    }

    this->m_pNoFilterInfo = CCLabelBMFont::create(
        "No groups match\nselected filters",
        "bigFont.fnt",
        500.f,
        kCCTextAlignmentCenter
    );
    this->m_pNoFilterInfo->setScale(.5f);
    this->m_pNoFilterInfo->setPosition(
        winSize.width / 2,
        winSize.height / 2 - 10.f
    );
    this->m_pNoFilterInfo->setVisible(false);
    this->m_mainLayer->addChild(this->m_pNoFilterInfo);

    this->m_pPageBtn = ButtonSprite::create(
        "Page n/nn", 0, 0, "goldFont.fnt", "GJ_button_05.png", 0, .5f
    );
    this->m_pPageBtn->setScale(.8f);

    auto pageBtn = CCMenuItemSpriteExtra::create(
        this->m_pPageBtn, this, menu_selector(GroupSummaryPopup::onGoToPage)
    );
    pageBtn->setPosition(
        this->m_pLrSize.width / 2 - 65.f,
        this->m_pLrSize.height / 2 - 25.f
    );
    this->m_buttonMenu->addChild(pageBtn);

    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);
    this->registerWithTouchDispatcher();

    this->updateGroups();
    this->updatePage();
}

void GroupSummaryPopup::addGroups(GameObject* obj) {
    for (auto i = 0; i < obj->m_nGroupCount; i++) {
        auto id = obj->m_pGroups[i];

        if (!m_mGroupInfo.count(id))
            m_mGroupInfo[id] = {};
        
        m_mGroupInfo[id].m_vObjects.push_back(obj);
    }
}

void GroupSummaryPopup::addGroups(EffectGameObject* obj) {
    this->addGroups(as<GameObject*>(obj));

    if (obj->m_nTargetGroupID) {
        if (!m_mGroupInfo.count(obj->m_nTargetGroupID))
            m_mGroupInfo[obj->m_nTargetGroupID] = {};
        m_mGroupInfo[obj->m_nTargetGroupID].m_vTriggers.push_back(obj);
    }

    if (obj->m_nCenterGroupID) {
        if (!m_mGroupInfo.count(obj->m_nCenterGroupID))
            m_mGroupInfo[obj->m_nCenterGroupID] = {};
        m_mGroupInfo[obj->m_nCenterGroupID].m_vTriggers.push_back(obj);
    }
}

void GroupSummaryPopup::updateGroups() {
    m_mGroupInfo = {};

    CCARRAY_FOREACH_B_TYPE(LevelEditorLayer::get()->getAllObjects(), obj, GameObject) {
        auto eobj = asEffectGameObject(obj);
        if (eobj) {
            this->addGroups(eobj);
        } else {
            this->addGroups(obj);
        }
    }

    this->updateFilters();
}

void GroupSummaryPopup::updateFilters(CCObject* noToggle) {
    this->m_vShowArray.clear();

    for (auto btn : this->m_vShowBtns) {
        if (btn != noToggle) {
            btn->toggle(this->m_eShow == static_cast<Show>(btn->getTag()));
        }
    }

    for (auto group = 1; group < 1000; group++) {
        auto sum = this->m_mGroupInfo[group];
        switch (this->m_eShow) {
            case kShowAll:
                this->m_vShowArray.push_back(group);
                break;
            
            case kShowOnlyOnesWithSomething:
                if (
                    sum.m_vObjects.size() ||
                    sum.m_vTriggers.size()
                )
                    this->m_vShowArray.push_back(group);
                break;
            
            case kShowOnesWithObjectsButNoTriggers:
                if (
                    sum.m_vObjects.size() &&
                    !sum.m_vTriggers.size()
                )
                    this->m_vShowArray.push_back(group);
                break;
            
            case kShowOnesWithTriggersButNoObjects:
                if (
                    !sum.m_vObjects.size() &&
                    sum.m_vTriggers.size()
                )
                    this->m_vShowArray.push_back(group);
                break;
        }
    }
}

void GroupSummaryPopup::updatePage() {
    for (auto const& node : m_vPageContent) {
        node->removeFromParent();
    }
    this->m_vPageContent.clear();

    int maxPage = this->m_vShowArray.size() / m_nItemCount;
    if (m_nPage > maxPage) {
        this->m_nPage = maxPage;
    }
    if (m_nPage < 0) {
        this->m_nPage = 0;
    }
    this->m_pNoFilterInfo->setVisible(!this->m_vShowArray.size());
    this->m_pGroupCount->setString(
        CCString::createWithFormat("Showing %d groups", this->m_vShowArray.size())->getCString()
    );
    this->m_pPageBtn->setString(
        CCString::createWithFormat("Page %d/%d", this->m_nPage + 1, maxPage + 1)->getCString()
    );

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    if (this->m_pList) {
        this->m_pList->removeFromParent();
    }

    auto arr = CCArray::create();

    for (auto ix = 0; ix < m_nItemCount; ix++) {
        auto i = ix + m_nPage * m_nItemCount;

        if (i >= static_cast<int>(this->m_vShowArray.size()))
            break;
        
        arr->addObject(CCInteger::create(this->m_vShowArray.at(i)));
    }

    this->m_pList = GroupListView::create(
        this, arr, 340.0f, 210.0f
    );
    this->m_pList->setPosition(
        winSize / 2 - CCPoint { 170.0f, 118.0f }
    );
    this->m_mainLayer->addChild(this->m_pList);
}

void GroupSummaryPopup::onPage(CCObject* pSender) {
    this->incrementPage(
        as<int>(as<CCNode*>(pSender)->getUserData())
    );
}

void GroupSummaryPopup::onGoToPage(CCObject*) {
    InputPrompt::create("Go to Page", "#", [this](const char* t) -> void {
        if (t && strlen(t)) {
            try {
                this->goToPage(std::stoi(t) - 1);
            } catch (...) {}
        }
    }, "Go")->setFilter(inputf_Numeral)->show();
}

void GroupSummaryPopup::incrementPage(int page) {
    this->m_nPage += page;

    if (this->m_nPage < 0)
        this->m_nPage = 0;
    if (this->m_nPage > 999 / m_nItemCount)
        this->m_nPage = 999 / m_nItemCount;

    this->updatePage();
}

void GroupSummaryPopup::goToPage(int page) {
    this->m_nPage = page;
    this->updatePage();
}

void GroupSummaryPopup::onShow(CCObject* pSender) {
    if (this->m_eShow == static_cast<Show>(pSender->getTag())) {
        as<CCMenuItemToggler*>(pSender)->toggle(false);
    } else {
        this->m_eShow = static_cast<Show>(pSender->getTag());
        this->updateFilters(pSender);
        this->updatePage();
    }
}

void GroupSummaryPopup::keyDown(enumKeyCodes key) {
    switch (key) {
        case KEY_Left:
            this->incrementPage(-1);
            break;

        case KEY_Right:
            this->incrementPage(1);
            break;

        default:
            return BrownAlertDelegate::keyDown(key);
    }
}

void GroupSummaryPopup::onViewTrigger(CCObject* pSender) {
    auto ui = this->m_pEditor->m_pEditorUI;
    auto obj = as<GameObject*>(as<CCNode*>(pSender)->getUserObject());
    ui->deselectAll();
    ui->selectObject(obj, true);
    ui->updateButtons();
    focusGameLayerOnObject(ui, obj);
    this->onClose(nullptr);
}

decltype(GroupSummaryPopup::m_mGroupInfo) & GroupSummaryPopup::getGroupInfo() {
    return this->m_mGroupInfo;
}

GroupSummary & GroupSummaryPopup::getGroup(int group) {
    return this->m_mGroupInfo[group];
}

GroupSummaryPopup::~GroupSummaryPopup() {
    CCDirector::sharedDirector()->getTouchDispatcher()->decrementForcePrio(2);
}

GroupSummaryPopup* GroupSummaryPopup::create(LevelEditorLayer* lel) {
    auto ret = new GroupSummaryPopup;

    if (
        ret &&
        (ret->m_pEditor = lel) &&
        ret->init(460.0f, 280.0f, "GJ_square01.png", "Group Summary")
    ) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return ret;
}
