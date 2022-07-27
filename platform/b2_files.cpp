/*
File:   b2_files.cpp
Author: Taylor Robbins
Date:   07\11\2022
Description: 
	** Includes all the files we need to compile Box2D straight into the windows platform layer
*/

#if BOX2D_SUPPORTED

#pragma warning(push)
#pragma warning(disable:4701) //potentially uninitialized local variable 'normal' used

#define B2_USER_SETTINGS

#include <box2d/box2d.h>

#include <Box2D/src/collision/b2_broad_phase.cpp>
#include <Box2D/src/collision/b2_collide_circle.cpp>
#include <Box2D/src/collision/b2_collide_edge.cpp>
#include <Box2D/src/collision/b2_collide_polygon.cpp>
#include <Box2D/src/collision/b2_collision.cpp>
#include <Box2D/src/collision/b2_distance.cpp>
#include <Box2D/src/collision/b2_dynamic_tree.cpp>
#include <Box2D/src/collision/b2_time_of_impact.cpp>

#include <Box2D/src/collision/b2_chain_shape.cpp>
#include <Box2D/src/collision/b2_circle_shape.cpp>
#include <Box2D/src/collision/b2_edge_shape.cpp>
#include <Box2D/src/collision/b2_polygon_shape.cpp>

#include <Box2D/src/common/b2_block_allocator.cpp>
#include <Box2D/src/common/b2_draw.cpp>
#include <Box2D/src/common/b2_math.cpp>
#include <Box2D/src/common/b2_settings.cpp>
#include <Box2D/src/common/b2_stack_allocator.cpp>
#include <Box2D/src/common/b2_timer.cpp>

#include <Box2D/src/dynamics/b2_body.cpp>
#include <Box2D/src/dynamics/b2_contact_manager.cpp>
#include <Box2D/src/dynamics/b2_fixture.cpp>
#include <Box2D/src/dynamics/b2_island.cpp>
#include <Box2D/src/dynamics/b2_world.cpp>
#include <Box2D/src/dynamics/b2_world_callbacks.cpp>

#include <Box2D/src/dynamics/b2_chain_circle_contact.cpp>
#include <Box2D/src/dynamics/b2_chain_polygon_contact.cpp>
#include <Box2D/src/dynamics/b2_circle_contact.cpp>
#include <Box2D/src/dynamics/b2_contact.cpp>
#include <Box2D/src/dynamics/b2_contact_solver.cpp>
#include <Box2D/src/dynamics/b2_edge_circle_contact.cpp>
#include <Box2D/src/dynamics/b2_edge_polygon_contact.cpp>
#include <Box2D/src/dynamics/b2_polygon_circle_contact.cpp>
#include <Box2D/src/dynamics/b2_polygon_contact.cpp>

#include <Box2D/src/dynamics/b2_distance_joint.cpp>
#include <Box2D/src/dynamics/b2_friction_joint.cpp>
#include <Box2D/src/dynamics/b2_gear_joint.cpp>
#include <Box2D/src/dynamics/b2_joint.cpp>
#include <Box2D/src/dynamics/b2_motor_joint.cpp>
#include <Box2D/src/dynamics/b2_mouse_joint.cpp>
#include <Box2D/src/dynamics/b2_prismatic_joint.cpp>
#include <Box2D/src/dynamics/b2_pulley_joint.cpp>
#include <Box2D/src/dynamics/b2_revolute_joint.cpp>
#include <Box2D/src/dynamics/b2_weld_joint.cpp>
#include <Box2D/src/dynamics/b2_wheel_joint.cpp>

#include <Box2D/src/rope/b2_rope.cpp>

#pragma warning(push)

#endif //BOX2D_SUPPORTED
