#include "ModuleLoader.h"
#include "Console.h"
#pragma once


void ModuleLoader::LoadChampionModule()
{
	switch (ObjectManager::GetLocalPlayer()->BaseCharacterData->SkinHash)
	{
	case Character::Vayne:
		console.Print("Vayne Loaded");
		break;
	}
}
