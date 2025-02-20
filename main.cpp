// generated through GDMake https://github.com/HJfod/GDMake

// include GDMake & submodules
#include "BetterEdit.hpp"
#include "tools/AutoSave/Backup/LevelBackupManager.hpp"
#include "tools/AutoSave/autoSave.hpp"
#include "tools/CustomKeybinds/BEKeybinds.hpp"
#include "tools/CustomKeybinds/KeybindManager.hpp"
#include "tools/CustomKeybinds/SuperKeyboardManager.hpp"
#include "tools/CustomKeybinds/SuperMouseManager.hpp"
#include "tools/EditorLayerInput/LayerManager.hpp"
#include "tools/EditorLayerInput/editorLayerInput.hpp"
#include "tools/EnterToSearch/loadEnterSearch.hpp"
#include "tools/FLAlertLayerFix/FLAlertLayerFix.hpp"
#include "tools/other/dashOrbLine.hpp"
#include "tools/other/placeObjectsBefore.hpp"
#include "tools/other/teleportScaleFix.hpp"

#include <GDMake.h>

// #include <matdash.hpp>
// #include <matdash/boilerplate.hpp>
// #include <matdash/minhook.hpp>
// #include <matdash/console.hpp>

// std::vector<void(*)()>& matstuff::get_hooks() {
//     static std::vector<void(*)()> vec;
//     return vec;
// }

using namespace geode::prelude;

#define INIT_MANAGER(name)                \
	log::debug("Initializing {}", #name); \
	name::initGlobal()

$on_mod(DataSaved) {
	geode::log::debug("Saving BetterEdit");
	BetterEdit::sharedState()->save();
	geode::log::debug("Saving Finished");
}

$on_mod(Loaded) {
	// matdash::create_console();
	log::debug("Loading BetterEdit");
	log::debug("Adding patches");

	// patch(0x1e62a6,
	//     {
	//         0x8b, 0xcf,                                 // MOV ECX, EDI
	//         0xe8, 0x43, 0x00, 0x00, 0x00,               // CALL PauseLayer::goEdit
	//         0x90,                                       // PUSH EDI
	//         0x90, 0x90, 0x90, 0x90, 0x90, 0x90,         // CALL dword ptr
	//         0x90, 0x90, 0x90,                           // ADD ESP, 0x8
	//         0x90,                                       // PUSH EAX
	//         0x90,                                       // PUSH ECX
	//         0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,   // MOV dword ptr
	//         0x90, 0x90, 0x90, 0x90, 0x90, 0x90,         // CALL dword ptr
	//         0x90, 0x90, 0x90,                           // ADD ESP, 0x4
	//         0x90,                                       // PUSH EAX
	//         0x90, 0x90, 0x90, 0x90, 0x90, 0x90,         // CALL dword ptr
	//         0x90, 0x90, 0x90,                           // ADD ESP, 0xC
	//         0x90, 0x90,                                 // MOV ECX, EDI
	//         0x90,                                       // PUSH EAX
	//         0x90, 0x90, 0x90, 0x90, 0x90, 0x90,         // CALL dword ptr
	//     }
	// );

	// thanks to adaf
	// this enables pulses in FMODAudioEngine in every layer, instead of just in PlayLayer
	// patch(0x23b56, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });

	INIT_MANAGER(BetterEdit);
	// INIT_MANAGER(LevelBackupManager);
	// INIT_MANAGER(KeybindManager);
	// INIT_MANAGER(SuperKeyboardManager);
	// INIT_MANAGER(SuperMouseManager);

	log::debug("Loading tools");

	// loadBEKeybinds();
	// loadEnterSearch();
	// loadTeleportScaleFix();
	// loadFLAlertLayerFix();
	// loadPlaceObjectsBefore();
	loadDashOrbLines();
}
