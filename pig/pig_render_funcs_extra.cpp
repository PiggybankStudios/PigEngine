/*
File:   pig_render_funcs_extra.cpp
Author: Taylor Robbins
Date:   10\14\2022
Description: 
	** Holds a some more complicated rendering functions. Things that rely on multiple dependencies or involve higher level data
*/

// +--------------------------------------------------------------+
// |                    Steam Render Functions                    |
// +--------------------------------------------------------------+
#if STEAM_BUILD

#define STEAM_CARD_MIN_ASPECT_RATIO 2.2f

v2 RcDrawSteamFriendCard(u64 friendId, v2 position, v2 minSize, v2 maxSize, r32 scale, bool measureOnly = false)
{
	MyStr_t titleStr = NewStr("[Missing Info]");
	MyStr_t captionStr = NewStr("[Missing Info]");
	PlatSteamFriendOnlineStatus_t onlineStatus = PlatSteamFriendOnlineStatus_Unknown;
	
	PlatSteamFriendInfo_t* friendInfo = plat->GetSteamFriendInfoById(friendId);
	if (friendInfo != nullptr)
	{
		if (friendInfo->nickname.length > 0)
		{
			titleStr = TempPrintStr("%.*s (%.*s)", friendInfo->name.length, friendInfo->name.pntr, friendInfo->nickname.length, friendInfo->nickname.pntr);
		}
		else
		{
			titleStr = friendInfo->name;
		}
		captionStr = NewStr(GetPlatSteamFriendOnlineStatusStr(friendInfo->onlineStatus));
		onlineStatus = friendInfo->onlineStatus;
		//TODO: Once we have game playing info, replace caption with currently playing game
	}
	
	Font_t* titleFont = &pig->resources.fonts->debug;
	FontFaceSelector_t titleFontSelector = SelectFontFace(24, true);
	Font_t* captionFont = &pig->resources.fonts->debug;
	FontFaceSelector_t captionFontSelector = SelectFontFace(18, false);
	if (scale >= 4.0f)
	{
		titleFont = &pig->resources.fonts->large;
		titleFontSelector = SelectFontFace(48, true);
		captionFont = &pig->resources.fonts->debug;
		captionFontSelector = SelectFontFace(24, false);
	}
	
	TextMeasure_t titleMeasure = MeasureTextInFont(titleStr, titleFont, titleFontSelector);
	TextMeasure_t captionMeasure = MeasureTextInFont(captionStr, captionFont, captionFontSelector);
	
	r32 margin = 5 * scale;
	r32 padding = 3 * scale;
	rec mainRec = NewRec(position, 0, 0);
	rec iconRec = NewRec(margin, margin, 32 * scale, 32 * scale);
	rec titleRec = NewRec(iconRec.x + iconRec.width + padding, iconRec.y, titleMeasure.size);
	rec captionRec = NewRec(iconRec.x + iconRec.width + padding, titleRec.y + titleRec.height + padding, captionMeasure.size);
	mainRec.width = MaxR32(titleRec.x + titleRec.width, captionRec.x + captionRec.width) + margin;
	mainRec.height = MaxR32(iconRec.y + iconRec.height, captionRec.y + captionRec.height) + margin;
	if (minSize.width != 0 && mainRec.width < minSize.width) { mainRec.width = minSize.width; }
	if (minSize.height != 0 && mainRec.height < minSize.height) { mainRec.height = minSize.height; }
	if (mainRec.width / mainRec.height < STEAM_CARD_MIN_ASPECT_RATIO)
	{
		mainRec.width = mainRec.height * STEAM_CARD_MIN_ASPECT_RATIO;
	}
	if (maxSize.width != 0 && mainRec.width > maxSize.width) { mainRec.width = maxSize.width; }
	if (maxSize.height != 0 && mainRec.height > maxSize.height) { mainRec.height = maxSize.height; }
	iconRec.height = mainRec.height - margin*2;
	iconRec.width = iconRec.height;
	titleRec.x = iconRec.x + iconRec.width + padding;
	captionRec.x = iconRec.x + iconRec.width + padding;
	// RecAlign(&mainRec);
	iconRec    = iconRec + mainRec.topLeft;
	titleRec   = titleRec + mainRec.topLeft;
	captionRec = captionRec + mainRec.topLeft;
	// RecAlign(&iconRec);
	RecAlign(&titleRec);
	RecAlign(&captionRec);
	
	if (!measureOnly)
	{
		PlatSteamFriendAvatarSize_t avatarSize = PlatSteamFriendAvatarSize_Small;
		if (iconRec.width > 48 || iconRec.height > 48) { avatarSize = PlatSteamFriendAvatarSize_Medium; }
		if (iconRec.width > 96 || iconRec.height > 96) { avatarSize = PlatSteamFriendAvatarSize_Large; }
		SteamAvatar_t* steamAvatar = GetSteamAvatar(friendId, avatarSize);
		
		Color_t nameColor = MonokaiGray1;
		Color_t outlineColor = Transparent;
		if (onlineStatus == PlatSteamFriendOnlineStatus_Online)
		{
			nameColor = MonokaiGreen;
			outlineColor = MonokaiGreen;
		}
		else if (onlineStatus == PlatSteamFriendOnlineStatus_Busy)
		{
			nameColor = MonokaiMagenta;
			outlineColor = MonokaiMagenta;
		}
		else if (onlineStatus == PlatSteamFriendOnlineStatus_Away || onlineStatus == PlatSteamFriendOnlineStatus_Snooze)
		{
			nameColor = MonokaiBlue;
			outlineColor = MonokaiBlue;
		}
		
		RcDrawGradientRectangle(mainRec, false, MonokaiBack, ColorLerp(MonokaiBack, Black, 0.75f));
		
		rec oldViewport = rc->state.viewportRec;
		RcSetViewport(RecOverlap(mainRec, oldViewport));
		
		if (steamAvatar != nullptr)
		{
			RcBindTexture1(&steamAvatar->texture);
			RcDrawTexturedRectangle(iconRec, White);
		}
		else
		{
			v2 loadingTextPos = iconRec.topLeft + iconRec.size/2 + NewVec2(0, -RcGetLineHeight()/2 + RcGetMaxAscend());
			Vec2Align(&loadingTextPos);
			i32 numPeriods = RoundR32i(Animate(1, 4, 1000));
			
			RcDrawGradientRectangle(iconRec, false, ColorTransparent(White, 0.30f), ColorTransparent(White, 0.05f));
			RcBindFont(&pig->resources.fonts->debug, SelectFontFace(12));
			RcDrawTextPrintEx(loadingTextPos, MonokaiWhite, TextAlignment_Center, 0, "Loading%s%s%s",
				numPeriods >= 1 ? "." : " ",
				numPeriods >= 2 ? "." : " ",
				numPeriods >= 3 ? "." : " "
			);
		}
		if (outlineColor.a > 0)
		{
			RcDrawRectangleOutline(iconRec, ColorTransparent(outlineColor, Oscillate(0.5f, 1.0f, 5000, friendId * 14723)), 1 * scale);
		}
		
		RcBindFont(titleFont, titleFontSelector);
		RcDrawText(titleStr, titleRec.topLeft + titleMeasure.offset, nameColor, TextAlignment_Left);
		RcBindFont(captionFont, captionFontSelector);
		RcDrawText(captionStr, captionRec.topLeft + captionMeasure.offset, ColorDarken(nameColor, 20), TextAlignment_Left);
		
		RcSetViewport(oldViewport);
	}
	
	return mainRec.size;
}

#endif //STEAM_BUILD

