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

#include <chara_messages.h>
#include <cliserv.h>
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

/* Globals */

extern int server;
extern char display[256];      /* Name of display to use */
extern int server_open;
extern struct s_wfs_andor_setup andor_setup;
extern struct s_wfs_status wfs_status;
#ifdef GTK2
extern int main_page;
extern int andor_setup_page;
#endif
extern GtkWidget *notebook;
extern char engineering_mode;
extern char got_andor_setup;
extern int display_delay_ms;
extern char do_local_display;
extern char server_name[];
extern GtkWidget *temp_label;
extern int send_ready_for_display;
extern int movie_running;

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

/* messages.c */

void set_wfs_messages(void);
int message_wfs_andor_setup(int server, struct smessage *mess);
int message_wfs_andor_stop_movie(int server, struct smessage *mess);
int message_wfs_andor_update_setup(int server, struct smessage *mess);
int message_wfs_status(int server, struct smessage *mess);

/* andor.c */

void andor_setup_display(void);
void andor_setup_update(void);
void fill_andor_setup_page(GtkWidget *vbox);
void wfs_andor_send_callback(GtkButton *button, gpointer user_data);

/* display.c */

int message_wfs_andor_current_frame(int server, struct smessage *mess);
void clear_picture_callback(GtkButton *button, gpointer user_data);
void toggle_movie_running_callback(GtkButton *button, gpointer user_data);