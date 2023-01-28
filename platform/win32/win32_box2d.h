/*
File:   win32_box2d.h
Author: Taylor Robbins
Date:   07\11\2022
*/

#ifndef _WIN_32_BOX_2D_H
#define _WIN_32_BOX_2D_H

#if BOX2D_SUPPORTED

#define B2_USER_SETTINGS
#include <Box2D/box2d.h>
#include "b2_user_settings.h"

struct Box2dPhysicsState_t
{
	bool initialized;
	b2World world;
	u64 nextBodyId;
	r32 scale;
	v2 gravity;
	BktArray_t bodies; //PlatPhysicsBody_t
};

#endif //BOX2D_SUPPORTED

#endif //  _WIN_32_BOX_2D_H
