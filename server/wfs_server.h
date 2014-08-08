/************************************************************************/
/* wfs_server.h								*/
/*                                                                      */
/* General Header File.							*/
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY CLIENT/SERVER				*/
/*                 Based on the CHARA User Interface			*/
/*                 Based on the SUSI User Interface			*/
/*		In turn based on the CHIP User interface		*/
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA			*/
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@@chara.gsu.edu                                       */
/* WWW      : http://www.chara.gsu.edu			                */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar                          		*/
/* Date   : Aug 2012							*/
/************************************************************************/

#ifndef __WFS_CLISERV__
#define __WFS_CLISERV__

#define SERVER

#include <stdint.h>
#include <clock.h>
#include <cliserv.h>
#include <time.h>
#include <chara_messages.h>
#include <sockman.h>
#include <math.h>
#include <atmcdLXd.h>
#include <chara.h>
#include <zlib.h>
#include <pthread.h>

/* Which camera do we use? */

#define WFS_CAMERA 0

/* Camera Defaults */

#define DFT_ANDOR_EXPOSURE_TIME		0.001
#define DFT_ANDOR_SHUTTER 		ANDOR_SHUTTER_CLOSE
#define DFT_ANDOR_HBIN			1
#define DFT_ANDOR_VBIN			1
#define DFT_ANDOR_HSTART		1
#define DFT_ANDOR_HEND			90
#define DFT_ANDOR_VSTART		1
#define DFT_ANDOR_VEND			90
#define DFT_ANDOR_AMPLIFIER		ANDOR_CCD
#define DFT_ANDOR_PREAMP_GAIN		2
#define DFT_ANDOR_EM_GAIN		100
#define DFT_ANDOR_TEMPERATURE		-50
#define DFT_ANDOR_VERTICAL_SPEED	0
#define DFT_ANDOR_CCD_HORIZONTAL_SPEED	0
#define DFT_ANDOR_EMCCD_HORIZONTAL_SPEED 0
#define DFT_ANDOR_EM_ADVANCED		0
#define DFT_ANDOR_CAMERA_LINK		1

#define WFS_PERIODIC			1

extern bool verbose;
extern char wfs_name[256];
extern int  scope_number;
extern struct s_wfs_andor_setup andor_setup;
extern struct s_wfs_status wfs_status;
extern at_u16 *image_data;
extern bool save_fits;
extern bool use_cameralink;
extern int number_of_processed_frames;
extern float **data_frame;
extern float **dark_frame;
extern float data_threshhold;

/* Prototypes */

/* wfs_server.c */

int main(int argc, char **argv);
void close_function(void);
int wfs_top_job(void);
int wfs_periodic_job(void);
void print_usage_message(char *name);

/* wfs_messages.c */

void setup_wfs_messages(void);
int message_wfs_andor_setup(struct smessage *message);
int message_wfs_andor_set_acqmode(struct smessage *message);
int message_wfs_andor_reopen(struct smessage *message);
int message_wfs_andor_set_exptime(struct smessage *message);
int message_wfs_andor_set_shutter(struct smessage *message);
int message_wfs_andor_set_image(struct smessage *message);
int message_wfs_andor_set_amplifier(struct smessage *message);
int message_wfs_andor_set_temperature(struct smessage *message);
int message_wfs_andor_usb_onoff(struct smessage *message);
int message_wfs_andor_current_frame(struct smessage *message);
int message_wfs_andor_preamp_gain(struct smessage *message);
int message_wfs_andor_em_advanced(struct smessage *message);
int message_wfs_andor_cooler_onoff(struct smessage *message);
int message_wfs_andor_set_em_gain(struct smessage *message);
int message_wfs_andor_vertical_speed(struct smessage *message);
int message_wfs_andor_horizontal_speed(struct smessage *message);
int message_wfs_andor_get_setup(struct smessage *message);
int message_wfs_andor_save_fits(struct smessage *message);
int message_wfs_andor_camlink_onoff(struct smessage *message);

/* wfs_andor.c */

int andor_open(int iSelectedCamera, struct s_wfs_andor_image image,
	       int preamp_gain, int vertical_speed, int ccd_horizontal_speed,
		int em_horizontal_speed);
int andor_setup_camera(struct s_wfs_andor_setup setup);
int andor_close(void);
int andor_send_setup(void);
int andor_set_acqmode(int acqmode);
int andor_set_exptime(float exptime);
int andor_set_shutter(int exptime);
int andor_set_image(struct s_wfs_andor_image image);
int andor_set_crop_mode(int heigth, int width, int vbin, int hbin);
int andor_set_amplifier(int amplifier);
int andor_start_acquisition(void);
int andor_abort_acquisition(void);
int andor_get_status(void);
int andor_wait_for_data(int timeout);
int andor_wait_for_idle(int timeout);
int andor_get_acquired_data(void);
int andor_set_temperature(int temperature);
int andor_get_temperature(void);
int andor_cooler_on(void);
int andor_cooler_off(void);
int andor_get_preamp_gain(int index, float *gain);
int andor_set_preamp_gain(int gain);
int andor_set_em_advanced(int em_advanced);
int andor_set_em_gain(int gain);
int andor_get_total_number_images_acquired(void);
int andor_get_oldest_image(void);
int andor_get_vertical_speed(int index, float *speed);
int andor_set_vertical_speed(int index);
int andor_get_horizontal_speed(int type, int index, float *speed);
int andor_set_horizontal_speed(int type, int index);
int andor_set_camera_link(int onoff);

/* wfs_andor_usb_data.c */

int andor_start_usb_thread(void);
int andor_stop_usb_thread(void);
int andor_start_usb(void);
int andor_stop_usb(void);
void *andor_usb_thread(void *arg);
void lock_usb_mutex(void);
void unlock_usb_mutex(void);

/* wfs_data.c */

void process_data(long time_stamp, int nx, int ny, unsigned short int *data);

/* wfs_andor_camlink_data.c */

void videoirqfunc(int sig);
int andor_start_camlink_thread(void);
int andor_stop_camlink_thread(void);
int andor_start_camlink(void);
int andor_stop_camlink(void);
void *andor_camlink_thread(void *arg);
void lock_camlink_mutex(void);
void unlock_camlink_mutex(void);
#endif
