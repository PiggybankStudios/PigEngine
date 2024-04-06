/*
File:   opengl_types.h
Author: Taylor Robbins
Date:   04\04\2024
*/

#ifndef _OPENGL_TYPES_H
#define _OPENGL_TYPES_H

enum OpenGlExtensionVendor_t
{
	OpenGlExtensionVendor_Unknown = 0,
	OpenGlExtensionVendor_ARB, //Architecture Review Board (Approved Extensions)
	OpenGlExtensionVendor_EXT, //Shared between vendors
	OpenGlExtensionVendor_EXTX, //Shared between vendors (Experimental?)
	OpenGlExtensionVendor_NV, //NVIDIA?
	OpenGlExtensionVendor_NVX, //NVIDIA (Experimental?)
	OpenGlExtensionVendor_AMD, //Advanced Micro Devices, Inc.
	OpenGlExtensionVendor_SGI, //Silicon Graphics Inc.
	OpenGlExtensionVendor_SGIS, //Silicon Graphics Inc. (limited set of machines)
	OpenGlExtensionVendor_SGIX, //Silicon Graphics Inc. (Experimental)
	OpenGlExtensionVendor_HP, //Hewlett-Packard
	OpenGlExtensionVendor_SUN, //Sun Microsystems
	OpenGlExtensionVendor_GLX, //X Window System
	OpenGlExtensionVendor_WGL, //Win32
	OpenGlExtensionVendor_ATI, //ATI Technologies Inc.
	OpenGlExtensionVendor_IBM, //International Business Machines
	OpenGlExtensionVendor_WIN, //Microsoft
	OpenGlExtensionVendor_INTEL, //Intel
	OpenGlExtensionVendor_MESA, //Brian Paul's Freeware Portal OpenGL Implementation
	OpenGlExtensionVendor_KTX, //Kinetix (maker of 3DS Max)
	OpenGlExtensionVendor_KHR, //Khronos?
	OpenGlExtensionVendor_OVR, //?
	OpenGlExtensionVendor_NumVendors,
};
const char* GetOpenGlExtensionVendorStr(OpenGlExtensionVendor_t enumValue)
{
	switch (enumValue)
	{
		case OpenGlExtensionVendor_Unknown: return "Unknown";
		case OpenGlExtensionVendor_ARB:     return "ARB";
		case OpenGlExtensionVendor_EXT:     return "EXT";
		case OpenGlExtensionVendor_EXTX:    return "EXTX";
		case OpenGlExtensionVendor_NV:      return "NV";
		case OpenGlExtensionVendor_NVX:     return "NVX";
		case OpenGlExtensionVendor_AMD:     return "AMD";
		case OpenGlExtensionVendor_SGI:     return "SGI";
		case OpenGlExtensionVendor_SGIS:    return "SGIS";
		case OpenGlExtensionVendor_SGIX:    return "SGIX";
		case OpenGlExtensionVendor_HP:      return "HP";
		case OpenGlExtensionVendor_SUN:     return "SUN";
		case OpenGlExtensionVendor_GLX:     return "GLX";
		case OpenGlExtensionVendor_WGL:     return "WGL";
		case OpenGlExtensionVendor_ATI:     return "ATI";
		case OpenGlExtensionVendor_IBM:     return "IBM";
		case OpenGlExtensionVendor_WIN:     return "WIN";
		case OpenGlExtensionVendor_INTEL:   return "INTEL";
		case OpenGlExtensionVendor_MESA:    return "MESA";
		case OpenGlExtensionVendor_KTX:     return "KTX";
		case OpenGlExtensionVendor_KHR:     return "KHR";
		case OpenGlExtensionVendor_OVR:     return "OVR";
		default: return "Unknown";
	}
}

enum OpenGlExtension_t
{
	OpenGlExtension_None = 0,
	
	//OpenGL 1.0 (1992)
	
	//OpenGL 1.1 (1997)
	OpenGlExtension_GL_EXT_texture_object,
	OpenGlExtension_GL_EXT_copy_texture, //unsupported on my machine
	OpenGlExtension_GL_EXT_subtexture, //unsupported on my machine
	OpenGlExtension_GL_EXT_texture, //unsupported on my machine
	OpenGlExtension_GL_EXT_blend_logic_op, //unsupported on my machine
	OpenGlExtension_GL_EXT_polygon_offset, //unsupported on my machine
	OpenGlExtension_GL_EXT_vertex_array,
	
	//OpenGL 1.2 (1998)
	OpenGlExtension_GL_EXT_blend_minmax,
	OpenGlExtension_GL_EXT_blend_subtract,
	OpenGlExtension_GL_EXT_blend_color,
	OpenGlExtension_GL_EXT_histogram, //unsupported on my machine
	OpenGlExtension_GL_SGI_color_matrix, //unsupported on my machine
	OpenGlExtension_GL_EXT_convolution, //unsupported on my machine
	OpenGlExtension_GL_HP_convolution_border_modes, //unsupported on my machine
	OpenGlExtension_GL_SUN_convolution_border_modes, //unsupported on my machine
	OpenGlExtension_GL_EXT_color_subtable, //unsupported on my machine
	OpenGlExtension_GL_SGI_color_table, //unsupported on my machine
	OpenGlExtension_GL_EXT_draw_range_elements,
	OpenGlExtension_GL_SGIS_texture_lod,
	OpenGlExtension_GL_SGIS_texture_edge_clamp, //unsupported on my machine
	OpenGlExtension_GL_EXT_separate_specular_color,
	OpenGlExtension_GL_EXT_rescale_normal,
	OpenGlExtension_GL_EXT_packed_pixels,
	OpenGlExtension_GL_EXT_bgra,
	OpenGlExtension_GL_EXT_texture3D,
	
	//OpenGL 1.3 (2001)
	OpenGlExtension_GL_ARB_transpose_matrix,
	OpenGlExtension_GL_ARB_texture_border_clamp,
	OpenGlExtension_GL_ARB_texture_env_dot3,
	OpenGlExtension_GL_ARB_texture_env_combine,
	OpenGlExtension_GL_ARB_texture_env_add,
	OpenGlExtension_GL_ARB_multitexture,
	OpenGlExtension_GL_ARB_multisample,
	OpenGlExtension_GL_ARB_texture_cube_map,
	OpenGlExtension_GL_ARB_texture_compression,
	
	//OpenGL 1.4 (2002)
	OpenGlExtension_GL_ARB_window_pos,
	OpenGlExtension_GL_ARB_texture_mirrored_repeat,
	OpenGlExtension_GL_EXT_texture_lod_bias,
	OpenGlExtension_GL_ARB_texture_env_crossbar,
	OpenGlExtension_GL_EXT_stencil_wrap,
	OpenGlExtension_GL_EXT_blend_func_separate,
	OpenGlExtension_GL_EXT_secondary_color,
	OpenGlExtension_GL_ARB_point_parameters,
	OpenGlExtension_GL_EXT_multi_draw_arrays,
	OpenGlExtension_GL_EXT_fog_coord,
	OpenGlExtension_GL_ARB_depth_texture,
	OpenGlExtension_GL_ARB_shadow,
	OpenGlExtension_GL_NV_blend_square,
	OpenGlExtension_GL_SGIS_generate_mipmap,
	
	//OpenGL 1.5 (2003)
	OpenGlExtension_GL_EXT_shader_funcs, //unsupported on my machine
	OpenGlExtension_GL_ARB_occlusion_query,
	OpenGlExtension_GL_ARB_vertex_buffer_object,
	
	//OpenGL 2.0 (2004)
	OpenGlExtension_GL_ATI_separate_stencil, //unsupported on my machine
	OpenGlExtension_GL_EXT_stencil_two_side,
	OpenGlExtension_GL_ARB_point_sprite,
	OpenGlExtension_GL_ARB_texture_non_power_of_two,
	OpenGlExtension_GL_ARB_draw_buffers,
	OpenGlExtension_GL_ARB_shading_language_100,
	OpenGlExtension_GL_ARB_vertex_shader,
	OpenGlExtension_GL_ARB_fragment_shader,
	OpenGlExtension_GL_ARB_shader_objects,
	
	//OpenGL 2.1 (2006)
	OpenGlExtension_GL_EXT_texture_sRGB,
	OpenGlExtension_GL_ARB_pixel_buffer_object,
	
	//OpenGL 3.0 (2008)
	OpenGlExtension_GL_EXT_framebuffer_sRGB,
	OpenGlExtension_GL_ARB_transform_feedback, //unsupported on my machine
	OpenGlExtension_GL_EXT_texture_compression_rgtc,
	OpenGlExtension_GL_EXT_draw_buffers2,
	OpenGlExtension_GL_EXT_texture_array,
	OpenGlExtension_GL_EXT_texture_integer,
	OpenGlExtension_GL_NV_half_float,
	OpenGlExtension_GL_EXT_half_float_pixel, //unsupported on my machine
	OpenGlExtension_GL_ARB_color_buffer_float,
	OpenGlExtension_GL_NV_depth_buffer_float,
	OpenGlExtension_GL_ARB_texture_float,
	OpenGlExtension_GL_EXT_packed_float,
	OpenGlExtension_GL_EXT_texture_shared_exponent,
	OpenGlExtension_GL_NV_conditional_render,
	OpenGlExtension_GL_ARB_vertex_array_object,
	OpenGlExtension_GL_ARB_framebuffer_object,
	
	//OpenGL 3.1 (2009)
	OpenGlExtension_GL_ARB_texture_rectangle,
	OpenGlExtension_GL_ARB_texture_buffer_object,
	OpenGlExtension_GL_NV_primitive_restart,
	OpenGlExtension_GL_ARB_copy_buffer,
	OpenGlExtension_GL_ARB_draw_instanced,
	OpenGlExtension_GL_ARB_uniform_buffer_object,
	
	//OpenGL 3.2 (2009)
	OpenGlExtension_GL_ARB_geometry_shader4,
	OpenGlExtension_GL_ARB_sync,
	OpenGlExtension_GL_ARB_depth_clamp,
	OpenGlExtension_GL_ARB_texture_multisample,
	OpenGlExtension_GL_ARB_seamless_cube_map,
	OpenGlExtension_GL_ARB_provoking_vertex,
	OpenGlExtension_GL_ARB_fragment_coord_conventions,
	OpenGlExtension_GL_ARB_draw_elements_base_vertex,
	OpenGlExtension_GL_ARB_vertex_array_bgra,
	
	//OpenGL 3.3 (2010)
	OpenGlExtension_GL_ARB_vertex_type_2_10_10_10_rev,
	OpenGlExtension_GL_ARB_instanced_arrays,
	OpenGlExtension_GL_ARB_timer_query,
	OpenGlExtension_GL_ARB_texture_swizzle,
	OpenGlExtension_GL_ARB_texture_rbg10_a2ui, //unsupported on my machine
	OpenGlExtension_GL_ARB_sampler_objects,
	OpenGlExtension_GL_ARB_occlusion_query2,
	OpenGlExtension_GL_ARB_explicit_attrib_location,
	OpenGlExtension_GL_ARB_blend_func_extended,
	OpenGlExtension_GL_ARB_shader_bit_encoding,
	
	//OpenGL 4.0 (2010)
	OpenGlExtension_GL_ARB_draw_buffers_blend,
	OpenGlExtension_GL_ARB_transform_feedback2,
	OpenGlExtension_GL_ARB_transform_feedback3,
	OpenGlExtension_GL_ARB_texture_cube_map_array,
	OpenGlExtension_GL_ARB_texture_buffer_object_rgb32,
	OpenGlExtension_GL_ARB_tessellation_shader,
	OpenGlExtension_GL_ARB_sample_shading,
	OpenGlExtension_GL_ARB_draw_indirect,
	OpenGlExtension_GL_ARB_texture_query_lod,
	OpenGlExtension_GL_ARB_gpu_shader5,
	OpenGlExtension_GL_ARB_gpu_shader_fp64,
	OpenGlExtension_GL_ARB_shader_subroutine,
	OpenGlExtension_GL_ARB_texture_gather,
	
	//OpenGL 4.1 (2010)
	OpenGlExtension_GL_ARB_viewport_array,
	OpenGlExtension_GL_ARB_vertex_attrib_64_bit, //unsupported on my machine
	OpenGlExtension_GL_ARB_shader_precision,
	OpenGlExtension_GL_ARB_ES2_compatibility,
	OpenGlExtension_GL_ARB_separate_shader_objects,
	OpenGlExtension_GL_ARB_get_program_binary,
	
	//OpenGL 4.2 (2011)
	OpenGlExtension_GL_ARB_texture_compression_BPTC,
	OpenGlExtension_GL_ARB_conservative_depth,
	OpenGlExtension_GL_ARB_map_buffer_alignment,
	OpenGlExtension_GL_ARB_shading_language_packing,
	OpenGlExtension_GL_ARB_compressed_texture_pixel_storage,
	OpenGlExtension_GL_ARB_internalformat_query,
	OpenGlExtension_GL_ARB_base_instance,
	OpenGlExtension_GL_ARB_shading_language_420pack,
	OpenGlExtension_GL_ARB_transform_feedback_instanced,
	OpenGlExtension_GL_ARB_texture_storage,
	OpenGlExtension_GL_ARB_shader_image_load_store,
	OpenGlExtension_GL_ARB_shader_atomic_counters,
	
	//OpenGL 4.3 (2012)
	OpenGlExtension_GL_GLX_ARB_robustness_isolation, //unsupported on my machine
	OpenGlExtension_GL_WGL_ARB_robustness_isolation, //unsupported on my machine
	OpenGlExtension_GL_ARB_robustness_isolation, //unsupported on my machine
	OpenGlExtension_GL_ARB_robust_buffer_access_behavior,
	OpenGlExtension_GL_ARB_vertex_attrib_binding,
	OpenGlExtension_GL_ARB_texture_view,
	OpenGlExtension_GL_ARB_texture_storage_multisample,
	OpenGlExtension_GL_ARB_texture_query_levels,
	OpenGlExtension_GL_ARB_texture_buffer_range,
	OpenGlExtension_GL_ARB_stencil_texturing,
	OpenGlExtension_GL_ARB_shader_storage_buffer_object,
	OpenGlExtension_GL_ARB_shader_image_size,
	OpenGlExtension_GL_ARB_program_interface_query,
	OpenGlExtension_GL_ARB_multi_draw_indirect,
	OpenGlExtension_GL_ARB_invalidate_subdata,
	OpenGlExtension_GL_ARB_internalformat_query2,
	OpenGlExtension_GL_ARB_framebuffer_no_attachments,
	OpenGlExtension_GL_ARB_fragment_layer_viewport,
	OpenGlExtension_GL_ARB_explicit_uniform_location,
	OpenGlExtension_GL_ARB_ES3_compatibility,
	OpenGlExtension_GL_ARB_copy_image,
	OpenGlExtension_GL_ARB_compute_shader,
	OpenGlExtension_GL_ARB_clear_buffer_object,
	OpenGlExtension_GL_ARB_arrays_of_arrays,
	OpenGlExtension_GL_KHR_debug,
	
	//OpenGL 4.4 (2013)
	OpenGlExtension_GL_ARB_vertex_type_10f_11f_11f_rev,
	OpenGlExtension_GL_ARB_texture_stencil8,
	OpenGlExtension_GL_ARB_texture_mirror_clamp_to_edge,
	OpenGlExtension_GL_ARB_query_buffer_object,
	OpenGlExtension_GL_ARB_multi_bind,
	OpenGlExtension_GL_ARB_enhanced_layouts,
	OpenGlExtension_GL_ARB_clear_texture,
	OpenGlExtension_GL_ARB_buffer_storage,
	
	//OpenGl 4.5 (2014)
	OpenGlExtension_GL_ARB_texture_barrier,
	OpenGlExtension_GL_ARB_shader_texture_image_samples,
	OpenGlExtension_GL_KHR_robustness,
	OpenGlExtension_GL_ARB_get_texture_sub_image,
	OpenGlExtension_GL_ARB_direct_state_access,
	OpenGlExtension_GL_ARB_derivative_control,
	OpenGlExtension_GL_ARB_conditional_render_inverted,
	OpenGlExtension_GL_ARB_ES3_1_compatibility,
	OpenGlExtension_GL_ARB_cull_distance,
	OpenGlExtension_GL_ARB_clip_control,
	
	//OpenGL 4.6 (2017)
	OpenGlExtension_GL_ARB_shader_group_vote,
	OpenGlExtension_GL_ARB_shader_atomic_counter_ops,
	OpenGlExtension_GL_KHR_no_error,
	OpenGlExtension_GL_ARB_polygon_offset_clamp,
	OpenGlExtension_GL_ARB_texture_filter_anisotropic,
	OpenGlExtension_GL_ARB_pipeline_statistics_query,
	OpenGlExtension_GL_ARB_transform_feedback_overflow_query,
	OpenGlExtension_GL_ARB_indirect_parameters,
	OpenGlExtension_GL_ARB_shader_draw_parameters,
	OpenGlExtension_GL_ARB_gl_spirv,
	OpenGlExtension_GL_ARB_spirv_extensions,
	
	//Non-Core ARB Extensions
	OpenGlExtension_GL_ARB_ES3_2_compatibility,
	OpenGlExtension_GL_ARB_bindless_texture,
	OpenGlExtension_GL_ARB_compute_variable_group_size,
	OpenGlExtension_GL_ARB_debug_output,
	OpenGlExtension_GL_ARB_depth_buffer_float,
	OpenGlExtension_GL_ARB_fragment_program,
	OpenGlExtension_GL_ARB_fragment_program_shadow,
	OpenGlExtension_GL_ARB_fragment_shader_interlock,
	OpenGlExtension_GL_ARB_framebuffer_sRGB,
	OpenGlExtension_GL_ARB_gpu_shader_int64,
	OpenGlExtension_GL_ARB_half_float_pixel,
	OpenGlExtension_GL_ARB_half_float_vertex,
	OpenGlExtension_GL_ARB_map_buffer_range,
	OpenGlExtension_GL_ARB_parallel_shader_compile,
	OpenGlExtension_GL_ARB_post_depth_coverage,
	OpenGlExtension_GL_ARB_robustness,
	OpenGlExtension_GL_ARB_sample_locations,
	OpenGlExtension_GL_ARB_seamless_cubemap_per_texture,
	OpenGlExtension_GL_ARB_shader_ballot,
	OpenGlExtension_GL_ARB_shader_clock,
	OpenGlExtension_GL_ARB_shader_texture_lod,
	OpenGlExtension_GL_ARB_shader_viewport_layer_array,
	OpenGlExtension_GL_ARB_shading_language_include,
	OpenGlExtension_GL_ARB_sparse_buffer,
	OpenGlExtension_GL_ARB_sparse_texture,
	OpenGlExtension_GL_ARB_sparse_texture2,
	OpenGlExtension_GL_ARB_sparse_texture_clamp,
	OpenGlExtension_GL_ARB_texture_compression_rgtc,
	OpenGlExtension_GL_ARB_texture_filter_minmax,
	OpenGlExtension_GL_ARB_texture_rg,
	OpenGlExtension_GL_ARB_texture_rgb10_a2ui,
	OpenGlExtension_GL_ARB_vertex_attrib_64bit,
	OpenGlExtension_GL_ARB_vertex_program,
	OpenGlExtension_GL_ARB_imaging,
	OpenGlExtension_GL_ARB_cl_event,
	OpenGlExtension_GL_ARB_compatibility,
	OpenGlExtension_GL_ARB_create_context_no_error,
	OpenGlExtension_GL_ARB_matrix_palette,
	OpenGlExtension_GL_ARB_robustness_application_isolation,
	OpenGlExtension_GL_ARB_shader_stencil_export,
	OpenGlExtension_GL_ARB_shadow_ambient,
	OpenGlExtension_GL_ARB_vertex_blend,
	
	//Non-Core EXT Extensions
	OpenGlExtension_GL_EXT_bindable_uniform,
	OpenGlExtension_GL_EXT_direct_state_access,
	OpenGlExtension_GL_EXT_draw_instanced,
	OpenGlExtension_GL_EXT_framebuffer_blit,
	OpenGlExtension_GL_EXT_framebuffer_multisample,
	OpenGlExtension_GL_EXT_framebuffer_multisample_blit_scaled,
	OpenGlExtension_GL_EXT_geometry_shader4,
	OpenGlExtension_GL_EXT_gpu_program_parameters,
	OpenGlExtension_GL_EXT_gpu_shader4,
	OpenGlExtension_GL_EXT_multiview_texture_multisample,
	OpenGlExtension_GL_EXT_multiview_timer_query,
	OpenGlExtension_GL_EXT_polygon_offset_clamp,
	OpenGlExtension_GL_EXT_post_depth_coverage,
	OpenGlExtension_GL_EXT_provoking_vertex,
	OpenGlExtension_GL_EXT_raster_multisample,
	OpenGlExtension_GL_EXT_separate_shader_objects,
	OpenGlExtension_GL_EXT_shader_image_load_formatted,
	OpenGlExtension_GL_EXT_shader_image_load_store,
	OpenGlExtension_GL_EXT_shader_integer_mix,
	OpenGlExtension_GL_EXT_sparse_texture2,
	OpenGlExtension_GL_EXT_texture_buffer_object,
	OpenGlExtension_GL_EXT_texture_compression_latc,
	OpenGlExtension_GL_EXT_texture_filter_minmax,
	OpenGlExtension_GL_EXT_texture_sRGB_R8,
	OpenGlExtension_GL_EXT_texture_sRGB_decode,
	OpenGlExtension_GL_EXT_texture_shadow_lod,
	OpenGlExtension_GL_EXT_texture_storage,
	OpenGlExtension_GL_EXT_texture_swizzle,
	OpenGlExtension_GL_EXT_timer_query,
	OpenGlExtension_GL_EXT_transform_feedback2,
	OpenGlExtension_GL_EXT_vertex_array_bgra,
	OpenGlExtension_GL_EXT_vertex_attrib_64bit,
	OpenGlExtension_GL_EXT_win32_keyed_mutex,
	OpenGlExtension_GL_EXT_window_rectangles,
	OpenGlExtension_GL_EXT_422_pixels,
	OpenGlExtension_GL_EXT_clip_volume_hint,
	OpenGlExtension_GL_EXT_cmyka,
	OpenGlExtension_GL_EXT_coordinate_frame,
	OpenGlExtension_GL_EXT_cull_vertex,
	OpenGlExtension_GL_EXT_fragment_lighting,
	OpenGlExtension_GL_EXT_index_array_formats,
	OpenGlExtension_GL_EXT_index_func,
	OpenGlExtension_GL_EXT_index_material,
	OpenGlExtension_GL_EXT_index_texture,
	OpenGlExtension_GL_EXT_light_texture,
	OpenGlExtension_GL_EXT_misc_attribute,
	OpenGlExtension_GL_EXT_multisample,
	OpenGlExtension_GL_EXT_paletted_texture,
	OpenGlExtension_GL_EXT_pixel_transform,
	OpenGlExtension_GL_EXT_pixel_transform_color_table,
	OpenGlExtension_GL_EXT_scene_marker,
	OpenGlExtension_GL_EXT_shared_texture_palette,
	OpenGlExtension_GL_EXT_static_vertex_array,
	OpenGlExtension_GL_EXT_texture_env,
	OpenGlExtension_GL_EXT_texture_perturb_normal,
	OpenGlExtension_GL_EXT_texture_rectangle,
	OpenGlExtension_GL_EXT_vertex_array_set,
	OpenGlExtension_GL_EXT_vertex_array_setXXX,
	OpenGlExtension_GL_EXT_vertex_shader,
	OpenGlExtension_GL_EXT_vertex_weighting,
	OpenGlExtension_GL_EXT_EGL_image_array,
	OpenGlExtension_GL_EXT_EGL_image_external_wrap_modes,
	OpenGlExtension_GL_EXT_EGL_image_storage,
	OpenGlExtension_GL_EXT_EGL_image_storage_compression,
	OpenGlExtension_GL_EXT_EGL_sync,
	OpenGlExtension_GL_EXT_YUV_target,
	OpenGlExtension_GL_EXT_base_instance,
	OpenGlExtension_GL_EXT_blend_func_extended,
	OpenGlExtension_GL_EXT_buffer_storage,
	OpenGlExtension_GL_EXT_clear_texture,
	OpenGlExtension_GL_EXT_clip_control,
	OpenGlExtension_GL_EXT_clip_cull_distance,
	OpenGlExtension_GL_EXT_color_buffer_float,
	OpenGlExtension_GL_EXT_color_buffer_half_float,
	OpenGlExtension_GL_EXT_compressed_ETC1_RGB8_sub_texture,
	OpenGlExtension_GL_EXT_conservative_depth,
	OpenGlExtension_GL_EXT_copy_image,
	OpenGlExtension_GL_EXT_debug_label,
	OpenGlExtension_GL_EXT_debug_marker,
	OpenGlExtension_GL_EXT_depth_clamp,
	OpenGlExtension_GL_EXT_discard_framebuffer,
	OpenGlExtension_GL_EXT_disjoint_timer_query,
	OpenGlExtension_GL_EXT_draw_buffers,
	OpenGlExtension_GL_EXT_draw_buffers_indexed,
	OpenGlExtension_GL_EXT_draw_elements_base_vertex,
	OpenGlExtension_GL_EXT_draw_transform_feedback,
	OpenGlExtension_GL_EXT_external_buffer,
	OpenGlExtension_GL_EXT_external_objects,
	OpenGlExtension_GL_EXT_external_objects_fd,
	OpenGlExtension_GL_EXT_external_objects_win32,
	OpenGlExtension_GL_EXT_float_blend,
	OpenGlExtension_GL_EXT_frag_depth,
	OpenGlExtension_GL_EXT_fragment_shading_rate,
	OpenGlExtension_GL_EXT_framebuffer_blit_layers,
	OpenGlExtension_GL_EXT_geometry_shader,
	OpenGlExtension_GL_EXT_gpu_shader5,
	OpenGlExtension_GL_EXT_instanced_arrays,
	OpenGlExtension_GL_EXT_map_buffer_range,
	OpenGlExtension_GL_EXT_multi_draw_indirect,
	OpenGlExtension_GL_EXT_multiple_textures,
	OpenGlExtension_GL_EXT_multisample_compatibility,
	OpenGlExtension_GL_EXT_multisampled_render_to_texture,
	OpenGlExtension_GL_EXT_multisampled_render_to_texture2,
	OpenGlExtension_GL_EXT_multiview_draw_buffers,
	OpenGlExtension_GL_EXT_multiview_tessellation_geometry_shader,
	OpenGlExtension_GL_EXT_occlusion_query_boolean,
	OpenGlExtension_GL_EXT_primitive_bounding_box,
	OpenGlExtension_GL_EXT_protected_textures,
	OpenGlExtension_GL_EXT_pvrtc_sRGB,
	OpenGlExtension_GL_EXT_read_format_bgra,
	OpenGlExtension_GL_EXT_render_snorm,
	OpenGlExtension_GL_EXT_robustness,
	OpenGlExtension_GL_EXT_sRGB,
	OpenGlExtension_GL_EXT_sRGB_write_control,
	OpenGlExtension_GL_EXT_separate_depth_stencil,
	OpenGlExtension_GL_EXT_shader_framebuffer_fetch,
	OpenGlExtension_GL_EXT_shader_group_vote,
	OpenGlExtension_GL_EXT_shader_implicit_conversions,
	OpenGlExtension_GL_EXT_shader_io_blocks,
	OpenGlExtension_GL_EXT_shader_non_constant_global_initializers,
	OpenGlExtension_GL_EXT_shader_pixel_local_storage,
	OpenGlExtension_GL_EXT_shader_pixel_local_storage2,
	OpenGlExtension_GL_EXT_shader_samples_identical,
	OpenGlExtension_GL_EXT_shader_texture_lod,
	OpenGlExtension_GL_EXT_shadow_samplers,
	OpenGlExtension_GL_EXT_sparse_texture,
	OpenGlExtension_GL_EXT_stencil_clear_tag,
	OpenGlExtension_GL_EXT_swap_control,
	OpenGlExtension_GL_EXT_tessellation_shader,
	OpenGlExtension_GL_EXT_texenv_op,
	OpenGlExtension_GL_EXT_texture_border_clamp,
	OpenGlExtension_GL_EXT_texture_buffer,
	OpenGlExtension_GL_EXT_texture_compression_astc_decode_mode,
	OpenGlExtension_GL_EXT_texture_compression_bptc,
	OpenGlExtension_GL_EXT_texture_compression_s3tc_srgb,
	OpenGlExtension_GL_EXT_texture_cube_map_array,
	OpenGlExtension_GL_EXT_texture_format_BGRA8888,
	OpenGlExtension_GL_EXT_texture_format_sRGB_override,
	OpenGlExtension_GL_EXT_texture_mirror_clamp_to_edge,
	OpenGlExtension_GL_EXT_texture_norm16,
	OpenGlExtension_GL_EXT_texture_query_lod,
	OpenGlExtension_GL_EXT_texture_rg,
	OpenGlExtension_GL_EXT_texture_sRGB_RG8,
	OpenGlExtension_GL_EXT_texture_snorm,
	OpenGlExtension_GL_EXT_texture_storage_compression,
	OpenGlExtension_GL_EXT_texture_type_2_10_10_10_REV,
	OpenGlExtension_GL_EXT_texture_view,
	OpenGlExtension_GL_EXT_transform_feedback,
	OpenGlExtension_GL_EXT_unpack_subimage,
	OpenGlExtension_GL_EXT_x11_sync_object,
	OpenGlExtension_GL_EXT_abgr,
	OpenGlExtension_GL_EXT_blend_equation_separate,
	OpenGlExtension_GL_EXT_compiled_vertex_array,
	OpenGlExtension_GL_EXT_depth_bounds_test,
	OpenGlExtension_GL_EXT_framebuffer_object,
	OpenGlExtension_GL_EXT_packed_depth_stencil,
	OpenGlExtension_GL_EXT_point_parameters,
	OpenGlExtension_GL_EXT_shadow_funcs,
	OpenGlExtension_GL_EXT_texture_compression_dxt1,
	OpenGlExtension_GL_EXT_texture_compression_s3tc,
	OpenGlExtension_GL_EXT_texture_cube_map,
	OpenGlExtension_GL_EXT_texture_edge_clamp,
	OpenGlExtension_GL_EXT_texture_env_add,
	OpenGlExtension_GL_EXT_texture_env_combine,
	OpenGlExtension_GL_EXT_texture_env_dot3,
	OpenGlExtension_GL_EXT_texture_filter_anisotropic,
	OpenGlExtension_GL_EXT_texture_mirror_clamp,
	
	//Non-Core Nvidia Extensions
	OpenGlExtension_GL_NV_alpha_to_coverage_dither_control,
	OpenGlExtension_GL_NV_bindless_multi_draw_indirect,
	OpenGlExtension_GL_NV_bindless_multi_draw_indirect_count,
	OpenGlExtension_GL_NV_bindless_texture,
	OpenGlExtension_GL_NV_blend_equation_advanced,
	OpenGlExtension_GL_NV_blend_minmax_factor,
	OpenGlExtension_GL_NV_command_list,
	OpenGlExtension_GL_NV_compute_program5,
	OpenGlExtension_GL_NV_conservative_raster,
	OpenGlExtension_GL_NV_conservative_raster_dilate,
	OpenGlExtension_GL_NV_copy_image,
	OpenGlExtension_GL_NV_draw_texture,
	OpenGlExtension_GL_NV_draw_vulkan_image,
	OpenGlExtension_GL_NV_explicit_multisample,
	OpenGlExtension_GL_NV_fill_rectangle,
	OpenGlExtension_GL_NV_fragment_coverage_to_color,
	OpenGlExtension_GL_NV_fragment_shader_interlock,
	OpenGlExtension_GL_NV_framebuffer_mixed_samples,
	OpenGlExtension_GL_NV_framebuffer_multisample_coverage,
	OpenGlExtension_GL_NV_geometry_shader4,
	OpenGlExtension_GL_NV_geometry_shader_passthrough,
	OpenGlExtension_GL_NV_gpu_multicast,
	OpenGlExtension_GL_NV_gpu_program4,
	OpenGlExtension_GL_NV_gpu_program5,
	OpenGlExtension_GL_NV_gpu_program5_mem_extended,
	OpenGlExtension_GL_NV_gpu_shader5,
	OpenGlExtension_GL_NV_internalformat_sample_query,
	OpenGlExtension_GL_NV_memory_attachment,
	OpenGlExtension_GL_NV_memory_object_sparse,
	OpenGlExtension_GL_NV_multisample_coverage,
	OpenGlExtension_GL_NV_parameter_buffer_object,
	OpenGlExtension_GL_NV_parameter_buffer_object2,
	OpenGlExtension_GL_NV_path_rendering,
	OpenGlExtension_GL_NV_path_rendering_shared_edge,
	OpenGlExtension_GL_NV_query_resource,
	OpenGlExtension_GL_NV_query_resource_tag,
	OpenGlExtension_GL_NV_sample_locations,
	OpenGlExtension_GL_NV_sample_mask_override_coverage,
	OpenGlExtension_GL_NV_shader_atomic_counters,
	OpenGlExtension_GL_NV_shader_atomic_float,
	OpenGlExtension_GL_NV_shader_atomic_fp16_vector,
	OpenGlExtension_GL_NV_shader_atomic_int64,
	OpenGlExtension_GL_NV_shader_buffer_load,
	OpenGlExtension_GL_NV_shader_storage_buffer_object,
	OpenGlExtension_GL_NV_shader_subgroup_partitioned,
	OpenGlExtension_GL_NV_shader_thread_group,
	OpenGlExtension_GL_NV_shader_thread_shuffle,
	OpenGlExtension_GL_NV_texture_barrier,
	OpenGlExtension_GL_NV_texture_multisample,
	OpenGlExtension_GL_NV_texture_rectangle_compressed,
	OpenGlExtension_GL_NV_timeline_semaphore,
	OpenGlExtension_GL_NV_transform_feedback,
	OpenGlExtension_GL_NV_transform_feedback2,
	OpenGlExtension_GL_NV_uniform_buffer_std430_layout,
	OpenGlExtension_GL_NV_uniform_buffer_unified_memory,
	OpenGlExtension_GL_NV_vertex_attrib_integer_64bit,
	OpenGlExtension_GL_NV_vertex_buffer_unified_memory,
	OpenGlExtension_GL_NV_viewport_array2,
	OpenGlExtension_GL_NV_viewport_swizzle,
	OpenGlExtension_GL_NV_element_array,
	OpenGlExtension_GL_NV_evaluators,
	OpenGlExtension_GL_NV_texgen_emboss,
	OpenGlExtension_GL_NV_texture_expand_normal,
	OpenGlExtension_GL_NV_copy_depth_to_color,
	OpenGlExtension_GL_NV_depth_clamp,
	OpenGlExtension_GL_NV_fence,
	OpenGlExtension_GL_NV_float_buffer,
	OpenGlExtension_GL_NV_fog_distance,
	OpenGlExtension_GL_NV_fragment_program,
	OpenGlExtension_GL_NV_fragment_program2,
	OpenGlExtension_GL_NV_fragment_program_option,
	OpenGlExtension_GL_NV_light_max_exponent,
	OpenGlExtension_GL_NV_multisample_filter_hint,
	OpenGlExtension_GL_NV_occlusion_query,
	OpenGlExtension_GL_NV_packed_depth_stencil,
	OpenGlExtension_GL_NV_pixel_data_range,
	OpenGlExtension_GL_NV_point_sprite,
	OpenGlExtension_GL_NV_register_combiners,
	OpenGlExtension_GL_NV_register_combiners2,
	OpenGlExtension_GL_NV_texgen_reflection,
	OpenGlExtension_GL_NV_texture_compression_vtc,
	OpenGlExtension_GL_NV_texture_env_combine4,
	OpenGlExtension_GL_NV_texture_rectangle,
	OpenGlExtension_GL_NV_texture_shader,
	OpenGlExtension_GL_NV_texture_shader2,
	OpenGlExtension_GL_NV_texture_shader3,
	OpenGlExtension_GL_NV_vertex_array_range,
	OpenGlExtension_GL_NV_vertex_array_range2,
	OpenGlExtension_GL_NV_vertex_program,
	OpenGlExtension_GL_NV_vertex_program1_1,
	OpenGlExtension_GL_NV_vertex_program2,
	OpenGlExtension_GL_NV_vertex_program2_option,
	OpenGlExtension_GL_NV_vertex_program3,
	OpenGlExtension_GL_NV_3dvision_settings,
	OpenGlExtension_GL_NV_EGL_stream_consumer_external,
	OpenGlExtension_GL_NV_bgr,
	OpenGlExtension_GL_NV_clip_space_w_scaling,
	OpenGlExtension_GL_NV_compute_shader_derivatives,
	OpenGlExtension_GL_NV_conservative_raster_pre_snap,
	OpenGlExtension_GL_NV_conservative_raster_pre_snap_triangles,
	OpenGlExtension_GL_NV_conservative_raster_underestimation,
	OpenGlExtension_GL_NV_copy_buffer,
	OpenGlExtension_GL_NV_deep_texture3D,
	OpenGlExtension_GL_NV_draw_buffers,
	OpenGlExtension_GL_NV_draw_instanced,
	OpenGlExtension_GL_NV_explicit_attrib_location,
	OpenGlExtension_GL_NV_fbo_color_attachments,
	OpenGlExtension_GL_NV_fragment_program4,
	OpenGlExtension_GL_NV_fragment_shader_barycentric,
	OpenGlExtension_GL_NV_framebuffer_blit,
	OpenGlExtension_GL_NV_framebuffer_multisample,
	OpenGlExtension_GL_NV_generate_mipmap_sRGB,
	OpenGlExtension_GL_NV_geometry_program4,
	OpenGlExtension_GL_NV_image_formats,
	OpenGlExtension_GL_NV_instanced_arrays,
	OpenGlExtension_GL_NV_mesh_shader,
	OpenGlExtension_GL_NV_non_square_matrices,
	OpenGlExtension_GL_NV_pack_subimage,
	OpenGlExtension_GL_NV_packed_float,
	OpenGlExtension_GL_NV_pixel_buffer_object,
	OpenGlExtension_GL_NV_platform_binary,
	OpenGlExtension_GL_NV_polygon_mode,
	OpenGlExtension_GL_NV_present_video,
	OpenGlExtension_GL_NV_primitive_shading_rate,
	OpenGlExtension_GL_NV_read_buffer,
	OpenGlExtension_GL_NV_read_depth_stencil,
	OpenGlExtension_GL_NV_representative_fragment_test,
	OpenGlExtension_GL_NV_robustness_video_memory_purge,
	OpenGlExtension_GL_NV_sRGB_formats,
	OpenGlExtension_GL_NV_scissor_exclusive,
	OpenGlExtension_GL_NV_shader_atomic_float64,
	OpenGlExtension_GL_NV_shader_buffer_store,
	OpenGlExtension_GL_NV_shader_noperspective_interpolation,
	OpenGlExtension_GL_NV_shader_texture_footprint,
	OpenGlExtension_GL_NV_shading_rate_image,
	OpenGlExtension_GL_NV_shadow_samplers_array,
	OpenGlExtension_GL_NV_shadow_samplers_cube,
	OpenGlExtension_GL_NV_stereo_view_rendering,
	OpenGlExtension_GL_NV_tessellation_program5,
	OpenGlExtension_GL_NV_texture_array,
	OpenGlExtension_GL_NV_texture_border_clamp,
	OpenGlExtension_GL_NV_texture_compression_latc,
	OpenGlExtension_GL_NV_texture_compression_s3tc,
	OpenGlExtension_GL_NV_texture_compression_s3tc_update,
	OpenGlExtension_GL_NV_texture_npot_2D_mipmap,
	OpenGlExtension_GL_NV_vdpau_interop,
	OpenGlExtension_GL_NV_vdpau_interop2,
	OpenGlExtension_GL_NV_vertex_program4,
	OpenGlExtension_GL_NV_video_capture,
	OpenGlExtension_GL_NV_viewport_array,
	OpenGlExtension_WGL_NV_render_depth_texture,
	OpenGlExtension_WGL_NV_render_texture_rectangle,
	OpenGlExtension_WGL_NV_DX_interop,
	OpenGlExtension_WGL_NV_DX_interop2,
	OpenGlExtension_WGL_NV_delay_before_swap,
	OpenGlExtension_WGL_NV_gpu_affinity,
	OpenGlExtension_WGL_NV_multigpu_context,
	OpenGlExtension_WGL_NV_swap_group,
	OpenGlExtension_WGL_NV_video_output,
	
	//Non-Core AMD Extensions
	OpenGlExtension_GL_AMD_seamless_cubemap_per_texture,
	OpenGlExtension_GL_AMD_multi_draw_indirect,
	OpenGlExtension_GL_AMD_vertex_shader_viewport_index,
	OpenGlExtension_GL_AMD_vertex_shader_layer,
	
	//Non-Core Other Extensions (that my GPU supports)
	OpenGlExtension_GL_ATI_draw_buffers,
	OpenGlExtension_GL_ATI_texture_float,
	OpenGlExtension_GL_ATI_texture_mirror_once,
	OpenGlExtension_GL_IBM_rasterpos_clip,
	OpenGlExtension_GL_IBM_texture_mirrored_repeat,
	OpenGlExtension_GL_S3_s3tc,
	OpenGlExtension_GL_SGIX_depth_texture,
	OpenGlExtension_GL_SGIX_shadow,
	OpenGlExtension_GL_SUN_slice_accum,
	OpenGlExtension_GL_WIN_swap_hint,
	OpenGlExtension_WGL_EXT_swap_control,
	OpenGlExtension_GL_EXT_Cg_shader,
	OpenGlExtension_GL_EXTX_framebuffer_mixed_formats,
	OpenGlExtension_GL_EXT_pixel_buffer_object,
	OpenGlExtension_GL_EXT_texture_lod,
	OpenGlExtension_GL_EXT_import_sync_object,
	OpenGlExtension_GL_KHR_context_flush_control,
	OpenGlExtension_GL_EXT_memory_object,
	OpenGlExtension_GL_EXT_memory_object_win32,
	OpenGlExtension_GL_KHR_parallel_shader_compile,
	OpenGlExtension_GL_KHR_robust_buffer_access_behavior,
	OpenGlExtension_GL_EXT_semaphore,
	OpenGlExtension_GL_EXT_semaphore_win32,
	OpenGlExtension_GL_KHR_shader_subgroup,
	OpenGlExtension_GL_KTX_buffer_region,
	OpenGlExtension_GL_NV_blend_equation_advanced_coherent,
	OpenGlExtension_GL_NV_ES1_1_compatibility,
	OpenGlExtension_GL_NV_ES3_1_compatibility,
	OpenGlExtension_GL_NV_feature_query,
	OpenGlExtension_GL_NV_gpu_program4_1,
	OpenGlExtension_GL_NV_gpu_program_fp64,
	OpenGlExtension_GL_NVX_conditional_render,
	OpenGlExtension_GL_NVX_linked_gpu_multicast,
	OpenGlExtension_GL_NVX_gpu_multicast2,
	OpenGlExtension_GL_NVX_progress_fence,
	OpenGlExtension_GL_NVX_gpu_memory_info,
	OpenGlExtension_GL_NVX_multigpu_info,
	OpenGlExtension_GL_NVX_nvenc_interop,
	OpenGlExtension_GL_KHR_blend_equation_advanced,
	OpenGlExtension_GL_KHR_blend_equation_advanced_coherent,
	OpenGlExtension_GL_OVR_multiview,
	OpenGlExtension_GL_OVR_multiview2,
	
	OpenGlExtension_NumExtensions,
};

const char* GetOpenGlExtensionStr(OpenGlExtension_t extension)
{
	switch (extension)
	{
		case OpenGlExtension_None:                                           return "None";
		case OpenGlExtension_GL_EXT_texture_object:                          return "GL_EXT_texture_object";
		case OpenGlExtension_GL_EXT_copy_texture:                            return "GL_EXT_copy_texture";
		case OpenGlExtension_GL_EXT_subtexture:                              return "GL_EXT_subtexture";
		case OpenGlExtension_GL_EXT_texture:                                 return "GL_EXT_texture";
		case OpenGlExtension_GL_EXT_blend_logic_op:                          return "GL_EXT_blend_logic_op";
		case OpenGlExtension_GL_EXT_polygon_offset:                          return "GL_EXT_polygon_offset";
		case OpenGlExtension_GL_EXT_vertex_array:                            return "GL_EXT_vertex_array";
		case OpenGlExtension_GL_EXT_blend_minmax:                            return "GL_EXT_blend_minmax";
		case OpenGlExtension_GL_EXT_blend_subtract:                          return "GL_EXT_blend_subtract";
		case OpenGlExtension_GL_EXT_blend_color:                             return "GL_EXT_blend_color";
		case OpenGlExtension_GL_EXT_histogram:                               return "GL_EXT_histogram";
		case OpenGlExtension_GL_SGI_color_matrix:                            return "GL_SGI_color_matrix";
		case OpenGlExtension_GL_EXT_convolution:                             return "GL_EXT_convolution";
		case OpenGlExtension_GL_HP_convolution_border_modes:                 return "GL_HP_convolution_border_modes";
		case OpenGlExtension_GL_SUN_convolution_border_modes:                return "GL_SUN_convolution_border_modes";
		case OpenGlExtension_GL_EXT_color_subtable:                          return "GL_EXT_color_subtable";
		case OpenGlExtension_GL_SGI_color_table:                             return "GL_SGI_color_table";
		case OpenGlExtension_GL_EXT_draw_range_elements:                     return "GL_EXT_draw_range_elements";
		case OpenGlExtension_GL_SGIS_texture_lod:                            return "GL_SGIS_texture_lod";
		case OpenGlExtension_GL_SGIS_texture_edge_clamp:                     return "GL_SGIS_texture_edge_clamp";
		case OpenGlExtension_GL_EXT_separate_specular_color:                 return "GL_EXT_separate_specular_color";
		case OpenGlExtension_GL_EXT_rescale_normal:                          return "GL_EXT_rescale_normal";
		case OpenGlExtension_GL_EXT_packed_pixels:                           return "GL_EXT_packed_pixels";
		case OpenGlExtension_GL_EXT_bgra:                                    return "GL_EXT_bgra";
		case OpenGlExtension_GL_EXT_texture3D:                               return "GL_EXT_texture3D";
		case OpenGlExtension_GL_ARB_transpose_matrix:                        return "GL_ARB_transpose_matrix";
		case OpenGlExtension_GL_ARB_texture_border_clamp:                    return "GL_ARB_texture_border_clamp";
		case OpenGlExtension_GL_ARB_texture_env_dot3:                        return "GL_ARB_texture_env_dot3";
		case OpenGlExtension_GL_ARB_texture_env_combine:                     return "GL_ARB_texture_env_combine";
		case OpenGlExtension_GL_ARB_texture_env_add:                         return "GL_ARB_texture_env_add";
		case OpenGlExtension_GL_ARB_multitexture:                            return "GL_ARB_multitexture";
		case OpenGlExtension_GL_ARB_multisample:                             return "GL_ARB_multisample";
		case OpenGlExtension_GL_ARB_texture_cube_map:                        return "GL_ARB_texture_cube_map";
		case OpenGlExtension_GL_ARB_texture_compression:                     return "GL_ARB_texture_compression";
		case OpenGlExtension_GL_ARB_window_pos:                              return "GL_ARB_window_pos";
		case OpenGlExtension_GL_ARB_texture_mirrored_repeat:                 return "GL_ARB_texture_mirrored_repeat";
		case OpenGlExtension_GL_EXT_texture_lod_bias:                        return "GL_EXT_texture_lod_bias";
		case OpenGlExtension_GL_ARB_texture_env_crossbar:                    return "GL_ARB_texture_env_crossbar";
		case OpenGlExtension_GL_EXT_stencil_wrap:                            return "GL_EXT_stencil_wrap";
		case OpenGlExtension_GL_EXT_blend_func_separate:                     return "GL_EXT_blend_func_separate";
		case OpenGlExtension_GL_EXT_secondary_color:                         return "GL_EXT_secondary_color";
		case OpenGlExtension_GL_ARB_point_parameters:                        return "GL_ARB_point_parameters";
		case OpenGlExtension_GL_EXT_multi_draw_arrays:                       return "GL_EXT_multi_draw_arrays";
		case OpenGlExtension_GL_EXT_fog_coord:                               return "GL_EXT_fog_coord";
		case OpenGlExtension_GL_ARB_depth_texture:                           return "GL_ARB_depth_texture";
		case OpenGlExtension_GL_ARB_shadow:                                  return "GL_ARB_shadow";
		case OpenGlExtension_GL_NV_blend_square:                             return "GL_NV_blend_square";
		case OpenGlExtension_GL_SGIS_generate_mipmap:                        return "GL_SGIS_generate_mipmap";
		case OpenGlExtension_GL_EXT_shader_funcs:                            return "GL_EXT_shader_funcs";
		case OpenGlExtension_GL_ARB_occlusion_query:                         return "GL_ARB_occlusion_query";
		case OpenGlExtension_GL_ARB_vertex_buffer_object:                    return "GL_ARB_vertex_buffer_object";
		case OpenGlExtension_GL_ATI_separate_stencil:                        return "GL_ATI_separate_stencil";
		case OpenGlExtension_GL_EXT_stencil_two_side:                        return "GL_EXT_stencil_two_side";
		case OpenGlExtension_GL_ARB_point_sprite:                            return "GL_ARB_point_sprite";
		case OpenGlExtension_GL_ARB_texture_non_power_of_two:                return "GL_ARB_texture_non_power_of_two";
		case OpenGlExtension_GL_ARB_draw_buffers:                            return "GL_ARB_draw_buffers";
		case OpenGlExtension_GL_ARB_shading_language_100:                    return "GL_ARB_shading_language_100";
		case OpenGlExtension_GL_ARB_vertex_shader:                           return "GL_ARB_vertex_shader";
		case OpenGlExtension_GL_ARB_fragment_shader:                         return "GL_ARB_fragment_shader";
		case OpenGlExtension_GL_ARB_shader_objects:                          return "GL_ARB_shader_objects";
		case OpenGlExtension_GL_EXT_texture_sRGB:                            return "GL_EXT_texture_sRGB";
		case OpenGlExtension_GL_ARB_pixel_buffer_object:                     return "GL_ARB_pixel_buffer_object";
		case OpenGlExtension_GL_EXT_framebuffer_sRGB:                        return "GL_EXT_framebuffer_sRGB";
		case OpenGlExtension_GL_ARB_transform_feedback:                      return "GL_ARB_transform_feedback";
		case OpenGlExtension_GL_EXT_texture_compression_rgtc:                return "GL_EXT_texture_compression_rgtc";
		case OpenGlExtension_GL_EXT_draw_buffers2:                           return "GL_EXT_draw_buffers2";
		case OpenGlExtension_GL_EXT_texture_array:                           return "GL_EXT_texture_array";
		case OpenGlExtension_GL_EXT_texture_integer:                         return "GL_EXT_texture_integer";
		case OpenGlExtension_GL_NV_half_float:                               return "GL_NV_half_float";
		case OpenGlExtension_GL_EXT_half_float_pixel:                        return "GL_EXT_half_float_pixel";
		case OpenGlExtension_GL_ARB_color_buffer_float:                      return "GL_ARB_color_buffer_float";
		case OpenGlExtension_GL_NV_depth_buffer_float:                       return "GL_NV_depth_buffer_float";
		case OpenGlExtension_GL_ARB_texture_float:                           return "GL_ARB_texture_float";
		case OpenGlExtension_GL_EXT_packed_float:                            return "GL_EXT_packed_float";
		case OpenGlExtension_GL_EXT_texture_shared_exponent:                 return "GL_EXT_texture_shared_exponent";
		case OpenGlExtension_GL_NV_conditional_render:                       return "GL_NV_conditional_render";
		case OpenGlExtension_GL_ARB_vertex_array_object:                     return "GL_ARB_vertex_array_object";
		case OpenGlExtension_GL_ARB_framebuffer_object:                      return "GL_ARB_framebuffer_object";
		case OpenGlExtension_GL_ARB_texture_rectangle:                       return "GL_ARB_texture_rectangle";
		case OpenGlExtension_GL_ARB_texture_buffer_object:                   return "GL_ARB_texture_buffer_object";
		case OpenGlExtension_GL_NV_primitive_restart:                        return "GL_NV_primitive_restart";
		case OpenGlExtension_GL_ARB_copy_buffer:                             return "GL_ARB_copy_buffer";
		case OpenGlExtension_GL_ARB_draw_instanced:                          return "GL_ARB_draw_instanced";
		case OpenGlExtension_GL_ARB_uniform_buffer_object:                   return "GL_ARB_uniform_buffer_object";
		case OpenGlExtension_GL_ARB_geometry_shader4:                        return "GL_ARB_geometry_shader4";
		case OpenGlExtension_GL_ARB_sync:                                    return "GL_ARB_sync";
		case OpenGlExtension_GL_ARB_depth_clamp:                             return "GL_ARB_depth_clamp";
		case OpenGlExtension_GL_ARB_texture_multisample:                     return "GL_ARB_texture_multisample";
		case OpenGlExtension_GL_ARB_seamless_cube_map:                       return "GL_ARB_seamless_cube_map";
		case OpenGlExtension_GL_ARB_provoking_vertex:                        return "GL_ARB_provoking_vertex";
		case OpenGlExtension_GL_ARB_fragment_coord_conventions:              return "GL_ARB_fragment_coord_conventions";
		case OpenGlExtension_GL_ARB_draw_elements_base_vertex:               return "GL_ARB_draw_elements_base_vertex";
		case OpenGlExtension_GL_ARB_vertex_array_bgra:                       return "GL_ARB_vertex_array_bgra";
		case OpenGlExtension_GL_ARB_vertex_type_2_10_10_10_rev:              return "GL_ARB_vertex_type_2_10_10_10_rev";
		case OpenGlExtension_GL_ARB_instanced_arrays:                        return "GL_ARB_instanced_arrays";
		case OpenGlExtension_GL_ARB_timer_query:                             return "GL_ARB_timer_query";
		case OpenGlExtension_GL_ARB_texture_swizzle:                         return "GL_ARB_texture_swizzle";
		case OpenGlExtension_GL_ARB_texture_rbg10_a2ui:                      return "GL_ARB_texture_rbg10_a2ui";
		case OpenGlExtension_GL_ARB_sampler_objects:                         return "GL_ARB_sampler_objects";
		case OpenGlExtension_GL_ARB_occlusion_query2:                        return "GL_ARB_occlusion_query2";
		case OpenGlExtension_GL_ARB_explicit_attrib_location:                return "GL_ARB_explicit_attrib_location";
		case OpenGlExtension_GL_ARB_blend_func_extended:                     return "GL_ARB_blend_func_extended";
		case OpenGlExtension_GL_ARB_shader_bit_encoding:                     return "GL_ARB_shader_bit_encoding";
		case OpenGlExtension_GL_ARB_draw_buffers_blend:                      return "GL_ARB_draw_buffers_blend";
		case OpenGlExtension_GL_ARB_transform_feedback2:                     return "GL_ARB_transform_feedback2";
		case OpenGlExtension_GL_ARB_transform_feedback3:                     return "GL_ARB_transform_feedback3";
		case OpenGlExtension_GL_ARB_texture_cube_map_array:                  return "GL_ARB_texture_cube_map_array";
		case OpenGlExtension_GL_ARB_texture_buffer_object_rgb32:             return "GL_ARB_texture_buffer_object_rgb32";
		case OpenGlExtension_GL_ARB_tessellation_shader:                     return "GL_ARB_tessellation_shader";
		case OpenGlExtension_GL_ARB_sample_shading:                          return "GL_ARB_sample_shading";
		case OpenGlExtension_GL_ARB_draw_indirect:                           return "GL_ARB_draw_indirect";
		case OpenGlExtension_GL_ARB_texture_query_lod:                       return "GL_ARB_texture_query_lod";
		case OpenGlExtension_GL_ARB_gpu_shader5:                             return "GL_ARB_gpu_shader5";
		case OpenGlExtension_GL_ARB_gpu_shader_fp64:                         return "GL_ARB_gpu_shader_fp64";
		case OpenGlExtension_GL_ARB_shader_subroutine:                       return "GL_ARB_shader_subroutine";
		case OpenGlExtension_GL_ARB_texture_gather:                          return "GL_ARB_texture_gather";
		case OpenGlExtension_GL_ARB_viewport_array:                          return "GL_ARB_viewport_array";
		case OpenGlExtension_GL_ARB_vertex_attrib_64_bit:                    return "GL_ARB_vertex_attrib_64_bit";
		case OpenGlExtension_GL_ARB_shader_precision:                        return "GL_ARB_shader_precision";
		case OpenGlExtension_GL_ARB_ES2_compatibility:                       return "GL_ARB_ES2_compatibility";
		case OpenGlExtension_GL_ARB_separate_shader_objects:                 return "GL_ARB_separate_shader_objects";
		case OpenGlExtension_GL_ARB_get_program_binary:                      return "GL_ARB_get_program_binary";
		case OpenGlExtension_GL_ARB_texture_compression_BPTC:                return "GL_ARB_texture_compression_BPTC";
		case OpenGlExtension_GL_ARB_conservative_depth:                      return "GL_ARB_conservative_depth";
		case OpenGlExtension_GL_ARB_map_buffer_alignment:                    return "GL_ARB_map_buffer_alignment";
		case OpenGlExtension_GL_ARB_shading_language_packing:                return "GL_ARB_shading_language_packing";
		case OpenGlExtension_GL_ARB_compressed_texture_pixel_storage:        return "GL_ARB_compressed_texture_pixel_storage";
		case OpenGlExtension_GL_ARB_internalformat_query:                    return "GL_ARB_internalformat_query";
		case OpenGlExtension_GL_ARB_base_instance:                           return "GL_ARB_base_instance";
		case OpenGlExtension_GL_ARB_shading_language_420pack:                return "GL_ARB_shading_language_420pack";
		case OpenGlExtension_GL_ARB_transform_feedback_instanced:            return "GL_ARB_transform_feedback_instanced";
		case OpenGlExtension_GL_ARB_texture_storage:                         return "GL_ARB_texture_storage";
		case OpenGlExtension_GL_ARB_shader_image_load_store:                 return "GL_ARB_shader_image_load_store";
		case OpenGlExtension_GL_ARB_shader_atomic_counters:                  return "GL_ARB_shader_atomic_counters";
		case OpenGlExtension_GL_GLX_ARB_robustness_isolation:                return "GL_GLX_ARB_robustness_isolation";
		case OpenGlExtension_GL_WGL_ARB_robustness_isolation:                return "GL_WGL_ARB_robustness_isolation";
		case OpenGlExtension_GL_ARB_robustness_isolation:                    return "GL_ARB_robustness_isolation";
		case OpenGlExtension_GL_ARB_robust_buffer_access_behavior:           return "GL_ARB_robust_buffer_access_behavior";
		case OpenGlExtension_GL_ARB_vertex_attrib_binding:                   return "GL_ARB_vertex_attrib_binding";
		case OpenGlExtension_GL_ARB_texture_view:                            return "GL_ARB_texture_view";
		case OpenGlExtension_GL_ARB_texture_storage_multisample:             return "GL_ARB_texture_storage_multisample";
		case OpenGlExtension_GL_ARB_texture_query_levels:                    return "GL_ARB_texture_query_levels";
		case OpenGlExtension_GL_ARB_texture_buffer_range:                    return "GL_ARB_texture_buffer_range";
		case OpenGlExtension_GL_ARB_stencil_texturing:                       return "GL_ARB_stencil_texturing";
		case OpenGlExtension_GL_ARB_shader_storage_buffer_object:            return "GL_ARB_shader_storage_buffer_object";
		case OpenGlExtension_GL_ARB_shader_image_size:                       return "GL_ARB_shader_image_size";
		case OpenGlExtension_GL_ARB_program_interface_query:                 return "GL_ARB_program_interface_query";
		case OpenGlExtension_GL_ARB_multi_draw_indirect:                     return "GL_ARB_multi_draw_indirect";
		case OpenGlExtension_GL_ARB_invalidate_subdata:                      return "GL_ARB_invalidate_subdata";
		case OpenGlExtension_GL_ARB_internalformat_query2:                   return "GL_ARB_internalformat_query2";
		case OpenGlExtension_GL_ARB_framebuffer_no_attachments:              return "GL_ARB_framebuffer_no_attachments";
		case OpenGlExtension_GL_ARB_fragment_layer_viewport:                 return "GL_ARB_fragment_layer_viewport";
		case OpenGlExtension_GL_ARB_explicit_uniform_location:               return "GL_ARB_explicit_uniform_location";
		case OpenGlExtension_GL_ARB_ES3_compatibility:                       return "GL_ARB_ES3_compatibility";
		case OpenGlExtension_GL_ARB_copy_image:                              return "GL_ARB_copy_image";
		case OpenGlExtension_GL_ARB_compute_shader:                          return "GL_ARB_compute_shader";
		case OpenGlExtension_GL_ARB_clear_buffer_object:                     return "GL_ARB_clear_buffer_object";
		case OpenGlExtension_GL_ARB_arrays_of_arrays:                        return "GL_ARB_arrays_of_arrays";
		case OpenGlExtension_GL_KHR_debug:                                   return "GL_KHR_debug";
		case OpenGlExtension_GL_ARB_vertex_type_10f_11f_11f_rev:             return "GL_ARB_vertex_type_10f_11f_11f_rev";
		case OpenGlExtension_GL_ARB_texture_stencil8:                        return "GL_ARB_texture_stencil8";
		case OpenGlExtension_GL_ARB_texture_mirror_clamp_to_edge:            return "GL_ARB_texture_mirror_clamp_to_edge";
		case OpenGlExtension_GL_ARB_query_buffer_object:                     return "GL_ARB_query_buffer_object";
		case OpenGlExtension_GL_ARB_multi_bind:                              return "GL_ARB_multi_bind";
		case OpenGlExtension_GL_ARB_enhanced_layouts:                        return "GL_ARB_enhanced_layouts";
		case OpenGlExtension_GL_ARB_clear_texture:                           return "GL_ARB_clear_texture";
		case OpenGlExtension_GL_ARB_buffer_storage:                          return "GL_ARB_buffer_storage";
		case OpenGlExtension_GL_ARB_texture_barrier:                         return "GL_ARB_texture_barrier";
		case OpenGlExtension_GL_ARB_shader_texture_image_samples:            return "GL_ARB_shader_texture_image_samples";
		case OpenGlExtension_GL_KHR_robustness:                              return "GL_KHR_robustness";
		case OpenGlExtension_GL_ARB_get_texture_sub_image:                   return "GL_ARB_get_texture_sub_image";
		case OpenGlExtension_GL_ARB_direct_state_access:                     return "GL_ARB_direct_state_access";
		case OpenGlExtension_GL_ARB_derivative_control:                      return "GL_ARB_derivative_control";
		case OpenGlExtension_GL_ARB_conditional_render_inverted:             return "GL_ARB_conditional_render_inverted";
		case OpenGlExtension_GL_ARB_ES3_1_compatibility:                     return "GL_ARB_ES3_1_compatibility";
		case OpenGlExtension_GL_ARB_cull_distance:                           return "GL_ARB_cull_distance";
		case OpenGlExtension_GL_ARB_clip_control:                            return "GL_ARB_clip_control";
		case OpenGlExtension_GL_ARB_shader_group_vote:                       return "GL_ARB_shader_group_vote";
		case OpenGlExtension_GL_ARB_shader_atomic_counter_ops:               return "GL_ARB_shader_atomic_counter_ops";
		case OpenGlExtension_GL_KHR_no_error:                                return "GL_KHR_no_error";
		case OpenGlExtension_GL_ARB_polygon_offset_clamp:                    return "GL_ARB_polygon_offset_clamp";
		case OpenGlExtension_GL_ARB_texture_filter_anisotropic:              return "GL_ARB_texture_filter_anisotropic";
		case OpenGlExtension_GL_ARB_pipeline_statistics_query:               return "GL_ARB_pipeline_statistics_query";
		case OpenGlExtension_GL_ARB_transform_feedback_overflow_query:       return "GL_ARB_transform_feedback_overflow_query";
		case OpenGlExtension_GL_ARB_indirect_parameters:                     return "GL_ARB_indirect_parameters";
		case OpenGlExtension_GL_ARB_shader_draw_parameters:                  return "GL_ARB_shader_draw_parameters";
		case OpenGlExtension_GL_ARB_gl_spirv:                                return "GL_ARB_gl_spirv";
		case OpenGlExtension_GL_ARB_spirv_extensions:                        return "GL_ARB_spirv_extensions";
		case OpenGlExtension_GL_ARB_ES3_2_compatibility:                     return "GL_ARB_ES3_2_compatibility";
		case OpenGlExtension_GL_ARB_bindless_texture:                        return "GL_ARB_bindless_texture";
		case OpenGlExtension_GL_ARB_compute_variable_group_size:             return "GL_ARB_compute_variable_group_size";
		case OpenGlExtension_GL_ARB_debug_output:                            return "GL_ARB_debug_output";
		case OpenGlExtension_GL_ARB_depth_buffer_float:                      return "GL_ARB_depth_buffer_float";
		case OpenGlExtension_GL_ARB_fragment_program:                        return "GL_ARB_fragment_program";
		case OpenGlExtension_GL_ARB_fragment_program_shadow:                 return "GL_ARB_fragment_program_shadow";
		case OpenGlExtension_GL_ARB_fragment_shader_interlock:               return "GL_ARB_fragment_shader_interlock";
		case OpenGlExtension_GL_ARB_framebuffer_sRGB:                        return "GL_ARB_framebuffer_sRGB";
		case OpenGlExtension_GL_ARB_gpu_shader_int64:                        return "GL_ARB_gpu_shader_int64";
		case OpenGlExtension_GL_ARB_half_float_pixel:                        return "GL_ARB_half_float_pixel";
		case OpenGlExtension_GL_ARB_half_float_vertex:                       return "GL_ARB_half_float_vertex";
		case OpenGlExtension_GL_ARB_map_buffer_range:                        return "GL_ARB_map_buffer_range";
		case OpenGlExtension_GL_ARB_parallel_shader_compile:                 return "GL_ARB_parallel_shader_compile";
		case OpenGlExtension_GL_ARB_post_depth_coverage:                     return "GL_ARB_post_depth_coverage";
		case OpenGlExtension_GL_ARB_robustness:                              return "GL_ARB_robustness";
		case OpenGlExtension_GL_ARB_sample_locations:                        return "GL_ARB_sample_locations";
		case OpenGlExtension_GL_ARB_seamless_cubemap_per_texture:            return "GL_ARB_seamless_cubemap_per_texture";
		case OpenGlExtension_GL_ARB_shader_ballot:                           return "GL_ARB_shader_ballot";
		case OpenGlExtension_GL_ARB_shader_clock:                            return "GL_ARB_shader_clock";
		case OpenGlExtension_GL_ARB_shader_texture_lod:                      return "GL_ARB_shader_texture_lod";
		case OpenGlExtension_GL_ARB_shader_viewport_layer_array:             return "GL_ARB_shader_viewport_layer_array";
		case OpenGlExtension_GL_ARB_shading_language_include:                return "GL_ARB_shading_language_include";
		case OpenGlExtension_GL_ARB_sparse_buffer:                           return "GL_ARB_sparse_buffer";
		case OpenGlExtension_GL_ARB_sparse_texture:                          return "GL_ARB_sparse_texture";
		case OpenGlExtension_GL_ARB_sparse_texture2:                         return "GL_ARB_sparse_texture2";
		case OpenGlExtension_GL_ARB_sparse_texture_clamp:                    return "GL_ARB_sparse_texture_clamp";
		case OpenGlExtension_GL_ARB_texture_compression_rgtc:                return "GL_ARB_texture_compression_rgtc";
		case OpenGlExtension_GL_ARB_texture_filter_minmax:                   return "GL_ARB_texture_filter_minmax";
		case OpenGlExtension_GL_ARB_texture_rg:                              return "GL_ARB_texture_rg";
		case OpenGlExtension_GL_ARB_texture_rgb10_a2ui:                      return "GL_ARB_texture_rgb10_a2ui";
		case OpenGlExtension_GL_ARB_vertex_attrib_64bit:                     return "GL_ARB_vertex_attrib_64bit";
		case OpenGlExtension_GL_ARB_vertex_program:                          return "GL_ARB_vertex_program";
		case OpenGlExtension_GL_ARB_imaging:                                 return "GL_ARB_imaging";
		case OpenGlExtension_GL_ARB_cl_event:                                return "GL_ARB_cl_event";
		case OpenGlExtension_GL_ARB_compatibility:                           return "GL_ARB_compatibility";
		case OpenGlExtension_GL_ARB_create_context_no_error:                 return "GL_ARB_create_context_no_error";
		case OpenGlExtension_GL_ARB_matrix_palette:                          return "GL_ARB_matrix_palette";
		case OpenGlExtension_GL_ARB_robustness_application_isolation:        return "GL_ARB_robustness_application_isolation";
		case OpenGlExtension_GL_ARB_shader_stencil_export:                   return "GL_ARB_shader_stencil_export";
		case OpenGlExtension_GL_ARB_shadow_ambient:                          return "GL_ARB_shadow_ambient";
		case OpenGlExtension_GL_ARB_vertex_blend:                            return "GL_ARB_vertex_blend";
		case OpenGlExtension_GL_EXT_bindable_uniform:                        return "GL_EXT_bindable_uniform";
		case OpenGlExtension_GL_EXT_direct_state_access:                     return "GL_EXT_direct_state_access";
		case OpenGlExtension_GL_EXT_draw_instanced:                          return "GL_EXT_draw_instanced";
		case OpenGlExtension_GL_EXT_framebuffer_blit:                        return "GL_EXT_framebuffer_blit";
		case OpenGlExtension_GL_EXT_framebuffer_multisample:                 return "GL_EXT_framebuffer_multisample";
		case OpenGlExtension_GL_EXT_framebuffer_multisample_blit_scaled:     return "GL_EXT_framebuffer_multisample_blit_scaled";
		case OpenGlExtension_GL_EXT_geometry_shader4:                        return "GL_EXT_geometry_shader4";
		case OpenGlExtension_GL_EXT_gpu_program_parameters:                  return "GL_EXT_gpu_program_parameters";
		case OpenGlExtension_GL_EXT_gpu_shader4:                             return "GL_EXT_gpu_shader4";
		case OpenGlExtension_GL_EXT_multiview_texture_multisample:           return "GL_EXT_multiview_texture_multisample";
		case OpenGlExtension_GL_EXT_multiview_timer_query:                   return "GL_EXT_multiview_timer_query";
		case OpenGlExtension_GL_EXT_polygon_offset_clamp:                    return "GL_EXT_polygon_offset_clamp";
		case OpenGlExtension_GL_EXT_post_depth_coverage:                     return "GL_EXT_post_depth_coverage";
		case OpenGlExtension_GL_EXT_provoking_vertex:                        return "GL_EXT_provoking_vertex";
		case OpenGlExtension_GL_EXT_raster_multisample:                      return "GL_EXT_raster_multisample";
		case OpenGlExtension_GL_EXT_separate_shader_objects:                 return "GL_EXT_separate_shader_objects";
		case OpenGlExtension_GL_EXT_shader_image_load_formatted:             return "GL_EXT_shader_image_load_formatted";
		case OpenGlExtension_GL_EXT_shader_image_load_store:                 return "GL_EXT_shader_image_load_store";
		case OpenGlExtension_GL_EXT_shader_integer_mix:                      return "GL_EXT_shader_integer_mix";
		case OpenGlExtension_GL_EXT_sparse_texture2:                         return "GL_EXT_sparse_texture2";
		case OpenGlExtension_GL_EXT_texture_buffer_object:                   return "GL_EXT_texture_buffer_object";
		case OpenGlExtension_GL_EXT_texture_compression_latc:                return "GL_EXT_texture_compression_latc";
		case OpenGlExtension_GL_EXT_texture_filter_minmax:                   return "GL_EXT_texture_filter_minmax";
		case OpenGlExtension_GL_EXT_texture_sRGB_R8:                         return "GL_EXT_texture_sRGB_R8";
		case OpenGlExtension_GL_EXT_texture_sRGB_decode:                     return "GL_EXT_texture_sRGB_decode";
		case OpenGlExtension_GL_EXT_texture_shadow_lod:                      return "GL_EXT_texture_shadow_lod";
		case OpenGlExtension_GL_EXT_texture_storage:                         return "GL_EXT_texture_storage";
		case OpenGlExtension_GL_EXT_texture_swizzle:                         return "GL_EXT_texture_swizzle";
		case OpenGlExtension_GL_EXT_timer_query:                             return "GL_EXT_timer_query";
		case OpenGlExtension_GL_EXT_transform_feedback2:                     return "GL_EXT_transform_feedback2";
		case OpenGlExtension_GL_EXT_vertex_array_bgra:                       return "GL_EXT_vertex_array_bgra";
		case OpenGlExtension_GL_EXT_vertex_attrib_64bit:                     return "GL_EXT_vertex_attrib_64bit";
		case OpenGlExtension_GL_EXT_win32_keyed_mutex:                       return "GL_EXT_win32_keyed_mutex";
		case OpenGlExtension_GL_EXT_window_rectangles:                       return "GL_EXT_window_rectangles";
		case OpenGlExtension_GL_EXT_422_pixels:                              return "GL_EXT_422_pixels";
		case OpenGlExtension_GL_EXT_clip_volume_hint:                        return "GL_EXT_clip_volume_hint";
		case OpenGlExtension_GL_EXT_cmyka:                                   return "GL_EXT_cmyka";
		case OpenGlExtension_GL_EXT_coordinate_frame:                        return "GL_EXT_coordinate_frame";
		case OpenGlExtension_GL_EXT_cull_vertex:                             return "GL_EXT_cull_vertex";
		case OpenGlExtension_GL_EXT_fragment_lighting:                       return "GL_EXT_fragment_lighting";
		case OpenGlExtension_GL_EXT_index_array_formats:                     return "GL_EXT_index_array_formats";
		case OpenGlExtension_GL_EXT_index_func:                              return "GL_EXT_index_func";
		case OpenGlExtension_GL_EXT_index_material:                          return "GL_EXT_index_material";
		case OpenGlExtension_GL_EXT_index_texture:                           return "GL_EXT_index_texture";
		case OpenGlExtension_GL_EXT_light_texture:                           return "GL_EXT_light_texture";
		case OpenGlExtension_GL_EXT_misc_attribute:                          return "GL_EXT_misc_attribute";
		case OpenGlExtension_GL_EXT_multisample:                             return "GL_EXT_multisample";
		case OpenGlExtension_GL_EXT_paletted_texture:                        return "GL_EXT_paletted_texture";
		case OpenGlExtension_GL_EXT_pixel_transform:                         return "GL_EXT_pixel_transform";
		case OpenGlExtension_GL_EXT_pixel_transform_color_table:             return "GL_EXT_pixel_transform_color_table";
		case OpenGlExtension_GL_EXT_scene_marker:                            return "GL_EXT_scene_marker";
		case OpenGlExtension_GL_EXT_shared_texture_palette:                  return "GL_EXT_shared_texture_palette";
		case OpenGlExtension_GL_EXT_static_vertex_array:                     return "GL_EXT_static_vertex_array";
		case OpenGlExtension_GL_EXT_texture_env:                             return "GL_EXT_texture_env";
		case OpenGlExtension_GL_EXT_texture_perturb_normal:                  return "GL_EXT_texture_perturb_normal";
		case OpenGlExtension_GL_EXT_texture_rectangle:                       return "GL_EXT_texture_rectangle";
		case OpenGlExtension_GL_EXT_vertex_array_set:                        return "GL_EXT_vertex_array_set";
		case OpenGlExtension_GL_EXT_vertex_array_setXXX:                     return "GL_EXT_vertex_array_setXXX";
		case OpenGlExtension_GL_EXT_vertex_shader:                           return "GL_EXT_vertex_shader";
		case OpenGlExtension_GL_EXT_vertex_weighting:                        return "GL_EXT_vertex_weighting";
		case OpenGlExtension_GL_EXT_EGL_image_array:                         return "GL_EXT_EGL_image_array";
		case OpenGlExtension_GL_EXT_EGL_image_external_wrap_modes:           return "GL_EXT_EGL_image_external_wrap_modes";
		case OpenGlExtension_GL_EXT_EGL_image_storage:                       return "GL_EXT_EGL_image_storage";
		case OpenGlExtension_GL_EXT_EGL_image_storage_compression:           return "GL_EXT_EGL_image_storage_compression";
		case OpenGlExtension_GL_EXT_EGL_sync:                                return "GL_EXT_EGL_sync";
		case OpenGlExtension_GL_EXT_YUV_target:                              return "GL_EXT_YUV_target";
		case OpenGlExtension_GL_EXT_base_instance:                           return "GL_EXT_base_instance";
		case OpenGlExtension_GL_EXT_blend_func_extended:                     return "GL_EXT_blend_func_extended";
		case OpenGlExtension_GL_EXT_buffer_storage:                          return "GL_EXT_buffer_storage";
		case OpenGlExtension_GL_EXT_clear_texture:                           return "GL_EXT_clear_texture";
		case OpenGlExtension_GL_EXT_clip_control:                            return "GL_EXT_clip_control";
		case OpenGlExtension_GL_EXT_clip_cull_distance:                      return "GL_EXT_clip_cull_distance";
		case OpenGlExtension_GL_EXT_color_buffer_float:                      return "GL_EXT_color_buffer_float";
		case OpenGlExtension_GL_EXT_color_buffer_half_float:                 return "GL_EXT_color_buffer_half_float";
		case OpenGlExtension_GL_EXT_compressed_ETC1_RGB8_sub_texture:        return "GL_EXT_compressed_ETC1_RGB8_sub_texture";
		case OpenGlExtension_GL_EXT_conservative_depth:                      return "GL_EXT_conservative_depth";
		case OpenGlExtension_GL_EXT_copy_image:                              return "GL_EXT_copy_image";
		case OpenGlExtension_GL_EXT_debug_label:                             return "GL_EXT_debug_label";
		case OpenGlExtension_GL_EXT_debug_marker:                            return "GL_EXT_debug_marker";
		case OpenGlExtension_GL_EXT_depth_clamp:                             return "GL_EXT_depth_clamp";
		case OpenGlExtension_GL_EXT_discard_framebuffer:                     return "GL_EXT_discard_framebuffer";
		case OpenGlExtension_GL_EXT_disjoint_timer_query:                    return "GL_EXT_disjoint_timer_query";
		case OpenGlExtension_GL_EXT_draw_buffers:                            return "GL_EXT_draw_buffers";
		case OpenGlExtension_GL_EXT_draw_buffers_indexed:                    return "GL_EXT_draw_buffers_indexed";
		case OpenGlExtension_GL_EXT_draw_elements_base_vertex:               return "GL_EXT_draw_elements_base_vertex";
		case OpenGlExtension_GL_EXT_draw_transform_feedback:                 return "GL_EXT_draw_transform_feedback";
		case OpenGlExtension_GL_EXT_external_buffer:                         return "GL_EXT_external_buffer";
		case OpenGlExtension_GL_EXT_external_objects:                        return "GL_EXT_external_objects";
		case OpenGlExtension_GL_EXT_external_objects_fd:                     return "GL_EXT_external_objects_fd";
		case OpenGlExtension_GL_EXT_external_objects_win32:                  return "GL_EXT_external_objects_win32";
		case OpenGlExtension_GL_EXT_float_blend:                             return "GL_EXT_float_blend";
		case OpenGlExtension_GL_EXT_frag_depth:                              return "GL_EXT_frag_depth";
		case OpenGlExtension_GL_EXT_fragment_shading_rate:                   return "GL_EXT_fragment_shading_rate";
		case OpenGlExtension_GL_EXT_framebuffer_blit_layers:                 return "GL_EXT_framebuffer_blit_layers";
		case OpenGlExtension_GL_EXT_geometry_shader:                         return "GL_EXT_geometry_shader";
		case OpenGlExtension_GL_EXT_gpu_shader5:                             return "GL_EXT_gpu_shader5";
		case OpenGlExtension_GL_EXT_instanced_arrays:                        return "GL_EXT_instanced_arrays";
		case OpenGlExtension_GL_EXT_map_buffer_range:                        return "GL_EXT_map_buffer_range";
		case OpenGlExtension_GL_EXT_multi_draw_indirect:                     return "GL_EXT_multi_draw_indirect";
		case OpenGlExtension_GL_EXT_multiple_textures:                       return "GL_EXT_multiple_textures";
		case OpenGlExtension_GL_EXT_multisample_compatibility:               return "GL_EXT_multisample_compatibility";
		case OpenGlExtension_GL_EXT_multisampled_render_to_texture:          return "GL_EXT_multisampled_render_to_texture";
		case OpenGlExtension_GL_EXT_multisampled_render_to_texture2:         return "GL_EXT_multisampled_render_to_texture2";
		case OpenGlExtension_GL_EXT_multiview_draw_buffers:                  return "GL_EXT_multiview_draw_buffers";
		case OpenGlExtension_GL_EXT_multiview_tessellation_geometry_shader:  return "GL_EXT_multiview_tessellation_geometry_shader";
		case OpenGlExtension_GL_EXT_occlusion_query_boolean:                 return "GL_EXT_occlusion_query_boolean";
		case OpenGlExtension_GL_EXT_primitive_bounding_box:                  return "GL_EXT_primitive_bounding_box";
		case OpenGlExtension_GL_EXT_protected_textures:                      return "GL_EXT_protected_textures";
		case OpenGlExtension_GL_EXT_pvrtc_sRGB:                              return "GL_EXT_pvrtc_sRGB";
		case OpenGlExtension_GL_EXT_read_format_bgra:                        return "GL_EXT_read_format_bgra";
		case OpenGlExtension_GL_EXT_render_snorm:                            return "GL_EXT_render_snorm";
		case OpenGlExtension_GL_EXT_robustness:                              return "GL_EXT_robustness";
		case OpenGlExtension_GL_EXT_sRGB:                                    return "GL_EXT_sRGB";
		case OpenGlExtension_GL_EXT_sRGB_write_control:                      return "GL_EXT_sRGB_write_control";
		case OpenGlExtension_GL_EXT_separate_depth_stencil:                  return "GL_EXT_separate_depth_stencil";
		case OpenGlExtension_GL_EXT_shader_framebuffer_fetch:                return "GL_EXT_shader_framebuffer_fetch";
		case OpenGlExtension_GL_EXT_shader_group_vote:                       return "GL_EXT_shader_group_vote";
		case OpenGlExtension_GL_EXT_shader_implicit_conversions:             return "GL_EXT_shader_implicit_conversions";
		case OpenGlExtension_GL_EXT_shader_io_blocks:                        return "GL_EXT_shader_io_blocks";
		case OpenGlExtension_GL_EXT_shader_non_constant_global_initializers: return "GL_EXT_shader_non_constant_global_initializers";
		case OpenGlExtension_GL_EXT_shader_pixel_local_storage:              return "GL_EXT_shader_pixel_local_storage";
		case OpenGlExtension_GL_EXT_shader_pixel_local_storage2:             return "GL_EXT_shader_pixel_local_storage2";
		case OpenGlExtension_GL_EXT_shader_samples_identical:                return "GL_EXT_shader_samples_identical";
		case OpenGlExtension_GL_EXT_shader_texture_lod:                      return "GL_EXT_shader_texture_lod";
		case OpenGlExtension_GL_EXT_shadow_samplers:                         return "GL_EXT_shadow_samplers";
		case OpenGlExtension_GL_EXT_sparse_texture:                          return "GL_EXT_sparse_texture";
		case OpenGlExtension_GL_EXT_stencil_clear_tag:                       return "GL_EXT_stencil_clear_tag";
		case OpenGlExtension_GL_EXT_swap_control:                            return "GL_EXT_swap_control";
		case OpenGlExtension_GL_EXT_tessellation_shader:                     return "GL_EXT_tessellation_shader";
		case OpenGlExtension_GL_EXT_texenv_op:                               return "GL_EXT_texenv_op";
		case OpenGlExtension_GL_EXT_texture_border_clamp:                    return "GL_EXT_texture_border_clamp";
		case OpenGlExtension_GL_EXT_texture_buffer:                          return "GL_EXT_texture_buffer";
		case OpenGlExtension_GL_EXT_texture_compression_astc_decode_mode:    return "GL_EXT_texture_compression_astc_decode_mode";
		case OpenGlExtension_GL_EXT_texture_compression_bptc:                return "GL_EXT_texture_compression_bptc";
		case OpenGlExtension_GL_EXT_texture_compression_s3tc_srgb:           return "GL_EXT_texture_compression_s3tc_srgb";
		case OpenGlExtension_GL_EXT_texture_cube_map_array:                  return "GL_EXT_texture_cube_map_array";
		case OpenGlExtension_GL_EXT_texture_format_BGRA8888:                 return "GL_EXT_texture_format_BGRA8888";
		case OpenGlExtension_GL_EXT_texture_format_sRGB_override:            return "GL_EXT_texture_format_sRGB_override";
		case OpenGlExtension_GL_EXT_texture_mirror_clamp_to_edge:            return "GL_EXT_texture_mirror_clamp_to_edge";
		case OpenGlExtension_GL_EXT_texture_norm16:                          return "GL_EXT_texture_norm16";
		case OpenGlExtension_GL_EXT_texture_query_lod:                       return "GL_EXT_texture_query_lod";
		case OpenGlExtension_GL_EXT_texture_rg:                              return "GL_EXT_texture_rg";
		case OpenGlExtension_GL_EXT_texture_sRGB_RG8:                        return "GL_EXT_texture_sRGB_RG8";
		case OpenGlExtension_GL_EXT_texture_snorm:                           return "GL_EXT_texture_snorm";
		case OpenGlExtension_GL_EXT_texture_storage_compression:             return "GL_EXT_texture_storage_compression";
		case OpenGlExtension_GL_EXT_texture_type_2_10_10_10_REV:             return "GL_EXT_texture_type_2_10_10_10_REV";
		case OpenGlExtension_GL_EXT_texture_view:                            return "GL_EXT_texture_view";
		case OpenGlExtension_GL_EXT_transform_feedback:                      return "GL_EXT_transform_feedback";
		case OpenGlExtension_GL_EXT_unpack_subimage:                         return "GL_EXT_unpack_subimage";
		case OpenGlExtension_GL_EXT_x11_sync_object:                         return "GL_EXT_x11_sync_object";
		case OpenGlExtension_GL_EXT_abgr:                                    return "GL_EXT_abgr";
		case OpenGlExtension_GL_EXT_blend_equation_separate:                 return "GL_EXT_blend_equation_separate";
		case OpenGlExtension_GL_EXT_compiled_vertex_array:                   return "GL_EXT_compiled_vertex_array";
		case OpenGlExtension_GL_EXT_depth_bounds_test:                       return "GL_EXT_depth_bounds_test";
		case OpenGlExtension_GL_EXT_framebuffer_object:                      return "GL_EXT_framebuffer_object";
		case OpenGlExtension_GL_EXT_packed_depth_stencil:                    return "GL_EXT_packed_depth_stencil";
		case OpenGlExtension_GL_EXT_point_parameters:                        return "GL_EXT_point_parameters";
		case OpenGlExtension_GL_EXT_shadow_funcs:                            return "GL_EXT_shadow_funcs";
		case OpenGlExtension_GL_EXT_texture_compression_dxt1:                return "GL_EXT_texture_compression_dxt1";
		case OpenGlExtension_GL_EXT_texture_compression_s3tc:                return "GL_EXT_texture_compression_s3tc";
		case OpenGlExtension_GL_EXT_texture_cube_map:                        return "GL_EXT_texture_cube_map";
		case OpenGlExtension_GL_EXT_texture_edge_clamp:                      return "GL_EXT_texture_edge_clamp";
		case OpenGlExtension_GL_EXT_texture_env_add:                         return "GL_EXT_texture_env_add";
		case OpenGlExtension_GL_EXT_texture_env_combine:                     return "GL_EXT_texture_env_combine";
		case OpenGlExtension_GL_EXT_texture_env_dot3:                        return "GL_EXT_texture_env_dot3";
		case OpenGlExtension_GL_EXT_texture_filter_anisotropic:              return "GL_EXT_texture_filter_anisotropic";
		case OpenGlExtension_GL_EXT_texture_mirror_clamp:                    return "GL_EXT_texture_mirror_clamp";
		case OpenGlExtension_GL_NV_alpha_to_coverage_dither_control:         return "GL_NV_alpha_to_coverage_dither_control";
		case OpenGlExtension_GL_NV_bindless_multi_draw_indirect:             return "GL_NV_bindless_multi_draw_indirect";
		case OpenGlExtension_GL_NV_bindless_multi_draw_indirect_count:       return "GL_NV_bindless_multi_draw_indirect_count";
		case OpenGlExtension_GL_NV_bindless_texture:                         return "GL_NV_bindless_texture";
		case OpenGlExtension_GL_NV_blend_equation_advanced:                  return "GL_NV_blend_equation_advanced";
		case OpenGlExtension_GL_NV_blend_minmax_factor:                      return "GL_NV_blend_minmax_factor";
		case OpenGlExtension_GL_NV_command_list:                             return "GL_NV_command_list";
		case OpenGlExtension_GL_NV_compute_program5:                         return "GL_NV_compute_program5";
		case OpenGlExtension_GL_NV_conservative_raster:                      return "GL_NV_conservative_raster";
		case OpenGlExtension_GL_NV_conservative_raster_dilate:               return "GL_NV_conservative_raster_dilate";
		case OpenGlExtension_GL_NV_copy_image:                               return "GL_NV_copy_image";
		case OpenGlExtension_GL_NV_draw_texture:                             return "GL_NV_draw_texture";
		case OpenGlExtension_GL_NV_draw_vulkan_image:                        return "GL_NV_draw_vulkan_image";
		case OpenGlExtension_GL_NV_explicit_multisample:                     return "GL_NV_explicit_multisample";
		case OpenGlExtension_GL_NV_fill_rectangle:                           return "GL_NV_fill_rectangle";
		case OpenGlExtension_GL_NV_fragment_coverage_to_color:               return "GL_NV_fragment_coverage_to_color";
		case OpenGlExtension_GL_NV_fragment_shader_interlock:                return "GL_NV_fragment_shader_interlock";
		case OpenGlExtension_GL_NV_framebuffer_mixed_samples:                return "GL_NV_framebuffer_mixed_samples";
		case OpenGlExtension_GL_NV_framebuffer_multisample_coverage:         return "GL_NV_framebuffer_multisample_coverage";
		case OpenGlExtension_GL_NV_geometry_shader4:                         return "GL_NV_geometry_shader4";
		case OpenGlExtension_GL_NV_geometry_shader_passthrough:              return "GL_NV_geometry_shader_passthrough";
		case OpenGlExtension_GL_NV_gpu_multicast:                            return "GL_NV_gpu_multicast";
		case OpenGlExtension_GL_NV_gpu_program4:                             return "GL_NV_gpu_program4";
		case OpenGlExtension_GL_NV_gpu_program5:                             return "GL_NV_gpu_program5";
		case OpenGlExtension_GL_NV_gpu_program5_mem_extended:                return "GL_NV_gpu_program5_mem_extended";
		case OpenGlExtension_GL_NV_gpu_shader5:                              return "GL_NV_gpu_shader5";
		case OpenGlExtension_GL_NV_internalformat_sample_query:              return "GL_NV_internalformat_sample_query";
		case OpenGlExtension_GL_NV_memory_attachment:                        return "GL_NV_memory_attachment";
		case OpenGlExtension_GL_NV_memory_object_sparse:                     return "GL_NV_memory_object_sparse";
		case OpenGlExtension_GL_NV_multisample_coverage:                     return "GL_NV_multisample_coverage";
		case OpenGlExtension_GL_NV_parameter_buffer_object:                  return "GL_NV_parameter_buffer_object";
		case OpenGlExtension_GL_NV_parameter_buffer_object2:                 return "GL_NV_parameter_buffer_object2";
		case OpenGlExtension_GL_NV_path_rendering:                           return "GL_NV_path_rendering";
		case OpenGlExtension_GL_NV_path_rendering_shared_edge:               return "GL_NV_path_rendering_shared_edge";
		case OpenGlExtension_GL_NV_query_resource:                           return "GL_NV_query_resource";
		case OpenGlExtension_GL_NV_query_resource_tag:                       return "GL_NV_query_resource_tag";
		case OpenGlExtension_GL_NV_sample_locations:                         return "GL_NV_sample_locations";
		case OpenGlExtension_GL_NV_sample_mask_override_coverage:            return "GL_NV_sample_mask_override_coverage";
		case OpenGlExtension_GL_NV_shader_atomic_counters:                   return "GL_NV_shader_atomic_counters";
		case OpenGlExtension_GL_NV_shader_atomic_float:                      return "GL_NV_shader_atomic_float";
		case OpenGlExtension_GL_NV_shader_atomic_fp16_vector:                return "GL_NV_shader_atomic_fp16_vector";
		case OpenGlExtension_GL_NV_shader_atomic_int64:                      return "GL_NV_shader_atomic_int64";
		case OpenGlExtension_GL_NV_shader_buffer_load:                       return "GL_NV_shader_buffer_load";
		case OpenGlExtension_GL_NV_shader_storage_buffer_object:             return "GL_NV_shader_storage_buffer_object";
		case OpenGlExtension_GL_NV_shader_subgroup_partitioned:              return "GL_NV_shader_subgroup_partitioned";
		case OpenGlExtension_GL_NV_shader_thread_group:                      return "GL_NV_shader_thread_group";
		case OpenGlExtension_GL_NV_shader_thread_shuffle:                    return "GL_NV_shader_thread_shuffle";
		case OpenGlExtension_GL_NV_texture_barrier:                          return "GL_NV_texture_barrier";
		case OpenGlExtension_GL_NV_texture_multisample:                      return "GL_NV_texture_multisample";
		case OpenGlExtension_GL_NV_texture_rectangle_compressed:             return "GL_NV_texture_rectangle_compressed";
		case OpenGlExtension_GL_NV_timeline_semaphore:                       return "GL_NV_timeline_semaphore";
		case OpenGlExtension_GL_NV_transform_feedback:                       return "GL_NV_transform_feedback";
		case OpenGlExtension_GL_NV_transform_feedback2:                      return "GL_NV_transform_feedback2";
		case OpenGlExtension_GL_NV_uniform_buffer_std430_layout:             return "GL_NV_uniform_buffer_std430_layout";
		case OpenGlExtension_GL_NV_uniform_buffer_unified_memory:            return "GL_NV_uniform_buffer_unified_memory";
		case OpenGlExtension_GL_NV_vertex_attrib_integer_64bit:              return "GL_NV_vertex_attrib_integer_64bit";
		case OpenGlExtension_GL_NV_vertex_buffer_unified_memory:             return "GL_NV_vertex_buffer_unified_memory";
		case OpenGlExtension_GL_NV_viewport_array2:                          return "GL_NV_viewport_array2";
		case OpenGlExtension_GL_NV_viewport_swizzle:                         return "GL_NV_viewport_swizzle";
		case OpenGlExtension_GL_NV_element_array:                            return "GL_NV_element_array";
		case OpenGlExtension_GL_NV_evaluators:                               return "GL_NV_evaluators";
		case OpenGlExtension_GL_NV_texgen_emboss:                            return "GL_NV_texgen_emboss";
		case OpenGlExtension_GL_NV_texture_expand_normal:                    return "GL_NV_texture_expand_normal";
		case OpenGlExtension_GL_NV_copy_depth_to_color:                      return "GL_NV_copy_depth_to_color";
		case OpenGlExtension_GL_NV_depth_clamp:                              return "GL_NV_depth_clamp";
		case OpenGlExtension_GL_NV_fence:                                    return "GL_NV_fence";
		case OpenGlExtension_GL_NV_float_buffer:                             return "GL_NV_float_buffer";
		case OpenGlExtension_GL_NV_fog_distance:                             return "GL_NV_fog_distance";
		case OpenGlExtension_GL_NV_fragment_program:                         return "GL_NV_fragment_program";
		case OpenGlExtension_GL_NV_fragment_program2:                        return "GL_NV_fragment_program2";
		case OpenGlExtension_GL_NV_fragment_program_option:                  return "GL_NV_fragment_program_option";
		case OpenGlExtension_GL_NV_light_max_exponent:                       return "GL_NV_light_max_exponent";
		case OpenGlExtension_GL_NV_multisample_filter_hint:                  return "GL_NV_multisample_filter_hint";
		case OpenGlExtension_GL_NV_occlusion_query:                          return "GL_NV_occlusion_query";
		case OpenGlExtension_GL_NV_packed_depth_stencil:                     return "GL_NV_packed_depth_stencil";
		case OpenGlExtension_GL_NV_pixel_data_range:                         return "GL_NV_pixel_data_range";
		case OpenGlExtension_GL_NV_point_sprite:                             return "GL_NV_point_sprite";
		case OpenGlExtension_GL_NV_register_combiners:                       return "GL_NV_register_combiners";
		case OpenGlExtension_GL_NV_register_combiners2:                      return "GL_NV_register_combiners2";
		case OpenGlExtension_GL_NV_texgen_reflection:                        return "GL_NV_texgen_reflection";
		case OpenGlExtension_GL_NV_texture_compression_vtc:                  return "GL_NV_texture_compression_vtc";
		case OpenGlExtension_GL_NV_texture_env_combine4:                     return "GL_NV_texture_env_combine4";
		case OpenGlExtension_GL_NV_texture_rectangle:                        return "GL_NV_texture_rectangle";
		case OpenGlExtension_GL_NV_texture_shader:                           return "GL_NV_texture_shader";
		case OpenGlExtension_GL_NV_texture_shader2:                          return "GL_NV_texture_shader2";
		case OpenGlExtension_GL_NV_texture_shader3:                          return "GL_NV_texture_shader3";
		case OpenGlExtension_GL_NV_vertex_array_range:                       return "GL_NV_vertex_array_range";
		case OpenGlExtension_GL_NV_vertex_array_range2:                      return "GL_NV_vertex_array_range2";
		case OpenGlExtension_GL_NV_vertex_program:                           return "GL_NV_vertex_program";
		case OpenGlExtension_GL_NV_vertex_program1_1:                        return "GL_NV_vertex_program1_1";
		case OpenGlExtension_GL_NV_vertex_program2:                          return "GL_NV_vertex_program2";
		case OpenGlExtension_GL_NV_vertex_program2_option:                   return "GL_NV_vertex_program2_option";
		case OpenGlExtension_GL_NV_vertex_program3:                          return "GL_NV_vertex_program3";
		case OpenGlExtension_GL_NV_3dvision_settings:                        return "GL_NV_3dvision_settings";
		case OpenGlExtension_GL_NV_EGL_stream_consumer_external:             return "GL_NV_EGL_stream_consumer_external";
		case OpenGlExtension_GL_NV_bgr:                                      return "GL_NV_bgr";
		case OpenGlExtension_GL_NV_clip_space_w_scaling:                     return "GL_NV_clip_space_w_scaling";
		case OpenGlExtension_GL_NV_compute_shader_derivatives:               return "GL_NV_compute_shader_derivatives";
		case OpenGlExtension_GL_NV_conservative_raster_pre_snap:             return "GL_NV_conservative_raster_pre_snap";
		case OpenGlExtension_GL_NV_conservative_raster_pre_snap_triangles:   return "GL_NV_conservative_raster_pre_snap_triangles";
		case OpenGlExtension_GL_NV_conservative_raster_underestimation:      return "GL_NV_conservative_raster_underestimation";
		case OpenGlExtension_GL_NV_copy_buffer:                              return "GL_NV_copy_buffer";
		case OpenGlExtension_GL_NV_deep_texture3D:                           return "GL_NV_deep_texture3D";
		case OpenGlExtension_GL_NV_draw_buffers:                             return "GL_NV_draw_buffers";
		case OpenGlExtension_GL_NV_draw_instanced:                           return "GL_NV_draw_instanced";
		case OpenGlExtension_GL_NV_explicit_attrib_location:                 return "GL_NV_explicit_attrib_location";
		case OpenGlExtension_GL_NV_fbo_color_attachments:                    return "GL_NV_fbo_color_attachments";
		case OpenGlExtension_GL_NV_fragment_program4:                        return "GL_NV_fragment_program4";
		case OpenGlExtension_GL_NV_fragment_shader_barycentric:              return "GL_NV_fragment_shader_barycentric";
		case OpenGlExtension_GL_NV_framebuffer_blit:                         return "GL_NV_framebuffer_blit";
		case OpenGlExtension_GL_NV_framebuffer_multisample:                  return "GL_NV_framebuffer_multisample";
		case OpenGlExtension_GL_NV_generate_mipmap_sRGB:                     return "GL_NV_generate_mipmap_sRGB";
		case OpenGlExtension_GL_NV_geometry_program4:                        return "GL_NV_geometry_program4";
		case OpenGlExtension_GL_NV_image_formats:                            return "GL_NV_image_formats";
		case OpenGlExtension_GL_NV_instanced_arrays:                         return "GL_NV_instanced_arrays";
		case OpenGlExtension_GL_NV_mesh_shader:                              return "GL_NV_mesh_shader";
		case OpenGlExtension_GL_NV_non_square_matrices:                      return "GL_NV_non_square_matrices";
		case OpenGlExtension_GL_NV_pack_subimage:                            return "GL_NV_pack_subimage";
		case OpenGlExtension_GL_NV_packed_float:                             return "GL_NV_packed_float";
		case OpenGlExtension_GL_NV_pixel_buffer_object:                      return "GL_NV_pixel_buffer_object";
		case OpenGlExtension_GL_NV_platform_binary:                          return "GL_NV_platform_binary";
		case OpenGlExtension_GL_NV_polygon_mode:                             return "GL_NV_polygon_mode";
		case OpenGlExtension_GL_NV_present_video:                            return "GL_NV_present_video";
		case OpenGlExtension_GL_NV_primitive_shading_rate:                   return "GL_NV_primitive_shading_rate";
		case OpenGlExtension_GL_NV_read_buffer:                              return "GL_NV_read_buffer";
		case OpenGlExtension_GL_NV_read_depth_stencil:                       return "GL_NV_read_depth_stencil";
		case OpenGlExtension_GL_NV_representative_fragment_test:             return "GL_NV_representative_fragment_test";
		case OpenGlExtension_GL_NV_robustness_video_memory_purge:            return "GL_NV_robustness_video_memory_purge";
		case OpenGlExtension_GL_NV_sRGB_formats:                             return "GL_NV_sRGB_formats";
		case OpenGlExtension_GL_NV_scissor_exclusive:                        return "GL_NV_scissor_exclusive";
		case OpenGlExtension_GL_NV_shader_atomic_float64:                    return "GL_NV_shader_atomic_float64";
		case OpenGlExtension_GL_NV_shader_buffer_store:                      return "GL_NV_shader_buffer_store";
		case OpenGlExtension_GL_NV_shader_noperspective_interpolation:       return "GL_NV_shader_noperspective_interpolation";
		case OpenGlExtension_GL_NV_shader_texture_footprint:                 return "GL_NV_shader_texture_footprint";
		case OpenGlExtension_GL_NV_shading_rate_image:                       return "GL_NV_shading_rate_image";
		case OpenGlExtension_GL_NV_shadow_samplers_array:                    return "GL_NV_shadow_samplers_array";
		case OpenGlExtension_GL_NV_shadow_samplers_cube:                     return "GL_NV_shadow_samplers_cube";
		case OpenGlExtension_GL_NV_stereo_view_rendering:                    return "GL_NV_stereo_view_rendering";
		case OpenGlExtension_GL_NV_tessellation_program5:                    return "GL_NV_tessellation_program5";
		case OpenGlExtension_GL_NV_texture_array:                            return "GL_NV_texture_array";
		case OpenGlExtension_GL_NV_texture_border_clamp:                     return "GL_NV_texture_border_clamp";
		case OpenGlExtension_GL_NV_texture_compression_latc:                 return "GL_NV_texture_compression_latc";
		case OpenGlExtension_GL_NV_texture_compression_s3tc:                 return "GL_NV_texture_compression_s3tc";
		case OpenGlExtension_GL_NV_texture_compression_s3tc_update:          return "GL_NV_texture_compression_s3tc_update";
		case OpenGlExtension_GL_NV_texture_npot_2D_mipmap:                   return "GL_NV_texture_npot_2D_mipmap";
		case OpenGlExtension_GL_NV_vdpau_interop:                            return "GL_NV_vdpau_interop";
		case OpenGlExtension_GL_NV_vdpau_interop2:                           return "GL_NV_vdpau_interop2";
		case OpenGlExtension_GL_NV_vertex_program4:                          return "GL_NV_vertex_program4";
		case OpenGlExtension_GL_NV_video_capture:                            return "GL_NV_video_capture";
		case OpenGlExtension_GL_NV_viewport_array:                           return "GL_NV_viewport_array";
		case OpenGlExtension_WGL_NV_render_depth_texture:                    return "WGL_NV_render_depth_texture";
		case OpenGlExtension_WGL_NV_render_texture_rectangle:                return "WGL_NV_render_texture_rectangle";
		case OpenGlExtension_WGL_NV_DX_interop:                              return "WGL_NV_DX_interop";
		case OpenGlExtension_WGL_NV_DX_interop2:                             return "WGL_NV_DX_interop2";
		case OpenGlExtension_WGL_NV_delay_before_swap:                       return "WGL_NV_delay_before_swap";
		case OpenGlExtension_WGL_NV_gpu_affinity:                            return "WGL_NV_gpu_affinity";
		case OpenGlExtension_WGL_NV_multigpu_context:                        return "WGL_NV_multigpu_context";
		case OpenGlExtension_WGL_NV_swap_group:                              return "WGL_NV_swap_group";
		case OpenGlExtension_WGL_NV_video_output:                            return "WGL_NV_video_output";
		case OpenGlExtension_GL_AMD_seamless_cubemap_per_texture:            return "GL_AMD_seamless_cubemap_per_texture";
		case OpenGlExtension_GL_AMD_multi_draw_indirect:                     return "GL_AMD_multi_draw_indirect";
		case OpenGlExtension_GL_AMD_vertex_shader_viewport_index:            return "GL_AMD_vertex_shader_viewport_index";
		case OpenGlExtension_GL_AMD_vertex_shader_layer:                     return "GL_AMD_vertex_shader_layer";			
		case OpenGlExtension_GL_ATI_draw_buffers:                            return "GL_ATI_draw_buffers";
		case OpenGlExtension_GL_ATI_texture_float:                           return "GL_ATI_texture_float";
		case OpenGlExtension_GL_ATI_texture_mirror_once:                     return "GL_ATI_texture_mirror_once";
		case OpenGlExtension_GL_IBM_rasterpos_clip:                          return "GL_IBM_rasterpos_clip";
		case OpenGlExtension_GL_IBM_texture_mirrored_repeat:                 return "GL_IBM_texture_mirrored_repeat";
		case OpenGlExtension_GL_S3_s3tc:                                     return "GL_S3_s3tc";
		case OpenGlExtension_GL_SGIX_depth_texture:                          return "GL_SGIX_depth_texture";
		case OpenGlExtension_GL_SGIX_shadow:                                 return "GL_SGIX_shadow";
		case OpenGlExtension_GL_SUN_slice_accum:                             return "GL_SUN_slice_accum";
		case OpenGlExtension_GL_WIN_swap_hint:                               return "GL_WIN_swap_hint";
		case OpenGlExtension_WGL_EXT_swap_control:                           return "WGL_EXT_swap_control";
		case OpenGlExtension_GL_EXT_Cg_shader:                               return "GL_EXT_Cg_shader";
		case OpenGlExtension_GL_EXTX_framebuffer_mixed_formats:              return "GL_EXTX_framebuffer_mixed_formats";
		case OpenGlExtension_GL_EXT_pixel_buffer_object:                     return "GL_EXT_pixel_buffer_object";
		case OpenGlExtension_GL_EXT_texture_lod:                             return "GL_EXT_texture_lod";
		case OpenGlExtension_GL_EXT_import_sync_object:                      return "GL_EXT_import_sync_object";
		case OpenGlExtension_GL_KHR_context_flush_control:                   return "GL_KHR_context_flush_control";
		case OpenGlExtension_GL_EXT_memory_object:                           return "GL_EXT_memory_object";
		case OpenGlExtension_GL_EXT_memory_object_win32:                     return "GL_EXT_memory_object_win32";
		case OpenGlExtension_GL_KHR_parallel_shader_compile:                 return "GL_KHR_parallel_shader_compile";
		case OpenGlExtension_GL_KHR_robust_buffer_access_behavior:           return "GL_KHR_robust_buffer_access_behavior";
		case OpenGlExtension_GL_EXT_semaphore:                               return "GL_EXT_semaphore";
		case OpenGlExtension_GL_EXT_semaphore_win32:                         return "GL_EXT_semaphore_win32";
		case OpenGlExtension_GL_KHR_shader_subgroup:                         return "GL_KHR_shader_subgroup";
		case OpenGlExtension_GL_KTX_buffer_region:                           return "GL_KTX_buffer_region";
		case OpenGlExtension_GL_NV_blend_equation_advanced_coherent:         return "GL_NV_blend_equation_advanced_coherent";
		case OpenGlExtension_GL_NV_ES1_1_compatibility:                      return "GL_NV_ES1_1_compatibility";
		case OpenGlExtension_GL_NV_ES3_1_compatibility:                      return "GL_NV_ES3_1_compatibility";
		case OpenGlExtension_GL_NV_feature_query:                            return "GL_NV_feature_query";
		case OpenGlExtension_GL_NV_gpu_program4_1:                           return "GL_NV_gpu_program4_1";
		case OpenGlExtension_GL_NV_gpu_program_fp64:                         return "GL_NV_gpu_program_fp64";
		case OpenGlExtension_GL_NVX_conditional_render:                      return "GL_NVX_conditional_render";
		case OpenGlExtension_GL_NVX_linked_gpu_multicast:                    return "GL_NVX_linked_gpu_multicast";
		case OpenGlExtension_GL_NVX_gpu_multicast2:                          return "GL_NVX_gpu_multicast2";
		case OpenGlExtension_GL_NVX_progress_fence:                          return "GL_NVX_progress_fence";
		case OpenGlExtension_GL_NVX_gpu_memory_info:                         return "GL_NVX_gpu_memory_info";
		case OpenGlExtension_GL_NVX_multigpu_info:                           return "GL_NVX_multigpu_info";
		case OpenGlExtension_GL_NVX_nvenc_interop:                           return "GL_NVX_nvenc_interop";
		case OpenGlExtension_GL_KHR_blend_equation_advanced:                 return "GL_KHR_blend_equation_advanced";
		case OpenGlExtension_GL_KHR_blend_equation_advanced_coherent:        return "GL_KHR_blend_equation_advanced_coherent";
		case OpenGlExtension_GL_OVR_multiview:                               return "GL_OVR_multiview";
		case OpenGlExtension_GL_OVR_multiview2:                              return "GL_OVR_multiview2";
		default: return "Unknown";
	}
}

Version_t GetOpenGlExtensionCoreAdoptionVersion(OpenGlExtension_t extension, bool* wasHeavilyModifiedOut = nullptr)
{
	SetOptionalOutPntr(wasHeavilyModifiedOut, false);
	switch (extension)
	{
		case OpenGlExtension_GL_EXT_texture_object:
		case OpenGlExtension_GL_EXT_copy_texture:
		case OpenGlExtension_GL_EXT_subtexture:
		case OpenGlExtension_GL_EXT_texture:
		case OpenGlExtension_GL_EXT_blend_logic_op:
		case OpenGlExtension_GL_EXT_polygon_offset:
		case OpenGlExtension_GL_EXT_vertex_array:
			return NewVersion(1, 1);
		
		case OpenGlExtension_GL_EXT_blend_minmax:
		case OpenGlExtension_GL_EXT_blend_subtract:
		case OpenGlExtension_GL_EXT_blend_color:
		case OpenGlExtension_GL_EXT_histogram:
		case OpenGlExtension_GL_SGI_color_matrix:
		case OpenGlExtension_GL_EXT_convolution:
		case OpenGlExtension_GL_HP_convolution_border_modes:
		case OpenGlExtension_GL_SUN_convolution_border_modes:
		case OpenGlExtension_GL_EXT_color_subtable:
		case OpenGlExtension_GL_SGI_color_table:
		case OpenGlExtension_GL_EXT_draw_range_elements:
		case OpenGlExtension_GL_SGIS_texture_lod:
		case OpenGlExtension_GL_SGIS_texture_edge_clamp:
		case OpenGlExtension_GL_EXT_separate_specular_color:
		case OpenGlExtension_GL_EXT_rescale_normal:
		case OpenGlExtension_GL_EXT_packed_pixels:
		case OpenGlExtension_GL_EXT_bgra:
		case OpenGlExtension_GL_EXT_texture3D:
			return NewVersion(1, 2);
		
		case OpenGlExtension_GL_ARB_transpose_matrix:
		case OpenGlExtension_GL_ARB_texture_border_clamp:
		case OpenGlExtension_GL_ARB_texture_env_dot3:
		case OpenGlExtension_GL_ARB_texture_env_combine:
		case OpenGlExtension_GL_ARB_texture_env_add:
		case OpenGlExtension_GL_ARB_multitexture:
		case OpenGlExtension_GL_ARB_multisample:
		case OpenGlExtension_GL_ARB_texture_cube_map:
		case OpenGlExtension_GL_ARB_texture_compression:
			return NewVersion(1, 3);
		
		case OpenGlExtension_GL_ARB_window_pos:
		case OpenGlExtension_GL_ARB_texture_mirrored_repeat:
		case OpenGlExtension_GL_EXT_texture_lod_bias:
		case OpenGlExtension_GL_ARB_texture_env_crossbar:
		case OpenGlExtension_GL_EXT_stencil_wrap:
		case OpenGlExtension_GL_EXT_blend_func_separate:
		case OpenGlExtension_GL_EXT_secondary_color:
		case OpenGlExtension_GL_ARB_point_parameters:
		case OpenGlExtension_GL_EXT_multi_draw_arrays:
		case OpenGlExtension_GL_EXT_fog_coord:
		case OpenGlExtension_GL_ARB_depth_texture:
		case OpenGlExtension_GL_ARB_shadow:
		case OpenGlExtension_GL_NV_blend_square:
		case OpenGlExtension_GL_SGIS_generate_mipmap:
			return NewVersion(1, 4);
		
		case OpenGlExtension_GL_EXT_shader_funcs:
		case OpenGlExtension_GL_ARB_occlusion_query:
		case OpenGlExtension_GL_ARB_vertex_buffer_object:
			return NewVersion(1, 5);
		
		case OpenGlExtension_GL_ATI_separate_stencil:
		case OpenGlExtension_GL_EXT_stencil_two_side:
		case OpenGlExtension_GL_ARB_point_sprite:
		case OpenGlExtension_GL_ARB_texture_non_power_of_two:
		case OpenGlExtension_GL_ARB_draw_buffers:
		case OpenGlExtension_GL_ARB_shading_language_100:
		case OpenGlExtension_GL_ARB_vertex_shader:
		case OpenGlExtension_GL_ARB_fragment_shader:
		case OpenGlExtension_GL_ARB_shader_objects:
			if (extension == OpenGlExtension_GL_ARB_shading_language_100) { SetOptionalOutPntr(wasHeavilyModifiedOut, true); }
			if (extension == OpenGlExtension_GL_ARB_fragment_shader) { SetOptionalOutPntr(wasHeavilyModifiedOut, true); }
			if (extension == OpenGlExtension_GL_ARB_shader_objects) { SetOptionalOutPntr(wasHeavilyModifiedOut, true); }
			return NewVersion(2, 0);
		
		case OpenGlExtension_GL_EXT_texture_sRGB:
		case OpenGlExtension_GL_ARB_pixel_buffer_object:
			return NewVersion(2, 1);
		
		case OpenGlExtension_GL_EXT_framebuffer_sRGB:
		case OpenGlExtension_GL_ARB_transform_feedback:
		case OpenGlExtension_GL_EXT_texture_compression_rgtc:
		case OpenGlExtension_GL_EXT_draw_buffers2:
		case OpenGlExtension_GL_EXT_texture_array:
		case OpenGlExtension_GL_EXT_texture_integer:
		case OpenGlExtension_GL_NV_half_float:
		case OpenGlExtension_GL_EXT_half_float_pixel:
		case OpenGlExtension_GL_ARB_color_buffer_float:
		case OpenGlExtension_GL_NV_depth_buffer_float:
		case OpenGlExtension_GL_ARB_texture_float:
		case OpenGlExtension_GL_EXT_packed_float:
		case OpenGlExtension_GL_EXT_texture_shared_exponent:
		case OpenGlExtension_GL_NV_conditional_render:
		case OpenGlExtension_GL_ARB_vertex_array_object:
		case OpenGlExtension_GL_ARB_framebuffer_object:
			return NewVersion(3, 0);
		
		case OpenGlExtension_GL_ARB_texture_rectangle:
		case OpenGlExtension_GL_ARB_texture_buffer_object:
		case OpenGlExtension_GL_NV_primitive_restart:
		case OpenGlExtension_GL_ARB_copy_buffer:
		case OpenGlExtension_GL_ARB_draw_instanced:
		case OpenGlExtension_GL_ARB_uniform_buffer_object:
			return NewVersion(3, 1);
		
		case OpenGlExtension_GL_ARB_geometry_shader4:
		case OpenGlExtension_GL_ARB_sync:
		case OpenGlExtension_GL_ARB_depth_clamp:
		case OpenGlExtension_GL_ARB_texture_multisample:
		case OpenGlExtension_GL_ARB_seamless_cube_map:
		case OpenGlExtension_GL_ARB_provoking_vertex:
		case OpenGlExtension_GL_ARB_fragment_coord_conventions:
		case OpenGlExtension_GL_ARB_draw_elements_base_vertex:
		case OpenGlExtension_GL_ARB_vertex_array_bgra:
			return NewVersion(3, 2);
		
		case OpenGlExtension_GL_ARB_vertex_type_2_10_10_10_rev:
		case OpenGlExtension_GL_ARB_instanced_arrays:
		case OpenGlExtension_GL_ARB_timer_query:
		case OpenGlExtension_GL_ARB_texture_swizzle:
		case OpenGlExtension_GL_ARB_texture_rbg10_a2ui:
		case OpenGlExtension_GL_ARB_sampler_objects:
		case OpenGlExtension_GL_ARB_occlusion_query2:
		case OpenGlExtension_GL_ARB_explicit_attrib_location:
		case OpenGlExtension_GL_ARB_blend_func_extended:
		case OpenGlExtension_GL_ARB_shader_bit_encoding:
			return NewVersion(3, 3);
		
		case OpenGlExtension_GL_ARB_draw_buffers_blend:
		case OpenGlExtension_GL_ARB_transform_feedback2:
		case OpenGlExtension_GL_ARB_transform_feedback3:
		case OpenGlExtension_GL_ARB_texture_cube_map_array:
		case OpenGlExtension_GL_ARB_texture_buffer_object_rgb32:
		case OpenGlExtension_GL_ARB_tessellation_shader:
		case OpenGlExtension_GL_ARB_sample_shading:
		case OpenGlExtension_GL_ARB_draw_indirect:
		case OpenGlExtension_GL_ARB_texture_query_lod:
		case OpenGlExtension_GL_ARB_gpu_shader5:
		case OpenGlExtension_GL_ARB_gpu_shader_fp64:
		case OpenGlExtension_GL_ARB_shader_subroutine:
		case OpenGlExtension_GL_ARB_texture_gather:
			return NewVersion(4, 0);
		
		case OpenGlExtension_GL_ARB_viewport_array:
		case OpenGlExtension_GL_ARB_vertex_attrib_64_bit:
		case OpenGlExtension_GL_ARB_shader_precision:
		case OpenGlExtension_GL_ARB_ES2_compatibility:
		case OpenGlExtension_GL_ARB_separate_shader_objects:
		case OpenGlExtension_GL_ARB_get_program_binary:
			return NewVersion(4, 1);
		
		case OpenGlExtension_GL_ARB_texture_compression_BPTC:
		case OpenGlExtension_GL_ARB_conservative_depth:
		case OpenGlExtension_GL_ARB_map_buffer_alignment:
		case OpenGlExtension_GL_ARB_shading_language_packing:
		case OpenGlExtension_GL_ARB_compressed_texture_pixel_storage:
		case OpenGlExtension_GL_ARB_internalformat_query:
		case OpenGlExtension_GL_ARB_base_instance:
		case OpenGlExtension_GL_ARB_shading_language_420pack:
		case OpenGlExtension_GL_ARB_transform_feedback_instanced:
		case OpenGlExtension_GL_ARB_texture_storage:
		case OpenGlExtension_GL_ARB_shader_image_load_store:
		case OpenGlExtension_GL_ARB_shader_atomic_counters:
			return NewVersion(4, 2);
		
		case OpenGlExtension_GL_GLX_ARB_robustness_isolation:
		case OpenGlExtension_GL_WGL_ARB_robustness_isolation:
		case OpenGlExtension_GL_ARB_robustness_isolation:
		case OpenGlExtension_GL_ARB_robust_buffer_access_behavior:
		case OpenGlExtension_GL_ARB_vertex_attrib_binding:
		case OpenGlExtension_GL_ARB_texture_view:
		case OpenGlExtension_GL_ARB_texture_storage_multisample:
		case OpenGlExtension_GL_ARB_texture_query_levels:
		case OpenGlExtension_GL_ARB_texture_buffer_range:
		case OpenGlExtension_GL_ARB_stencil_texturing:
		case OpenGlExtension_GL_ARB_shader_storage_buffer_object:
		case OpenGlExtension_GL_ARB_shader_image_size:
		case OpenGlExtension_GL_ARB_program_interface_query:
		case OpenGlExtension_GL_ARB_multi_draw_indirect:
		case OpenGlExtension_GL_ARB_invalidate_subdata:
		case OpenGlExtension_GL_ARB_internalformat_query2:
		case OpenGlExtension_GL_ARB_framebuffer_no_attachments:
		case OpenGlExtension_GL_ARB_fragment_layer_viewport:
		case OpenGlExtension_GL_ARB_explicit_uniform_location:
		case OpenGlExtension_GL_ARB_ES3_compatibility:
		case OpenGlExtension_GL_ARB_copy_image:
		case OpenGlExtension_GL_ARB_compute_shader:
		case OpenGlExtension_GL_ARB_clear_buffer_object:
		case OpenGlExtension_GL_ARB_arrays_of_arrays:
		case OpenGlExtension_GL_KHR_debug:
			return NewVersion(4, 3);
		
		case OpenGlExtension_GL_ARB_vertex_type_10f_11f_11f_rev:
		case OpenGlExtension_GL_ARB_texture_stencil8:
		case OpenGlExtension_GL_ARB_texture_mirror_clamp_to_edge:
		case OpenGlExtension_GL_ARB_query_buffer_object:
		case OpenGlExtension_GL_ARB_multi_bind:
		case OpenGlExtension_GL_ARB_enhanced_layouts:
		case OpenGlExtension_GL_ARB_clear_texture:
		case OpenGlExtension_GL_ARB_buffer_storage:
			return NewVersion(4, 4);
		
		case OpenGlExtension_GL_ARB_texture_barrier:
		case OpenGlExtension_GL_ARB_shader_texture_image_samples:
		case OpenGlExtension_GL_KHR_robustness:
		case OpenGlExtension_GL_ARB_get_texture_sub_image:
		case OpenGlExtension_GL_ARB_direct_state_access:
		case OpenGlExtension_GL_ARB_derivative_control:
		case OpenGlExtension_GL_ARB_conditional_render_inverted:
		case OpenGlExtension_GL_ARB_ES3_1_compatibility:
		case OpenGlExtension_GL_ARB_cull_distance:
		case OpenGlExtension_GL_ARB_clip_control:
			return NewVersion(4, 5);
		
		case OpenGlExtension_GL_ARB_shader_group_vote:
		case OpenGlExtension_GL_ARB_shader_atomic_counter_ops:
		case OpenGlExtension_GL_KHR_no_error:
		case OpenGlExtension_GL_ARB_polygon_offset_clamp:
		case OpenGlExtension_GL_ARB_texture_filter_anisotropic:
		case OpenGlExtension_GL_ARB_pipeline_statistics_query:
		case OpenGlExtension_GL_ARB_transform_feedback_overflow_query:
		case OpenGlExtension_GL_ARB_indirect_parameters:
		case OpenGlExtension_GL_ARB_shader_draw_parameters:
		case OpenGlExtension_GL_ARB_gl_spirv:
		case OpenGlExtension_GL_ARB_spirv_extensions:
			return NewVersion(4, 6);
		
		default:
			return NewVersion(0, 0);
	}
}

OpenGlExtensionVendor_t GetOpenGlExtensionVendor(OpenGlExtension_t extension)
{
	switch (extension)
	{
		case OpenGlExtension_GL_EXT_texture_object:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_array:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_minmax:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_subtract:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_color:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_range_elements:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_SGIS_texture_lod:                             return OpenGlExtensionVendor_SGIS;
		case OpenGlExtension_GL_EXT_separate_specular_color:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_rescale_normal:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_packed_pixels:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_bgra:                                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture3D:                                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_transpose_matrix:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_border_clamp:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_env_dot3:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_env_combine:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_env_add:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_multitexture:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_multisample:                              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_cube_map:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_compression:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_window_pos:                               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_mirrored_repeat:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_texture_lod_bias:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_texture_env_crossbar:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_stencil_wrap:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_func_separate:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_secondary_color:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_point_parameters:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_multi_draw_arrays:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_fog_coord:                                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_depth_texture:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shadow:                                   return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_NV_blend_square:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_SGIS_generate_mipmap:                         return OpenGlExtensionVendor_SGIS;
		case OpenGlExtension_GL_ARB_occlusion_query:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_buffer_object:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_stencil_two_side:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_point_sprite:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_non_power_of_two:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_draw_buffers:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shading_language_100:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_shader:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_shader:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_objects:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_texture_sRGB:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_ARB_pixel_buffer_object:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_framebuffer_sRGB:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_rgtc:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_buffers2:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_array:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_integer:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_NV_half_float:                                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_ARB_color_buffer_float:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_NV_depth_buffer_float:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_ARB_texture_float:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_packed_float:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_shared_exponent:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_NV_conditional_render:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_ARB_vertex_array_object:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_framebuffer_object:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_rectangle:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_buffer_object:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_NV_primitive_restart:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_ARB_copy_buffer:                              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_draw_instanced:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_uniform_buffer_object:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_geometry_shader4:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sync:                                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_depth_clamp:                              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_multisample:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_seamless_cube_map:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_provoking_vertex:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_coord_conventions:               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_draw_elements_base_vertex:                return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_array_bgra:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_type_2_10_10_10_rev:               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_instanced_arrays:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_timer_query:                              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_swizzle:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sampler_objects:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_occlusion_query2:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_explicit_attrib_location:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_blend_func_extended:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_bit_encoding:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_draw_buffers_blend:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_transform_feedback2:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_transform_feedback3:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_cube_map_array:                   return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_buffer_object_rgb32:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_tessellation_shader:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sample_shading:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_draw_indirect:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_query_lod:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_gpu_shader5:                              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_gpu_shader_fp64:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_subroutine:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_gather:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_viewport_array:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_precision:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_ES2_compatibility:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_separate_shader_objects:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_get_program_binary:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_compression_BPTC:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_conservative_depth:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_map_buffer_alignment:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shading_language_packing:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_compressed_texture_pixel_storage:         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_internalformat_query:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_base_instance:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shading_language_420pack:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_transform_feedback_instanced:             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_storage:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_image_load_store:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_atomic_counters:                   return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_robust_buffer_access_behavior:            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_attrib_binding:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_view:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_storage_multisample:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_query_levels:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_buffer_range:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_stencil_texturing:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_storage_buffer_object:             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_image_size:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_program_interface_query:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_multi_draw_indirect:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_invalidate_subdata:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_internalformat_query2:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_framebuffer_no_attachments:               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_layer_viewport:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_explicit_uniform_location:                return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_ES3_compatibility:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_copy_image:                               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_compute_shader:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_clear_buffer_object:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_arrays_of_arrays:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_KHR_debug:                                    return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_ARB_vertex_type_10f_11f_11f_rev:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_stencil8:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_mirror_clamp_to_edge:             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_query_buffer_object:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_multi_bind:                               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_enhanced_layouts:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_clear_texture:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_buffer_storage:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_barrier:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_texture_image_samples:             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_KHR_robustness:                               return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_ARB_get_texture_sub_image:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_direct_state_access:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_derivative_control:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_conditional_render_inverted:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_ES3_1_compatibility:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_cull_distance:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_clip_control:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_group_vote:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_atomic_counter_ops:                return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_KHR_no_error:                                 return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_ARB_polygon_offset_clamp:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_filter_anisotropic:               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_pipeline_statistics_query:                return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_transform_feedback_overflow_query:        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_indirect_parameters:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_draw_parameters:                   return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_gl_spirv:                                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_spirv_extensions:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_ES3_2_compatibility:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_bindless_texture:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_compute_variable_group_size:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_debug_output:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_depth_buffer_float:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_program:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_program_shadow:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_fragment_shader_interlock:                return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_framebuffer_sRGB:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_gpu_shader_int64:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_half_float_pixel:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_half_float_vertex:                        return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_map_buffer_range:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_parallel_shader_compile:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_post_depth_coverage:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_robustness:                               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sample_locations:                         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_seamless_cubemap_per_texture:             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_ballot:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_clock:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_texture_lod:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_viewport_layer_array:              return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shading_language_include:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sparse_buffer:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sparse_texture:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sparse_texture2:                          return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_sparse_texture_clamp:                     return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_compression_rgtc:                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_filter_minmax:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_rg:                               return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_texture_rgb10_a2ui:                       return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_attrib_64bit:                      return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_program:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_imaging:                                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_cl_event:                                 return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_compatibility:                            return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_create_context_no_error:                  return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_matrix_palette:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_robustness_application_isolation:         return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shader_stencil_export:                    return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_shadow_ambient:                           return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_ARB_vertex_blend:                             return OpenGlExtensionVendor_ARB;
		case OpenGlExtension_GL_EXT_bindable_uniform:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_direct_state_access:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_instanced:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_framebuffer_blit:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_framebuffer_multisample:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_framebuffer_multisample_blit_scaled:      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_geometry_shader4:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_gpu_program_parameters:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_gpu_shader4:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multiview_texture_multisample:            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multiview_timer_query:                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_polygon_offset_clamp:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_post_depth_coverage:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_provoking_vertex:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_raster_multisample:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_separate_shader_objects:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_image_load_formatted:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_image_load_store:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_integer_mix:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_sparse_texture2:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_buffer_object:                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_latc:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_filter_minmax:                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_sRGB_R8:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_sRGB_decode:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_shadow_lod:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_storage:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_swizzle:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_timer_query:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_transform_feedback2:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_array_bgra:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_attrib_64bit:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_win32_keyed_mutex:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_window_rectangles:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_422_pixels:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_clip_volume_hint:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_cmyka:                                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_coordinate_frame:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_cull_vertex:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_fragment_lighting:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_index_array_formats:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_index_func:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_index_material:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_index_texture:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_light_texture:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_misc_attribute:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multisample:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_paletted_texture:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_pixel_transform:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_pixel_transform_color_table:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_scene_marker:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shared_texture_palette:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_static_vertex_array:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_env:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_perturb_normal:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_rectangle:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_array_set:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_array_setXXX:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_shader:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_vertex_weighting:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_EGL_image_array:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_EGL_image_external_wrap_modes:            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_EGL_image_storage:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_EGL_image_storage_compression:            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_EGL_sync:                                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_YUV_target:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_base_instance:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_func_extended:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_buffer_storage:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_clear_texture:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_clip_control:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_clip_cull_distance:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_color_buffer_float:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_color_buffer_half_float:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_compressed_ETC1_RGB8_sub_texture:         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_conservative_depth:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_copy_image:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_debug_label:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_debug_marker:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_depth_clamp:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_discard_framebuffer:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_disjoint_timer_query:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_buffers:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_buffers_indexed:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_elements_base_vertex:                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_draw_transform_feedback:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_external_buffer:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_external_objects:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_external_objects_fd:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_external_objects_win32:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_float_blend:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_frag_depth:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_fragment_shading_rate:                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_framebuffer_blit_layers:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_geometry_shader:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_gpu_shader5:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_instanced_arrays:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_map_buffer_range:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multi_draw_indirect:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multiple_textures:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multisample_compatibility:                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multisampled_render_to_texture:           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multisampled_render_to_texture2:          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multiview_draw_buffers:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_multiview_tessellation_geometry_shader:   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_occlusion_query_boolean:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_primitive_bounding_box:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_protected_textures:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_pvrtc_sRGB:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_read_format_bgra:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_render_snorm:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_robustness:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_sRGB:                                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_sRGB_write_control:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_separate_depth_stencil:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_framebuffer_fetch:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_group_vote:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_implicit_conversions:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_io_blocks:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_non_constant_global_initializers:  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_pixel_local_storage:               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_pixel_local_storage2:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_samples_identical:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shader_texture_lod:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shadow_samplers:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_sparse_texture:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_stencil_clear_tag:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_swap_control:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_tessellation_shader:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texenv_op:                                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_border_clamp:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_buffer:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_astc_decode_mode:     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_bptc:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_s3tc_srgb:            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_cube_map_array:                   return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_format_BGRA8888:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_format_sRGB_override:             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_mirror_clamp_to_edge:             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_norm16:                           return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_query_lod:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_rg:                               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_sRGB_RG8:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_snorm:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_storage_compression:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_type_2_10_10_10_REV:              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_view:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_transform_feedback:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_unpack_subimage:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_x11_sync_object:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_abgr:                                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_blend_equation_separate:                  return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_compiled_vertex_array:                    return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_depth_bounds_test:                        return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_framebuffer_object:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_packed_depth_stencil:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_point_parameters:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_shadow_funcs:                             return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_dxt1:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_compression_s3tc:                 return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_cube_map:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_edge_clamp:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_env_add:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_env_combine:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_env_dot3:                         return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_filter_anisotropic:               return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_mirror_clamp:                     return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_NV_alpha_to_coverage_dither_control:          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_bindless_multi_draw_indirect:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_bindless_multi_draw_indirect_count:        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_bindless_texture:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_blend_equation_advanced:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_blend_minmax_factor:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_command_list:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_compute_program5:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_conservative_raster:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_conservative_raster_dilate:                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_copy_image:                                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_draw_texture:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_draw_vulkan_image:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_explicit_multisample:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fill_rectangle:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_coverage_to_color:                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_shader_interlock:                 return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_framebuffer_mixed_samples:                 return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_framebuffer_multisample_coverage:          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_geometry_shader4:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_geometry_shader_passthrough:               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_multicast:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_program4:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_program5:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_program5_mem_extended:                 return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_shader5:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_internalformat_sample_query:               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_memory_attachment:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_memory_object_sparse:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_multisample_coverage:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_parameter_buffer_object:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_parameter_buffer_object2:                  return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_path_rendering:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_path_rendering_shared_edge:                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_query_resource:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_query_resource_tag:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_sample_locations:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_sample_mask_override_coverage:             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_atomic_counters:                    return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_atomic_float:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_atomic_fp16_vector:                 return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_atomic_int64:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_buffer_load:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_storage_buffer_object:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_subgroup_partitioned:               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_thread_group:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_thread_shuffle:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_barrier:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_multisample:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_rectangle_compressed:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_timeline_semaphore:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_transform_feedback:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_transform_feedback2:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_uniform_buffer_std430_layout:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_uniform_buffer_unified_memory:             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_attrib_integer_64bit:               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_buffer_unified_memory:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_viewport_array2:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_viewport_swizzle:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_element_array:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_evaluators:                                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texgen_emboss:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_expand_normal:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_copy_depth_to_color:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_depth_clamp:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fence:                                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_float_buffer:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fog_distance:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_program:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_program2:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_program_option:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_light_max_exponent:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_multisample_filter_hint:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_occlusion_query:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_packed_depth_stencil:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_pixel_data_range:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_point_sprite:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_register_combiners:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_register_combiners2:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texgen_reflection:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_compression_vtc:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_env_combine4:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_rectangle:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_shader:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_shader2:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_shader3:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_array_range:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_array_range2:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program1_1:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program2:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program2_option:                    return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program3:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_3dvision_settings:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_EGL_stream_consumer_external:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_bgr:                                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_clip_space_w_scaling:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_compute_shader_derivatives:                return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_conservative_raster_pre_snap:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_conservative_raster_pre_snap_triangles:    return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_conservative_raster_underestimation:       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_copy_buffer:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_deep_texture3D:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_draw_buffers:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_draw_instanced:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_explicit_attrib_location:                  return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fbo_color_attachments:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_program4:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_fragment_shader_barycentric:               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_framebuffer_blit:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_framebuffer_multisample:                   return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_generate_mipmap_sRGB:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_geometry_program4:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_image_formats:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_instanced_arrays:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_mesh_shader:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_non_square_matrices:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_pack_subimage:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_packed_float:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_pixel_buffer_object:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_platform_binary:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_polygon_mode:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_present_video:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_primitive_shading_rate:                    return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_read_buffer:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_read_depth_stencil:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_representative_fragment_test:              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_robustness_video_memory_purge:             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_sRGB_formats:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_scissor_exclusive:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_atomic_float64:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_buffer_store:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_noperspective_interpolation:        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shader_texture_footprint:                  return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shading_rate_image:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shadow_samplers_array:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_shadow_samplers_cube:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_stereo_view_rendering:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_tessellation_program5:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_array:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_border_clamp:                      return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_compression_latc:                  return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_compression_s3tc:                  return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_compression_s3tc_update:           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_texture_npot_2D_mipmap:                    return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vdpau_interop:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vdpau_interop2:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_vertex_program4:                           return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_video_capture:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_viewport_array:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_render_depth_texture:                     return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_render_texture_rectangle:                 return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_DX_interop:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_DX_interop2:                              return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_delay_before_swap:                        return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_gpu_affinity:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_multigpu_context:                         return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_swap_group:                               return OpenGlExtensionVendor_NV;
		case OpenGlExtension_WGL_NV_video_output:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_AMD_seamless_cubemap_per_texture:             return OpenGlExtensionVendor_AMD;
		case OpenGlExtension_GL_AMD_multi_draw_indirect:                      return OpenGlExtensionVendor_AMD;
		case OpenGlExtension_GL_AMD_vertex_shader_viewport_index:             return OpenGlExtensionVendor_AMD;
		case OpenGlExtension_GL_AMD_vertex_shader_layer:                      return OpenGlExtensionVendor_AMD;
		case OpenGlExtension_GL_ATI_draw_buffers:                             return OpenGlExtensionVendor_ATI;
		case OpenGlExtension_GL_ATI_texture_float:                            return OpenGlExtensionVendor_ATI;
		case OpenGlExtension_GL_ATI_texture_mirror_once:                      return OpenGlExtensionVendor_ATI;
		case OpenGlExtension_GL_IBM_rasterpos_clip:                           return OpenGlExtensionVendor_IBM;
		case OpenGlExtension_GL_IBM_texture_mirrored_repeat:                  return OpenGlExtensionVendor_IBM;
		// case OpenGlExtension_GL_S3_s3tc:                                      return OpenGlExtensionVendor_S3;
		case OpenGlExtension_GL_SGIX_depth_texture:                           return OpenGlExtensionVendor_SGIX;
		case OpenGlExtension_GL_SGIX_shadow:                                  return OpenGlExtensionVendor_SGIX;
		case OpenGlExtension_GL_SUN_slice_accum:                              return OpenGlExtensionVendor_SUN;
		case OpenGlExtension_GL_WIN_swap_hint:                                return OpenGlExtensionVendor_WIN;
		case OpenGlExtension_WGL_EXT_swap_control:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_Cg_shader:                                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXTX_framebuffer_mixed_formats:               return OpenGlExtensionVendor_EXTX;
		case OpenGlExtension_GL_EXT_pixel_buffer_object:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_texture_lod:                              return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_import_sync_object:                       return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_KHR_context_flush_control:                    return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_EXT_memory_object:                            return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_memory_object_win32:                      return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_KHR_parallel_shader_compile:                  return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_KHR_robust_buffer_access_behavior:            return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_EXT_semaphore:                                return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_EXT_semaphore_win32:                          return OpenGlExtensionVendor_EXT;
		case OpenGlExtension_GL_KHR_shader_subgroup:                          return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_KTX_buffer_region:                            return OpenGlExtensionVendor_KTX;
		case OpenGlExtension_GL_NV_blend_equation_advanced_coherent:          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_ES1_1_compatibility:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_ES3_1_compatibility:                       return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_feature_query:                             return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_program4_1:                            return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NV_gpu_program_fp64:                          return OpenGlExtensionVendor_NV;
		case OpenGlExtension_GL_NVX_conditional_render:                       return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_linked_gpu_multicast:                     return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_gpu_multicast2:                           return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_progress_fence:                           return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_gpu_memory_info:                          return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_multigpu_info:                            return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_NVX_nvenc_interop:                            return OpenGlExtensionVendor_NVX;
		case OpenGlExtension_GL_KHR_blend_equation_advanced:                  return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_KHR_blend_equation_advanced_coherent:         return OpenGlExtensionVendor_KHR;
		case OpenGlExtension_GL_OVR_multiview:                                return OpenGlExtensionVendor_OVR;
		case OpenGlExtension_GL_OVR_multiview2:                               return OpenGlExtensionVendor_OVR;
		default: return OpenGlExtensionVendor_Unknown;
	}
}

struct OpenGlInfo_t
{
	Version_t version;
	bool extensionSupported[OpenGlExtension_NumExtensions];
};

#endif //  _OPENGL_TYPES_H
