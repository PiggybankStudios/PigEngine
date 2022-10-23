/*
File:   pig_steam.cpp
Author: Taylor Robbins
Date:   10\15\2022
Description: 
	** Holds functions that help us get information or perform operations through Steam
*/

#if STEAM_BUILD

void Pig_InitializeSteamAvatars()
{
	NotNull(pig);
	CreateVarArray(&pig->steamAvatars, mainHeap, sizeof(SteamAvatar_t));
}

bool LoadSteamAvatarForFriend(u64 friendId, PlatSteamFriendAvatarSize_t size)
{
	VarArrayLoop(&platInfo->steamFriendsList->friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &platInfo->steamFriendsList->friends, fIndex);
		if (!friendInfo->isRequestingAvatar && friendInfo->id == friendId)
		{
			return plat->RequestSteamFriendAvatar(friendInfo->id, size);
		}
	}
	return false;
}

SteamAvatar_t* GetSteamAvatar(u64 friendId, PlatSteamFriendAvatarSize_t size, bool beginLoad = true)
{
	VarArrayLoop(&pig->steamAvatars, aIndex)
	{
		VarArrayLoopGet(SteamAvatar_t, steamAvatar, &pig->steamAvatars, aIndex);
		if (steamAvatar->friendId == friendId && steamAvatar->size == size)
		{
			return steamAvatar;
		}
	}
	if (beginLoad) { LoadSteamAvatarForFriend(friendId, size); }
	return nullptr;
}

void Pig_UpdateSteamAvatars()
{
	VarArrayLoop(&platInfo->steamFriendsList->friends, fIndex)
	{
		VarArrayLoopGet(PlatSteamFriendInfo_t, friendInfo, &platInfo->steamFriendsList->friends, fIndex);
		if (friendInfo->avatarSize != PlatSteamFriendAvatarSize_None && !friendInfo->freeAvatarImageData)
		{
			SteamAvatar_t* existingAvatar = GetSteamAvatar(friendInfo->id, friendInfo->avatarSize, false);
			if (existingAvatar != nullptr)
			{
				DestroyTexture(&existingAvatar->texture);
			}
			else
			{
				SteamAvatar_t* newAvatar = VarArrayAdd(&pig->steamAvatars, SteamAvatar_t);
				NotNull(newAvatar);
				ClearPointer(newAvatar);
				newAvatar->friendId = friendInfo->id;
				newAvatar->size = friendInfo->avatarSize;
				existingAvatar = newAvatar;
			}
			
			if (!CreateTexture(mainHeap, &existingAvatar->texture, &friendInfo->avatarImageData, false, false))
			{
				PrintLine_E("Failed to create steam friend avatar texture from image data: friendId=%llu imageData.size=%llu (%d, %d)",
					friendInfo->id,
					friendInfo->avatarImageData.dataSize,
					friendInfo->avatarImageData.width, friendInfo->avatarImageData.height
				);
				VarArrayRemoveByPntr(&pig->steamAvatars, existingAvatar);
			}
			
			friendInfo->freeAvatarImageData = true;
		}
	}
}

#endif //STEAM_BUILD
