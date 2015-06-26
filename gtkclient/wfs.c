/************************************************************************/
/* wfs.c 		                                                */
/*                                                                      */
/* ROutines for handling the wfs setup and status display.		*/
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
/* Author : Theo ten Brummelaar                                         */
/* Date   : April 2014                                                  */
/************************************************************************/

#include "wfsgtk.h"

static int wfs_page_created = FALSE;
static int window_only = CENTROID_WINDOW_ONLY;
static int nonlinear = CENTROID_NONLINEAR;

static GtkWidget *entry_num_dark = NULL;
static GtkWidget *entry_threshold = NULL;
static GtkWidget *entry_num_frames = NULL;
static GtkWidget *entry_display_delay = NULL;
static GtkWidget *entry_num_mean = NULL;
static GtkWidget *entry_num_save_tt = NULL;
static GtkWidget *entry_num_save_data = NULL;
static GtkWidget *entry_gain_x = NULL;
static GtkWidget *entry_labao_x = NULL;
static GtkWidget *entry_damp_x = NULL;
static GtkWidget *entry_gain_y = NULL;
static GtkWidget *entry_labao_y = NULL;
static GtkWidget *entry_damp_y = NULL;

#define NUM_SERVO  2
static char *servo_names[NUM_SERVO] = {"OFF", "ON"};
static GtkWidget *servo_buttons[NUM_SERVO] = {NULL, NULL};
static GtkWidget *send_buttons[NUM_SERVO] = {NULL, NULL};

/************************************************************************/
/* void fill_wfs_page(GtkWidget *vbox);					*/
/*									*/
/* Fill out the wfs page.						*/
/************************************************************************/

void fill_wfs_page(GtkWidget *vbox)
{
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkObject *adjustment;
        GSList    *servo_group = NULL;
        GSList    *send_group = NULL;
	int	  i;

	/* First row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("BOXES");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (toggle_show_boxes_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(100, 1, 1000, 1, 1, 0);
			/* Climb Rate, Digits */
        entry_num_dark =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_num_dark, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_num_dark, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(entry_num_dark);

	button = gtk_button_new_with_label ("MK-DARK");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_make_dark_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("Z-DARK");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_RESET_BACKGROUND) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
        //adjustment = gtk_adjustment_new(0, 0, 32767, 0.1, 0, 0);
  	adjustment = gtk_adjustment_new(0, -500, 500, 0.5, 0, 0);
        entry_threshold =
			gtk_spin_button_new((GtkAdjustment *)adjustment,0.5,1);
        gtk_box_pack_start(GTK_BOX(hbox), entry_threshold, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_threshold, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(entry_threshold);

	button = gtk_button_new_with_label ("THRESHOLD");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_threshold_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, 32767, 1, 1, 0);
			/* Climb Rate, Digits */
        entry_num_frames =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_num_frames, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_num_frames, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(entry_num_frames);

	button = gtk_button_new_with_label ("#FRAMES");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_num_frames_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Second row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("CLOSE TIPTILT");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_closeloop_message_callback), P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("OPEN TIPTILT");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_closeloop_message_callback), P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("LOAD DEF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_SUBAP_LOAD_CENTROIDS_REF) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SAVE DEF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_SUBAP_WRITE_CENTROIDS_REF) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Third row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("REOPEN TT SOCKET");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_OPEN_TT_DATA_SOCKET) );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize(button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("WINDOW ONLY");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_centroid_callback),
                (gpointer)(&window_only) );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("ZERO CENTROIDS");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_SUBAP_CALC_CENTROIDS_REF) );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("NONLINEAR");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_centroid_callback),
                (gpointer)(&nonlinear) );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

	/* Fourth row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("TOGGLE TT DISP");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_toggle_show_tiptilt), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("TOGGLE ABER DISP");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_toggle_plot_aber), NULL);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
	gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
  	adjustment = gtk_adjustment_new(display_delay_ms, 0, 1000, 1.0, 0, 0);
        entry_display_delay =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_display_delay, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_display_delay, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(entry_display_delay);

	button = gtk_button_new_with_label ("DISP DELAY");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_display_del_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Row Five */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        label = gtk_label_new("X Gain:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, -5.0, 5.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_gain_x =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_gain_x, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_gain_x, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_gain_x);

        label = gtk_label_new("X LabAo:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, -5.0, 5.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_labao_x =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_labao_x, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_labao_x, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_labao_x);

        label = gtk_label_new("X Damp:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 0.0, 1.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_damp_x =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_damp_x, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_damp_x, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_damp_x);

        label = gtk_label_new("Y Gain:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, -5.0, 5.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_gain_y =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_gain_y, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_gain_y, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_gain_y);

        label = gtk_label_new("Y LabAo:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, -5.0, 5.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_labao_y =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_labao_y, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_labao_y, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_labao_y);

        label = gtk_label_new("Y Damp:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 0.0, 1.0, 0.01, .1, 0);
			/* Climb Rate, Digits */
        entry_damp_y =gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,4);
        gtk_box_pack_start(GTK_BOX(hbox), entry_damp_y, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_damp_y, WFS_WIDTH/10, WFS_HEIGHT);
        gtk_widget_show(entry_damp_y);

        /* Row 6 */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        /* And a series of radio buttons for the Servo control */

        label = gtk_label_new("Servo:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, 1.17*WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);

        for(i=0; i<NUM_SERVO; i++)
        {
                servo_buttons[i] =
                        gtk_radio_button_new_with_label(servo_group,
			servo_names[i]);
                servo_group = gtk_radio_button_group(
                        GTK_RADIO_BUTTON(servo_buttons[i]));
                gtk_widget_ref (servo_buttons[i]);
                gtk_object_set_data_full (GTK_OBJECT (hbox),
                        servo_names[i],
                        servo_buttons[i],
                        (GtkDestroyNotify) gtk_widget_unref);
                gtk_box_pack_start(GTK_BOX (hbox),
                        servo_buttons[i],
                        FALSE, FALSE, 0);
                gtk_widget_set_usize (servo_buttons[i],
                        WFS_WIDTH/12, WFS_HEIGHT);
                gtk_widget_show(servo_buttons[i]);
        }

        /* And a series of radio buttons for the Send control */

        label = gtk_label_new("Send:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, 1.17*WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);

        for(i=0; i<NUM_SERVO; i++)
        {
                send_buttons[i] =
                        gtk_radio_button_new_with_label(send_group,
			servo_names[i]);
                send_group = gtk_radio_button_group(
                        GTK_RADIO_BUTTON(send_buttons[i]));
                gtk_widget_ref (send_buttons[i]);
                gtk_object_set_data_full (GTK_OBJECT (hbox),
                        servo_names[i],
                        send_buttons[i],
                        (GtkDestroyNotify) gtk_widget_unref);
                gtk_box_pack_start(GTK_BOX (hbox),
                        send_buttons[i],
                        FALSE, FALSE, 0);
                gtk_widget_set_usize (send_buttons[i],
                        WFS_WIDTH/12, WFS_HEIGHT);
                gtk_widget_show(send_buttons[i]);
        }

			/* Value, min, max, step, pageup, pagedown */
  	adjustment = gtk_adjustment_new(100, 1, 10000, 1.0, 0, 0);
        entry_num_save_data =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_num_save_data, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_num_save_data,WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(entry_num_save_data);

	button = gtk_button_new_with_label ("SAVE DATA");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_num_save_data_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SEND");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_set_servo_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("ZERO TT");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_ZERO_TIPTILT) );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize(button, WFS_WIDTH/8, WFS_HEIGHT);
	gtk_widget_show(button);

        /* Row 7 */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("SERVO OFF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_closeloop_message_callback), P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SERVO ON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_closeloop_message_callback), P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SEND OFF");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_set_send_callback), P_FALSE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SEND ON");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_set_send_callback), P_TRUE);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
  	adjustment = gtk_adjustment_new(100, 1, 10000, 1.0, 0, 0);
        entry_num_save_tt =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_num_save_tt, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_num_save_tt, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(entry_num_save_tt);

	button = gtk_button_new_with_label ("SAVE TT");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_num_save_tt_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

			/* Value, min, max, step, pageup, pagedown */
  	adjustment = gtk_adjustment_new(100, 1, 10000, 1.0, 0, 0);
        entry_num_mean =
			gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), entry_num_mean, TRUE, TRUE, 0);
        gtk_widget_set_usize (entry_num_mean, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(entry_num_mean);

	button = gtk_button_new_with_label ("NUM MEAN");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
               GTK_SIGNAL_FUNC (wfs_num_mean_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* OK, show this */

	gtk_widget_show(vbox);

	/* It is now OK to update things */

	wfs_page_created = TRUE;

} /* fill_wfs_page() */

/************************************************************************/
/* wfs_make_dark_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_make_dark_callback(GtkButton *button, gpointer user_data)
{
	      struct smessage mess;
	int	num;

	      /* The temperature is separate. */

	      num =gtk_spin_button_get_value_as_int((GtkSpinButton *)entry_num_dark);
	      mess.type = WFS_TAKE_BACKGROUND;
	      mess.data = (unsigned char *)&num;
	      mess.length = sizeof(int);

	      send_message(server, &mess);

} /* wfs_make_dark_callback() */

/************************************************************************/
/* wfs_threshold_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_threshold_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;
	float	thresh;

	/* The temperature is separate. */

	thresh = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_threshold);

	mess.type = WFS_SET_THRESHOLD;
	mess.data = (unsigned char *)&thresh;
	mess.length = sizeof(thresh);

	send_message(server, &mess);

} /* wfs_threshold_callback() */

/************************************************************************/
/* message_wfs_set_threshold()						*/
/*									*/
/************************************************************************/

int message_wfs_set_threshold(int server, struct smessage *mess)
{
	if (mess->length != sizeof(float))
	{
		print_status(ERROR,"Got WFS_SET_THRESHOLD with wrong data.");
		return NOERROR;
	}

	if (!wfs_page_created) return NOERROR;

	gtk_spin_button_set_value((GtkSpinButton *)entry_threshold, 
	                      *((float *)mess->data));

	return NOERROR;

} /* message_wfs_set_threshold() */

/************************************************************************/
/* wfs_num_frames_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_num_frames_callback(GtkButton *button, gpointer user_data)
{
	      struct smessage mess;
	int	num_frames;

	      /* The temperature is separate. */

	      num_frames = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)entry_num_frames);

	      mess.type = WFS_SET_NUM_FRAMES;
	      mess.data = (unsigned char *)&num_frames;
	      mess.length = sizeof(num_frames);

	      send_message(server, &mess);

} /* wfs_num_frames_callback() */

/************************************************************************/
/* message_wfs_set_num_frames()						*/
/*									*/
/************************************************************************/

int message_wfs_set_num_frames(int server, struct smessage *mess)
{
	if (mess->length != sizeof(int))
	{
		print_status(ERROR,"Got WFS_SET_NUM_FRAMES with wrong data.");
		return NOERROR;
	}

	if (!wfs_page_created) return NOERROR;

	gtk_spin_button_set_value((GtkSpinButton *)entry_num_frames, 
	                      *((int *)mess->data));

	return NOERROR;

} /* message_wfs_set_num_frames() */

/************************************************************************/
/* toggle_show_boxes_callback()                                         */
/*                                                                      */
/************************************************************************/

void toggle_show_boxes_callback(GtkButton *button, gpointer user_data)
{
	show_boxes = !show_boxes;

} /* toggle_show_boxes_callback() */

/************************************************************************/
/* wfs_centroid_callback()						*/
/*                                                                      */
/* Send message to set type of centroiding we will use			*/
/************************************************************************/

void wfs_centroid_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;
	int centroid;

	centroid = *((int *)user_data);

	mess.type = WFS_CENTROID_TYPE;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&centroid;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_CENTROID_TYPE");
		return;
	}

} /* wfs_centroid_callback() */

/************************************************************************/
/* wfs_toggle_show_tiptilt()						*/
/*                                                                      */
/* Toggle on/off displahying of tiptilt signals.			*/
/************************************************************************/

void wfs_toggle_show_tiptilt(GtkButton *button, gpointer user_data)
{
	wfs_show_tiptilt_info_flag = !wfs_show_tiptilt_info_flag;

} /* wfs_toggle_show_tiptilt() */

/************************************************************************/
/* wfs_toggle_plot_aber()						*/
/*                                                                      */
/* Toggle on/off displahying of scope signals.				*/
/************************************************************************/

void wfs_toggle_plot_aber(GtkButton *button, gpointer user_data)
{

        if (plot_aber)
        {
                /* We turn things off */

                close_scope(&aber_scope);
                XFlush(theDisplay);
                plot_aber = FALSE;
        }
        else
        {
                aber_scope = open_scope(server_name, 100, 100, 400, 200);

                add_signal(&aber_scope, PLOT_RED, -0.75, 0.75);
                add_signal(&aber_scope, PLOT_GREEN, -0.75, 0.75);
                add_signal(&aber_scope, PLOT_PURPLE, -0.75, 0.75);
                add_signal(&aber_scope, PLOT_LIGHT_BLUE, -0.75, 0.75);
                add_signal(&aber_scope, PLOT_DARK_BLUE, -0.75, 0.75);
                add_signal(&aber_scope, PLOT_WHITE, -0.75, 0.75);
                add_signal(&aber_scope, 65535, 32768, 0, -0.75, 0.75);
                add_signal(&aber_scope, 32768, 65535, 0, -0.75, 0.75);

                plot_aber = TRUE;
        }

} /* wfs_toggle_plot_aber() */

/************************************************************************/
/* wfs_display_del_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_display_del_callback(GtkButton *button, gpointer user_data)
{
	display_delay_ms = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)entry_display_delay);

} /* wfs_display_del_callback() */

/************************************************************************/
/* wfs_num_mean_callback()                                             */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_num_mean_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;
	int num_mean;

	num_mean = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)entry_num_mean);

	mess.type = WFS_SET_NUM_MEAN;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&num_mean;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SET_NUM_MEAN");
		return;
	}

} /* wfs_num_mean_callback() */

/************************************************************************/
/* wfs_num_save_tt_callback()                                           */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_num_save_tt_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;
	int num_save;

	num_save = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)entry_num_save_tt);

	mess.type = WFS_SAVE_TIPTILT;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&num_save;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SAVE_TIPTILT");
		return;
	}

} /* wfs_num_save_tt_callback() */

/************************************************************************/
/* wfs_num_save_data_callback()                                         */
/*                                                                      */
/* Tries to reopen the socket to the server.                            */
/************************************************************************/

void wfs_num_save_data_callback(GtkButton *budataon, gpointer user_data)
{
	struct smessage mess;
	int num_save;

	num_save = gtk_spin_button_get_value_as_int(
		(GtkSpinButton *)entry_num_save_data);

	mess.type = WFS_SAVE_DATA;
	mess.length = sizeof(int);
	mess.data = (unsigned char *)&num_save;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SAVE_DATA");
		return;
	}

} /* wfs_num_save_data_callback() */

/************************************************************************/
/* wfs_set_servo_callback()						*/
/*									*/
/* Send new servo parameters to the server.				*/
/************************************************************************/

void wfs_set_servo_callback(GtkButton *button, gpointer user_data)
{
	struct s_wfs_tiptilt_servo data;
	int	i;
	struct smessage mess;

        data.on = FALSE;
        for(i=0; i<NUM_SERVO; i++)
        {
                if (gtk_toggle_button_get_active((
                        GtkToggleButton *)servo_buttons[i]))
                        data.on = i;
        }

        data.send = FALSE;
        for(i=0; i<NUM_SERVO; i++)
        {
                if (gtk_toggle_button_get_active((
                        GtkToggleButton *)send_buttons[i]))
                        data.send = i;
        }

	data.gain_x = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_gain_x);
	data.labao_x = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_labao_x);
	data.damp_x = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_damp_x);
	data.gain_y = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_gain_y);
	data.labao_y = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_labao_y);
	data.damp_y = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_damp_y);

      mess.type = WFS_SET_SERVO;
      mess.data = (unsigned char *)&data;
      mess.length = sizeof(struct s_wfs_tiptilt_servo);

      send_message(server, &mess);

} /* wfs_set_servo_callback() */

/************************************************************************/
/* update_tiptilt_servo()						*/
/* 									*/
/* Make sure the local values are up to date.				*/
/************************************************************************/

void update_tiptilt_servo(void)
{
	gtk_spin_button_set_value((GtkSpinButton *)entry_gain_x,
                        wfs_tiptilt_servo.gain_x);
	gtk_spin_button_set_value((GtkSpinButton *)entry_labao_x,
                        wfs_tiptilt_servo.labao_x);
	gtk_spin_button_set_value((GtkSpinButton *)entry_damp_x,
                        wfs_tiptilt_servo.damp_x);
	gtk_spin_button_set_value((GtkSpinButton *)entry_gain_y,
                        wfs_tiptilt_servo.gain_y);
	gtk_spin_button_set_value((GtkSpinButton *)entry_labao_y,
                        wfs_tiptilt_servo.labao_y);
	gtk_spin_button_set_value((GtkSpinButton *)entry_damp_y,
                        wfs_tiptilt_servo.damp_y);

	if (wfs_tiptilt_servo.on)
		gtk_toggle_button_set_active((GtkToggleButton *)
                servo_buttons[1], 1);
	else
		gtk_toggle_button_set_active((GtkToggleButton *)
                servo_buttons[0], 1);

	if (wfs_tiptilt_servo.send)
		gtk_toggle_button_set_active((GtkToggleButton *)
                send_buttons[1], 1);
	else
		gtk_toggle_button_set_active((GtkToggleButton *)
                send_buttons[0], 1);

} /* update_tiptilt_servo() */

/************************************************************************/
/* wfs_closeloop_message_callback()					*/
/*									*/
/************************************************************************/

void wfs_closeloop_message_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;

      mess.type = WFS_CLOSELOOP_MESSAGE;
      mess.data = (unsigned char *)user_data;
      mess.length = sizeof(int);

      send_message(server, &mess);

} /* wfs_closeloop_message_callback() */

/************************************************************************/
/* wfs_set_send_callback()						*/
/*									*/
/************************************************************************/

void wfs_set_send_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;

      mess.type = WFS_SET_SEND;
      mess.data = (unsigned char *)user_data;
      mess.length = sizeof(int);

      send_message(server, &mess);

} /* wfs_set_send_callback() */

