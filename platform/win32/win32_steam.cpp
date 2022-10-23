/*
File:   win32_steam.cpp
Author: Taylor Robbins
Date:   10\13\2022
Description: 
	** Holds functions that help us initialize and communicate with the Steam SDK
	** https://partner.steamgames.com/doc/sdk
*/

#if !STEAM_BUILD
void Win32_SteamInit() { } //do nothing
#endif

#if STEAM_BUILD

PLAT_API_GET_STEAM_FRIEND_INFO_BY_ID_DEFINITION(Win32_GetSteamFriendInfoById);
PLAT_API_GET_STEAM_FRIEND_GROUP_BY_ID_DEFINITION(Win32_GetSteamFriendGroupById);

// +==============================+
// |     SteamWarningCallback     |
// +==============================+
extern "C"
{
	void __cdecl SteamWarningCallback(int nSeverity, const char* pchDebugText)
	{
		if (nSeverity != 0) //warning
		{
			PrintLine_W("STEAM: %s", pchDebugText);
		}
		else //message
		{
			PrintLine_I("STEAM: %s", pchDebugText);
		}
	}
}

// +--------------------------------------------------------------+
// |                          Initialize                          |
// +--------------------------------------------------------------+
void Win32_FreePlatSteamFriendInfo(PlatSteamFriendsList_t* list, PlatSteamFriendInfo_t* friendInfo)
{
	NotNull3(list, list->allocArena, friendInfo);
	if (friendInfo->id != 0)
	{
		FreeString(list->allocArena, &friendInfo->name);
		FreeString(list->allocArena, &friendInfo->nickname);
		Win32_FreeImageData(&friendInfo->avatarImageData);
		VarArrayLoop(&friendInfo->presenceStrs, sIndex)
		{
			VarArrayLoopGet(PlatSteamFriendPresenceStr_t, presenceStr, &friendInfo->presenceStrs, sIndex);
			FreeString(list->allocArena, &presenceStr->key);
			FreeString(list->allocArena, &presenceStr->value);
		}
		FreeVarArray(&friendInfo->presenceStrs);
	}
	ClearPointer(friendInfo);
}
void Win32_FreePlatSteamFriendGroup(PlatSteamFriendsList_t* list, PlatSteamFriendGroup_t* group)
{
	NotNull3(list, list->allocArena, group);
	if (group->id != 0)
	{
		FreeString(list->allocArena, &group->name);
		FreeVarArray(&group->memberIds);
	}
	ClearPointer(group);
}

void Win32_SteamInit()
{
	if (!SteamAPI_Init())
	{
		Win32_InitError("Failed to initialize Steam API. Make sure you started the game from Steam and that you are logged in to an account that owns the game.");
	}
	Platform->steamAppId = SteamUtils()->GetAppID();
	Platform->steamUserId = SteamUser()->GetSteamID();
	SteamUtils()->SetWarningMessageHook(SteamWarningCallback);
	
	ClearStruct(Platform->steamFriendsList);
	Platform->steamFriendsList.allocArena = &Platform->mainHeap;
	Platform->steamFriendsList.nextFriendId = 1;
	Platform->steamFriendsList.nextGroupId = 1;
	Platform->steamFriendsList.lastQueryAttemptTime = 0;
	Platform->steamFriendsList.lastQuerySuccessTime = 0;
	Platform->steamFriendsList.lastQueryError = PlatSteamFriendsListQueryError_None;
	CreateVarArray(&Platform->steamFriendsList.friends, Platform->steamFriendsList.allocArena, sizeof(PlatSteamFriendInfo_t));
	CreateVarArray(&Platform->steamFriendsList.groups, Platform->steamFriendsList.allocArena, sizeof(PlatSteamFriendGroup_t));
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
PlatSteamFriendInfo_t* Win32_GetSteamFriendInfoBySteamId(CSteamID steamId)
{
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		if (friendInfo->steamId == steamId) { return friendInfo; }
	}
	return nullptr;
}

PlatSteamFriendGroup_t* Win32_GetSteamFriendGroupBySteamId(FriendsGroupID_t steamId)
{
	VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
	{
		VarArrayLoopGet(PlatSteamFriendGroup_t, friendGroup, &Platform->steamFriendsList.groups, gIndex);
		if (friendGroup->steamId == steamId) { return friendGroup; }
	}
	return nullptr;
}

PlatSteamFriendOnlineStatus_t Win32_GetOnlineStatusForPersonaState(EPersonaState personaState)
{
	switch (personaState)
	{
		case k_EPersonaStateOffline:         return PlatSteamFriendOnlineStatus_Offline;
		case k_EPersonaStateOnline:          return PlatSteamFriendOnlineStatus_Online;
		case k_EPersonaStateBusy:            return PlatSteamFriendOnlineStatus_Busy;
		case k_EPersonaStateAway:            return PlatSteamFriendOnlineStatus_Away;
		case k_EPersonaStateSnooze:          return PlatSteamFriendOnlineStatus_Snooze;
		// case k_EPersonaStateLookingForTrade: return PlatSteamFriendOnlineStatus_LookingForTrade; //TODO: Re-enable me when I download latest version of Steamworks SDK?
		case k_EPersonaStateLookingToPlay:   return PlatSteamFriendOnlineStatus_LookingToPlay;
		default: return PlatSteamFriendOnlineStatus_Unknown;
	}
}

PlatSteamFriendInfo_t* Win32_AddOrUpdateFriendBySteamId(CSteamID steamId)
{
	const char* friendName = SteamFriends()->GetFriendPersonaName(steamId);
	if (friendName == nullptr) { friendName = ""; }
	const char* friendNickname = SteamFriends()->GetPlayerNickname(steamId);
	if (friendNickname == nullptr) { friendNickname = ""; }
	EPersonaState friendState = SteamFriends()->GetFriendPersonaState(steamId);
	
	PlatSteamFriendInfo_t* friendInfo = Win32_GetSteamFriendInfoBySteamId(steamId);
	if (friendInfo == nullptr)
	{
		friendInfo = VarArrayAdd(&Platform->steamFriendsList.friends, PlatSteamFriendInfo_t);
		NotNull(friendInfo);
		
		ClearPointer(friendInfo);
		friendInfo->id = Platform->steamFriendsList.nextFriendId;
		Platform->steamFriendsList.nextFriendId++;
		friendInfo->steamId = steamId;
		friendInfo->name = NewStringInArenaNt(Platform->steamFriendsList.allocArena, friendName);
		friendInfo->nickname = NewStringInArenaNt(Platform->steamFriendsList.allocArena, friendNickname);
		CreateVarArray(&friendInfo->presenceStrs, Platform->steamFriendsList.allocArena, sizeof(PlatSteamFriendPresenceStr_t));
	}
	else
	{
		if (MyStrCompareNt(friendInfo->name.pntr, friendName) != 0)
		{
			FreeString(Platform->steamFriendsList.allocArena, &friendInfo->name);
			friendInfo->name = NewStringInArenaNt(Platform->steamFriendsList.allocArena, friendName);
		}
		if (MyStrCompareNt(friendInfo->nickname.pntr, friendNickname) != 0)
		{
			FreeString(Platform->steamFriendsList.allocArena, &friendInfo->nickname);
			friendInfo->nickname = NewStringInArenaNt(Platform->steamFriendsList.allocArena, friendNickname);
		}
	}
	friendInfo->onlineStatus = Win32_GetOnlineStatusForPersonaState(friendState);
	
	return friendInfo;
}

PlatSteamFriendGroup_t* Win32_AddOrUpdateFriendGroupBySteamId(FriendsGroupID_t steamId)
{
	const char* friendsGroupName = SteamFriends()->GetFriendsGroupName(steamId);
	if (friendsGroupName == nullptr) { friendsGroupName = ""; }
	int friendsGroupMemberCount = SteamFriends()->GetFriendsGroupMembersCount(steamId);
	
	PlatSteamFriendGroup_t* group = Win32_GetSteamFriendGroupBySteamId(steamId);
	if (group == nullptr)
	{
		group = VarArrayAdd(&Platform->steamFriendsList.groups, PlatSteamFriendGroup_t);
		NotNull(group);
		
		ClearPointer(group);
		group->id = Platform->steamFriendsList.nextGroupId;
		Platform->steamFriendsList.nextGroupId++;
		group->steamId = steamId;
		group->name = NewStringInArenaNt(Platform->steamFriendsList.allocArena, friendsGroupName);
		CreateVarArray(&group->memberIds, Platform->steamFriendsList.allocArena, sizeof(u64), friendsGroupMemberCount);
	}
	
	return group;
}

void Win32_UpdateFriendsList()
{
	
}

// +==============================+
// |   Win32_CompareFriendInfo    |
// +==============================+
// i32 Win32_CompareFriendInfo(const void* left, const void* right, void* contextPntr)
COMPARE_FUNC_DEFINITION(Win32_CompareFriendInfo)
{
	UNUSED(contextPntr);
	PlatSteamFriendInfo_t* leftFriend = (PlatSteamFriendInfo_t*)left;
	PlatSteamFriendInfo_t* rightFriend = (PlatSteamFriendInfo_t*)right;
	NotNull2(leftFriend, rightFriend);
	//Sort by online status primarily, and name secondarily
	if (leftFriend->onlineStatus != rightFriend->onlineStatus)
	{
		//NOTE: Snooze and Away both seem to be treated the same in the Steam friends list display, so we will treat them as the same status for sake of ordering
		if (!(leftFriend->onlineStatus == PlatSteamFriendOnlineStatus_Away || leftFriend->onlineStatus == PlatSteamFriendOnlineStatus_Snooze) ||
			!(rightFriend->onlineStatus == PlatSteamFriendOnlineStatus_Away || rightFriend->onlineStatus == PlatSteamFriendOnlineStatus_Snooze))
		{
			return (leftFriend->onlineStatus < rightFriend->onlineStatus) ? 1 : -1;
		}
	}
	return CompareFuncMyStr(&leftFriend->name, &rightFriend->name, nullptr);
}

// +==================================+
// | Win32_CompareFriendGroupMemberId |
// +==================================+
// i32 Win32_CompareFriendGroupMemberId(const void* left, const void* right, void* contextPntr)
COMPARE_FUNC_DEFINITION(Win32_CompareFriendGroupMemberId)
{
	UNUSED(contextPntr);
	u64* leftMemberId = (u64*)left;
	u64* rightMemberId = (u64*)right;
	PlatSteamFriendInfo_t* leftFriend = Win32_GetSteamFriendInfoById(*leftMemberId);
	PlatSteamFriendInfo_t* rightFriend = Win32_GetSteamFriendInfoById(*rightMemberId);
	NotNull2(leftFriend, rightFriend);
	return Win32_CompareFriendInfo(leftFriend, rightFriend, nullptr);
}

void Win32_UpdateFriendGroupMembers(PlatSteamFriendGroup_t* group)
{
	TempPushMark();
	int friendsGroupMemberCount = SteamFriends()->GetFriendsGroupMembersCount(group->steamId);
	CSteamID* groupMemberIDs = (friendsGroupMemberCount > 0) ? TempArray(CSteamID, friendsGroupMemberCount) : nullptr;
	Assert(groupMemberIDs != nullptr || friendsGroupMemberCount == 0);
	SteamFriends()->GetFriendsGroupMembersList(group->steamId, groupMemberIDs, friendsGroupMemberCount);
	
	PlatSteamFriendInfo_t** groupMemberFriendPntrs = (friendsGroupMemberCount > 0) ? TempArray(PlatSteamFriendInfo_t*, friendsGroupMemberCount) : nullptr;
	for (int mIndex = 0; mIndex < friendsGroupMemberCount; mIndex++)
	{
		groupMemberFriendPntrs[mIndex] = Win32_GetSteamFriendInfoBySteamId(groupMemberIDs[mIndex]);
	}
	
	
	//Add new members
	for (int mIndex = 0; mIndex < friendsGroupMemberCount; mIndex++)
	{
		CSteamID memberFriendId = groupMemberIDs[mIndex];
		PlatSteamFriendInfo_t* friendInfo = groupMemberFriendPntrs[mIndex];
		
		if (friendInfo != nullptr)
		{
			bool isFriendAlreadyInList = false;
			VarArrayLoop(&group->memberIds, iIndex)
			{
				VarArrayLoopGet(u64, memberIdPntr, &group->memberIds, iIndex);
				if (*memberIdPntr == friendInfo->id) { isFriendAlreadyInList = true; break; }
			}
			
			if (!isFriendAlreadyInList)
			{
				friendInfo->numGroups++;
				u64* newIdPntr = VarArrayAdd(&group->memberIds, u64);
				NotNull(newIdPntr);
				*newIdPntr = friendInfo->id;
			}
		}
	}
	
	//Remove old members
	VarArrayLoop(&group->memberIds, iIndex)
	{
		VarArrayLoopGet(u64, memberIdPntr, &group->memberIds, iIndex);
		PlatSteamFriendInfo_t* friendInfo = Win32_GetSteamFriendInfoById(*memberIdPntr);
		
		if (friendInfo == nullptr)
		{
			VarArrayRemove(&group->memberIds, iIndex, u64);
			iIndex--;
		}
		else
		{
			bool isFriendActuallyInGroup = false;
			for (u64 mIndex = 0; mIndex < friendsGroupMemberCount; mIndex++)
			{
				if (groupMemberIDs[mIndex] == friendInfo->steamId) { isFriendActuallyInGroup = true; break; }
			}
			
			if (!isFriendActuallyInGroup)
			{
				Assert(friendInfo->numGroups > 0);
				friendInfo->numGroups--;
				VarArrayRemove(&group->memberIds, iIndex, u64);
				iIndex--;
			}
		}
	}
	
	//Sort this list by name
	VarArraySort(&group->memberIds, Win32_CompareFriendGroupMemberId, nullptr);
	
	TempPopMark();
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void Win32_UpdateSteamStuff()
{
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		if (friendInfo->freeAvatarImageData)
		{
			friendInfo->freeAvatarImageData = false;
			friendInfo->avatarSize = PlatSteamFriendAvatarSize_None;
			Win32_FreeImageData(&friendInfo->avatarImageData);
		}
	}
}

void Win32_UpdateEngineInputSteamInfo(EngineInput_t* prevInput, EngineInput_t* newInput)
{
	UNUSED(prevInput);
	newInput->steamIpcCallCount = SteamUtils()->GetIPCCallCount();
}

// +--------------------------------------------------------------+
// |                        API Functions                         |
// +--------------------------------------------------------------+
// +==============================+
// | Win32_GetSteamFriendInfoById |
// +==============================+
// PlatSteamFriendInfo_t* GetSteamFriendInfoById(u64 id)
PLAT_API_GET_STEAM_FRIEND_INFO_BY_ID_DEFINITION(Win32_GetSteamFriendInfoById) //pre-declared at top of file
{
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		if (friendInfo->id == id) { return friendInfo; }
	}
	return nullptr;
}

// +===============================+
// | Win32_GetSteamFriendGroupById |
// +===============================+
// PlatSteamFriendGroup_t* GetSteamFriendGroupById(u64 id)
PLAT_API_GET_STEAM_FRIEND_GROUP_BY_ID_DEFINITION(Win32_GetSteamFriendGroupById) //pre-declared at top of file
{
	VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
	{
		VarArrayLoopGet(PlatSteamFriendGroup_t, friendGroup, &Platform->steamFriendsList.groups, gIndex);
		if (friendGroup->id == id) { return friendGroup; }
	}
	return nullptr;
}

// +==================================+
// | Win32_GetSteamFriendPresenceStr  |
// +==================================+
// PlatSteamFriendPresenceStr_t* GetSteamFriendPresenceStr(u64 friendId, MyStr_t keyStr)
PLAT_API_GET_STEAM_FRIEND_PRESENCE_STR_DEFINITION(Win32_GetSteamFriendPresenceStr)
{
	PlatSteamFriendInfo_t* friendInfo = Win32_GetSteamFriendInfoById(friendId);
	if (friendInfo == nullptr) { return nullptr; }
	VarArrayLoop(&friendInfo->presenceStrs, sIndex)
	{
		VarArrayLoopGet(PlatSteamFriendPresenceStr_t, presenceStr, &friendInfo->presenceStrs, sIndex);
		if (StrCompareIgnoreCase(presenceStr->key, keyStr) == 0)
		{
			return presenceStr;
		}
	}
	return nullptr;
}

// +==============================+
// | Win32_StartSteamFriendsQuery |
// +==============================+
// void StartSteamFriendsQuery()
PLAT_API_START_STEAM_FRIENDS_QUERY_DEFINITION(Win32_StartSteamFriendsQuery)
{
	AssertSingleThreaded();
	Platform->steamFriendsList.lastQueryAttemptTime = Win32_GetProgramTime(nullptr, false);
	Platform->steamFriendsList.lastQueryError = PlatSteamFriendsListQueryError_None;
	
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		friendInfo->needsToBeRemoved = true;
	}
	
	VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
	{
		VarArrayLoopGet(PlatSteamFriendGroup_t, group, &Platform->steamFriendsList.groups, gIndex);
		group->needsToBeRemoved = true;
	}
	
	#if 0
	int numFriendClans = SteamFriends()->GetClanCount();
	PrintLine_D("Enumerating %d clans", numFriendClans);
	for (int cIndex = 0; cIndex < numFriendClans; cIndex++)
	{
		CSteamID clanId = SteamFriends()->GetClanByIndex(cIndex);
		const char* clanName = SteamFriends()->GetClanName(clanId);
		if (clanName == nullptr) { clanName = ""; }
		PrintLine_D("Clan[%d]: \"%s\"", cIndex, clanName);
	}
	#endif
	
	int numFriendGroups = SteamFriends()->GetFriendsGroupCount();
	// PrintLine_D("Enumerating %d groups", numFriendGroups);
	for (int gIndex = 0; gIndex < numFriendGroups; gIndex++)
	{
		FriendsGroupID_t friendsGroupId = SteamFriends()->GetFriendsGroupIDByIndex(gIndex);
		if (friendsGroupId == k_FriendsGroupID_Invalid) { continue; }
		PlatSteamFriendGroup_t* group = Win32_AddOrUpdateFriendGroupBySteamId(friendsGroupId);
		group->needsToBeRemoved = false;
	}
	
	int numIncomingFriends = SteamFriends()->GetFriendCount(k_EFriendFlagFriendshipRequested);
	int numNormalFriends   = SteamFriends()->GetFriendCount(k_EFriendFlagImmediate);
	int numOutgoingFriends = SteamFriends()->GetFriendCount(k_EFriendFlagRequestingFriendship);
	for (int fIndex = 0; fIndex < numIncomingFriends || fIndex < numNormalFriends || fIndex < numOutgoingFriends; fIndex++)
	{
		CSteamID friendIds[3];
		friendIds[0] = ((fIndex < numIncomingFriends) ? SteamFriends()->GetFriendByIndex(fIndex, k_EFriendFlagFriendshipRequested)  : k_steamIDNil);
		friendIds[1] = ((fIndex < numNormalFriends)   ? SteamFriends()->GetFriendByIndex(fIndex, k_EFriendFlagImmediate)            : k_steamIDNil);
		friendIds[2] = ((fIndex < numOutgoingFriends) ? SteamFriends()->GetFriendByIndex(fIndex, k_EFriendFlagRequestingFriendship) : k_steamIDNil);
		PlatSteamFriendInfo_t* friendPntrs[3] = {};
		if (friendIds[0] != k_steamIDNil) { friendPntrs[0] = Win32_AddOrUpdateFriendBySteamId(friendIds[0]); }
		if (friendPntrs[0] != nullptr) { friendPntrs[0]->needsToBeRemoved = false; friendPntrs[0]->state = PlatSteamFriendState_Incoming; }
		if (friendIds[1] != k_steamIDNil) { friendPntrs[1] = Win32_AddOrUpdateFriendBySteamId(friendIds[1]); }
		if (friendPntrs[1] != nullptr) { friendPntrs[1]->needsToBeRemoved = false; friendPntrs[1]->state = PlatSteamFriendState_Normal;   }
		if (friendIds[2] != k_steamIDNil) { friendPntrs[2] = Win32_AddOrUpdateFriendBySteamId(friendIds[2]); }
		if (friendPntrs[2] != nullptr) { friendPntrs[2]->needsToBeRemoved = false; friendPntrs[2]->state = PlatSteamFriendState_Outgoing; }
	}
	
	VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
	{
		VarArrayLoopGet(PlatSteamFriendGroup_t, group, &Platform->steamFriendsList.groups, gIndex);
		Win32_UpdateFriendGroupMembers(group);
	}
	
	VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
	{
		VarArrayLoopGet(PlatSteamFriendGroup_t, group, &Platform->steamFriendsList.groups, gIndex);
		if (group->needsToBeRemoved)
		{
			//Decrement numGroups on all members
			VarArrayLoop(&group->memberIds, mIndex)
			{
				VarArrayLoopGet(u64, memberIdPntr, &group->memberIds, mIndex);
				PlatSteamFriendInfo_t* friendInfo = Win32_GetSteamFriendInfoById(*memberIdPntr);
				if (friendInfo != nullptr)
				{
					DebugAssert(friendInfo->numGroups > 0);
					friendInfo->numGroups--;
				}
			}
			
			Win32_FreePlatSteamFriendGroup(&Platform->steamFriendsList, group);
			VarArrayRemove(&Platform->steamFriendsList.groups, gIndex, PlatSteamFriendGroup_t);
			gIndex--;
		}
	}
	
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		if (friendInfo->needsToBeRemoved)
		{
			//Remove this friend from all groups
			VarArrayLoop(&Platform->steamFriendsList.groups, gIndex)
			{
				VarArrayLoopGet(PlatSteamFriendGroup_t, group, &Platform->steamFriendsList.groups, gIndex);
				VarArrayLoop(&group->memberIds, mIndex)
				{
					VarArrayLoopGet(u64, memberIdPntr, &group->memberIds, mIndex);
					if (*memberIdPntr == friendInfo->id)
					{
						VarArrayRemove(&group->memberIds, mIndex, u64);
						mIndex--;
					}
				}
			}
			
			Win32_FreePlatSteamFriendInfo(&Platform->steamFriendsList, friendInfo);
			VarArrayRemove(&Platform->steamFriendsList.friends, fIndex, PlatSteamFriendInfo_t);
			fIndex--;
		}
	}
	
	VarArraySort(&Platform->steamFriendsList.friends, Win32_CompareFriendInfo, nullptr);
	
	Platform->steamFriendsList.lastQuerySuccessTime = Win32_GetProgramTime(nullptr, false);
}

// +===============================+
// | Win32_UpdateSteamFriendStatus |
// +===============================+
// void UpdateSteamFriendStatus()
PLAT_API_UPDATE_STEAM_STATUS_DEFINITION(Win32_UpdateSteamFriendStatus)
{
	// Get current game info
	VarArrayLoop(&Platform->steamFriendsList.friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &Platform->steamFriendsList.friends, fIndex);
		friendInfo->lastStatusUpdateAttemptTime = Win32_GetProgramTime(nullptr, false);
		
		// +==============================+
		// |     Update In-Game Info      |
		// +==============================+
		FriendGameInfo_t gameInfo = {};
		if (SteamFriends()->GetFriendGamePlayed(friendInfo->steamId, &gameInfo))
		{
			friendInfo->isInGame = true;
			friendInfo->inGameSteamAppId   = gameInfo.m_gameID.AppID();
			friendInfo->inGameLobbySteamId = gameInfo.m_steamIDLobby;
			friendInfo->inGameIP           = gameInfo.m_unGameIP;
			friendInfo->inGamePort         = gameInfo.m_usGamePort;
			friendInfo->inGameQueryPort    = gameInfo.m_usQueryPort;
		}
		else
		{
			friendInfo->isInGame = false;
		}
		
		// +==============================+
		// |  Update Rich Presence Strs   |
		// +==============================+
		{
			VarArrayLoop(&friendInfo->presenceStrs, sIndex)
			{
				VarArrayLoopGet(PlatSteamFriendPresenceStr_t, presenceStr, &friendInfo->presenceStrs, sIndex);
				presenceStr->needsToBeRemoved = true;
			}
			
			SteamFriends()->RequestFriendRichPresence(friendInfo->steamId);
			int numRichPresenceKeys = SteamFriends()->GetFriendRichPresenceKeyCount(friendInfo->steamId);
			// PrintLine_D("Rich presence keys: %d", numRichPresenceKeys);
			for (int sIndex = 0; sIndex < numRichPresenceKeys; sIndex++)
			{
				const char* richPresenceKey = SteamFriends()->GetFriendRichPresenceKeyByIndex(friendInfo->steamId, sIndex);
				if (richPresenceKey == nullptr || richPresenceKey[0] == '\0') { continue; }
				const char* richPresenceValue = SteamFriends()->GetFriendRichPresence(friendInfo->steamId, richPresenceKey);
				if (richPresenceValue == nullptr) { continue; }
				MyStr_t richPresenceKeyStr = NewStr(richPresenceKey);
				MyStr_t richPresenceValueStr = NewStr(richPresenceValue);
				PlatSteamFriendPresenceStr_t* existingStr = Win32_GetSteamFriendPresenceStr(friendInfo->id, richPresenceKeyStr);
				if (existingStr != nullptr)
				{
					existingStr->needsToBeRemoved = false;
					if (!StrEquals(existingStr->value, richPresenceValueStr))
					{
						FreeString(Platform->steamFriendsList.allocArena, &existingStr->value);
						existingStr->value = AllocString(Platform->steamFriendsList.allocArena, &richPresenceValueStr);
						NotNullStr(&existingStr->value);
					}
				}
				else
				{
					PlatSteamFriendPresenceStr_t* newStr = VarArrayAdd(&friendInfo->presenceStrs, PlatSteamFriendPresenceStr_t);
					NotNull(newStr);
					ClearPointer(newStr);
					existingStr->needsToBeRemoved = false;
					existingStr->key = AllocString(Platform->steamFriendsList.allocArena, &richPresenceKeyStr);
					NotNullStr(&existingStr->key);
					existingStr->value = AllocString(Platform->steamFriendsList.allocArena, &richPresenceValueStr);
					NotNullStr(&existingStr->value);
				}
			}
			
			VarArrayLoop(&friendInfo->presenceStrs, sIndex)
			{
				VarArrayLoopGet(PlatSteamFriendPresenceStr_t, presenceStr, &friendInfo->presenceStrs, sIndex);
				if (presenceStr->needsToBeRemoved)
				{
					FreeString(Platform->steamFriendsList.allocArena, &presenceStr->key);
					FreeString(Platform->steamFriendsList.allocArena, &presenceStr->value);
					VarArrayRemove(&friendInfo->presenceStrs, sIndex, PlatSteamFriendPresenceStr_t);
					sIndex--;
				}
			}
		}
		
		friendInfo->lastStatusUpdateSuccessTime = Win32_GetProgramTime(nullptr, false);
	}
}

// +================================+
// | Win32_RequestSteamFriendAvatar |
// +================================+
// bool RequestSteamFriendAvatar(u64 friendId, PlatSteamFriendAvatarSize_t size)
PLAT_API_REQUEST_STREAM_FRIEND_AVATAR(Win32_RequestSteamFriendAvatar)
{
	Assert(size != PlatSteamFriendAvatarSize_None);
	PlatSteamFriendInfo_t* friendInfo = Win32_GetSteamFriendInfoById(friendId);
	NotNull(friendInfo);
	
	friendInfo->isRequestingAvatar = true;
	int avatarHandle = 0;
	switch (size)
	{
		case PlatSteamFriendAvatarSize_Small:  avatarHandle = SteamFriends()->GetSmallFriendAvatar(friendInfo->steamId); break;
		case PlatSteamFriendAvatarSize_Medium: avatarHandle = SteamFriends()->GetMediumFriendAvatar(friendInfo->steamId); break;
		case PlatSteamFriendAvatarSize_Large:  avatarHandle = SteamFriends()->GetLargeFriendAvatar(friendInfo->steamId); break;
		default: AssertMsg(false, "Invalid size value passed to Win32_RequestSteamFriendAvatar"); break;
	}
	
	u32 avatarImageWidth = 0;
	u32 avatarImageHeight = 0;
	bool getImageSizeSuccess = SteamUtils()->GetImageSize(avatarHandle, &avatarImageWidth, &avatarImageHeight);
	
	if (!getImageSizeSuccess)
	{
		friendInfo->isRequestingAvatar = false;
		return false;
	}
	
	//TODO: Should we put some limit on max width/height of the avatar so we don't attempt to allocate a BUNCH of memory?
	
	PlatImageData_t imageData = {};
	imageData.allocArena = &Platform->stdHeap;
	imageData.pixelSize = 4;
	imageData.width = (i32)avatarImageWidth;
	imageData.height = (i32)avatarImageHeight;
	imageData.rowSize = imageData.pixelSize * avatarImageWidth;
	imageData.dataSize = imageData.rowSize * avatarImageHeight;
	Assert(imageData.dataSize <= INT_MAX);
	imageData.data8 = AllocArray(imageData.allocArena, u8, imageData.dataSize);
	NotNull(imageData.data8);
	
	bool getImageRgbaSuccess = SteamUtils()->GetImageRGBA(avatarHandle, imageData.data8, (int)imageData.dataSize);
	if (!getImageRgbaSuccess)
	{
		Win32_FreeImageData(&imageData);
		friendInfo->isRequestingAvatar = false;
		return false;
	}
	
	if (friendInfo->avatarSize != PlatSteamFriendAvatarSize_None)
	{
		Win32_FreeImageData(&friendInfo->avatarImageData);
		friendInfo->avatarSize = PlatSteamFriendAvatarSize_None;
		friendInfo->freeAvatarImageData = false;
	}
	friendInfo->avatarSize = size;
	friendInfo->freeAvatarImageData = false;
	MyMemCopy(&friendInfo->avatarImageData, &imageData, sizeof(PlatImageData_t));
	friendInfo->isRequestingAvatar = false;
	
	return true;
}

#endif //STEAM_BUILD
