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

#include <cliserv.h>
#define SERVER
#undef __CLISERV__
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
#include <nrc.h>

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
#define DFT_ANDOR_EM_GAIN		30
#define DFT_ANDOR_TEMPERATURE		-50
#define DFT_ANDOR_VERTICAL_SPEED	2
#define DFT_ANDOR_CCD_HORIZONTAL_SPEED	0
#define DFT_ANDOR_EMCCD_HORIZONTAL_SPEED 0
#define DFT_ANDOR_EM_ADVANCED		0
#define DFT_ANDOR_CAMERA_LINK		1

#define WFS_PERIODIC			1

#define REF_CENTROID_FILENAME          "reference_centroids.dat"

#define	DFT_DENOM_CLAMP_SUBAP		6
#define	DFT_MIN_FLUX_SUBAP		10
#define	DFT_CLAMP_FLUX_SUBAP		-1000

#define MAX_MIRROR_DELTA		0.1
#define MAX_MIRROR			1.75

#warning We need to update these calibration numbers

#define MIR_CALIB			6.8 /* From old system */
#define DET_CALIB			2.1 /* From old system */

#define DEFAULT_GAIN_X			(-0.15)
#define DEFAULT_DAMP_X			(0.25)
#define DEFAULT_GAIN_Y			(-0.15)
#define DEFAULT_DAMP_Y			(0.25)

/*Shack Hartmann rotation stage */

#define TDC                             "/dev/ttyUSB1"
#define TDC_BRATE                       B115200
#define DFT_TDC_MAX_VEL                 5.
#define DFT_TDC_ACCEL                   2.
#define DFT_TDC_LED_ACT                 FALSE

/* macro */

#define min(a,b)                        (((a)<(b))?(a):(b))
#define max(a,b)                        (((a)>(b))?(a):(b))

extern bool verbose;
extern char wfs_name[256];
extern int  scope_number;
extern struct s_wfs_andor_setup andor_setup;
extern at_u16 *image_data;
extern bool save_fits;
extern bool use_cameralink;
extern int number_of_processed_frames;
extern float **data_frame;
extern float **dark_frame;
extern float **calc_dark_frame;
extern int dark_frame_num;
extern float dark_frame_mean;
extern float dark_frame_stddev;
extern float **raw_frame;
extern float **sum_frame;
extern int sum_frame_num;
extern float data_threshold; /* For data in terms of STDDEV of dark frame. */
extern struct s_wfs_tdc_status tdc_status;
extern struct s_wfs_subap_centroids subap_centroids_mean;
extern struct s_wfs_subap_centroids subap_centroids_ref;
extern struct s_wfs_subap_centroids subap_centroids_offset;
extern struct s_wfs_subap_centroids subap_centroids;
extern struct s_wfs_clamp_fluxes clamp_fluxes;
extern struct s_wfs_tiptilt wfs_tiptilt;
extern int num_mean_aberrations;
extern struct s_wfs_aberrations wfs_mean_aberrations;
extern struct s_wfs_aberrations wfs_aberrations;
extern bool set_subap_centroids_ref;
extern struct s_wfs_tiptilt_modulation wfs_tiptilt_modulation;
extern struct s_wfs_tiptilt_servo wfs_tiptilt_servo;
extern bool fake_mirror;
extern float max_radius;
extern bool new_mean_aberrations;
extern bool send_tiptilt_servo;


/* Prototypes */

/* wfs_server.c */

int main(int argc, char **argv);
void close_function(void);
int wfs_top_job(void);
int wfs_periodic_job(void);
void print_usage_message(char *name);
int wfs_write_ref_centroids(char *filename);
int wfs_load_ref_centroids(char *filename);

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
int message_wfs_subap_send_centroids_ref(struct smessage *message);
int message_wfs_subap_shift_rot_scale_centroids_ref(struct smessage *message);
int message_wfs_subap_write_centroids_ref(struct smessage *message);
int message_wfs_subap_load_centroids_ref(struct smessage *message);
int message_wfs_open_tt_data_socket(struct smessage *message);
int message_wfs_get_clamp_fluxes(struct smessage *message);
int message_wfs_subap_calc_centroids_ref(struct smessage *mess);
int message_wfs_set_num_mean(struct smessage *mess);
int message_wfs_set_num_mean(struct smessage *mess);
int message_wfs_set_modulation(struct smessage *mess);
int message_wfs_set_servo(struct smessage *mess);
int message_wfs_get_servo(struct smessage *mess);
int message_wfs_closeloop_message(struct smessage *mess);
int message_wfs_set_send(struct smessage *mess);
int message_wfs_zero_tiptilt(struct smessage *mess);

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

void process_data(long time_stamp);
int message_wfs_take_background(struct smessage *message);
int message_wfs_reset_background(struct smessage *message);
int message_wfs_set_threshold(struct smessage *message);
int message_wfs_set_num_frames(struct smessage *message);
int message_wfs_save_tiptilt(struct smessage *message);
void complete_tiptilt_record(void);
int message_wfs_save_data(struct smessage *message);
void complete_data_record(void);

/* wfs_andor_camlink_data.c */

void videoirqfunc(int sig);
int andor_start_camlink_thread(void);
int andor_stop_camlink_thread(void);
int andor_start_camlink(void);
int andor_stop_camlink(void);
void *andor_camlink_thread(void *arg);
void lock_camlink_mutex(void);
void unlock_camlink_mutex(void);

/* wfs_wavefront.c */

void subap_calc_pitch(void);
int subap_send_centroids_ref(void);
void wfs_simulation_centroid(float centerx, float centery,
                             float pitch, float angle);
void subap_calc_pix_mask(void);
void calculate_centroids(void);
int message_wfs_centroid_type(struct smessage *message);
int message_wfs_set_clamp_flux(struct smessage *message);
void send_fluxes(void);

/* open_tiptilt_data_socket.c */

int open_tiptilt_data_socket(char *scope_name);
int close_tiptilt_data_socket(void);
int send_tiptilt_data(float Az, float El);
int     client_write_ready(int fd);

/* wfs_text_message.c */

int setup_text_message(void);
void send_wfs_text_message(char *fmt, ...);
void broadcast_text_message(void);

/* wfs_tiptilt.c */

void calculate_tiptilt(void);
void servo_tiptilt(void);

/* sh_control.c */

long char_to_dec(char *s, int nbyte, bool ledian);
char *dec_to_char(long n, bool lendian);
int tdc_open(void);
int tdc_initialize(void);
int tdc_close(void);
int tdc_get_response(int num_bytes, char first_return_byte, char *buffer);
int tdc_identify(void);
int tdc_home(void);
int tdc_setvelparams(struct s_wfs_tdc_status status);
int tdc_getvelparams(void);
int tdc_move_completed(void);
int tdc_rel_move(float step);
int tdc_abs_move(float step);
int tdc_move_stopped(void);
int tdc_move_stop(void);
int tdc_act_avmodes(void);
int tdc_deact_avmodes(void);
int tdc_set_poscounter(float pos);
int tdc_get_poscounter(void);
int tdc_get_char(void);

/* sh_messages.c */

void setup_tdc_messages(void);
int message_tdc_open(struct smessage *message);
int message_tdc_initialize(struct smessage *message);
int message_tdc_close(struct smessage *message);
int message_tdc_identify(struct smessage *message);
int message_tdc_home(struct smessage *message);
int message_tdc_setvelparams(struct smessage *message);
int message_tdc_getvelparams(struct smessage *message);
int message_tdc_vel_slower(struct smessage *message);
int message_tdc_vel_faster(struct smessage *message);
int message_tdc_rel_move(struct smessage *message);
int message_tdc_abs_move(struct smessage *message);
int message_tdc_move_stop(struct smessage *message);
int message_tdc_act_avmodes(struct smessage *message);
int message_tdc_deact_avmodes(struct smessage *message);
int message_tdc_set_poscounter(struct smessage *message);
int message_tdc_get_poscounter(struct smessage *message);
int message_tdc_update_status(struct smessage *message);

/* wfs_labao_tiptilt.c */

void initialize_labao_tiptilt(void);
int reopen_labao_tiptilt_socket(int argc, char **argv);
void open_labao_tiptilt_socket(void);
int flush_labao_tiptilt(int argc, char **argv);
void stop_labao_tiptilt(void);
void *do_labao_tiptilt(void *arg);
void current_labao_tiptilt(float *az, float *el);
float current_labao_receive_rate(void);
int     labao_tiptilt_waiting(int fd);

/* wfs_autoalign.c */

int open_telescope_connection(int argc, char **argv);
int message_telescope_status(int server, struct smessage *mess);
int message_wfs_start_focus_parabola(struct smessage *message);
void autoalign_focus_to_wfs(void);
int message_wfs_start_align_beacon(struct smessage *message);
void autoalign_beacon_parabola(void);
int message_wfs_stop_autoalign(struct smessage *message);



#endif
