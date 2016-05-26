/************************************************************************/
/* adjust.c 		                                                */
/*                                                                      */
/* ROutines for handling the adjust setup and status display.		*/
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

static int adjust_page_created = FALSE;

static GtkWidget *entry_ref_centroid_x;
static GtkWidget *entry_ref_centroid_y;
static GtkWidget *entry_ref_centroid_pitch;
static GtkWidget *entry_ref_centroid_angle;
static GtkWidget *entry_ref_centroid_step;
static GtkWidget *entry_ref_centroid_rot_angle;
static GtkWidget *entry_ref_centroid_scale_value;
static GtkWidget *entry_min_flux_subap;
static GtkWidget *entry_denom_clamp_subap;
static GtkWidget *entry_clamp_flux_subap;
static GtkWidget *entry_amp_x;
static GtkWidget *entry_amp_y;
static int modulation_none = WFS_MODULATION_NONE;
static int modulation_square = WFS_MODULATION_SQUARE_WAVE;
static int modulation_sin = WFS_MODULATION_SIN_WAVE;

/************************************************************************/
/* void fill_adjust_page(GtkWidget *vbox);				*/
/*									*/
/* Fill out the wfs page.						*/
/************************************************************************/

void fill_adjust_page(GtkWidget *vbox)
{
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkObject *adjustment;

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);
	label = gtk_label_new("Manual Control of Centroid Boxes:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH, WFS_HEIGHT);
        gtk_widget_show(label);

	/* First row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

  	button = gtk_button_new_with_label ("Set:");
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	      GTK_SIGNAL_FUNC (subap_send_centroids_ref_callback),NULL );
  	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
  	gtk_widget_show(button);

	label = gtk_label_new("Pos X");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(45, -500, 500, 0.1, 0,0);
  	entry_ref_centroid_x =gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_x, TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_ref_centroid_x, WFS_WIDTH/9 , WFS_HEIGHT);
  	gtk_widget_show(entry_ref_centroid_x);

	label = gtk_label_new("Pos Y");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(45, -500, 500, 0.1, 0,0);
  	entry_ref_centroid_y =gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_y, TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_ref_centroid_y, WFS_WIDTH/9 , WFS_HEIGHT);
  	gtk_widget_show(entry_ref_centroid_y);

	label = gtk_label_new("Pitch");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(12.4, -500, 500, 0.1, 0,0);
  	entry_ref_centroid_pitch = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_pitch, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_ref_centroid_pitch, WFS_WIDTH/9,WFS_HEIGHT);
  	gtk_widget_show(entry_ref_centroid_pitch);

	label = gtk_label_new("Angle");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0, -180, 180, 0.1, 0,0);
  	entry_ref_centroid_angle = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,0);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_angle, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_ref_centroid_angle, WFS_WIDTH/9,WFS_HEIGHT);
  	gtk_widget_show(entry_ref_centroid_angle);

	/* Second row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

  	button = gtk_button_new_with_label ("Up");
  	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	      GTK_SIGNAL_FUNC (subap_centroids_ref_up_callback),NULL );
  	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
  	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
  	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("Down");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (subap_centroids_ref_down_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("Left");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (subap_centroids_ref_left_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("Right");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (subap_centroids_ref_right_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	adjustment = gtk_adjustment_new(1, 0, 10, 0.1, 0,0);
	entry_ref_centroid_step = 
	gtk_spin_button_new((GtkAdjustment *)adjustment,0.1,1);
	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_step, 
		TRUE,TRUE, 0);
	gtk_widget_set_usize (entry_ref_centroid_step, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(entry_ref_centroid_step);

	button = gtk_button_new_with_label ("Rotate");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (subap_centroids_ref_rot_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	adjustment = gtk_adjustment_new(0, 0, 360, 1, 0,0);
	entry_ref_centroid_rot_angle = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,0);
	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_rot_angle, 
		TRUE,TRUE, 0);
	gtk_widget_set_usize (entry_ref_centroid_rot_angle, 
		WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(entry_ref_centroid_rot_angle);

	button = gtk_button_new_with_label ("Scale");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
		GTK_SIGNAL_FUNC (subap_centroids_ref_scale_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	adjustment = gtk_adjustment_new(1., 0.1, 10., 0.1, 0,0);
	entry_ref_centroid_scale_value = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,2);
	gtk_box_pack_start(GTK_BOX(hbox), entry_ref_centroid_scale_value, 
		TRUE,TRUE, 0);
	gtk_widget_set_usize (entry_ref_centroid_scale_value, 
		WFS_WIDTH/9 , WFS_HEIGHT);
	gtk_widget_show(entry_ref_centroid_scale_value);

	/* Third row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);
	label = gtk_label_new("Centroid Calculation Parameters:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH, WFS_HEIGHT);
        gtk_widget_show(label);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("FLUX");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (wfs_set_clamp_fluxes_callback),NULL );
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/9, WFS_HEIGHT);
	gtk_widget_show(button);

	label = gtk_label_new("Min Flux: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0.0, 0, 5000, 0.1, 0,0);
  	entry_min_flux_subap = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_min_flux_subap, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_min_flux_subap, WFS_WIDTH/9,WFS_HEIGHT);
  	gtk_widget_show(entry_min_flux_subap);

	label = gtk_label_new("Fix Denom: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0.0, 0, 500, 0.1, 0,0);
  	entry_denom_clamp_subap = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_denom_clamp_subap, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_denom_clamp_subap, WFS_WIDTH/9,WFS_HEIGHT);
  	gtk_widget_show(entry_denom_clamp_subap);

	label = gtk_label_new("Clamp Lev: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/9, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0.0, -1000, 500, 0.1, 0,0);
  	entry_clamp_flux_subap = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.1,1);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_clamp_flux_subap, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_clamp_flux_subap, WFS_WIDTH/9,WFS_HEIGHT);
  	gtk_widget_show(entry_clamp_flux_subap);

	/* Fourth row */

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);
	label = gtk_label_new("Tip/Tilt Modulation Control:");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH, WFS_HEIGHT);
        gtk_widget_show(label);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
        gtk_widget_show(hbox);

	button = gtk_button_new_with_label ("SQUARE");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (tiptilt_modulation_callback), &modulation_square);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/7, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("SIN");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (tiptilt_modulation_callback), &modulation_sin);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/7, WFS_HEIGHT);
	gtk_widget_show(button);

	button = gtk_button_new_with_label ("NONE");
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (tiptilt_modulation_callback), &modulation_none);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (button), 1);
	gtk_widget_set_usize (button, WFS_WIDTH/7, WFS_HEIGHT);
	gtk_widget_show(button);

	label = gtk_label_new("X Amp: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/7, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0.0, 0.0, 1.0, 0.05, 0,0);
  	entry_amp_x = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.05,2);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_amp_x, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_amp_x, WFS_WIDTH/7,WFS_HEIGHT);
  	gtk_widget_show(entry_amp_x);

	label = gtk_label_new("Y Amp: ");
        gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);
        gtk_widget_set_usize (label, WFS_WIDTH/7, WFS_HEIGHT);
        gtk_widget_show(label);

  	adjustment = gtk_adjustment_new(0.0, 0.0, 1.0, 0.05, 0,0);
  	entry_amp_y = gtk_spin_button_new(
		(GtkAdjustment *)adjustment,0.05,2);
  	gtk_box_pack_start(GTK_BOX(hbox), entry_amp_y, 
		TRUE,TRUE, 0);
  	gtk_widget_set_usize (entry_amp_y, WFS_WIDTH/7,WFS_HEIGHT);
  	gtk_widget_show(entry_amp_y);

	/* OK, show this */

	gtk_widget_show(vbox);

	/* It is now OK to update things */

	adjust_page_created = TRUE;

} /* fill_wfs_page() */

/************************************************************************/
/* subap_send_centroids_ref_callback()                                  */
/*                                                                      */
/* send the reference centroids positions to the server                 */
/************************************************************************/

void subap_send_centroids_ref_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float data[4] = {0, 0, 0, 0};

	data[0] = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_x);
	data[1] = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_y);
	data[2] = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_pitch);
	data[3] = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_angle);

	mess.type = WFS_SUBAP_SEND_CENTROIDS_REF;
	mess.length = sizeof(float) * 4;
	mess.data = (unsigned char *)data;

	if (!send_message(server, &mess))
	{
		print_status(ERROR,"Failed to send reference centroids.\n");
		return ;
	}

} /* subap_send_centroids_ref_callback()*/

/************************************************************************/
/* subap_centroids_ref_up_callback()					*/
/*                                                                      */
/* to move the all the reference centroids up                           */
/************************************************************************/

void subap_centroids_ref_up_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float step = 0.;

	step = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_step);

	/* if the data of the message is between 1000 - 1999 */
	/* the centroids move up by data - 1000 */

	step += 1000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&step;

	if (!send_message(server, &mess))
	{
		print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}

} /* subap_centroids_ref_up_callback */

/************************************************************************/
/* subap_centroids_ref_down_callback					*/
/*                                                                      */
/* to move the all the reference centroids down                         */
/************************************************************************/

void subap_centroids_ref_down_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float step = 0.;

	step = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_ref_centroid_step);

	/* if the data of the message is between 2000 - 2999 */
	/* the centroids move down by data - 2000 */

	step += 2000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&step;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}

} /* subap_centroids_ref_down_callback() */

/************************************************************************/
/* subap_centroids_ref_left_callback()					*/
/*                                                                      */
/* to move the all the reference centroids left                         */
/************************************************************************/

void subap_centroids_ref_left_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float step = 0.;

	step = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_ref_centroid_step);

	/* if the data of the message is between 3000 - 3999 */
	/* the centroids move left by data - 3000 */

	step += 3000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&step;

	if (!send_message(server, &mess))
	{
	  	print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}

} /* subap_centroids_ref_left_callback */

/************************************************************************/
/* subap_centroids_ref_right_callback()					*/
/*                                                                      */
/* to move the all the reference centroids right                        */
/************************************************************************/

void subap_centroids_ref_right_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float step = 0.;

	step = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_ref_centroid_step);

	/* if the data of the message is between 4000 - 4999 */
	/* the centroids move right by data - 4000 */

	step += 4000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&step;

	if (!send_message(server, &mess))
	{
	  	print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}


} /* subap_centroids_ref_right_callback() */

/************************************************************************/
/* subap_centroids_ref_rot_callback()					*/
/*                                                                      */
/* to rotate the all the reference centroids                            */
/************************************************************************/

void subap_centroids_ref_rot_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float rot = 0.;

	rot = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_ref_centroid_rot_angle);

	/* if the data of the message is between 5000 - 5999 */
	/* the centroids move rot by data - 5000 */

	rot += 5000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&rot;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}


} /* subap_centroids_ref_rot_callback() */

/************************************************************************/
/* subap_centroids_ref_scale_callback()					*/
/*                                                                      */
/* to scale the distance between all the reference centroids            */
/************************************************************************/

void subap_centroids_ref_scale_callback(GtkButton *button, gpointer user_data)
{

	struct smessage mess;
	float scale = 0.;

	scale = gtk_spin_button_get_value_as_float(
			(GtkSpinButton *)entry_ref_centroid_scale_value);

	/* if the data of the message is between 6000 - 6999 */
	/* the centroids move rot by data - 6000 */

	scale += 6000;

	mess.type = WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF;
	mess.length = sizeof(float);
	mess.data = (unsigned char *)&scale;

	if (!send_message(server, &mess)) 
	{
	  	print_status(ERROR,
		"Failed to send WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF");
		return;
	}

} /* subap_centroids_ref_scale_callback() */

/************************************************************************/
/* update_clamp_fluxes()						*/
/*									*/
/* Update teh value of the clamp fluxes.				*/
/************************************************************************/

void update_clamp_fluxes(void)
{
        if (!adjust_page_created) return;

        gtk_spin_button_set_value((GtkSpinButton *)entry_denom_clamp_subap,
                              clamp_fluxes.denom_clamp_subap);

        gtk_spin_button_set_value((GtkSpinButton *)entry_min_flux_subap,
                              clamp_fluxes.min_flux_subap);

        gtk_spin_button_set_value((GtkSpinButton *)entry_clamp_flux_subap,
                              clamp_fluxes.clamp_flux_subap);

} /* update_clamp_fluxes() */

/************************************************************************/
/* wfs_set_clamp_fluxes_callback()                                      */
/*                                                                      */
/* Send new clamp fluxes.						*/
/************************************************************************/

void wfs_set_clamp_fluxes_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;

	clamp_fluxes.denom_clamp_subap = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_denom_clamp_subap);
	clamp_fluxes.min_flux_subap = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_min_flux_subap);
	clamp_fluxes.clamp_flux_subap = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_clamp_flux_subap);

	mess.type = WFS_SET_CLAMP_FLUXES;
	mess.data = (unsigned char *)&clamp_fluxes;
	mess.length = sizeof(struct s_wfs_clamp_fluxes);

       	send_message(server, &mess);

} /* wfs_set_clamp_fluxes_callback() */

/************************************************************************/
/* tiptilt_modulation_callback()                                        */
/*                                                                      */
/* Send new modulation.							*/
/************************************************************************/

void tiptilt_modulation_callback(GtkButton *button, gpointer user_data)
{
	struct smessage mess;
	struct s_wfs_tiptilt_modulation data;

	data.amplitude_x = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_amp_x);
	data.amplitude_y = gtk_spin_button_get_value_as_float(
		(GtkSpinButton *)entry_amp_y);
	data.current_offset_x = 0.0;
	data.current_offset_y = 0.0;
	data.last_change = 0;
	data.cycle = 0.0;
	data.delta_cycle = 2.0 * M_PI / 
				(float)andor_setup.processed_frames_per_second;
	data.which = *((int *)user_data);

	mess.type = WFS_SET_MODULATION;
	mess.data = (unsigned char *)&data;
	mess.length = sizeof(struct s_wfs_tiptilt_modulation);

       	send_message(server, &mess);

} /* tiptilt_modulation_callback() */
