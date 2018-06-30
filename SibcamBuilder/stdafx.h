// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
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

struct header {
	int unk0 = 1;
	int unk1 = 8;
	int unk2 = 0x0c;
	int unk3 = 0x20051014;
	int pad0 = 0;
	int last_frame = 0;
	int ptr_ptr_cam = 0x84;
	int unk4 = 1;
	int ptr_frame_data = 0;
	int ptr_end_frame_data = 0;
	int num_frame_data = 0;
	int pad1 = 0;
	int ptr_eof = 0;
	int pad2[20] = { 0 };
	int ptr_cam = 0x0178;
	int unk5 = 3;
};

//some values are arbitrary, since overwritten on frame 0
struct cam_setup {
	int pad0 = 0xffff0000;
	int pad1 = 0xffffffff;
	int pad2 = 0xffffffff;
	int pos_x = 0x43b1a0b4;
	int pos_y = 0xc1e45bb2;
	int pos_z = 0xc2a2696e;
	int rot_x = 0xbfba177c;
	int rot_y = 0x3ff47f30;
	int rot_z = 0xbdc4c7a6;
	float scale_x = 1;
	float scale_y = 1;
	float scale_z = 1;
	int ptr_ptr_frames = 0x184;
	int pad3 = 0;
	int pad4 = 0;
	int ptr_fov = 0;
	int pad5[43] = { 0 };
	char cam_type0 = 'C';
	char cam_type1 = 'a';
	char cam_type2 = 'm';
	char cam_type3 = 'e';
	char cam_type4 = 'r';
	char cam_type5 = 'a';
	char cam_type6 = '0';
	char cam_type7 = '2';
	char cam_type8 = 0;
	char cam_type9 = 0;
	char cam_type10 = 0;
	char cam_type11 = 0;
	int ptr_frames = 0x1ac;
	int num_frames = 0;
	int unk3 = 3;
	int rot_x_1 = rot_x;
	int rot_y_1 = rot_y;
	int rot_z_1 = rot_z;
	int rot_x_2 = rot_x;
	int rot_y_2 = rot_y;
	int rot_z_2 = rot_z;
	int pad6 = 0;
};

//hardcoded to default setting for now, no per-frame changes after frame 0
struct fov {
	//int def_fov = 0x3f83fcfc;
	int def_fov = 0x3f2a25c2;
	int ptr_fov_data = 0;
	int num_fov = 1;
	int pad0 = 0;
	int fov_frame = 0;
	int frame_fov = def_fov;
	int unk0 = 0;
	int unk1 = 0;
};

int build_frame_data(const int&, const frame_data&, float*);
int build_frames(const int&, int*);
int build_sibcam(const int&, float*, int*, const int&);
int to_euler_angles(const quaternion&, float&, float&, float&);