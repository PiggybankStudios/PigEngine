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

//TODO: Move me somewhere project specific or add a define to enable this conditionally for that project
#if 0
// +--------------------------------------------------------------+
// |                   LDtk Level Debug Render                    |
// +--------------------------------------------------------------+
void RcDrawLdtkLevel(LdtkProject_t* project, LdtkLevel_t* level, v2 position, r32 scale, bool drawTiles = true, bool drawEntities = true)
{
	UNUSED(project);
	rec levelBounds = NewRec(position, ToVec2(level->bounds.size) * scale);
	RcDrawRectangle(levelBounds, ColorTransparent(Black, 0.5f));
	
	// +==============================+
	// | Render Tiles Layer Instances |
	// +==============================+
	if (drawTiles)
	{
		VarArrayLoop(&level->layerInstances, lIndex)
		{
			VarArrayLoopGet(LdtkLayerInstance_t, layerInstance, &level->layerInstances, lIndex);
			if (layerInstance->type == LdtkLayerType_Tiles)
			{
				NotNull(layerInstance->tiles.tilesetDefPntr);
				LdtkTilesetDef_t* tilesetDef = layerInstance->tiles.tilesetDefPntr;
				v2i gridSize = layerInstance->tiles.gridSize;
				v2i tileSize = layerInstance->tiles.tileSize;
				
				if (lIndex == 0)
				{
					for (i32 tileY = 0; tileY < gridSize.height; tileY++)
					{
						for (i32 tileX = 0; tileX < gridSize.width; tileX++)
						{
							if (((tileX + tileY) % 2) == 0)
							{
								rec tileBounds = NewRec(0, 0, ToVec2(tileSize) * scale);
								tileBounds.topLeft = levelBounds.topLeft + Vec2Multiply(tileBounds.size, NewVec2((r32)tileX, (r32)tileY));
								RcDrawRectangle(tileBounds, ColorTransparent(White, 0.5f));
							}
						}
					}
				}
				
				VarArrayLoop(&layerInstance->tiles.tileInstances, iIndex)
				{
					VarArrayLoopGet(LdtkTileInstance_t, tileInstance, &layerInstance->tiles.tileInstances, iIndex);
					NotNull(tileInstance->tilePntr);
					LdtkTilesetTile_t* tileDef = tileInstance->tilePntr;
					NotNull(tilesetDef->spriteSheetPntr);
					AccessResource(tilesetDef->spriteSheetPntr);
					v2i frame = NewVec2i((i32)(tileDef->id % tilesetDef->gridSize.width), (i32)(tileDef->id / tilesetDef->gridSize.width));
					rec tileBounds = NewRec(levelBounds.topLeft + (ToVec2(tileInstance->position) * scale), ToVec2(tileSize) * scale);
					RcBindSpriteSheet(tilesetDef->spriteSheetPntr);
					RcDrawSheetFrame(frame, tileBounds, White);
				}
			}
		}
	}
	
	// +==============================+
	// |   Render Entity Instances    |
	// +==============================+
	if (drawEntities)
	{
		VarArrayLoop(&level->layerInstances, lIndex)
		{
			VarArrayLoopGet(LdtkLayerInstance_t, layerInstance, &level->layerInstances, lIndex);
			if (layerInstance->type == LdtkLayerType_Entities)
			{
				VarArrayLoop(&layerInstance->entities.entityInstances, eIndex)
				{
					VarArrayLoopGet(LdtkEntityInstance_t, entityInstance, &layerInstance->entities.entityInstances, eIndex);
					NotNull(entityInstance->entityDefPntr);
					LdtkEntityDef_t* entityDef = entityInstance->entityDefPntr;
					rec entityBounds = RecScale(ToRec(entityInstance->bounds), scale);
					entityBounds.topLeft += levelBounds.topLeft;
					if (entityDef->usesTileIcon)
					{
						NotNull2(entityDef->iconTilesetPntr, entityDef->iconTilePntr);
						LdtkTilesetDef_t* tilesetDef = entityDef->iconTilesetPntr;
						LdtkTilesetTile_t* tileDef = entityDef->iconTilePntr;
						NotNull(tilesetDef->spriteSheetPntr);
						AccessResource(tilesetDef->spriteSheetPntr);
						v2i frame = NewVec2i((i32)(tileDef->id % tilesetDef->gridSize.width), (i32)(tileDef->id / tilesetDef->gridSize.width));
						RcBindSpriteSheet(tilesetDef->spriteSheetPntr);
						RcDrawSheetFrame(frame, entityBounds, White);
					}
					else
					{
						RcDrawRectangle(entityBounds, MonokaiPurple);
					}
				}
			}
		}
	}
}
#endif

// +--------------------------------------------------------------+
// |                       Box2D Rendering                        |
// +--------------------------------------------------------------+
#if BOX2D_SUPPORTED

void RcDrawPhysicsBody(PlatPhysicsBody_t* body, Color_t color, r32 textScale = 1.0f)
{
	PlatPhysicsBodyState_t bodyState;
	bool getStateSuccess = plat->GetPhysicsBodyState(body, &bodyState);
	Assert(getStateSuccess);
	
	if (body->bodyDef.shape == PlatPhysicsBodyShape_Rectangle)
	{
		obb2 bodyObb = NewObb2D(bodyState.position, body->bodyDef.rectangle.size, bodyState.rotation);
		RcDrawObb2(bodyObb, color);
	}
	else if (body->bodyDef.shape == PlatPhysicsBodyShape_Circle)
	{
		RcDrawCircle(bodyState.position, body->bodyDef.circle.radius, color);
	}
	//TODO: Implement polygon rendering!
	else
	{
		RcDrawPoint(bodyState.position, 4, color);
	}
	
	if (textScale != 0)
	{
		RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace(), textScale);
		RcDrawTextPrintEx(bodyState.position, ColorComplementary(color), TextAlignment_Center, 0, "%llu", body->id);
	}
}

#endif
