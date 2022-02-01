/*
File:   pig_notification_macros.cpp
Author: Taylor Robbins
Date:   01\16\2022
Description: 
	** Holds a bunch of macros that allow us to easily fire off notifications
*/

#define PIG_NOTIFICATIONS_DEFAULT_LIFESPAN  10000 //ms

PigNotification_t* PigPushNotification(PigNotificationQueue_t* queue, const char* filePath, u32 fileLine, const char* functionName, DbgLevel_t dbgLevel, u64 lifespan, const char* message);
PigNotification_t* PigPushNotificationPrint(PigNotificationQueue_t* queue, const char* filePath, u32 fileLine, const char* functionName, DbgLevel_t dbgLevel, u64 lifespan, const char* formatString, ...);

#define NotifyWrite_D(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Debug, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_D(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Debug, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Dx(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Debug, (lifespan), message)
#define NotifyPrint_Dx(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Debug, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_R(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Regular, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_R(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Regular, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Rx(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Regular, (lifespan), message)
#define NotifyPrint_Rx(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Regular, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_I(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Info, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_I(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Info, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Ix(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Info, (lifespan), message)
#define NotifyPrint_Ix(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Info, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_N(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Notify, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_N(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Notify, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Nx(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Notify, (lifespan), message)
#define NotifyPrint_Nx(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Notify, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_O(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Other, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_O(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Other, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Ox(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Other, (lifespan), message)
#define NotifyPrint_Ox(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Other, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_W(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Warning, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_W(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Warning, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Wx(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Warning, (lifespan), message)
#define NotifyPrint_Wx(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Warning, (lifespan), formatString, ##__VA_ARGS__)

#define NotifyWrite_E(message)                      PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Error, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, message)
#define NotifyPrint_E(formatString, ...)            PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Error, PIG_NOTIFICATIONS_DEFAULT_LIFESPAN, formatString, ##__VA_ARGS__)
#define NotifyWrite_Ex(lifespan, message)           PigPushNotification     (&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Error, (lifespan), message)
#define NotifyPrint_Ex(lifespan, formatString, ...) PigPushNotificationPrint(&pig->notificationsQueue, __FILE__, __LINE__, __func__, DbgLevel_Error, (lifespan), formatString, ##__VA_ARGS__)
