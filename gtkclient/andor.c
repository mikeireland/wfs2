/************************************************************************/
/* andor.c 		                                                */
/*                                                                      */
/* ROutines for handling the andor setup and status display.		*/
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
/* Date   : April 2006                                                  */
/************************************************************************/

#include "wfsgtk.h"

static int andor_page_created = FALSE;
static GtkWidget *running_label;
static GtkWidget *shutter_label;
static GtkWidget *fps_label;
static GtkWidget *preamp_gain_label;
static GtkWidget *missed_frames_label;
static GtkWidget *readmode_label;
static GtkWidget *temp_status_label;
static GtkWidget *temp_current_label;
static GtkWidget *max_frame_size_label;
static GtkWidget *current_frame_size_label;
static GtkWidget *npix_label;
static GtkWidget *vspeed_label;
static GtkWidget *hspeed_label[ANDOR_NUM_AMPLIFIERS];

static GtkWidget *temp_button;
static GtkWidget *exp_button;
static GtkWidget *em_gain_button;
static GtkWidget *preamp_gain_button;
static GtkWidget *hbin_button;
static GtkWidget *vbin_button;
static GtkWidget *hstart_button;
static GtkWidget *hend_button;
static GtkWidget *vstart_button;
static GtkWidget *vend_button;
static GtkWidget *vspeed_button;
static GtkWidget *hspeed_button[ANDOR_NUM_AMPLIFIERS];

static char *amp_names[ANDOR_NUM_AMPLIFIERS] = {"EMCCD", "CCD"};
static GtkWidget *amp_buttons[ANDOR_NUM_AMPLIFIERS] = {NULL, NULL};

#define NUM_EM	2
static char *em_names[NUM_EM] = {"OFF", "ON"};
static GtkWidget *em_buttons[NUM_EM] = {NULL, NULL};

/************************************************************************/
/* andor_setup_display()						*/
/*									*/
/* Display the setup. This could be dangerous if the buttons have	*/
/* not yet been defined.						*/
/************************************************************************/

void andor_setup_display(void)
{
	char	s[234];
	int	i;

	if (!andor_page_created) return;

	/* Now the things specific to this page */

	if (andor_setup.running)
		gtk_label_set_text((GtkLabel *) running_label, "Running: YES");
	else
		gtk_label_set_text((GtkLabel *) running_label, "Running: NO");

	switch(andor_setup.shutter)
	{
		case ANDOR_SHUTTER_AUTO: gtk_label_set_text((GtkLabel *) 
					    shutter_label, "Shutter: AUTO");
					 break;

		case ANDOR_SHUTTER_OPEN: gtk_label_set_text((GtkLabel *) 
					    shutter_label, "Shutter: OPEN");
					 break;

		case ANDOR_SHUTTER_CLOSE: gtk_label_set_text((GtkLabel *) 
					    shutter_label, "Shutter: CLOSED");
					 break;

		default: gtk_label_set_text((GtkLabel *) 
					    shutter_label, "Shutter: UNKNOWN");
					 break;
	}

	sprintf(s,"Preamp Gain: %.2f", andor_setup.preamp_gain);
	gtk_label_set_text((GtkLabel *) preamp_gain_label, s);

	sprintf(s,"VSpeed: %.2f uS", andor_setup.vertical_speed);
	gtk_label_set_text((GtkLabel *) vspeed_label, s);

	for(i=0; i<ANDOR_NUM_AMPLIFIERS; i++)
	{
	    sprintf(s,"%s HS: %.2f MHz", 
		amp_names[i], andor_setup.horizontal_speed[i]);
	    gtk_label_set_text((GtkLabel *) hspeed_label[i], s);
	}

	sprintf(s,"Full Frame: %dx%d", andor_setup.width, andor_setup.height);
	gtk_label_set_text((GtkLabel *) max_frame_size_label, s);

	sprintf(s,"Current Frame: %dx%d", andor_setup.npixx, andor_setup.npixy);
	gtk_label_set_text((GtkLabel *) current_frame_size_label, s);

	sprintf(s,"Num Pixels: %d", andor_setup.npix);
	gtk_label_set_text((GtkLabel *) npix_label, s);

	gtk_spin_button_set_value((GtkSpinButton *)temp_button, 
                        andor_setup.target_temperature);

	gtk_spin_button_set_value((GtkSpinButton *)exp_button, 
                        andor_setup.exposure_time);

	gtk_spin_button_set_value((GtkSpinButton *)em_gain_button, 
                        andor_setup.em_gain);

	gtk_spin_button_set_value((GtkSpinButton *)preamp_gain_button, 
                        andor_setup.preamp_gain_index+1);

	gtk_toggle_button_set_active((GtkToggleButton *) 
		amp_buttons[andor_setup.amplifier], 1);

	gtk_toggle_button_set_active((GtkToggleButton *) 
		em_buttons[andor_setup.em_advanced], 1);

	gtk_spin_button_set_value((GtkSpinButton *)hbin_button, 
                        andor_setup.image.hbin);

	gtk_spin_button_set_value((GtkSpinButton *)vbin_button, 
                        andor_setup.image.vbin);

	gtk_spin_button_set_value((GtkSpinButton *)hstart_button, 
                        andor_setup.image.hstart);

	gtk_spin_button_set_value((GtkSpinButton *)hend_button, 
                        andor_setup.image.hend);

	gtk_spin_button_set_value((GtkSpinButton *)vstart_button, 
                        andor_setup.image.vstart);

	gtk_spin_button_set_value((GtkSpinButton *)vend_button, 
                        andor_setup.image.vend);

	gtk_spin_button_set_value((GtkSpinButton *)vspeed_button, 
                        andor_setup.vertical_speed_index+1);

	for(i=0; i<ANDOR_NUM_AMPLIFIERS; i++)
	{
	    gtk_spin_button_set_value((GtkSpinButton *)hspeed_button[i], 
                        andor_setup.horizontal_speed_index[i]+1);
	}

	/* Now, the more volotile ones. */

	andor_setup_update();

} /* andor_setup_display() */

/************************************************************************/
/* andor_setup_update()							*/
/*									*/
/* Update the setup. Only changes a subset of fields.			*/
/************************************************************************/

void andor_setup_update(void)
{
	char	s[234];

	if (!andor_page_created) return;

	/* First the temperature for all to see */

	if (andor_setup.temperature_status)
		sprintf(s,"ON/%.1fC", andor_setup.temperature);
	else
		sprintf(s,"OFF/%.1fC", andor_setup.temperature);

        gtk_label_set_text((GtkLabel *) temp_label, s);

	/* Now some local Andor fields */

	switch(andor_setup.read_mode)
	{
		case ANDOR_READMODE_FULL_VERTICAL_BINNING:
			sprintf(s,"Read Mode: FULL VERT BIN");
			break;

		case ANDOR_READMODE_MULTI_TRACK:
			sprintf(s,"Read Mode: MULTI-TRACK");
			break;

		case ANDOR_READMODE_RANDOM_TRACK:
			sprintf(s,"Read Mode: RANDOM-TRACK");
			break;

		case ANDOR_READMODE_SINGLE_TRACK:
			sprintf(s,"Read Mode: SINGLE-TRACK");
			break;

		case ANDOR_READMODE_IMAGE:
			sprintf(s,"Read Mode: IMAGE");
			break;

		default: sprintf(s,"Read Mode: UNKNOWN");
			break;
	}
	gtk_label_set_text((GtkLabel *) readmode_label, s);

	switch(andor_setup.temperature_status)
	{
		case ANDOR_TEMPERATURE_OFF:
					    sprintf(s,"Temp Status: OFF");
					    break;

		case ANDOR_TEMPERATURE_STABILIZED:
					    sprintf(s,"Temp Status: STABLE");
					    break;

		case ANDOR_TEMPERATURE_NOT_REACHED:
					    sprintf(s,"Temp Status: COOLING");
					    break;

		case ANDOR_TEMPERATURE_DRIFT:
					    sprintf(s,"Temp Status: DRIFT");
					    break;

		case ANDOR_TEMPERATURE_NOT_STABILIZED:
					   sprintf(s,"Temp Status: NOT STABLE");
					    break;

		default: sprintf(s,"Temp Status: UNKNOWN");
			break;
	}

	gtk_label_set_text((GtkLabel *) temp_status_label, s);

	sprintf(s,"Temperature: %.2f C", andor_setup.temperature);
	gtk_label_set_text((GtkLabel *) temp_current_label, s);

	sprintf(s,"Missed FPS: %.2f", andor_setup.missed_frames_per_second);
	gtk_label_set_text((GtkLabel *) missed_frames_label, s);

	sprintf(s,"Cam %.0f Usb %.0f CL %.0f Proc %.0f", 
			andor_setup.cam_frames_per_second,
			andor_setup.usb_frames_per_second,
			andor_setup.camlink_frames_per_second,
			andor_setup.processed_frames_per_second);
	gtk_label_set_text((GtkLabel *) fps_label, s);

} /* andor_setup_update() */

/************************************************************************/
/* void fill_andor_setup_page(GtkWidget *vbox);				*/
/*									*/
/* Fill out the andor setup page.					*/
/************************************************************************/

void fill_andor_setup_page(GtkWidget *vbox)
{
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkObject *adjustment;
	GSList 	  *amp_group = NULL;
	GSList 	  *em_group = NULL;
	int	  i;
	char	  s[234];

	/* First row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        running_label = gtk_label_new("Running: ???");
        gtk_box_pack_start(GTK_BOX(hbox), running_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (running_label, 3*WFS_WIDTH/16, WFS_HEIGHT);
        gtk_widget_show(running_label);

        shutter_label = gtk_label_new("Shutter: ???");
        gtk_box_pack_start(GTK_BOX(hbox), shutter_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (shutter_label, 3*WFS_WIDTH/16, WFS_HEIGHT);
        gtk_widget_show(shutter_label);

        fps_label = gtk_label_new("FPS: ???");
        gtk_box_pack_start(GTK_BOX(hbox), fps_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (fps_label, 3*WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(fps_label);

	button = gtk_button_new_with_label ("GET");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_message_callback),
                (gpointer)(message_array+WFS_ANDOR_GET_SETUP) );
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	button = gtk_button_new_with_label ("SEND");
        gtk_signal_connect (GTK_OBJECT (button), "clicked",
                GTK_SIGNAL_FUNC (wfs_andor_send_callback), NULL);
        gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (button), 1);
        gtk_widget_set_usize (button, WFS_WIDTH/8, WFS_HEIGHT);
        gtk_widget_show(button);

	/* Second row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        readmode_label = gtk_label_new("Read Mode: ???");
        gtk_box_pack_start(GTK_BOX(hbox), readmode_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (readmode_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(readmode_label);

        temp_status_label = gtk_label_new("Temp Status: ???");
        gtk_box_pack_start(GTK_BOX(hbox), temp_status_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (temp_status_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(temp_status_label);

        temp_current_label = gtk_label_new("Temperature: ???");
        gtk_box_pack_start(GTK_BOX(hbox), temp_current_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (temp_current_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(temp_current_label);

        missed_frames_label = gtk_label_new("Missed: ???");
        gtk_box_pack_start(GTK_BOX(hbox), missed_frames_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (missed_frames_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(missed_frames_label);

	/* Third row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        max_frame_size_label = gtk_label_new("Full Frame: ???x???");
        gtk_box_pack_start(GTK_BOX(hbox), max_frame_size_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (max_frame_size_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(max_frame_size_label);

        current_frame_size_label = gtk_label_new("Current Frame: ???x???");
        gtk_box_pack_start(GTK_BOX(hbox),current_frame_size_label,TRUE,TRUE, 0);
        gtk_widget_set_usize (current_frame_size_label, WFS_WIDTH/4,WFS_HEIGHT);
        gtk_widget_show(current_frame_size_label);

        npix_label = gtk_label_new("Num Pixels: ???");
        gtk_box_pack_start(GTK_BOX(hbox), npix_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (npix_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(npix_label);

        preamp_gain_label = gtk_label_new("Preamp Gain: ???");
        gtk_box_pack_start(GTK_BOX(hbox), preamp_gain_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (preamp_gain_label, WFS_WIDTH/4, WFS_HEIGHT);
        gtk_widget_show(preamp_gain_label);

	/* Fourth row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        label = gtk_label_new("Set Temp:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, andor_setup.minimum_temperature,
			andor_setup.maximum_temperature, 1, 1, 0);
			/* Climb Rate, Digits */
        temp_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), temp_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (temp_button, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(temp_button);

        label = gtk_label_new("Exposure:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 0.001, 2.0, 0.001, 0.1, 0);
			/* Climb Rate, Digits */
        exp_button = gtk_spin_button_new((GtkAdjustment *)adjustment,0.001,3);
        gtk_box_pack_start(GTK_BOX(hbox), exp_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (exp_button, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(exp_button);

        label = gtk_label_new("PreAmp Gain:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, 
			andor_setup.num_preamp_gains, 1, 1, 0);
			/* Climb Rate, Digits */
        preamp_gain_button = gtk_spin_button_new((GtkAdjustment *)adjustment,
			1,0);
        gtk_box_pack_start(GTK_BOX(hbox), preamp_gain_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (preamp_gain_button, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(preamp_gain_button);

	/* Fifth row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	/* A label to tell us what this is */

	label = gtk_label_new("Amplifier:");
        gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

	/* And a series of radio buttons for the amplifier selection */

	for(i=0; i<ANDOR_NUM_AMPLIFIERS; i++)
	{
		amp_buttons[i] = 
			gtk_radio_button_new_with_label(amp_group,amp_names[i]);
		amp_group = gtk_radio_button_group(
			GTK_RADIO_BUTTON(amp_buttons[i]));
		gtk_widget_ref (amp_buttons[i]);
		gtk_object_set_data_full (GTK_OBJECT (hbox), 
			amp_names[i], 
			amp_buttons[i], 
			(GtkDestroyNotify) gtk_widget_unref);
		gtk_box_pack_start(GTK_BOX (hbox), 
			amp_buttons[i], 
			FALSE, FALSE, 0);
		gtk_widget_set_usize (amp_buttons[i], 
			WFS_WIDTH/9, WFS_HEIGHT);
		gtk_widget_show(amp_buttons[i]);
	}

	/* A label to tell us what this is */

	label = gtk_label_new("High Gain:");
        gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

	/* And a series of radio buttons for the amplifier selection */

	for(i=0; i<NUM_EM; i++)
	{
		em_buttons[i] = 
			gtk_radio_button_new_with_label(em_group,em_names[i]);
		em_group = gtk_radio_button_group(
			GTK_RADIO_BUTTON(em_buttons[i]));
		gtk_widget_ref (em_buttons[i]);
		gtk_object_set_data_full (GTK_OBJECT (hbox), 
			em_names[i], 
			em_buttons[i], 
			(GtkDestroyNotify) gtk_widget_unref);
		gtk_box_pack_start(GTK_BOX (hbox), 
			em_buttons[i], 
			FALSE, FALSE, 0);
		gtk_widget_set_usize (em_buttons[i], 
			WFS_WIDTH/9, WFS_HEIGHT);
		gtk_widget_show(em_buttons[i]);
	}

        label = gtk_label_new("EM Gain:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, andor_setup.minimum_em_gain, 
#warning Forcing gain maximum to 1000
			1000, 1, 1, 0);
			//andor_setup.maximum_em_gain, 1, 1, 10);
			/* Climb Rate, Digits */
        em_gain_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1,0);
        gtk_box_pack_start(GTK_BOX(hbox), em_gain_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (em_gain_button, WFS_WIDTH/6, WFS_HEIGHT);
        gtk_widget_show(em_gain_button);

	/* Sixth row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        vspeed_label = gtk_label_new("VSpeed");
        gtk_box_pack_start(GTK_BOX(hbox), vspeed_label, TRUE, TRUE, 0);
        gtk_widget_set_usize (vspeed_label, 2*WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(vspeed_label);

			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, 
			andor_setup.num_vertical_speeds, 1, 1, 0);
			/* Climb Rate, Digits */
        vspeed_button = gtk_spin_button_new((GtkAdjustment *)adjustment,
			1,0);
        gtk_box_pack_start(GTK_BOX(hbox), vspeed_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (vspeed_button, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(vspeed_button);

	for(i=0; i< ANDOR_NUM_AMPLIFIERS; i++)
	{
	    sprintf(s,"%s HS:", amp_names[i]);
            hspeed_label[i] = gtk_label_new(s);
            gtk_box_pack_start(GTK_BOX(hbox), hspeed_label[i], TRUE, TRUE, 0);
            gtk_widget_set_usize (hspeed_label[i], 2*WFS_WIDTH/9, WFS_HEIGHT);
            gtk_widget_show(hspeed_label[i]);

			/* Value, min, max, step, pageup, pagedown */
            adjustment = gtk_adjustment_new(0, 1, 
			andor_setup.num_horizontal_speeds[i],1, 1, 0);
			/* Climb Rate, Digits */
            hspeed_button[i] = gtk_spin_button_new((GtkAdjustment *)adjustment,
			1,0);
            gtk_box_pack_start(GTK_BOX(hbox), hspeed_button[i], TRUE, TRUE, 0);
            gtk_widget_set_usize (hspeed_button[i], WFS_WIDTH/9, WFS_HEIGHT);
            gtk_widget_show(hspeed_button[i]);
	}

	/* Seventh row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

        label = gtk_label_new("Hbin:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.width, 1, 1, 0);
			/* Climb Rate, Digits */
        hbin_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), hbin_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (hbin_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(hbin_button);

        label = gtk_label_new("Vbin:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.height, 1, 1, 0);
			/* Climb Rate, Digits */
        vbin_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), vbin_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (vbin_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(vbin_button);

        label = gtk_label_new("Hstart:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.width, 1, 1, 0);
			/* Climb Rate, Digits */
        hstart_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), hstart_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (hstart_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(hstart_button);

        label = gtk_label_new("Hend:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.width, 1, 1, 0);
			/* Climb Rate, Digits */
        hend_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), hend_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (hend_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(hend_button);

        label = gtk_label_new("Vstart:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.height, 1, 1, 0);
			/* Climb Rate, Digits */
        vstart_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), vstart_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (vstart_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(vstart_button);

        label = gtk_label_new("Vend:");
        gtk_box_pack_start(GTK_BOX(hbox), label , TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(label);
			/* Value, min, max, step, pageup, pagedown */
        adjustment = gtk_adjustment_new(0, 1, andor_setup.height, 1, 1, 0);
			/* Climb Rate, Digits */
        vend_button = gtk_spin_button_new((GtkAdjustment *)adjustment,1.0,0);
        gtk_box_pack_start(GTK_BOX(hbox), vend_button, TRUE, TRUE, 0);
        gtk_widget_set_usize (vend_button, WFS_WIDTH/12, WFS_HEIGHT);
        gtk_widget_show(vend_button);

	/* OK, show the box */

        gtk_widget_show(vbox);

	/* It is now OK to update things */

	andor_page_created = TRUE;

} /* fill_andor_setup_page() */

/************************************************************************/
/* wfs_andor_send_callback()						*/
/*									*/
/* Send the current configuration to the server.			*/
/************************************************************************/

void wfs_andor_send_callback(GtkButton *button, gpointer user_data)
{
	int	target_temp;
	struct smessage mess;
	int 	i;
	struct  s_wfs_andor_setup setup;
	int	amp_action;
	int	em_action;
	
	/* OK, send all the values */

	amp_action = -1;
	for(i=0; i<ANDOR_NUM_AMPLIFIERS; i++)
	{
		if (gtk_toggle_button_get_active((
			GtkToggleButton *)amp_buttons[i]))
			amp_action = i;
	}

	if (amp_action == -1)
		print_status(ERROR, "Rediculous amplifer selection.");
	else
		setup.amplifier = amp_action;

	em_action = -1;
	for(i=0; i<NUM_EM; i++)
	{
		if (gtk_toggle_button_get_active((
			GtkToggleButton *)em_buttons[i]))
			em_action = i;
	}

	if (em_action == -1)
		print_status(ERROR, "Rediculous EM Advanced selection.");
	else
	{
		setup.em_advanced = em_action;
	}

	setup.em_gain = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)em_gain_button);

	setup.horizontal_speed_index[ANDOR_EMCCD] = 
		gtk_spin_button_get_value_as_int(
			(GtkSpinButton *)hspeed_button[ANDOR_EMCCD]) - 1;

	setup.horizontal_speed_index[ANDOR_CCD] = 
		gtk_spin_button_get_value_as_int(
			(GtkSpinButton *)hspeed_button[ANDOR_CCD]) - 1;

	setup.vertical_speed_index = 
		gtk_spin_button_get_value_as_int(
			(GtkSpinButton *)vspeed_button) - 1;

	setup.preamp_gain_index = gtk_spin_button_get_value_as_int(
			(GtkSpinButton *)preamp_gain_button) - 1;

	setup.image.hbin = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)hbin_button);
	setup.image.vbin = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)vbin_button);
	setup.image.hstart = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)hstart_button);
	setup.image.hend = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)hend_button);
	setup.image.vstart = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)vstart_button);
	setup.image.vend = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)vend_button);

	setup.exposure_time = 
            gtk_spin_button_get_value_as_float((GtkSpinButton *)exp_button);

	/* Now send them */

	mess.type = WFS_ANDOR_SETUP;
	mess.data = (unsigned char *)&setup;
	mess.length = sizeof(struct s_wfs_andor_setup);
	send_message(server, &mess);

	/* The temperature is separate. */

	target_temp = 
            gtk_spin_button_get_value_as_int((GtkSpinButton *)temp_button);
	mess.type = WFS_ANDOR_SET_TEMPERATURE;
	mess.data = (unsigned char *)&target_temp;
	mess.length = sizeof(int);
        send_message(server, &mess);

} /* wfs_andor_send_callback() */
