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
		return { self->m_trashBtn->getPositionX() + 50.0f, self->m_trashBtn->getPositionY() };
	}

	return { self->m_playbackBtn->getPositionX() + 45.0f, self->m_playbackBtn->getPositionY() };
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
		as<CCMenuItemSpriteExtra*>(this->m_swipeBtn->getParent()->getChildByTag(TOGGLEUI_TAG));

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
// 	if (self_->m_editorLayer->m_playbackMode != kPlaybackModePlaying) {
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

// 	float prevScale = m_editorLayer->m_pObjectLayer->getScale();
// 	auto swipeStart =
// 		m_editorLayer->m_pObjectLayer->convertToNodeSpace(m_obSwipeStart) * prevScale;

// 	bool swipe = KeybindManager::get()->isModifierPressed("gd.edit.swipe_modifier");

// 	patch(0x90ff6, swipe ?
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0xeb, 0x0d, } :
//         std::vector<uint8_t> { 0x90, 0x90, 0x90, 0x90,  0x90, 0x90, },
//         false, true
//     );

// 	matdash::orig<&EditorUI_ccTouchMoved>(self_, touch, event);

// 	auto nSwipeStart =
// 		m_editorLayer->m_pObjectLayer->convertToNodeSpace(m_obSwipeStart) * prevScale;

// 	auto rel = swipeStart - nSwipeStart;
// 	rel = rel * (m_editorLayer->m_pObjectLayer->getScale() / prevScale);

// 	if (BetterEdit::getEnableRelativeSwipe()) {
// 		m_obSwipeStart = m_obSwipeStart + rel;
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
// 		self_->m_editorLayer->m_playbackMode != kPlaybackModePlaying &&
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

		auto volume = FMODAudioEngine::sharedEngine()->m_backgroundMusicVolume;
		if ((*reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(this) + 0x130) ||
			 m_editorLayer->m_playbackMode == PlaybackMode::Playing) &&
			volume && BetterEdit::getPulseObjectsInEditor()) {
			g_hasResetObjectsScale = false;

			auto fmod = FMODAudioEngine::sharedEngine();
			auto pulse = fmod->m_pulse1 + fmod->m_pulse2 + fmod->m_pulse3;
			pulse /= 3.f;

			auto const f = [&](CCObject* obj_) {
				auto obj = geode::cast::typeinfo_cast<GameObject*>(obj_);
				if (obj != nullptr && obj->m_useAudioScale) {
					obj->setRScale(pulse);
				}
			};
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddBottom->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeBottom2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeBottom3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeBottom4->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeBottom->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddBottom2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddBottom3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddBottom4->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNode->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddTop2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeAddTop3->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeTop2->getChildren(), obj_)
			f(obj_);
			CCARRAY_FOREACH_B(m_editorLayer->m_batchNodeTop3->getChildren(), obj_)
			f(obj_);
		}
		else if (!g_hasResetObjectsScale) {
			g_hasResetObjectsScale = true;

			auto const arr = m_editorLayer->getAllObjects();
			CCObject* obj_;

			CCARRAY_FOREACH(arr, obj_) {
				auto obj = geode::cast::typeinfo_cast<GameObject*>(obj_);
				if (obj != nullptr && obj->m_useAudioScale) {
					obj->setRScale(1.f);
				}
			}
		}
	}
};

#include <Geode/modify/EditorUI.hpp>

class $modify(EditorUI) {
	void destructor() {
		saveClipboard(this);
		resetSliderPercent(this);
		// getAutoSaveTimer(this)->resetTimer();

		EditorUI::~EditorUI();
	}

	bool init(LevelEditorLayer* lel) {
		makeVisibilityPatches();
		setupRotateSaws();
		
		if (!EditorUI::init(lel)) {
			return false;
		}

		auto winSize = CCDirector::sharedDirector()->getWinSize();

		auto currentZoomLabel = CCLabelBMFont::create("", "bigFont.fnt");
		currentZoomLabel->setScale(.5f);
		currentZoomLabel->setPosition(winSize.width / 2, winSize.height - 60.0f);
		currentZoomLabel->setTag(ZOOMLABEL_TAG);
		currentZoomLabel->setOpacity(0);
		currentZoomLabel->setZOrder(99999);
		this->addChild(currentZoomLabel);

		CCMenuItemSpriteExtra* toggleBtn;
		m_swipeBtn->getParent()->addChild(
			CCNodeConstructor<CCMenuItemSpriteExtra*>()
				.fromNode(CCMenuItemSpriteExtra::create(
					CCNodeConstructor()
						.fromBESprite("BE_eye-on-btn.png")
						.scale(.6f)
						.exec([](auto t) -> void {
							t->setContentSize(t->getScaledContentSize());
						})
						.done(),
					this, (SEL_MenuHandler)&EditorUI_CB::onToggleShowUI
				))
				.tag(TOGGLEUI_TAG)
				.move(getShowButtonPosition(this))
				.save(&toggleBtn)
				.done()
		);
		updateToggleButtonSprite(toggleBtn);

		// TODO: 
		// loadEditorLayerInput(this);
		// setupGroupFilterButton(this);
		// loadGridButtons(this);
		loadSliderPercent(this);
		// loadClipboard(this);
		// loadAutoSaveTimer(this);
		// loadVisibilityTab(this);

		if (BetterEdit::isEditorViewOnlyMode()) {
			auto viewOnlyLabel = CCLabelBMFont::create("View-Only Mode", "bigFont.fnt");

			viewOnlyLabel->setScale(.4f);
			viewOnlyLabel->setOpacity(90);
			viewOnlyLabel->setPosition(winSize.width / 2, winSize.height - 30.0f);

			this->addChild(viewOnlyLabel);

			auto backButton = CCMenuItemSpriteExtra::create(
				CCNodeConstructor().fromFrameName("GJ_arrow_01_001.png").scale(.75f).done(), this,
				menu_selector(EditorUI_CB::onExitViewMode)
			);

			backButton->setPosition(
				m_playbackBtn->getPositionX(), m_trashBtn->getPositionY()
			);
			backButton->setTag(VIEWMODE_BACKBTN_TAG);

			m_playbackBtn->getParent()->addChild(backButton);

			this->showUI(false);
		}

		BetterEdit::sharedState()->m_bHookConflictFound = false;

		g_hasResetObjectsScale = true;
		this->schedule(schedule_selector(EditorUIPulse::updateObjectsPulse));

		return true;
	}

	CCArray* createCustomItems() {
		setIgnoreNewObjectsForSliderPercent(true);

		auto ret = EditorUI::createCustomItems();

		setIgnoreNewObjectsForSliderPercent(false);

		return ret;
	}

	void onDeleteCustomItem(CCObject* pSender) {
		setIgnoreNewObjectsForSliderPercent(true);

		EditorUI::onDeleteCustomItem(pSender);

		setIgnoreNewObjectsForSliderPercent(false);
	}

	void onNewCustomItem(CCObject* pSender) {
		setIgnoreNewObjectsForSliderPercent(true);

		EditorUI::onNewCustomItem(pSender);

		setIgnoreNewObjectsForSliderPercent(false);
	}

	void showUI(bool show) {
		if (BetterEdit::isEditorViewOnlyMode()) {
			show = false;
		}

		if (!g_showUI) {
			show = false;
		}

		EditorUI::showUI(show);

		g_uiIsVisible = show;

		if (BetterEdit::isEditorViewOnlyMode()) {
			CCARRAY_FOREACH_B_TYPE(this->m_playbackBtn->getParent()->getChildren(), node, CCNode) {
				node->setVisible(node->getTag() == VIEWMODE_BACKBTN_TAG);
			}
		}

		auto toggleBtn =
			as<CCMenuItemSpriteExtra*>(this->m_swipeBtn->getParent()->getChildByTag(TOGGLEUI_TAG));

		this->m_tabsMenu->setVisible(this->m_selectedMode == 2 && show);
		CATCH_NULL(this->m_copyBtn->getParent()->getChildByTag(7777))->setVisible(show);
		// TODO: 
		// showGridButtons(this, show);
		// showLayerControls(this, show);
		// showVisibilityTab(this, show);

		if (toggleBtn) {
			toggleBtn->setVisible(show || !g_showUI);
			this->m_playtestBtn->setVisible(g_showUI);
			if (show) {
				updateToggleButtonSprite(toggleBtn);
			}
		}
		// showPositionLabel(this, show);
	}

	void updateZoom(float zoom) {
		EditorUI::updateZoom(zoom);

		if (!BetterEdit::getDisableZoomText()) {
			auto zLabel = as<CCLabelBMFont*>(this->getChildByTag(ZOOMLABEL_TAG));

			if (zLabel) {
				zLabel->setString(
					("Zoom: "_s +
					 BetterEdit::formatToString(this->m_editorLayer->getObjectLayer()->getScale(), 2u) +
					 "x"_s)
						.c_str()
				);
				zLabel->setOpacity(255);
				zLabel->stopAllActions();
				zLabel->runAction(CCSequence::create(
					CCDelayTime::create(.5f), CCFadeOut::create(.5f), nullptr
				));
			}
		}

		updatePercentLabelPosition(this);
	}


	// Credits to Alk1m123 (https://github.com/altalk23) for this scale fix
	// this lets you scale multiple objects without it fucking up the position
	void scaleObjects(CCArray* objs, float scale, CCPoint centerPos) {
		CCObject* obj;

		// maybe add some scale anchor point feature, as itd
		// only require changing the centerPos here

		// prevent the scale from being 0
		// as that can cause weird behaviour
		if (scale > -0.01f && scale < 0.01f) {
			scale = std::copysign(0.01f, scale);
		}
		bool lockPos = false;
		// TODO: reeenable
		// if (m_scaleControl) {
		// 	auto fancyWidget =
		// 		geode::cast::typeinfo_cast<ScaleTextDelegate*>(m_scaleControl->getChildByTag(7777));
		// 	if (fancyWidget) {
		// 		lockPos = fancyWidget->m_bLockPosEnabled;
		// 	}
		// }
		CCARRAY_FOREACH(objs, obj) {
			auto gameObj = reinterpret_cast<GameObject*>(obj);
			auto pos = gameObj->getPosition();
			float newScale = gameObj->m_multiScaleMultiplier * scale;
			float newMultiplier = newScale / gameObj->m_scale;
			auto newPos = (pos - centerPos) * newMultiplier + (centerPos - pos);

			// this is just GameObject::updateCustomScale
			// although that does some rounding so its stupid
			gameObj->m_scale = newScale;
			gameObj->setRScale(1.f);
			gameObj->m_textureRectDirty = true;
			gameObj->m_isObjectRectDirty = true;

			if (!lockPos) {
				this->moveObject(gameObj, newPos);
			}
		}
	}
};
