/************************************************************************/
/* wfsgtk.h                                                             */
/*                                                                      */
/* HEader for WFS client.		                                */
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY USER INTERFACE                        */
/*                 Based on the SUSI User Interface                     */
/*              In turn based on the CHIP User interface                */
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA                 */
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu                                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar                                         */
/* Date   : April 2009							*/
/************************************************************************/

#include <cliserv.h>
#include <chara_messages.h>
#include <chara.h>
#include <cliserv_clock.h>
#include <astrogtk.h>
#include <stdlib.h>
#include <nrc.h>
#include <nsimpleX.h>
#include <sys/time.h>
#include <zlib.h>

#ifdef GTK2
#define WFS_WIDTH		700
#define WFS_HEIGHT		0
#else
#define WFS_WIDTH		150
#define WFS_HEIGHT		24
#endif

/* macro */

#define min(a,b)                        (((a)<(b))?(a):(b))
#define max(a,b)                        (((a)>(b))?(a):(b))

/* Globals */

extern int server;
extern char display[256];      /* Name of display to use */
extern int server_open;
extern struct s_wfs_andor_setup andor_setup;
#ifdef GTK2
extern int main_page;
extern int andor_setup_page;
extern int wfs_page;
extern int adjust_page;
extern int align_page;
extern int rot_page;
#endif
extern GtkWidget *notebook;
extern char engineering_mode;
extern char got_andor_setup;
extern int display_delay_ms;
extern char do_local_display;
extern char server_name[];
extern char scope_name[];
extern GtkWidget *temp_label;
extern int send_ready_for_display;
extern int movie_running;
extern struct s_wfs_subap_centroids subap_centroids_ref;
extern struct s_wfs_subap_centroids subap_centroids_offset;
extern struct s_wfs_subap_centroids subap_centroids;
extern bool show_boxes;
extern GtkWidget *message_label;
extern struct s_wfs_clamp_fluxes clamp_fluxes;
extern bool wfs_show_tiptilt_info_flag;
extern struct s_wfs_tiptilt wfs_tiptilt;
extern bool plot_aber;
extern struct s_wfs_aberrations wfs_mean_aberrations;
extern struct s_wfs_tiptilt_servo wfs_tiptilt_servo;
extern scope aber_scope;
extern int streak_mode;

/* Prototypes */

/* wfsgtk.c */

int main(int  argc, char *argv[]); 
gint delete_event( GtkWidget *widget, GdkEvent  *event, gpointer   data ); 
gint background_code(gpointer data);
void reopen_socket_callback(GtkButton *button, gpointer user_data);
void wfs_message_callback(GtkButton *widget, gpointer type);
void wfs_signal_callback(GtkButton *button, gpointer signal);
void print_usage_message(char *name);
void wfs_usb_onoff_callback(GtkButton *button, gpointer signal);
void wfs_cooler_onoff_callback(GtkButton *button, gpointer signal);
void wfs_shutter_auto_callback(GtkButton *button, gpointer signal);
void wfs_shutter_open_callback(GtkButton *button, gpointer signal);
void wfs_shutter_close_callback(GtkButton *button, gpointer signal);
void wfs_camlink_onoff_callback(GtkButton *button, gpointer signal);
void update_mean_aberrations(void);

/* messages.c */

void set_wfs_messages(void);
int message_wfs_andor_setup(int server, struct smessage *mess);
int message_wfs_andor_stop_movie(int server, struct smessage *mess);
int message_wfs_andor_update_setup(int server, struct smessage *mess);
int message_wfs_subap_get_centroids_ref(int server, struct smessage *mess);
int message_wfs_subap_get_centroids(int server, struct smessage *mess);
int message_wfs_subap_get_centroids_offset(int server, struct smessage *mess);
int message_wfs_text_message(int server, struct smessage *mess);
int message_wfs_set_clamp_fluxes(int server, struct smessage *mess);
int message_wfs_tiptilt_info(int server, struct smessage *mess);
int message_wfs_mean_aberrations(int server, struct smessage *mess);
int message_wfs_set_servo(int server, struct smessage *mess);

/* andor.c */

void andor_setup_display(void);
void andor_setup_update(void);
void fill_andor_setup_page(GtkWidget *vbox);
void wfs_andor_send_callback(GtkButton *button, gpointer user_data);

/* wfs.c */

void fill_wfs_page(GtkWidget *vbox);
void wfs_make_dark_callback(GtkButton *button, gpointer signal);
void wfs_threshold_callback(GtkButton *button, gpointer signal);
int message_wfs_set_threshold(int server, struct smessage *mess);
void wfs_num_frames_callback(GtkButton *button, gpointer signal);
int message_wfs_set_num_frames(int server, struct smessage *mess);
void toggle_show_boxes_callback(GtkButton *button, gpointer signal);
void open_tt_data_socket_callback(GtkButton *button, gpointer user_data);
void wfs_centroid_callback(GtkButton *button, gpointer user_data);
void wfs_toggle_show_tiptilt(GtkButton *button, gpointer user_data);
void wfs_toggle_plot_aber(GtkButton *button, gpointer user_data);
void wfs_aber_del_callback(GtkButton *button, gpointer user_data);
void wfs_display_del_callback(GtkButton *button, gpointer user_data);
void wfs_num_mean_callback(GtkButton *button, gpointer user_data);
void wfs_num_save_data_callback(GtkButton *button, gpointer user_data);
void wfs_num_save_tt_callback(GtkButton *button, gpointer user_data);
void wfs_set_servo_callback(GtkButton *button, gpointer user_data);
void wfs_closeloop_message_callback(GtkButton *button, gpointer user_data);
void wfs_set_send_callback(GtkButton *button, gpointer user_data);
void update_tiptilt_servo(void);

/* align.c */

void fill_align_page(GtkWidget *vbox);
void wfs_autoalign_focus_callback(GtkButton *button, gpointer data);
void wfs_autoalign_scope_callback(GtkButton *button, gpointer data);
void wfs_autoalign_beacon_callback(GtkButton *button, gpointer data);
void wfs_autoalign_boxes_callback(GtkButton *button, gpointer data);
void wfs_toggle_streak_callback(GtkButton *button, gpointer data);

/* adjust.c */

void fill_adjust_page(GtkWidget *vbox);
void subap_calc_centroids_ref_callback(GtkButton *button, gpointer user_data);
void subap_send_centroids_ref_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_up_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_down_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_left_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_right_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_rot_callback(GtkButton *button, gpointer user_data);
void subap_centroids_ref_scale_callback(GtkButton *button, gpointer user_data);
void update_clamp_fluxes(void);
void tiptilt_modulation_callback(GtkButton *button, gpointer user_data);

/* display.c */

int message_wfs_andor_current_frame(int server, struct smessage *mess);
void clear_picture_callback(GtkButton *button, gpointer user_data);
void toggle_movie_running_callback(GtkButton *button, gpointer user_data);
void wfs_toggle_center_callback(GtkButton *button, gpointer user_data);
void wfs_toggle_cog_callback(GtkButton *button, gpointer user_data);
void wfs_set_clamp_fluxes_callback(GtkButton *button, gpointer signal);
void do_tt_display(void);
void clear_tt_display(void);
void clear_scope_display(void);

/* rotation.c */

void fill_rotation_page(GtkWidget *vbox);
void tdc_message_callback(GtkButton *widget, gpointer type);
void tdc_update(void);
int message_tdc_update_status(int server, struct smessage *mess);
