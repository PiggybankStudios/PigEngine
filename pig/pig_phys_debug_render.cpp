/*
File:   pig_phys_debug_render.cpp
Author: Taylor Robbins
Date:   01\07\2022
Description: 
	** Holds the implementations for MyPhysRenderer_c class that helps us rander qu3e scenes
*/

void MyPhysRenderer_c::SetPenColor(f32 r, f32 g, f32 b, f32 a)
{
	drawColor = ToColor(NewColorf(r, g, b, a));
}

void MyPhysRenderer_c::SetPenPosition(f32 x, f32 y, f32 z)
{
	drawPos = NewVec3(x, y, z);
}

void MyPhysRenderer_c::SetScale(f32 sx, f32 sy, f32 sz)
{
	drawScale = NewVec3(sx, sy, sz);
}

void MyPhysRenderer_c::SetTriNormal(f32 x, f32 y, f32 z)
{
	drawNormal = NewVec3(x, y, z);
}

void MyPhysRenderer_c::Point()
{
	RcDrawBox(NewBoxCentered(drawPos, NewVec3(0.1f, 0.1f, 0.1f)), drawColor);
};

void MyPhysRenderer_c::Line(f32 x, f32 y, f32 z)
{
	v3 endPos = NewVec3(x, y, z);
	RcDrawLine3D(drawPos, endPos, 1, drawColor);
	drawPos = endPos;
}

void MyPhysRenderer_c::Triangle(f32 x1, f32 y1, f32 z1, f32 x2, f32 y2, f32 z2, f32 x3, f32 y3, f32 z3)
{
	RcBindTexture1(&rc->invalidTexture);
	RcDrawTexturedTriangle(
		NewVec3(x1, y1, z1), NewVec3(x3, y3, z3), NewVec3(x2, y2, z2),
		NewVec2(0, 0), NewVec2(1, 0), NewVec2(0, 1),
		ColorLerp(White, drawColor, 0.2f)
	);
}
