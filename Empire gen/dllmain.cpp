// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
using namespace ArgScript;
using namespace Simulator;
void Initialize()
{
	// This method is executed when the game starts, before the user interface is shown
	// Here you can do things such as:
	//  - Add new cheats
	//  - Add new simulator classes
	//  - Add new game modes
	//  - Add new space tools
	//  - Change materials
	
	
}
//member_detour(CreateSpaceCommEvent_detour, Simulator::cComm auto (uint32_t, PlanetID, uint32_t, uint32_t, void*, int, unisgned int)) {
	
void expand(cEmpire* empire) {
	auto cStarManager = cStarManager::Get();
	empire->mEmpireMoney = 100001;
	auto homeworld = empire->GetHomeStarRecord();
	cStarManager->RequirePlanetsForStar(homeworld);
	StarRequestFilter filter;
	filter.minDistance = 0.1f;
	filter.techLevels = 0;
	filter.AddTechLevel(TechLevel::None);
	filter.AddTechLevel(TechLevel::Creature);
	filter.RemoveStarType(StarType::GalacticCore);
	filter.RemoveStarType(StarType::ProtoPlanetary);
	filter.RemoveStarType(StarType::BlackHole);
	int randomnumber = Math::rand(100);
	int systemstocapture;
	if (randomnumber == 100) {
		systemstocapture = 20;
	}
	else if (randomnumber > 96) {
		systemstocapture = 12;
	}
	else if (randomnumber > 91) {
		systemstocapture = 9;
	}
	else if (randomnumber > 81) {
		systemstocapture = 6;
	}
	else if (randomnumber > 71) {
		systemstocapture = 5;
	}
	else if (randomnumber > 51) {
		systemstocapture = 4;
	}
	else if (randomnumber > 31) {
		systemstocapture = 3;
	}
	else if (randomnumber > 11) {
		systemstocapture = 2;
	}
	else {
		systemstocapture = 1;
	}
	int homeplanets = GetPlayerEmpire()->mStars.size();
	if (systemstocapture >= homeplanets) {
		systemstocapture -= 1;
	}
	else if (20 < homeplanets) {
		systemstocapture *= round(1 + (0.03 * (homeplanets - 20)));
	}

	auto empireid = empire->GetEmpireID();
	for (int i = 0; i < systemstocapture; i++) {
		auto nearbystar = cStarManager->FindClosestStar(homeworld->mPosition, filter);

		cStarManager->RequirePlanetsForStar(nearbystar);
		empire->AddStarOwnership(nearbystar);
		nearbystar->mEmpireID = empireid;
		nearbystar->mTechLevel = TechLevel::Empire;
		for (auto planet : nearbystar->GetPlanetRecords()) {
			if (planet->mType == PlanetType::T3 || planet->mType == PlanetType::T2 || planet->mType == PlanetType::T1 || planet->mType == PlanetType::T0) {
				//attempt a colonization here
				cPlanetRecord::FillPlanetDataForTechLevel(planet.get(), TechLevel::Empire);
				planet->mTechLevel = TechLevel::Empire;
				for (auto whateverthefuck : planet->mCivData) {
					whateverthefuck->mPoliticalID = empireid;
				}
				break;
			}

		}
	}
}
//};
member_detour(GetEmpireForStar__detour, Simulator::cStarManager, cEmpire* (cStarRecord*)) {
	cEmpire* detoured(cStarRecord * starRecord) {
		auto empire = original_function(this, starRecord);
		if (starRecord->mEmpireID) {
			if (empire->mEmpireName == u"unknown" && empire->mEmpireMoney != 100001) {
				expand(empire);
			}
		}
		return empire;
	}

};
member_detour(CreateSpaceCommEvent__detour, Simulator::cCommManager, cCommEvent* (uint32_t, PlanetID, uint32_t, uint32_t, void*, int, unsigned int)) {

	cCommEvent* detoured(uint32_t empireid, PlanetID sourceplanet, uint32_t fileid, uint32_t dialogID, void* pMission, int priority, unsigned int duration) {

		auto commevent = original_function(this, empireid, sourceplanet, fileid, dialogID, pMission, priority, duration);
		auto cStarManager = cStarManager::Get();
		auto empire = cStarManager->GetEmpire(empireid);
		
		if ((dialogID == 2939165228 || dialogID == 3729496045) && empire->mEmpireMoney != 100001 ) {
			expand(empire);
		
		}
		return commevent;
	}
};
void Dispose()
{
	// This method is called when the game is closing
}
void AttachDetours(){
	// Call the attach() method on any detours you want to add
	// For example: cViewer_SetRenderType_detour::attach(GetAddress(cViewer, SetRenderType));
	GetEmpireForStar__detour::attach(GetAddress(cStarManager, GetEmpireForStar));
	CreateSpaceCommEvent__detour::attach(GetAddress(cCommManager, CreateSpaceCommEvent));
}


// Generally, you don't need to touch any code here
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		ModAPI::AddPostInitFunction(Initialize);
		ModAPI::AddDisposeFunction(Dispose);

		PrepareDetours(hModule);
		AttachDetours();
		CommitDetours();
		break;

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

