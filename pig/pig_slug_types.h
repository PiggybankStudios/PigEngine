/*
File:   pig_slug_types.h
Author: Taylor Robbins
Date:   06\03\2022
Description:
	** Holds a bunch of conversion functions to go to and from Slug library types and our own types
*/

#ifndef _PIG_SLUG_TYPES_H
#define _PIG_SLUG_TYPES_H

#if SLUG_SUPPORTED

//TODO: Add these types?
// Matrix2D
// Bivector3D
// QuadraticBezier2D
// Vertex
// MeshVertex
// Triangle

#define VertexType_Slug_Size sizeof(Terathon::Slug::Vertex)

#define SlugInteger2D_Zero Terathon::Integer2D{ 0, 0 }
#define SlugInteger4D_Zero Terathon::Integer4D{ 0, 0, 0, 0 }
#define SlugVector2D_Zero  Terathon::Vector2D{ 0.0f, 0.0f }
#define SlugVector4D_Zero  Terathon::Vector4D{ 0.0f, 0.0f, 0.0f, 0.0f }
#define SlugPoint2D_Zero   Terathon::Point2D{ 0.0f, 0.0f }
#define SlugPoint3D_Zero   Terathon::Point3D{ 0.0f, 0.0f, 0.0f }
#define SlugBox2D_Zero     Terathon::Box2D{ 0.0f, 0.0f, 0.0f, 0.0f }
#define SlugColor4U_White  Terathon::Color4U{ 255, 255, 255, 255 }

v2i ToVec2i(Terathon::Integer2D slugInteger2d)
{
	return NewVec2i(slugInteger2d.x, slugInteger2d.y);
}
Terathon::Integer2D ToSlugInteger2D(v2i vector)
{
	return Terathon::Integer2D{ vector.x, vector.y };
}

v4i ToVec4i(Terathon::Integer4D slugInteger4d)
{
	return NewVec4i(slugInteger4d.x, slugInteger4d.y, slugInteger4d.z, slugInteger4d.w);
}
Terathon::Integer4D ToSlugInteger4D(v4i vector)
{
	return Terathon::Integer4D{ vector.x, vector.y, vector.z, vector.w };
}

v2 ToVec2(Terathon::Vector2D slugVector2d)
{
	return NewVec2(slugVector2d.x, slugVector2d.y);
}
Terathon::Vector2D ToSlugVector2D(v2 vector)
{
	return Terathon::Vector2D{ vector.x, vector.y };
}

v4 ToVec4(Terathon::Vector4D slugVector4d)
{
	return NewVec4(slugVector4d.x, slugVector4d.y, slugVector4d.z, slugVector4d.w);
}
Terathon::Vector4D ToSlugVector4D(v4 vector)
{
	return Terathon::Vector4D{ vector.x, vector.y, vector.z, vector.w };
}

v2 ToVec2(Terathon::Point2D slugPoint2d)
{
	return NewVec2(slugPoint2d.x, slugPoint2d.y);
}
Terathon::Point2D ToSlugPoint2D(v2 vector)
{
	return Terathon::Point2D{ vector.x, vector.y };
}

v3 ToVec3(Terathon::Point3D slugPoint3d)
{
	return NewVec3(slugPoint3d.x, slugPoint3d.y, slugPoint3d.z);
}
Terathon::Point3D ToSlugPoint3D(v3 vector)
{
	return Terathon::Point3D{ vector.x, vector.y, vector.z };
}

rec ToRec(Terathon::Box2D slugBox2d)
{
	return NewRec(slugBox2d.min.x, slugBox2d.min.y, slugBox2d.max.x - slugBox2d.min.x, slugBox2d.max.y - slugBox2d.min.y);
}
Terathon::Box2D ToSlugBox2D(rec rectangle)
{
	return Terathon::Box2D{ rectangle.x, rectangle.y, rectangle.x + rectangle.width, rectangle.y + rectangle.height };
}

Color_t ToColor(Terathon::Color4U slugColor4u)
{
	return NewColor(slugColor4u.red, slugColor4u.green, slugColor4u.blue, slugColor4u.alpha);
}
Terathon::Color4U ToSlugColor4U(Color_t color)
{
	// Terathon::Color4U result = { color.r, color.g, color.b, color.a };
	return Terathon::Color4U{ color.r, color.g, color.b, color.a };
}

//NOTE: Actually is Terathon::Demo::Matrix4D so we don't have it unless we want to pull it in manually
// mat4 ToMat4(Terathon::Matrix4D slugMatrix4d)
// {
// 	return NewMat4(
// 		slugMatrix4d(0, 0), slugMatrix4d(0, 1), slugMatrix4d(0, 2), slugMatrix4d(0, 3),
// 		slugMatrix4d(1, 0), slugMatrix4d(1, 1), slugMatrix4d(1, 2), slugMatrix4d(1, 3),
// 		slugMatrix4d(2, 0), slugMatrix4d(2, 1), slugMatrix4d(2, 2), slugMatrix4d(2, 3),
// 		slugMatrix4d(3, 0), slugMatrix4d(3, 1), slugMatrix4d(3, 2), slugMatrix4d(3, 3)
// 	);
// }

#endif //SLUG_SUPPORTED

#endif //  _PIG_SLUG_TYPES_H
