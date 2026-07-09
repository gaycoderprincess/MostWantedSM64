#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <mutex>
#include <random>
#include <thread>
#include <toml++/toml.hpp>

#include "nya_dx9_hookbase.h"
#include "nya_commonhooklib.h"
#include "nya_commonmath.h"
#include "nfsmw.h"

#include "include/chloemenulib.h"

std::vector<void(*)()> aDrawing3DLoopFunctions;
std::vector<void(*)()> aDrawing3DLoopFunctionsOnce;

#include "util.h"

void UpdateD3DProperties() {
	g_pd3dDevice = GameD3DDevice;
	ghWnd = GameWindow;
	GetRacingResolution(&nResX, &nResY);
}

void HookLoop() {}

#include "components/render3d.h"

#include "hooks/carrender.h"

#include "include/surface_terrains.h"
#include "include/audio_defines.h"
#include "include/sm64_defs.h"
#include "include/libsm64.h"
#include "components/sm64.h"
#include "components/customcamera.h"

void CameraHook(CameraMover* pMover) {
	DLLDirSetter _setdir;

	if (CustomCamera::IsMario()) {
		static CNyaTimer gTimer;
		gTimer.Process();
		CustomCamera::SetTargetCar(GetLocalPlayerVehicle(), nullptr);
		CustomCamera::ProcessCam(pMover->pCamera, gTimer.fDeltaTime);
	}
}

void Render3DLoop() {
	UpdateD3DProperties();

	DLLDirSetter _setdir;

	static IDirect3DStateBlock9* state = nullptr;
	if (g_pd3dDevice->CreateStateBlock(D3DSBT_ALL, &state) != D3D_OK) {
		return;
	}

	if (state->Capture() < 0) {
		state->Release();
		return;
	}

	for (auto& func : aDrawing3DLoopFunctions) {
		func();
	}

	for (auto& func : aDrawing3DLoopFunctionsOnce) {
		func();
	}
	aDrawing3DLoopFunctionsOnce.clear();

	state->Apply();
	state->Release();

	CommonMain(false);
}

void M64ModMenu() {
	DLLDirSetter _setdir;

	ChloeMenuLib::BeginMenu();

	if (SM64::bAvailable && DrawMenuOption("Mario Debug")) {
		ChloeMenuLib::BeginMenu();

		if (DrawMenuOption("mario enable")) {
			SM64::bEnabled = true;
		}

		if (DrawMenuOption("mario disable")) {
			SM64::DisableMario();
		}

		DrawMenuOption(std::format("position {:.2f} {:.2f} {:.2f}",SM64::marioState.position[0],SM64::marioState.position[1],SM64::marioState.position[2]));
		DrawMenuOption(std::format("velocity {:.2f} {:.2f} {:.2f}",SM64::marioState.velocity[0],SM64::marioState.velocity[1],SM64::marioState.velocity[2]));
		DrawMenuOption(std::format("marioId {}",SM64::marioId));
		DrawMenuOption(std::format("numTrianglesUsed {}",SM64::marioGeometry.numTrianglesUsed));
		DrawMenuOption(std::format("geom 0 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[0],SM64::marioGeometry.position[1],SM64::marioGeometry.position[2]));
		DrawMenuOption(std::format("geom 1 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[3],SM64::marioGeometry.position[4],SM64::marioGeometry.position[5]));
		DrawMenuOption(std::format("geom 2 {:.2f} {:.2f} {:.2f}",SM64::marioGeometry.position[6],SM64::marioGeometry.position[7],SM64::marioGeometry.position[8]));

		auto bak = SM64::marioScalar;
		QuickValueEditor("marioScalar", SM64::marioScalar);
		if (bak != SM64::marioScalar) {
			NyaVec3 v = {0,0,0};
			if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
				v = *ply->GetPosition();
				v.y -= 1;
			}

			SM64::ResetMario(v);
		}

		if (DrawMenuOption("mario teleport")) {
			NyaVec3 v = {0,0,0};
			if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
				v = *ply->GetPosition();
				v.y -= 1;
			}

			SM64::ResetMario(v);
		}

		auto worldFacing = SM64::GetMarioWorldFacing();
		DrawMenuOption(std::format("facing {:.2f} {:.2f} {:.2f}",worldFacing[0],worldFacing[1],worldFacing[2]));

		if (DrawMenuOption("mario teleport fwd")) {
			auto v = SM64::GetMarioWorldPos();
			v += SM64::GetMarioWorldFacing() * 5;
			SM64::ResetMario(v);
		}

		if (DrawMenuOption("mario damage")) {
			sm64_mario_take_damage(SM64::marioId, 1, 0, 0, 0, 0);
		}

		if (!SM64::aCollisionTris.empty()) {
			auto col = SM64::aCollisionTris[0];
			DrawMenuOption(std::format("col 0 {:.2f} {:.2f} {:.2f}",col.fPt0[0],col.fPt0[1],col.fPt0[2]));
			DrawMenuOption(std::format("col 1 {:.2f} {:.2f} {:.2f}",col.fPt1[0],col.fPt1[1],col.fPt1[2]));
			DrawMenuOption(std::format("col 2 {:.2f} {:.2f} {:.2f}",col.fPt2[0],col.fPt2[1],col.fPt2[2]));
		}

		ChloeMenuLib::EndMenu();
	}

	ChloeMenuLib::EndMenu();
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x3C4040) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.3 (.exe size of 6029312 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			srand(time(0));

			GetCurrentDirectoryW(MAX_PATH, gDLLDir);

			ChloeMenuLib::RegisterMenu("Cwoee 64", &M64ModMenu);

			if (std::filesystem::exists("NFSMWSM64_gcp.toml")) {
				auto config = toml::parse_file("NFSMWSM64_gcp.toml");
				SM64::marioScalar = config["world_scale"].value_or(SM64::marioScalar);
			}

			for (auto& func : ChloeHook::aHooks) {
				func();
			}

			NyaHooks::LateInitHook::Init();
			NyaHooks::LateInitHook::aFunctions.push_back([](){
				if (!GetModuleHandleA("vulkan-1.dll") && !GetModuleHandleA("winevulkan.dll")) {
					MessageBoxA(nullptr, "WARNING: DXVK is not installed properly! Make sure you've placed d3d9.dll from the mod's archive into the game folder or you WILL encounter stability issues!", "nya?!~", MB_ICONERROR);
				}
			});
			NyaHooks::CameraMoverHook::Init();
			NyaHooks::CameraMoverHook::aFunctions.push_back(CameraHook);
			NyaHooks::RenderWorldHook::Init();
			NyaHooks::RenderWorldHook::aPostFunctions.push_back(Render3DLoop);

			//SkipFE = true;
		} break;
		default:
			break;
	}
	return TRUE;
}