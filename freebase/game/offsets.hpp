// Basics Offsets
#pragma once
#define UWORLD 0x1714D538   
#define GAME_INSTANCE 0x210
#define LOCAL_PLAYERS 0x38
#define PLAYER_CONTROLLER 0x30
#define LOCAL_PAWN 0x350 
#define PAWN_PRIVATE 0x320 
#define ROOT_COMPONENT 0x1B0
#define RELATIVE_LOCATION 0x138
#define PLAYER_STATE 0x2C8
#define TEAM_INDEX 0x1259  
#define GAME_STATE 0x198 
#define PLAYER_ARRAY 0x2C0
#define MESH 0x328
#define COMPONENT_TO_WORLD 0x1E0
#define BONE_ARRAY 0x5C8
#define BONE_ARRAY_CACHE 0x5D8
#define CURRENT_WEAPON 0xa80
#define HABANERO_COMPONENT 0xa48
#define RANKED_PROGRESS 0xd0 
#define PLATFORM 0x430 
#define PLAYERNAME 0xb08

//FOV CHANGER OFFSETS
#define DEFAULTFOV 0x2BC
#define BASEFOV 0x370 
#define PLAYERCAMERAMANAGER 0x360

// World esp offsets 
#define Levels 0x1b0 //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=UWorld&member=Levels
#define Actors 0x1A8 //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=ULevel&member=Actors
#define PrimaryPickupItemEntry 0x370 //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=AFortPickup&member=PrimaryPickupItemEntry
#define Itemname 0x40 //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=UItemDefinitionBase&member=ItemName
#define Rarity 0xa2 //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=UFortItemDefinition&member=Rarity
#define bAlreadySearched 0xe02   //https://dumpspace.spuckwaffel.com/Games/?hash=6b77eceb&type=classes&idx=ABuildingContainer&member=bAlreadySearched%20:%201