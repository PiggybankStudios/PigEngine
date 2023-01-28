/*
File:   b2_user_settings.h
Author: Taylor Robbins
Date:   07\11\2022
Description:
	** #included by Box2D source files and allows us to redirect things like b2Alloc and b2Log
*/

#ifndef _B2_USER_SETTINGS_H
#define _B2_USER_SETTINGS_H

// +--------------------------------------------------------------+
// |                      Tunable Constants                       |
// +--------------------------------------------------------------+
// You can use this to change the length scale used by your game.
// For example for inches you could use 39.4.
#define b2_lengthUnitsPerMeter 1.0f

// The maximum number of vertices on a convex polygon. You cannot increase
// this too much because b2BlockAllocator has a maximum object size.
#define b2_maxPolygonVertices	8

// +==============================+
// |          User data           |
// +==============================+
// You can define this to inject whatever data you want in b2Body
struct B2_API b2BodyUserData
{
	b2BodyUserData()
	{
		pointer = 0;
	}
	
	// For legacy compatibility
	uintptr_t pointer;
};

// You can define this to inject whatever data you want in b2Fixture
struct B2_API b2FixtureUserData
{
	b2FixtureUserData()
	{
		pointer = 0;
	}
	
	// For legacy compatibility
	uintptr_t pointer;
};

// You can define this to inject whatever data you want in b2Joint
struct B2_API b2JointUserData
{
	b2JointUserData()
	{
		pointer = 0;
	}
	
	// For legacy compatibility
	uintptr_t pointer;
};

// +--------------------------------------------------------------+
// |               Allocation and Logging Functions               |
// +--------------------------------------------------------------+
inline void* b2Alloc(int32 size);
inline void b2Free(void* mem);
inline void b2Log(const char* string, ...);

#endif //  _B2_USER_SETTINGS_H
