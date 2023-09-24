#include "EditorUI.hpp"

#include "../BetterEdit.hpp"
#include "../tools/AutoSave/Backup/BackupViewLayer.hpp"
#include "../tools/AutoSave/autoSave.hpp"
#include "../tools/EditorLayerInput/editorLayerInput.hpp"
#include "../tools/GlobalClipboard/clipboardHook.hpp"
#include "../tools/GridSize/gridButton.hpp"
#include "../tools/GroupIDFilter/groupfilter.hpp"
#include "../tools/LevelPercent/levelPercent.hpp"
#include "../tools/SliderImprovement/ScaleTextDelegate.hpp"
#include "../tools/SliderImprovement/passTouch.hpp"
#include "../tools/VisibilityTab/loadVisibilityTab.hpp"
#include "EditorPauseLayer.hpp"

#include <chrono>
#include <thread>

using namespace gdmake;
using namespace cocos2d;

static constexpr int const ZOOMLABEL_TAG = 6976;
static constexpr int const TOGGLEUI_TAG = 6979;
static constexpr int const VIEWMODE_BACKBTN_TAG = 59305;

bool g_showUI = true;
bool g_uiIsVisible = true;
bool g_hasResetObjectsScale = true;
bool g_lastSnap = false;
std::chrono::time_point<std::chrono::system_clock> g_lastTouchTime =
	std::chrono::system_clock::now();

// TODO: Clean up this whole file because man is it ugly
// TODO: (move shit out to their own files)

void toggleShowUI(EditorUI* self) {
	self->showUI(!g_uiIsVisible);
}

CCPoint getShowButtonPosition(EditorUI* self) {
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	auto ratio = winSize.width / winSize.height;

	if (ratio > 1.5f) {
		return { self->m_pTrashBtn->getPositionX() + 50.0f, self->m_pTrashBtn->getPositionY() };
	}

	return { self->m_pPlaybackBtn->getPositionX() + 45.0f, self->m_pPlaybackBtn->getPositionY() };
}

void updateToggleButtonSprite(CCMenuItemSpriteExtra* btn) {
	auto spr = btn->getNormalImage();
	spr->retain();

	btn->setNormalImage(CCNodeConstructor()
							.fromBESprite(g_showUI ? "BE_eye-on-btn.png" : "BE_eye-off-btn.png")
							.scale(spr->getScale())
							.csize(spr->getContentSize())
							.done());

	btn->getNormalImage()->setPosition(spr->getPosition());
	btn->getNormalImage()->setAnchorPoint(spr->getAnchorPoint());
	btn->getNormalImage()->setScale(spr->getScale());
	btn->setOpacity(g_showUI ? 255 : 90);

	spr->release();
}

void EditorUI_CB::onToggleShowUI(CCObject*) {
	g_showUI = !g_showUI;
	this->showUI(g_showUI);

	auto btn =
		as<CCMenuItemSpriteExtra*>(this->m_pSwipeBtn->getParent()->getChildByTag(TOGGLEUI_TAG));

	if (btn && !g_showUI) {
		updateToggleButtonSprite(btn);
	}
}

void EditorUI_CB::onExitViewMode(CCObject*) {
	CCDirector::sharedDirector()->popSceneWithTransition(0.5f, cocos2d::kPopTransitionFade);
}

bool touchIntersectsInput(CCNode* input, CCTouch* touch) {
	if (!input) {
		return false;
	}

	auto inp = reinterpret_cast<CCTextInputNode*>(input);
	auto isize = inp->getScaledContentSize();

	auto rect =
		cocos2d::CCRect { inp->getPositionX() - isize.width / 2,
						  inp->getPositionY() - isize.height / 2, isize.width, isize.height };

	if (!rect.containsPoint(input->getParent()->convertTouchToNodeSpace(touch))) {
		reinterpret_cast<CCTextInputNode*>(input)->getTextField()->detachWithIME();
		return false;
	}
	else {
		return true;
	}
}

// bool EditorUI_ccTouchBegan(EditorUI* self, CCTouch* touch, CCEvent* event) {
// 	auto self_ = reinterpret_cast<EditorUI*>(reinterpret_cast<uintptr_t>(self) - 0xEC);

// 	g_bHoldingDownTouch = true;

// 	if (touchIntersectsInput(self_->getChildByTag(LAYERINPUT_TAG), touch)) {
// 		return true;
// 	}
// 	if (touchIntersectsInput(getGridButtonParent(self_)->getChildByTag(ZOOMINPUT_TAG), touch)) {
// 		return true;
// 	}

// 	if (pointIntersectsControls(self_, touch, event)) {
// 		return true;
// 	}

// 	if (BetterEdit::sharedState()->m_bHookConflictFound) {
// 		BetterEdit::showHookConflictMessage();
// 	}

// 	bool swipe = KeybindManager::get()->isModifierPressed("gd.edit.swipe_modifier");

// 	patch(0x90951, swipe ?
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0xe9, 0x6b, 0x02, 0x00, 0x00, 0x00 } :
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, },
//         false, true
//     );
// 	patch(0x90be1, swipe ?
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, } :
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0xeb, 0x23, },
//         false, true
//     );
// 	patch(0x90c20, swipe ?
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0xeb, 0x04, } :
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, },
//         false, true
//     );

// 	bool move = KeybindManager::get()->isModifierPressed("gd.edit.move_modifier");
// 	self_->m_bSpaceKeyPressed = move;
// 	if (self_->m_editorLayer->m_ePlaybackMode != kPlaybackModePlaying) {
// 		self_->m_bMoveModifier = move;
// 	}

// 	// patch(0x90984, { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });

// 	bool duplicate = KeybindManager::get()->isModifierPressed("gd.edit.duplicate_modifier");
// 	patch(
// 		0x909dd,
// 		duplicate ? std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }
// 				  : std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90, 0xeb, 0x22 },
// 		false, true
// 	);

// 	bool snap = KeybindManager::get()->isModifierPressed("gd.edit.snap_modifier");
// 	bool free_move = KeybindManager::get()->isModifierPressed("gd.edit.free_move_modifier");
// 	patch(
// 		0x90971,
// 		(duplicate || snap || free_move)
// 			? std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90, 0xeb, 0x17 }
// 			: std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 },
// 		false, true
// 	);

// 	g_lastSnap = snap;

// 	return matdash::orig<&EditorUI_ccTouchBegan>(self, touch, event);
// }

// MAT_GDMAKE_HOOK(0x907b0, EditorUI_ccTouchBegan);

// void EditorUI_ccTouchMoved(EditorUI* self_, CCTouch* touch, CCEvent* event) {
// 	auto self = reinterpret_cast<EditorUI*>(reinterpret_cast<uintptr_t>(self_) - 0xEC);

// 	float prevScale = self->m_editorLayer->m_pObjectLayer->getScale();
// 	auto swipeStart =
// 		self->m_editorLayer->m_pObjectLayer->convertToNodeSpace(self->m_obSwipeStart) * prevScale;

// 	bool swipe = KeybindManager::get()->isModifierPressed("gd.edit.swipe_modifier");

// 	patch(0x90ff6, swipe ?
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0xeb, 0x0d, } :
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, },
//         false, true
//     );

// 	matdash::orig<&EditorUI_ccTouchMoved>(self_, touch, event);

// 	auto nSwipeStart =
// 		self->m_editorLayer->m_pObjectLayer->convertToNodeSpace(self->m_obSwipeStart) * prevScale;

// 	auto rel = swipeStart - nSwipeStart;
// 	rel = rel * (self->m_editorLayer->m_pObjectLayer->getScale() / prevScale);

// 	if (BetterEdit::getEnableRelativeSwipe()) {
// 		self->m_obSwipeStart = self->m_obSwipeStart + rel;
// 	}
// }

// MAT_GDMAKE_HOOK(0x90cd0, EditorUI_ccTouchMoved);

// void EditorUI_ccTouchEnded(
// 	EditorUI* self,

// 	CCTouch* touch, CCEvent* event
// ) {
// 	g_bHoldingDownTouch = false;

// 	auto now = std::chrono::system_clock::now();

// 	auto self_ = as<EditorUI*>(as<uintptr_t>(self) - 0xEC);
// 	if (!BetterEdit::getDisableDoubleClick() &&
// 		self_->m_editorLayer->m_ePlaybackMode != kPlaybackModePlaying &&
// 		std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastTouchTime).count() <
// 			KeybindManager::get()->getDoubleClickInterval()) {
// 		if (CCDirector::sharedDirector()->getKeyboardDispatcher()->getControlKeyPressed()) {
// 			self_->editGroup(nullptr);
// 		}
// 		else if (CCDirector::sharedDirector()->getKeyboardDispatcher()->getShiftKeyPressed()) {
// 			self_->editObject2(nullptr);
// 		}
// 		else {
// 			self_->editObject(nullptr);
// 		}

// 		self_->m_bTouchDown = false;
// 		self_->m_nTouchID = -1;
// 		self_->stopActionByTag(0x7b);

// 		g_lastTouchTime = now;

// 		return;
// 	}

// 	g_lastTouchTime = now;

// 	matdash::orig<&EditorUI_ccTouchEnded>(self, touch, event);
// }

// MAT_GDMAKE_HOOK(0x911a0, EditorUI_ccTouchEnded);

// void EditorUI_clickOnPosition(EditorUI* self, CCPoint point) {
// 	if (!BetterEdit::isEditorViewOnlyMode()) {
// 		return matdash::orig<&EditorUI_clickOnPosition>(self, point);
// 	}
// }

// MAT_GDMAKE_HOOK(0x78860, EditorUI_clickOnPosition);

void EditorUI_destructorHook(EditorUI* self) {
	saveClipboard(self);
	resetSliderPercent(self);
	// getAutoSaveTimer(self)->resetTimer();

	return matdash::orig<&EditorUI_destructorHook>(self);
}

MAT_GDMAKE_HOOK(0x76090, EditorUI_destructorHook);

class EditorUIPulse : public EditorUI {
public:
	void updateObjectsPulse(float dt) {
		// bool snap = KeybindManager::get()->isModifierPressed("gd.edit.snap_modifier");
		// if (snap != g_lastSnap) {
		// 	this->toggleSnap(nullptr);
		// 	g_lastSnap = snap;
		// }

		// too lazy to add these to gd.h
		// theyre isMusicPlaying and isPlaybackMode

		auto volume = FMODAudioEngine::sharedEngine()->m_fBackgroundMusicVolume;
		if ((*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x130) ||
			 m_editorLayer->m_ePlaybackMode == kPlaybackModePlaying) &&
			volume && BetterEdit::getPulseObjectsInEditor()) {
			g_hasResetObjectsScale = false;

			auto fmod = FMODAudioEngine::sharedEngine();
			auto pulse = fmod->m_fPulse1 + fmod->m_fPulse2 + fmod->m_fPulse3;
			pulse /= 3.f;

			auto const f = [&](CCObject* obj_) {
				auto obj = geode::cast::typeinfo_cast<GameObject*>(obj_);
				if (obj != nullptr && obj->m_unk32C) {
					obj->setRScale(pulse);
				}
			};
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddBottom->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeBottom2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeBottom3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeBottom4->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeBottom->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddBottom2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddBottom3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddBottom4->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNode->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddTop2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeAddTop3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeTop2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_pBatchNodeTop3->getChildren(), obj_)
			f(obj_);
		}
		else if (!g_hasResetObjectsScale) {
			g_hasResetObjectsScale = true;

			auto const arr = m_editorLayer->getAllObjects();
			CCObject* obj_;

			CCARRAY_FOREACH(arr, obj_) {
				auto obj = geode::cast::typeinfo_cast<GameObject*>(obj_);
				if (obj != nullptr && obj->m_unk32C) {
					obj->setRScale(1.f);
				}
			}
		}
	}
};

bool EditorUI_init(EditorUI* self, LevelEditorLayer* lel) {
	makeVisibilityPatches();
	setupRotateSaws();

	if (!matdash::orig<&EditorUI_init>(self, lel)) {
		return false;
	}

	auto winSize = CCDirector::sharedDirector()->getWinSize();

	auto currentZoomLabel = CCLabelBMFont::create("", "bigFont.fnt");
	currentZoomLabel->setScale(.5f);
	currentZoomLabel->setPosition(winSize.width / 2, winSize.height - 60.0f);
	currentZoomLabel->setTag(ZOOMLABEL_TAG);
	currentZoomLabel->setOpacity(0);
	currentZoomLabel->setZOrder(99999);
	self->addChild(currentZoomLabel);

	CCMenuItemSpriteExtra* toggleBtn;
	self->m_pSwipeBtn->getParent()->addChild(
		CCNodeConstructor<CCMenuItemSpriteExtra*>()
			.fromNode(CCMenuItemSpriteExtra::create(
				CCNodeConstructor()
					.fromBESprite("BE_eye-on-btn.png")
					.scale(.6f)
					.exec([](auto t) -> void {
						t->setContentSize(t->getScaledContentSize());
					})
					.done(),
				self, (SEL_MenuHandler)&EditorUI_CB::onToggleShowUI
			))
			.tag(TOGGLEUI_TAG)
			.move(getShowButtonPosition(self))
			.save(&toggleBtn)
			.done()
	);
	updateToggleButtonSprite(toggleBtn);

	loadEditorLayerInput(self);
	setupGroupFilterButton(self);
	loadGridButtons(self);
	loadSliderPercent(self);
	loadClipboard(self);
	loadAutoSaveTimer(self);
	loadVisibilityTab(self);

	if (BetterEdit::isEditorViewOnlyMode()) {
		auto viewOnlyLabel = CCLabelBMFont::create("View-Only Mode", "bigFont.fnt");

		viewOnlyLabel->setScale(.4f);
		viewOnlyLabel->setOpacity(90);
		viewOnlyLabel->setPosition(winSize.width / 2, winSize.height - 30.0f);

		self->addChild(viewOnlyLabel);

		auto backButton = CCMenuItemSpriteExtra::create(
			CCNodeConstructor().fromFrameName("GJ_arrow_01_001.png").scale(.75f).done(), self,
			menu_selector(EditorUI_CB::onExitViewMode)
		);

		backButton->setPosition(
			self->m_pPlaybackBtn->getPositionX(), self->m_pTrashBtn->getPositionY()
		);
		backButton->setTag(VIEWMODE_BACKBTN_TAG);

		self->m_pPlaybackBtn->getParent()->addChild(backButton);

		self->showUI(false);
	}

	BetterEdit::sharedState()->m_bHookConflictFound = false;

	g_hasResetObjectsScale = true;
	self->schedule(schedule_selector(EditorUIPulse::updateObjectsPulse));

	return true;
}

MAT_GDMAKE_HOOK(0x76310, EditorUI_init);

CCArray* EditorUI_createCustomItems(EditorUI* self) {
	setIgnoreNewObjectsForSliderPercent(true);

	auto ret = matdash::orig<&EditorUI_createCustomItems>(self);

	setIgnoreNewObjectsForSliderPercent(false);

	return ret;
}

MAT_GDMAKE_HOOK(0x7a370, EditorUI_createCustomItems);

void EditorUI_onDeleteCustomItem(EditorUI* self, CCObject* pSender) {
	setIgnoreNewObjectsForSliderPercent(true);

	matdash::orig<&EditorUI_onDeleteCustomItem>(self, pSender);

	setIgnoreNewObjectsForSliderPercent(false);
}

MAT_GDMAKE_HOOK(0x7a280, EditorUI_onDeleteCustomItem);

void EditorUI_onNewCustomItem(EditorUI* self, CCObject* pSender) {
	setIgnoreNewObjectsForSliderPercent(true);

	matdash::orig<&EditorUI_onNewCustomItem>(self, pSender);

	setIgnoreNewObjectsForSliderPercent(false);
}

MAT_GDMAKE_HOOK(0x79fd0, EditorUI_onNewCustomItem);

void EditorUI_showUI(EditorUI* self, bool show) {
	if (BetterEdit::isEditorViewOnlyMode()) {
		show = false;
	}

	if (!g_showUI) {
		show = false;
	}

	matdash::orig<&EditorUI_showUI>(self, show);

	g_uiIsVisible = show;

	if (BetterEdit::isEditorViewOnlyMode()) {
		CCARRAY_FOREACH_B_TYPE(self->m_pPlaybackBtn->getParent()->getChildren(), node, CCNode) {
			node->setVisible(node->getTag() == VIEWMODE_BACKBTN_TAG);
		}
	}

	auto toggleBtn =
		as<CCMenuItemSpriteExtra*>(self->m_pSwipeBtn->getParent()->getChildByTag(TOGGLEUI_TAG));

	self->m_tabsMenu->setVisible(self->m_nSelectedMode == 2 && show);
	CATCH_NULL(self->m_pCopyBtn->getParent()->getChildByTag(7777))->setVisible(show);
	showGridButtons(self, show);
	showLayerControls(self, show);
	showVisibilityTab(self, show);

	if (toggleBtn) {
		toggleBtn->setVisible(show || !g_showUI);
		self->m_pPlaytestBtn->setVisible(g_showUI);
		if (show) {
			updateToggleButtonSprite(toggleBtn);
		}
	}
	// showPositionLabel(self, show);
}

MAT_GDMAKE_HOOK(0x87180, EditorUI_showUI);

void EditorUI_updateZoom(EditorUI* self) {
	matdash::orig<&EditorUI_updateZoom>(self);

	float zoom;
	__asm { movss zoom, xmm1 }

	if (!BetterEdit::getDisableZoomText()) {
		auto zLabel = as<CCLabelBMFont*>(self->getChildByTag(ZOOMLABEL_TAG));

		if (zLabel) {
			zLabel->setString(
				("Zoom: "_s +
				 BetterEdit::formatToString(self->m_editorLayer->getObjectLayer()->getScale(), 2u) +
				 "x"_s)
					.c_str()
			);
			zLabel->setOpacity(255);
			zLabel->stopAllActions();
			zLabel->runAction(CCSequence::create(
				CCArray::create(CCDelayTime::create(.5f), CCFadeOut::create(.5f), nullptr)
			));
		}
	}

	updatePercentLabelPosition(self);
}

MAT_GDMAKE_HOOK(0x878a0, EditorUI_updateZoom);

// Credits to Alk1m123 (https://github.com/altalk23) for this scale fix
// this lets you scale multiple objects without it fucking up the position
void EditorUI_scaleObjects(EditorUI* self, CCArray* objs, CCPoint centerPos) {
	float scale;
	__asm movss scale, xmm2;
	CCObject* obj;

	// maybe add some scale anchor point feature, as itd
	// only require changing the centerPos here

	// prevent the scale from being 0
	// as that can cause weird behaviour
	if (scale > -0.01f && scale < 0.01f) {
		scale = std::copysign(0.01f, scale);
	}
	bool lockPos = false;
	if (self->m_pScaleControl) {
		auto fancyWidget =
			dynamic_cast<ScaleTextDelegate*>(self->m_pScaleControl->getChildByTag(7777));
		if (fancyWidget) {
			lockPos = fancyWidget->m_bLockPosEnabled;
		}
	}
	CCARRAY_FOREACH(objs, obj) {
		auto gameObj = reinterpret_cast<GameObject*>(obj);
		auto pos = gameObj->getPosition();
		float newScale = gameObj->m_fMultiScaleMultiplier * scale;
		float newMultiplier = newScale / gameObj->m_scale;
		auto newPos = (pos - centerPos) * newMultiplier + (centerPos - pos);

		// this is just GameObject::updateCustomScale
		// although that does some rounding so its stupid
		gameObj->m_scale = newScale;
		gameObj->setRScale(1.f);
		gameObj->m_bTextureRectDirty = true;
		gameObj->m_bIsObjectRectDirty = true;

		if (!lockPos) {
			self->moveObject(gameObj, newPos);
		}
	}
}

MAT_GDMAKE_HOOK(0x8f150, EditorUI_scaleObjects);
