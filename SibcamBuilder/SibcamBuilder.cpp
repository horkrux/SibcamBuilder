#include "stdafx.h"
#pragma comment(lib, "Ws2_32.lib")

using std::free;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: SibcamBuilder [-c] <path>\n");
		exit(0);
	}
	int err;
	char err_buff[256];
	FILE * havok;
	FILE * xaf;
	char buffer[256];
	int argfile = 1;
	bool freecam = false;
	bool has_fov = false;
	bool for_console = false;
	fov_data* fov_list;

	if (argc == 3) {
		argfile = 2;
		if (strcmp(argv[1], "-c")) {
			printf("Usage: SibcamBuilder [-c] <path>\n");
			exit(0);
		}
		else {
			for_console = true;
		}
	}
	if ((err = fopen_s(&havok, argv[argfile], "r")) != 0) {
		strerror_s(err_buff, 256, err);
		fprintf(stderr, err_buff);
		exit(0);
	}
	else {
		for (int i = 0; i < 2; i++) {
			if (fgets(buffer, 256, havok) != NULL) {
				buffer[0] = '\0';
			}
			else {
				printf("unexpected EOF");
				fclose(havok);
				exit(0);
			}
		}
		if (fgets(buffer, 256, havok) != NULL) {
			int num_frames;
			int num_fov = 1;
			sscanf_s(buffer, "%*s FrameNumber=\"%d\"", &num_frames);
			frame_data * data = (frame_data*)malloc(sizeof(frame_data)*num_frames);
			//read transformations for every frame
			for (int i = 0; i < num_frames; i++) {
				buffer[0] = '\0';
				if (fgets(buffer, 256, havok) != NULL) {
					sscanf_s(buffer, "%*s px=\"%f\" py=\"%f\" pz=\"%f\" rx=\"%lf\" ry=\"%lf\" rz=\"%lf\" rw=\"%lf\"", &data[i].px, &data[i].py, &data[i].pz, &data[i].q.x, &data[i].q.y, &data[i].q.z, &data[i].q.w);
				}
				else {
					printf("unexpected EOF");
					free(data);
					fclose(havok);
					exit(0);
				}
			}
			
			size_t path_length = strlen(argv[argfile]);
			char* xaf_path = (char*)malloc(sizeof(char)*path_length+1);
			strcpy_s(xaf_path, (path_length+1)*sizeof(char), argv[argfile]);
			xaf_path[path_length - 9] = '\0';
			strcat_s(xaf_path, path_length-5, "xaf");
			
			char type[20];
			int frame_rate = 0;
			int ticks = 0;

			if (!fopen_s(&xaf, xaf_path, "r")) {
				fgets(buffer, sizeof(buffer), xaf);
				buffer[0] = '\0';
				if (fgets(buffer, sizeof(buffer), xaf)) {
					if (sscanf_s(buffer, " <SceneInfo %*s startTick=\"0\" %*s frameRate=\"%i\" ticksPerFrame=\"%i\"", &frame_rate, &ticks)) {
						buffer[0] = '\0';
						while (fgets(buffer, sizeof(buffer), xaf)) {
							if (sscanf_s(buffer, " <Controller name=\"%*s \\ Object (%s Camera) \\ FOV\"", type, 20)) {
								if (strcmp(type, "Free")) {
									freecam = true;
								}
								buffer[0] = '\0';
								if (fgets(buffer, sizeof(buffer), xaf)) {
									if (sscanf_s(buffer, " <Keys count=\"%i\"", &num_fov)) {
										fov_list = (fov_data*)malloc(sizeof(fov_data)*num_fov);
										int key_num;
										float fov;
										float tan_in;
										float tan_out;
										for (int i = 0; i < num_fov; i++) {
											buffer[0] = '\0';
											if (fgets(buffer, sizeof(buffer), xaf)) {
												if (sscanf_s(buffer, " <Key t=\"%i\" %*s %*s %*s %*s %*s %*s %*s %*s v=\"%f \" inTanVal=\"%f \" outTanVal=\"%f \"", &key_num, &fov, &tan_in, &tan_out)) {
													has_fov = true;
													if (ticks) {
														fov_list[i].frame_num = key_num / ticks;
													}
													else {
														free(fov_list);
														has_fov = false;
														num_fov = 1;
														break;
													}
													fov_list[i].frame_fov = fov;
													fov_list[i].tan_in = tan_in;
													fov_list[i].tan_out = tan_out;
												}
												else {
													free(fov_list);
													has_fov = false;
													num_fov = 1;
													break;
												}
											}
											else {
												free(fov_list);
												has_fov = false;
												num_fov = 1;
												break;
											}
										}
									}
								}
								break;
							}
						}
					}
				}
				
				fclose(xaf);
			}

			float * data_buff = (float*)malloc(sizeof(float) * 12 * (num_frames-1) + 6 * 4);
			int * frame_buff = (int*)malloc(sizeof(int) * 8 * num_frames);
			build_frame_data(num_frames, *data, data_buff);
			build_frames(num_frames, frame_buff);

			if (!has_fov) {
				fov_list = (fov_data*)malloc(sizeof(fov_data));
				fov_list[0].frame_num = 0;
				fov_list[0].frame_fov = 0;
			}

			if (for_console) {
				build_sibcam_console(num_frames, num_fov, data_buff, frame_buff, fov_list, freecam);
			}
			else {
				build_sibcam(num_frames, num_fov, data_buff, frame_buff, *fov_list, freecam);
			}

			free(fov_list);
			free(data);
			free(data_buff);
			free(frame_buff);
			free(xaf_path);
			fclose(havok);
		}
		else {
			printf("unexpected EOF");
			fclose(havok);
			exit(0);
		}
	}
    return 0;
}

int build_frame_data(const int &num_frames, const frame_data &data, float * buffer) {
	float rot_x = 0;
	float rot_y = 0;
	float rot_z = 0;

	for (int i = 0; i < num_frames; i++) {
		to_euler_angles((&data)[i].q, rot_x, rot_y, rot_z);
		if (i == 0) {
			buffer[12 * i] = (&data)[i].px / 100;
		}
		else {
			buffer[12 * i] = ((&data)[i-1].px + ((&data)[i-1].px - (&data)[i].px)) / 100;
		}
		buffer[12 * i + 1] = (&data)[i].py / 100;
		buffer[12 * i + 2] = (-1)*(&data)[i].pz / -100;
		if (i == (num_frames - 1)) {
			buffer[12 * i + 3] = rot_x;
			buffer[12 * i + 4] = rot_y;
			buffer[12 * i + 5] = rot_z;
		}
		else {
			buffer[12 * i + 6] = rot_x;
			buffer[12 * i + 7] = rot_y;
			buffer[12 * i + 8] = rot_z;
		}
		if (i == 0) {
			buffer[12 * i + 3] = -0.0f;
			buffer[12 * i + 4] = -0.0f;
			buffer[12 * i + 5] = -0.0f;
			buffer[12 * i + 9] = 1.0f;
			buffer[12 * i + 10] = 1.0f;
			buffer[12 * i + 11] = 1.0f;
		}
		else if (i != (num_frames-1)){
			buffer[12 * i + 3] = buffer[12 * i] - buffer[12 * (i - 1)];
			buffer[12 * i + 4] = ((&data)[i].py - (&data)[i-1].py)/100;
			buffer[12 * i + 5] = ((-1)*(&data)[i].pz - (-1)*(&data)[i-1].pz)/100;
			buffer[12 * i + 9] = rot_x - buffer[12 * (i-1) + 6];
			buffer[12 * i + 10] = rot_y - buffer[12 * (i - 1) + 7];
			buffer[12 * i + 11] = rot_z - buffer[12 * (i - 1) + 8];
		}
	}
	return 0;
}

int build_frames(const int &num_frames, int * buffer) {
	for (int i = 0; i < num_frames; i++) {
			buffer[8 * i] = i;
			buffer[8 * i + 1] = i * 4;
			if (i == (num_frames - 1)) {
				buffer[8 * i + 2] = 1;
				buffer[8 * i + 3] = 1;
				buffer[8 * i + 4] = buffer[8 * i + 1] + 1;
				buffer[8 * i + 5] = 1;
				buffer[8 * i + 6] = 1;
			}
			else {
				buffer[8 * i + 2] = buffer[8 * i + 1] + 1;
				buffer[8 * i + 3] = buffer[8 * i + 2];
				buffer[8 * i + 4] = buffer[8 * i + 3] + 1;
				if (i == 0) {
					buffer[8 * i + 5] = 1;
					buffer[8 * i + 6] = 1;
				}
				else {
					buffer[8 * i + 5] = buffer[8 * i + 4] + 1;
					buffer[8 * i + 6] = buffer[8 * i + 5];
				}
			}
			
			buffer[8 * i + 7] = 3;
	}
	return 0;
}

int build_sibcam(const int &num_frames, const int &num_fov, float * buffer_data, int * buffer_frames, const fov_data &fov_list, const bool &freecam) {
	int err;
	FILE * output;
	char err_buff[256];
	header header;
	cam_setup cam;
	fov_header fov;
	if ((err = fopen_s(&output, "camera_win32.sibcam", "w+b")) != 0) {
		strerror_s(err_buff, 256, err);
		fprintf(stderr, err_buff);
		return 1;
	}
	else {
		header.last_frame = num_frames - 1;
		header.ptr_frame_data = sizeof(header) + sizeof(cam) + sizeof(fov) + sizeof(fov_data) * num_fov + 4*8*num_frames;
		header.ptr_end_frame_data = header.ptr_frame_data + 4 * 12 * (num_frames - 1) + 6 * 4;
		header.num_frame_data = (header.ptr_end_frame_data - header.ptr_frame_data)/12;
		header.ptr_eof = header.ptr_end_frame_data;
		fwrite(&header, sizeof(header), 1, output);
		cam.ptr_fov = sizeof(header) + sizeof(cam) + 4 * 8 * num_frames;
		cam.num_frames = num_frames;
		if (freecam) {
			char temp[10] = "FreeCam01";
			for (int i = 0; i < 9; i++) {
				cam.cam_type[i] = temp[i];
			}
		}
		fwrite(&cam, sizeof(cam_setup), 1, output);
		fwrite(buffer_frames, sizeof(int), num_frames * 8, output);
		fov.ptr_fov_data = sizeof(header) + sizeof(cam) + 4 * 8 * num_frames + 4 * 4;
		fov.num_fov = num_fov;
		fov.def_fov = (&fov_list)[0].frame_fov;
		fwrite(&fov, sizeof(fov), 1, output);
		fwrite(&fov_list, sizeof(fov_data), num_fov, output);
		fwrite(buffer_data, sizeof(float), 12 * (num_frames-1) + 6, output);

		//pad out the file
		int num_pads = (16 - (header.ptr_end_frame_data % 16)) / 4;
		if (num_pads) {
			int * padding = (int*)malloc(sizeof(int)*num_pads);
			for (int i = 0; i < num_pads; i++) {
				padding[i] = 0;
			}
			fwrite(padding, sizeof(int), num_pads, output);
			free(padding);
		}
		//fwrite(buffer, sizeof(float), num_frames * 8, output);
	}
	fclose(output);
	return 0;
}

int build_sibcam_console(const int &num_frames, const int &num_fov, float * buffer_data, int * buffer_frames, fov_data * fov_list, const bool &freecam) {
	int err;
	FILE * output;
	char err_buff[256];
	header header;
	cam_setup cam;
	fov_header fov;
	if ((err = fopen_s(&output, "camera.sibcam", "w+b")) != 0) {
		strerror_s(err_buff, 256, err);
		fprintf(stderr, err_buff);
		return 1;
	}
	else {
		header.last_frame = num_frames - 1;
		header.ptr_frame_data = sizeof(header) + sizeof(cam) + sizeof(fov) + sizeof(fov_data) * num_fov + 4 * 8 * num_frames;
		header.ptr_end_frame_data = header.ptr_frame_data + 4 * 12 * (num_frames - 1) + 6 * 4;
		header.num_frame_data = (header.ptr_end_frame_data - header.ptr_frame_data) / 12;
		header.ptr_eof = header.ptr_end_frame_data;

		header.unk0 = *(uint32_t*)little_to_big((long*)&header.unk0);
		header.unk1 = *(uint32_t*)little_to_big((long*)&header.unk1);
		header.unk2 = *(uint32_t*)little_to_big((long*)&header.unk2);
		header.unk3 = *(uint32_t*)little_to_big((long*)&header.unk3);
		header.last_frame = *(uint32_t*)little_to_big((long*)&header.last_frame);
		header.ptr_ptr_cam = *(uint32_t*)little_to_big((long*)&header.ptr_ptr_cam);
		header.unk4 = *(uint32_t*)little_to_big((long*)&header.unk4);
		header.ptr_frame_data = *(uint32_t*)little_to_big((long*)&header.ptr_frame_data);
		header.ptr_end_frame_data = *(uint32_t*)little_to_big((long*)&header.ptr_end_frame_data);
		header.num_frame_data = *(uint32_t*)little_to_big((long*)&header.num_frame_data);
		header.ptr_eof = *(uint32_t*)little_to_big((long*)&header.ptr_eof);
		header.ptr_cam = *(uint32_t*)little_to_big((long*)&header.ptr_cam);
		header.unk5 = *(uint32_t*)little_to_big((long*)&header.unk5);

		fwrite(&header, sizeof(header), 1, output);

		cam.ptr_fov = sizeof(header) + sizeof(cam) + 4 * 8 * num_frames;
		cam.num_frames = num_frames;

		cam.pos_x = *(float*)little_to_big((long*)&cam.pos_x);
		cam.pos_y = *(float*)little_to_big((long*)&cam.pos_y);
		cam.pos_z = *(float*)little_to_big((long*)&cam.pos_z);
		cam.rot_x = *(float*)little_to_big((long*)&cam.rot_x);
		cam.rot_y = *(float*)little_to_big((long*)&cam.rot_y);
		cam.rot_z = *(float*)little_to_big((long*)&cam.rot_z);
		cam.scale_x = *(float*)little_to_big((long*)&cam.scale_x);
		cam.scale_y = *(float*)little_to_big((long*)&cam.scale_y);
		cam.scale_z = *(float*)little_to_big((long*)&cam.scale_z);
		cam.ptr_ptr_frames = *(uint32_t*)little_to_big((long*)&cam.ptr_ptr_frames);
		cam.ptr_fov = *(uint32_t*)little_to_big((long*)&cam.ptr_fov);
		cam.ptr_frames = *(uint32_t*)little_to_big((long*)&cam.ptr_frames);
		cam.num_frames = *(uint32_t*)little_to_big((long*)&cam.num_frames);
		cam.unk3 = *(uint32_t*)little_to_big((long*)&cam.unk3);
		cam.rot_x_1 = *(float*)little_to_big((long*)&cam.rot_x_1);
		cam.rot_y_1 = *(float*)little_to_big((long*)&cam.rot_y_1);
		cam.rot_z_1 = *(float*)little_to_big((long*)&cam.rot_z_1);
		cam.rot_x_2 = *(float*)little_to_big((long*)&cam.rot_x_2);
		cam.rot_y_2 = *(float*)little_to_big((long*)&cam.rot_y_2);
		cam.rot_z_2 = *(float*)little_to_big((long*)&cam.rot_z_2);

		fwrite(&cam, sizeof(cam_setup), 1, output);

		for (int i = 0; i < num_frames * 8; i++) {
			buffer_frames[i] = *(uint32_t*)little_to_big((long*)&buffer_frames[i]);
		}

		fwrite(buffer_frames, sizeof(int), num_frames * 8, output);
		fov.ptr_fov_data = sizeof(header) + sizeof(cam) + 4 * 8 * num_frames + 4 * 4;
		fov.num_fov = num_fov;
		fov.def_fov = fov_list[0].frame_fov;

		fov.def_fov = *(float*)little_to_big((long*)&fov.def_fov);
		fov.ptr_fov_data = *(uint32_t*)little_to_big((long*)&fov.ptr_fov_data);
		fov.num_fov = *(uint32_t*)little_to_big((long*)&fov.num_fov);

		fwrite(&fov, sizeof(fov), 1, output);

		for (int i = 0; i < num_fov; i++) {
			fov_list[i].frame_num = *(uint32_t*)little_to_big((long*)&fov_list[i].frame_num);
			fov_list[i].frame_fov = *(float*)little_to_big((long*)&fov_list[i].frame_fov);
			fov_list[i].tan_in = *(float*)little_to_big((long*)&fov_list[i].tan_in);
			fov_list[i].tan_out = *(float*)little_to_big((long*)&fov_list[i].tan_out);
		}

		fwrite(fov_list, sizeof(fov_data), num_fov, output);

		for (int i = 0; i < 12 * (num_frames - 1) + 6; i++) {
			buffer_data[i] = *(float*)little_to_big((long*)&buffer_data[i]);
		}

		fwrite(buffer_data, sizeof(float), 12 * (num_frames - 1) + 6, output);

		//pad out the file
		int num_pads = (16 - (header.ptr_end_frame_data % 16)) / 4;
		if (num_pads) {
			int * padding = (int*)malloc(sizeof(int)*num_pads);
			for (int i = 0; i < num_pads; i++) {
				padding[i] = 0;
			}
			fwrite(padding, sizeof(int), num_pads, output);
			free(padding);
		}
		//fwrite(buffer, sizeof(float), num_frames * 8, output);
	}
	fclose(output);
	return 0;
}
//1st test orientation
/*int to_euler_angles(const quaternion &q, float &roll, float &pitch, float &yaw) {
	// roll (x-axis rotation)
	double sinr = +2.0 * (q.w * q.x + q.y * q.z);
	//double cosr = q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z;
	double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
	roll = (-1)*(float)atan2(sinr, cosr);
	
	// pitch (y-axis rotation)
	double sinp = +2.0 * (q.w * q.y - q.z * q.x);
	if (fabs(sinp) >= 1) {
		yaw = (float)copysign(M_PI / 2, sinp); // use 90 degrees if out of range
	}
	else {
		//pitch = (float)copysign(M_PI / 2, sinp);
		yaw = (float)asin(sinp);
	}

	// yaw (z-axis rotation)
	double siny = +2.0 * (q.w * q.z + q.x * q.y);
	double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
	pitch = (float)atan2(siny, cosy)+M_PI;
	
	printf("%f, %f, %f\n", roll, pitch, yaw);
	return 0;
}*/

int to_euler_angles(const quaternion &q, float &pitch, float &roll, float &yaw) {
	double sinr = +2.0 * (q.w * q.x + q.y * q.z);
	//double cosr = q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z;
	double cosr = +1.0 - 2.0 * (q.x * q.x + q.y * q.y);
	roll = (float)atan2(sinr, cosr);
	
	double sinp = +2.0 * (q.w * q.y + q.z * q.x);
	if (fabs(sinp) >= 1) {
		pitch = (float)copysign(M_PI / 2, sinp)+M_PI/2;
	}
	else {
		//pitch = (float)copysign(M_PI / 2, sinp);
		pitch = (float)asin(sinp)+M_PI/2;
	}
	
	double siny = +2.0 * (q.w * q.z + q.x * q.y);
	double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
	yaw = (float)atan2(siny, cosy)+M_PI;
	return 0;
}

long* little_to_big(long* value) {
	long temp = htonl(*(long*)value);
	return &temp;
}

/*int to_quaternion(double roll, double pitch, double yaw)
{
	quaternion q;
	// Abbreviations for the various angular functions
	double cy = cos(yaw * 0.5);
	double sy = sin(yaw * 0.5);
	double cr = cos(roll * 0.5);
	double sr = sin(roll * 0.5);
	double cp = cos(pitch * 0.5);
	double sp = sin(pitch * 0.5);

	q.w = sr*sp*sy + cr*cp*cy;
	q.x = sr*cp*cy - sp*sy*cr;
	q.y = (-1)*sr*sp*cy + sy*cr*cp;
	q.z = sr*sy*cp + sp*cr*cy;
	printf("%f, %f, %f, %f\n", q.x, q.y, q.z, q.w);
	return 0;
}*/