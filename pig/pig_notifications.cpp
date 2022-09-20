/*
File:   pig_notifications.cpp
Author: Taylor Robbins
Date:   01\16\2022
Description: 
	** Holds functions that help us pop up little notifications with text that
	** give the user some information (often errors of some kind)
*/

#define PIG_NOTIFICATIONS_ALPHA             0.7f
#define PIG_NOTIFICATIONS_SHOW_ANIM_TIME    300 //ms
#define PIG_NOTIFICATIONS_VOLUME            0.1f

#define PIG_NOTIFICATIONS_MAX_WIDTH                 400 //px
#define PIG_NOTIFICATIONS_MIN_HEIGHT                60 //px
#define PIG_NOTIFICATIONS_ROUNDED_RADIUS            12 //px
#define PIG_NOTIFICATIONS_PADDING_LR                30 //px
#define PIG_NOTIFICATIONS_PADDING_UD                10 //px
#define PIG_NOTIFICATIONS_MARGIN                    10 //px
#define PIG_NOTIFICATIONS_MARGIN_BETWEEN            4 //px
#define PIG_NOTIFICATIONS_REPEAT_CNT_PADDING        5 //px
#define PIG_NOTIFICATIONS_REPEAT_CNT_ROUNDED_RADIUS 10 //px

void FreePigNotification(PigNotification_t* notification)
{
	NotNull(notification);
	FreeString(notification->allocArena, &notification->message);
	FreeString(notification->allocArena, &notification->filePath);
	FreeString(notification->allocArena, &notification->funcName);
	ClearPointer(notification);
}

// +--------------------------------------------------------------+
// |                        Initialization                        |
// +--------------------------------------------------------------+
void PigInitNotifications(PigNotificationQueue_t* queue)
{
	NotNull(queue);
	ClearPointer(queue);
	queue->nextId = 1;
	queue->initialized = true;
}

// +--------------------------------------------------------------+
// |                       Helper Functions                       |
// +--------------------------------------------------------------+
PigNotification_t* PigAllocNotification(PigNotificationQueue_t* queue, MemArena_t* memArena, DbgLevel_t dbgLevel, const char* message)
{
	NotNull(queue);
	NotNull(memArena);
	if (!queue->initialized) { return nullptr; }
	
	for (u64 nIndex = 0; nIndex < PIG_MAX_NUM_NOTIFICATIONS; nIndex++)
	{
		PigNotification_t* notification = &queue->notifications[nIndex];
		if (notification->alive && notification->dbgLevel == dbgLevel && StrEquals(notification->message, message))
		{
			IncrementU64(notification->repeatCount);
			notification->appearTime = ProgramTime;
			return nullptr;
		}
	}
	
	if (queue->notifications[PIG_MAX_NUM_NOTIFICATIONS-1].alive)
	{
		FreePigNotification(&queue->notifications[PIG_MAX_NUM_NOTIFICATIONS-1]);
	}
	//shift all notifications up
	for (u64 nIndex = PIG_MAX_NUM_NOTIFICATIONS-1; nIndex > 0; nIndex--)
	{
		MyMemCopy(&queue->notifications[nIndex], &queue->notifications[nIndex-1], sizeof(PigNotification_t));
	}
	PigNotification_t* result = &queue->notifications[0];
	ClearPointer(result);
	result->id = queue->nextId;
	queue->nextId++;
	result->allocArena = memArena;
	result->alive = true;
	result->showing = true;
	result->showAnimTime = 0.0f;
	if (message != nullptr) { result->message = NewStringInArenaNt(memArena, message); }
	result->dbgLevel = dbgLevel;
	result->appearTime = ProgramTime;
	return result;
}
PigNotification_t* PigPushNotification(PigNotificationQueue_t* queue, const char* filePath, u32 fileLine, const char* functionName, DbgLevel_t dbgLevel, u64 lifespan, const char* message) //pre-declared in pig_notification_macros.cpp
{
	NotNull(queue);
	NotNull(message);
	if (!queue->initialized) { return nullptr; }
	
	AppDebugOutput(DbgFlag_Inverted, filePath, fileLine, functionName, dbgLevel, true, message);
	
	PigNotification_t* notification = PigAllocNotification(queue, fixedHeap, dbgLevel, message);
	if (notification != nullptr)
	{
		if (filePath != nullptr)
		{
			MyStr_t fileName = GetFileNamePart(NewStr(filePath));
			notification->filePath = AllocString(notification->allocArena, &fileName);
		}
		if (functionName != nullptr)
		{
			notification->funcName = NewStringInArenaNt(notification->allocArena, functionName);
		}
		notification->fileLineNumber = fileLine;
		notification->repeatCount = 1;
		notification->lifespan = lifespan;
	}
	
	PlaySound(&pig->resources.sounds->notification, 1.0f);
	// PlaySawNote(PIG_NOTIFICATIONS_VOLUME*0.75f, FREQUENCY_C3, 200, 80, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut);
	// PlaySawNote(PIG_NOTIFICATIONS_VOLUME, FREQUENCY_E4, 280, 160, EasingStyle_BackOut, 100, EasingStyle_QuadraticInOut);
	
	return notification;
}
PigNotification_t* PigPushNotificationPrint(PigNotificationQueue_t* queue, const char* filePath, u32 fileLine, const char* functionName, DbgLevel_t dbgLevel, u64 lifespan, const char* formatString, ...) //pre-declared in pig_notification_macros.cpp
{
	NotNull(queue);
	if (!queue->initialized) { return nullptr; }
	PigNotification_t* result = nullptr;
	
	TempPushMark();
	TempPrintVa(message, messageLength, formatString);
	if (message != nullptr)
	{
		result = PigPushNotification(queue, filePath, fileLine, functionName, dbgLevel, lifespan, message);
	}
	else
	{
		result = PigPushNotification(queue, filePath, fileLine, functionName, dbgLevel, lifespan, formatString);
	}
	TempPopMark();
	
	return result;
}

// +--------------------------------------------------------------+
// |                   Layout and CaptureMouse                    |
// +--------------------------------------------------------------+
void PigNotificationsLayout(PigNotificationQueue_t* queue)
{
	NotNull(queue);
	RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
	
	queue->notificationsSize = Vec2_Zero;
	for (u64 nIndex = 0; nIndex < PIG_MAX_NUM_NOTIFICATIONS; nIndex++)
	{
		PigNotification_t* notification = &queue->notifications[nIndex];
		if (notification->alive)
		{
			if (notification->message.pntr != nullptr)
			{
				TextMeasure_t textMeasure = RcMeasureText(notification->message, PIG_NOTIFICATIONS_MAX_WIDTH);
				notification->textSize = textMeasure.size;
				notification->textOffset = NewVec2(PIG_NOTIFICATIONS_PADDING_LR, PIG_NOTIFICATIONS_PADDING_UD) + textMeasure.offset;
			}
			else
			{
				notification->textSize = Vec2_Zero;
				notification->textOffset = Vec2_Zero;
			}
			
			if (notification->repeatCount > 1)
			{
				TempPushMark();
				TextMeasure_t textMeasure = RcMeasureText(TempPrintStr("\bx%llu\b", notification->repeatCount));
				notification->repeatCountTextSize = textMeasure.size;
				notification->repeatCountTextOffset = Vec2Fill(PIG_NOTIFICATIONS_REPEAT_CNT_PADDING) + textMeasure.offset;
				TempPopMark();
			}
			else
			{
				notification->repeatCountTextSize = Vec2_Zero;
				notification->repeatCountTextOffset = Vec2_Zero;
			}
			
			notification->mainRec.size = notification->textSize + NewVec2(PIG_NOTIFICATIONS_PADDING_LR*2, PIG_NOTIFICATIONS_PADDING_UD*2);
			if (notification->mainRec.height < PIG_NOTIFICATIONS_MIN_HEIGHT)
			{
				r32 heightDiff = PIG_NOTIFICATIONS_MIN_HEIGHT - notification->mainRec.height;
				notification->textOffset.y += heightDiff/2;
				notification->mainRec.height += heightDiff;
			}
			notification->mainRec.x = -notification->mainRec.width * EaseQuadraticIn(notification->showAnimTime);
			notification->mainRec.y = -queue->notificationsSize.height - notification->mainRec.height;
			RecAlign(&notification->mainRec);
			
			if (notification->repeatCount > 1)
			{
				notification->repeatCountRec.size = notification->repeatCountTextSize + Vec2Fill(PIG_NOTIFICATIONS_REPEAT_CNT_PADDING*2);
				RecLayoutLeftOf(&notification->repeatCountRec, notification->mainRec.x + notification->mainRec.width, 3);
				notification->repeatCountRec.y = notification->mainRec.y - 3;
				RecAlign(&notification->repeatCountRec);
			}
			
			//TODO: layout the exit btn rec
			
			r32 effectiveHeight = (notification->mainRec.height + PIG_NOTIFICATIONS_MARGIN_BETWEEN) * EaseQuadraticIn(notification->showAnimTime);
			queue->notificationsSize.height += effectiveHeight;
			if (queue->notificationsSize.width < -notification->mainRec.x) { queue->notificationsSize.width = -notification->mainRec.x; }
		}
	}
	queue->mainRec.size = queue->notificationsSize;
	RecLayoutLeftOf(&queue->mainRec, ScreenSize.width, PIG_NOTIFICATIONS_MARGIN);
	RecLayoutTopOf(&queue->mainRec, ScreenSize.height, PIG_NOTIFICATIONS_MARGIN);
	RecAlign(&queue->mainRec);
}

void PigNotificationsCaptureMouse(PigNotificationQueue_t* queue)
{
	NotNull(queue);
	PigNotificationsLayout(queue);
	
	v2 notificationsBasePos = queue->mainRec.topLeft + queue->mainRec.size;
	for (u64 nIndex = 0; nIndex < PIG_MAX_NUM_NOTIFICATIONS; nIndex++)
	{
		PigNotification_t* notification = &queue->notifications[nIndex];
		if (notification->alive)
		{
			MouseHitRecPrint(notification->mainRec + notificationsBasePos, "Notification%llu", nIndex);
		}
	}
}

// +--------------------------------------------------------------+
// |                            Update                            |
// +--------------------------------------------------------------+
void PigUpdateNotifications(PigNotificationQueue_t* queue)
{
	NotNull(queue);
	PigNotificationsLayout(queue);
	
	v2 notificationsBasePos = queue->mainRec.topLeft + queue->mainRec.size;
	for (u64 nIndex = 0; nIndex < PIG_MAX_NUM_NOTIFICATIONS; nIndex++)
	{
		PigNotification_t* notification = &queue->notifications[nIndex];
		if (notification->alive)
		{
			// +==============================+
			// |    Update Show Anim Time     |
			// +==============================+
			if (notification->showing) { UpdateAnimationUp(&notification->showAnimTime, PIG_NOTIFICATIONS_SHOW_ANIM_TIME); }
			else
			{
				if (UpdateAnimationDown(&notification->showAnimTime, PIG_NOTIFICATIONS_SHOW_ANIM_TIME))
				{
					FreePigNotification(notification);
					notification->alive = false;
					//Shift the notifications above this down
					for (u64 nIndex2 = nIndex; nIndex2+1 < PIG_MAX_NUM_NOTIFICATIONS; nIndex2++)
					{
						MyMemCopy(&queue->notifications[nIndex2], &queue->notifications[nIndex2+1], sizeof(PigNotification_t));
					}
					ClearStruct(queue->notifications[PIG_MAX_NUM_NOTIFICATIONS-1]);
					nIndex--;
					continue;
				}
			}
			
			if (IsMouseOverPrint("Notification%llu", nIndex))
			{
				pigOut->cursorType = PlatCursor_Pointer;
				//TODO: Should we do something on click?
			}
			
			if (notification->showing && TimeSince(notification->appearTime) >= notification->lifespan && !IsMouseOverNamedPartial("Notification"))
			{
				notification->showing = false;
			}
		}
	}
}

// +--------------------------------------------------------------+
// |                            Render                            |
// +--------------------------------------------------------------+
void PigRenderNotifications(PigNotificationQueue_t* queue)
{
	NotNull(queue);
	PigNotificationsLayout(queue);
	
	RcBindFont(&pig->resources.fonts->debug, SelectDefaultFontFace());
	
	// RcDrawRectangleOutline(queue->mainRec, White, 1, true);
	
	v2 notificationsBasePos = queue->mainRec.topLeft + queue->mainRec.size;
	for (u64 nIndex = 0; nIndex < PIG_MAX_NUM_NOTIFICATIONS; nIndex++)
	{
		PigNotification_t* notification = &queue->notifications[nIndex];
		if (notification->alive)
		{
			rec mainRec = notification->mainRec + notificationsBasePos;
			v2 textPos = mainRec.topLeft + notification->textOffset;
			Vec2Align(&textPos);
			
			// RcSetDepth(0.01f);
			RcDrawRoundedRectangle(mainRec, PIG_NOTIFICATIONS_ROUNDED_RADIUS, ColorTransparent(Black, PIG_NOTIFICATIONS_ALPHA));
			RcDrawText(notification->message, textPos, GetDbgLevelTextColor(notification->dbgLevel), TextAlignment_Left, PIG_NOTIFICATIONS_MAX_WIDTH);
			
			if (notification->repeatCount > 1)
			{
				// RcSetDepth(0.00f);
				rec repeatCountRec = notification->repeatCountRec + notificationsBasePos;
				v2 repeatCountTextPos = repeatCountRec.topLeft + notification->repeatCountTextOffset;
				Vec2Align(&repeatCountTextPos);
				RcDrawRoundedRectangle(repeatCountRec, PIG_NOTIFICATIONS_REPEAT_CNT_ROUNDED_RADIUS, MonokaiRed);
				RcDrawTextPrint(repeatCountTextPos, MonokaiWhite, "\bx%llu\b", notification->repeatCount);
			}
		}
	}
	// RcSetDepth(0);
}