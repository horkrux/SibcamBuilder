// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <stdint.h>
#include <tchar.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath> 

struct quaternion {
	double x;
	double y;
	double z;
	double w;
};

struct frame_data {
	float px;
	float py;
	float pz;
	struct quaternion q;
};

typedef struct {
	uint32_t unk0 = 1;
	uint32_t unk1 = 8;
	uint32_t unk2 = 0x0c;
	uint32_t unk3 = 0x20051014;
	uint32_t pad0 = 0;
	uint32_t last_frame = 0;
	uint32_t ptr_ptr_cam = 0x84;
	uint32_t unk4 = 1;
	uint32_t ptr_frame_data = 0;
	uint32_t ptr_end_frame_data = 0;
	uint32_t num_frame_data = 0;
	uint32_t pad1 = 0;
	uint32_t ptr_eof = 0;
	uint32_t pad2[20] = { 0 };
	uint32_t ptr_cam = 0x0178;
	uint32_t unk5 = 3;
} header;

//some values are arbitrary, since overwritten on frame 0
typedef struct {
	uint32_t pad0 = 0xffff0000;
	uint32_t pad1 = 0xffffffff;
	uint32_t pad2 = 0xffffffff;
	float pos_x = 355.255f;
	float pos_y = -28.5448f;
	float pos_z = -81.2059f;
	float rot_x = -1.45384f;
	float rot_y = 1.91013f;
	float rot_z = -0.0960839f;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float scale_z = 1.0f;
	uint32_t ptr_ptr_frames = 0x184;
	uint32_t pad3 = 0;
	uint32_t pad4 = 0;
	uint32_t ptr_fov = 0;
	uint32_t pad5[43] = { 0 };
	char cam_type[12] = "Camera02";
	uint32_t ptr_frames = 0x1ac;
	uint32_t num_frames = 0;
	uint32_t unk3 = 3;
	float rot_x_1 = rot_x;
	float rot_y_1 = rot_y;
	float rot_z_1 = rot_z;
	float rot_x_2 = rot_x;
	float rot_y_2 = rot_y;
	float rot_z_2 = rot_z;
	uint32_t pad6 = 0;
} cam_setup;

//hardcoded to default setting for now, no per-frame changes after frame 0
typedef struct {
	//int def_fov = 0x3f83fcfc;
	uint32_t def_fov = 0x3f2a25c2;
	uint32_t ptr_fov_data = 0;
	uint32_t num_fov = 1;
	uint32_t pad0 = 0;
	uint32_t fov_frame = 0;
	uint32_t frame_fov = def_fov;
	uint32_t unk0 = 0;
	uint32_t unk1 = 0;
} fov;

int build_frame_data(const int&, const frame_data&, float*);
int build_frames(const int&, int*);
int build_sibcam(const int&, float*, int*, const int&);
int to_euler_angles(const quaternion&, float&, float&, float&);