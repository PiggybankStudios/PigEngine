/*
File:   win32_box2d.cpp
Author: Taylor Robbins
Date:   07\11\2022
Description: 
	** Exposes an a set of functions for the engine to use to manage simulating 2D physics using Box2D
*/

#if BOX2D_SUPPORTED

inline void* b2Alloc(int32 size)
{
	return AllocMem(&Platform->box2dHeap, size);
}

// If you implement b2Alloc, you should also implement this function.
inline void b2Free(void* mem)
{
	FreeMem(&Platform->box2dHeap, mem);
}

// Implement this to use your own logging.
inline void b2Log(const char* string, ...)
{
	TempPushMark();
	TempPrintVa(outputString, outputLength, string);
	Win32_DebugOutputFromPlat(0x00, __FILE__, __LINE__, __func__, DbgLevel_Regular, false, outputString);
	TempPopMark();
}

// +--------------------------------------------------------------+
// |                    Platform API Functions                    |
// +--------------------------------------------------------------+
// +==============================+
// |   Win32_InitPhysicsEngine    |
// +==============================+
// void InitPhysicsEngine(v2 gravity, r32 scale)
PLAT_API_INIT_PHYSICS_ENGINE(Win32_InitPhysicsEngine)
{
	AssertSingleThreaded();
	ClearStruct(Platform->physics);
	Platform->physics.gravity = gravity;
	Platform->physics.scale = scale;
	Platform->physics.nextBodyId = 1;
	CreateBktArray(&Platform->physics.bodies, &Platform->box2dHeap, sizeof(PlatPhysicsBody_t));
	
	b2Vec2 gravityVec(gravity.x, gravity.y);
	new (&Platform->physics.world) b2World(gravityVec);
	
	Platform->physics.initialized = true;
}

// +==============================+
// |   Win32_DestroyPhysicsBody   |
// +==============================+
// void DestroyPhysicsBody(PlatPhysicsBody_t* body)
PLAT_API_DESTROY_PHYSICS_BODY(Win32_DestroyPhysicsBody)
{
	AssertSingleThreaded();
	Assert(Platform->physics.initialized);
	NotNull(body);
	Assert(IsPntrInBktArray(&Platform->physics.bodies, body));
	Assert(body->id != 0);
	Platform->physics.world.DestroyBody(body->bodyPntr);
	body->bodyPntr = nullptr;
	body->id = 0;
}

// +==============================+
// |  Win32_DestroyPhysicsEngine  |
// +==============================+
// void DestroyPhysicsEngine()
PLAT_API_DESTROY_PHYSICS_ENGINE(Win32_DestroyPhysicsEngine)
{
	AssertSingleThreaded();
	Assert(Platform->physics.initialized);
	for (u64 bIndex = 0; bIndex < Platform->physics.bodies.length; bIndex++)
	{
		PlatPhysicsBody_t* body = BktArrayGet(&Platform->physics.bodies, PlatPhysicsBody_t, bIndex);
		if (body->id != 0)
		{
			Win32_DestroyPhysicsBody(body);
		}
	}
	FreeBktArray(&Platform->physics.bodies);
	Platform->physics.world.~b2World();
	ClearStruct(Platform->physics);
}

// +==============================+
// |      Win32_PhysicsTick       |
// +==============================+
// void PhysicsTick(r64 stepTimeMs, i32 numVelocityIterations, i32 numPositionIterations)
PLAT_API_PHYSICS_TICK(Win32_PhysicsTick)
{
	Assert(Platform->physics.initialized);
	r32 physTimeStep = (1.0f / 60.0f) * (r32)(stepTimeMs / (1000/60.0f)); //60Hz
	Platform->physics.world.Step(physTimeStep, numVelocityIterations, numPositionIterations); //8 velocity iterations, 3 position iterations
}

PlatPhysicsBody_t* Win32_AllocatePhysicsBody(bool allocateIfNeeded = true)
{
	for (u64 bIndex = 0; bIndex < Platform->physics.bodies.length; bIndex++)
	{
		PlatPhysicsBody_t* body = BktArrayGet(&Platform->physics.bodies, PlatPhysicsBody_t, bIndex);
		if (body->id == 0) { return body; }
	}
	if (allocateIfNeeded)
	{
		PlatPhysicsBody_t* newBody = BktArrayAdd(&Platform->physics.bodies, PlatPhysicsBody_t);
		NotNull(newBody);
		return newBody;
	}
	else { return nullptr; }
}

// +==============================+
// |   Win32_CreatePhysicsBody    |
// +==============================+
// PlatPhysicsBody_t* CreatePhysicsBody(const PlatPhysicsBodyDef_t* definition)
PLAT_API_CREATE_PHYSICS_BODY(Win32_CreatePhysicsBody)
{
	AssertSingleThreaded();
	Assert(Platform->physics.initialized);
	NotNull(definition);
	Assert(definition->shape != PlatPhysicsBodyShape_None);
	Assert(definition->shape < PlatPhysicsBodyShape_NumShapes);
	r32 scale = Platform->physics.scale;
	
	PlatPhysicsBody_t* result = Win32_AllocatePhysicsBody();
	if (result == nullptr) { return nullptr; }
	ClearPointer(result);
	result->id = Platform->physics.nextBodyId;
	Platform->physics.nextBodyId++;
	
	rec polygonBounds = Rec_Zero;
	if (definition->shape == PlatPhysicsBodyShape_Polygon)
	{
		for (u64 vIndex = 0; vIndex < definition->numPolygonVerts; vIndex++)
		{
			if (vIndex == 0) { polygonBounds = NewRec(definition->polygonVerts[vIndex] / scale, 0, 0); }
			else { polygonBounds = RecExpandToVec2(polygonBounds, definition->polygonVerts[vIndex] / scale); }
		}
	}
	
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	if (definition->type == PlatPhysicsBodyType_Static)         { bodyDef.type = b2_staticBody;    }
	else if (definition->type == PlatPhysicsBodyType_Dynamic)   { bodyDef.type = b2_dynamicBody;   }
	else if (definition->type == PlatPhysicsBodyType_Kinematic) { bodyDef.type = b2_kinematicBody; }
	else if (definition->type == PlatPhysicsBodyType_Sensor)    { bodyDef.type = b2_staticBody;    }
	else { Unimplemented(); }
	
	b2CircleShape circleShape;
	b2PolygonShape polygonShape;
	if (definition->shape == PlatPhysicsBodyShape_Circle)
	{
		bodyDef.position.Set(definition->circle.center.x / scale, definition->circle.center.y / scale);
		circleShape.m_radius = definition->circle.radius / scale;
	}
	else if (definition->shape == PlatPhysicsBodyShape_Rectangle)
	{
		bodyDef.position.Set((definition->rectangle.x + definition->rectangle.width/2) / scale, (definition->rectangle.y + definition->rectangle.height/2) / scale);
		polygonShape.SetAsBox((definition->rectangle.width/2) / scale, (definition->rectangle.height/2) / scale);
	}
	else if (definition->shape == PlatPhysicsBodyShape_Polygon)
	{
		bodyDef.position.Set(polygonBounds.x, polygonBounds.y);
		b2Vec2* b2PolygonVerts = AllocArray(TempArena, b2Vec2, definition->numPolygonVerts);
		NotNull(b2PolygonVerts);
		for (u64 vIndex = 0; vIndex < definition->numPolygonVerts; vIndex++) { b2PolygonVerts[vIndex] = b2Vec2((definition->polygonVerts[vIndex].x / scale) - polygonBounds.x, (definition->polygonVerts[vIndex].y / scale) - polygonBounds.y); }
		polygonShape.Set(b2PolygonVerts, (i32)definition->numPolygonVerts);
	}
	else { Unimplemented(); }
	
	result->bodyPntr = Platform->physics.world.CreateBody(&bodyDef);
	NotNull(result->bodyPntr);
	
	b2FixtureDef fixtureDef;
	if (definition->shape == PlatPhysicsBodyShape_Circle)    { fixtureDef.shape = &circleShape;  }
	else if (definition->shape == PlatPhysicsBodyShape_Rectangle) { fixtureDef.shape = &polygonShape; }
	else if (definition->shape == PlatPhysicsBodyShape_Polygon)   { fixtureDef.shape = &polygonShape; }
	else { Unimplemented(); }
	fixtureDef.friction = definition->friction;
	fixtureDef.restitution = definition->restitution;
	fixtureDef.density = definition->density;
	fixtureDef.filter.categoryBits = definition->colCategoryBits;
	fixtureDef.filter.maskBits = definition->colMaskBits;
	fixtureDef.filter.groupIndex = definition->groupIndex;
	
	result->bodyPntr->CreateFixture(&fixtureDef);
	
	MyMemCopy(&result->bodyDef, definition, sizeof(PlatPhysicsBodyDef_t));
	if (definition->shape == PlatPhysicsBodyShape_Polygon)
	{
		result->bodyDef.polygonVerts = AllocArray(&Platform->box2dHeap, v2, definition->numPolygonVerts);
		NotNull(result->bodyDef.polygonVerts);
		MyMemCopy(result->bodyDef.polygonVerts, definition->polygonVerts, sizeof(v2) * definition->numPolygonVerts);
	}
	
	return result;
}

// +==============================+
// | Win32_SetPhysicsBodyVelocity |
// +==============================+
// void SetPhysicsBodyVelocity(PlatPhysicsBody_t* body, v2 velocity, r32 angularVelocity)
PLAT_API_SET_PHYSICS_BODY_VELOCITY(Win32_SetPhysicsBodyVelocity)
{
	Assert(Platform->physics.initialized);
	NotNull(body);
	NotNull(body->bodyPntr);
	r32 scale = Platform->physics.scale;
	body->bodyPntr->SetLinearVelocity(b2Vec2(velocity.x / scale, velocity.y / scale));
	body->bodyPntr->SetAngularVelocity(angularVelocity);
}

// +==============================+
// |  Win32_GetPhysicsBodyState   |
// +==============================+
// bool GetPhysicsBodyState(const PlatPhysicsBody_t* body, PlatPhysicsBodyState_t* stateOut)
PLAT_API_GET_PHYSICS_BODY_STATE(Win32_GetPhysicsBodyState)
{
	Assert(Platform->physics.initialized);
	NotNull2(body, stateOut);
	if (body->id == 0 || body->bodyPntr == nullptr) { return false; }
	r32 scale = Platform->physics.scale;
	b2Vec2 position = body->bodyPntr->GetPosition();
	b2Vec2 velocity = body->bodyPntr->GetLinearVelocity();
	stateOut->position = NewVec2(position.x * scale, position.y * scale);
	stateOut->velocity = NewVec2(velocity.x * scale, velocity.y * scale);
	stateOut->rotation = body->bodyPntr->GetAngle();
	stateOut->angularVelocity = body->bodyPntr->GetAngularVelocity();
	return true;
}

#endif //BOX2D_SUPPORTED
