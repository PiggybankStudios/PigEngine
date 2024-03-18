/*
File:   pig_render_funcs_imgui.cpp
Author: Taylor Robbins
Date:   03\01\2024
Description: 
	** Holds the function that actually renders ImDrawData from Dear ImGui (using our Rc functions)
*/

void RcRenderImDrawData(ImDrawData* imDrawData) //pre-declared in pig_func_defs.h
{
	NotNull(imDrawData);
	MemArena_t* scratch = GetScratchArena();
	
	bool oldFaceCullingValue = rc->state.faceCulling;
	rec oldViewport = rc->state.viewportRec;
	Shader_t* oldShader = rc->state.boundShader;
	RcSetFaceCulling(false);
	// TODO: Rather than one vertex buffer that we resize and shove everything into, we could be using GL_STREAM_DRAW maybe, and just put each CmdList's data into the buffer as we go?
	
	// +===============================+
	// | Resize VertBuffer (if needed) |
	// +===============================+
	if (!pig->imgui.vertBuffer.isValid ||
		pig->imgui.vertBuffer.numVertices < imDrawData->TotalVtxCount ||
		pig->imgui.vertBuffer.numIndices < imDrawData->TotalIdxCount)
	{
		if (pig->imgui.vertBuffer.isValid) { DestroyVertBuffer(&pig->imgui.vertBuffer); }
		Assert(sizeof(ImDrawIdx) == sizeof(i16));
		bool createBufferSuccess = CreateVertBuffer2DWithIndicesI16(
			mainHeap,
			&pig->imgui.vertBuffer,
			true,
			imDrawData->TotalVtxCount,
			nullptr,
			imDrawData->TotalIdxCount,
			nullptr,
			false, //copyVertices
			false //copyIndices
		);
		Assert(createBufferSuccess);
	}
	
	// +========================================+
	// | Fill VertBuffer with Vertices/Indices  |
	// +========================================+
	u64 bufferVtxOffset = 0;
	u64 bufferIdxOffset = 0;
	for (int lIndex = 0; lIndex < imDrawData->CmdListsCount; lIndex++)
	{
		const ImDrawList* imCmdList = imDrawData->CmdLists[lIndex];
		if (imCmdList->VtxBuffer.Size > 0)
		{
			PushMemMark(scratch);
			//TODO: Currently we have to convert all the vertices into our normal Vertex2D_t format. We should add support for ImGui style vertices being directly rendered!
			Vertex2D_t* convertedVertices = AllocArray(scratch, Vertex2D_t, imCmdList->VtxBuffer.Size);
			for (u64 vIndex = 0; vIndex < imCmdList->VtxBuffer.Size; vIndex++)
			{
				ImDrawVert* imVert = &imCmdList->VtxBuffer.Data[vIndex];
				convertedVertices[vIndex].position = NewVec3(imVert->pos.x, imVert->pos.y, 0.0f);
				u32 fixedImguiColor = ((imVert->col & 0xFF00FF00) |
					((imVert->col & 0x00FF0000) >> 16) |
					((imVert->col & 0x000000FF) << 16)
				);
				convertedVertices[vIndex].color = ToVec4(NewColor(fixedImguiColor));
				convertedVertices[vIndex].texCoord = NewVec2(imVert->uv.x, imVert->uv.y);
			}
			bool vertUploadSuccess = ChangeVertBufferVertices2D(
				&pig->imgui.vertBuffer,
				bufferVtxOffset,
				imCmdList->VtxBuffer.Size,
				convertedVertices
			);
			Assert(vertUploadSuccess);
			PopMemMark(scratch);
		}
		
		bufferVtxOffset += imCmdList->VtxBuffer.Size;
		bool indexUploadSuccess = ChangeVertBufferIndicesI16(
			&pig->imgui.vertBuffer,
			bufferIdxOffset,
			imCmdList->IdxBuffer.Size,
			imCmdList->IdxBuffer.Data
		);
		Assert(indexUploadSuccess);
		bufferIdxOffset += imCmdList->IdxBuffer.Size;
	}
	
	// +==============================+
	// |    Execute Imgui Commands    |
	// +==============================+
	RcBindShader(&pig->imgui.mainShader);
	RcBindVertBuffer(&pig->imgui.vertBuffer);
	u64 listVtxOffset = 0;
	u64 listIdxOffset = 0;
	for (int lIndex = 0; lIndex < imDrawData->CmdListsCount; lIndex++)
	{
		const ImDrawList* imCmdList = imDrawData->CmdLists[lIndex];
		for (int cIndex = 0; cIndex < imCmdList->CmdBuffer.Size; cIndex++)
		{
			const ImDrawCmd* imCmd = &imCmdList->CmdBuffer[cIndex];
			if (imCmd->UserCallback == ImDrawCallback_ResetRenderState)
			{
				Unimplemented(); //TODO: Implement me!
			}
			else if (imCmd->UserCallback != nullptr)
			{
				imCmd->UserCallback(imCmdList, imCmd);
			}
			else
			{
				RcSetViewport(NewRec(imCmd->ClipRect.x, imCmd->ClipRect.y, imCmd->ClipRect.z - imCmd->ClipRect.x, imCmd->ClipRect.w - imCmd->ClipRect.y));
				RcBindTexture1((Texture_t*)imCmd->GetTexID());
				RcDrawBuffer(VertBufferPrimitive_Triangles, listIdxOffset + imCmd->IdxOffset, imCmd->ElemCount, listVtxOffset + imCmd->VtxOffset);
			}
		}
		listVtxOffset += imCmdList->VtxBuffer.Size;
		listIdxOffset += imCmdList->IdxBuffer.Size;
	}
	
	FreeScratchArena(scratch);
	RcBindShader(oldShader);
	RcSetFaceCulling(oldFaceCullingValue);
	RcSetViewport(oldViewport);
}
