/*
File:   pig_slug_render.cpp
Author: Taylor Robbins
Date:   06\09\2022
Description: 
	** Holds functions that help us render Slug related things
*/

void RenderSlugText(const SlugText_t* text, v2 position)
{
	RcBindShader(&text->font->pipelineShader);
	// TODO: glBlendFuncSeparate?
	// TODO: glBlendEquation?
	RcSetWorldMatrix(Mat4Translate2(position));
	RcBindTexture1(&text->font->curveTexture);
	RcBindTexture2(&text->font->bandTexture);
	RcBindTexture2(&text->font->bandTexture);
	RcBindVertBuffer(&text->vertBuffer);
	RcDrawBuffer(VertBufferPrimitive_Triangles);
}
