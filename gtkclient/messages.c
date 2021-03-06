/************************************************************************/
/* messages.c 		                                                */
/*                                                                      */
/* Some message handlers and other related routines. Note some message	*/
/* handlers will not be here since they are specific to certain		*/
/* windows.								*/
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

/************************************************************************/
/* set_wfs_messages()							*/
/*									*/
/* Routine to create pointers to all the messages we have.		*/
/************************************************************************/

void set_wfs_messages(void)
{
	/* Here are the standard GTK messages */

        if (!add_message_job(server, NOOP, message_noop) ||
            !add_message_job(server, MESSAGE, message_message) ||
            !add_message_job(server, PING, message_ping) ||
            !add_message_job(server, UI_STATUS, message_status) ||
            !add_message_job(server, SERVER_STOPPING, message_server_stopping)) 
        {
                fprintf(stderr, "Failed to add standard messages job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_ANDOR_SETUP, message_wfs_andor_setup))
        {
                fprintf(stderr, "Failed to add WFS_ANDOR_SETUP job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_ANDOR_CURRENT_FRAME,
		message_wfs_andor_current_frame))
        {
                fprintf(stderr, "Failed to add WFS_ANDOR_CURRENT_FRAME job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_ANDOR_STOP_MOVIE,
		message_wfs_andor_stop_movie))
        {
                fprintf(stderr, "Failed to add WFS_ANDOR_STOP_MOVIE job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_ANDOR_UPDATE_SETUP,
			message_wfs_andor_update_setup))
        {
                fprintf(stderr, "Failed to add WFS_ANDOR_UPDATE SETUP job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_SET_THRESHOLD,
			message_wfs_set_threshold))
        {
                fprintf(stderr, "Failed to add WFS_SET_THRESHOLD job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_SET_NUM_FRAMES,
			message_wfs_set_num_frames))
        {
                fprintf(stderr, "Failed to add WFS_SET_NUM_FRAMES job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_SUBAP_GET_CENTROIDS_REF,
			message_wfs_subap_get_centroids_ref))
        {
                fprintf(stderr, "Failed to add WFS_SUBAP_GET_CENTROIDS_REF job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_SUBAP_GET_CENTROIDS,
			message_wfs_subap_get_centroids))
        {
                fprintf(stderr, "Failed to add WFS_SUBAP_GET_CENTROIDS job.\n");
                exit(-8);
        }

        if (!add_message_job(server, WFS_SUBAP_GET_CENTROIDS_OFFSET,
			message_wfs_subap_get_centroids_offset))
        {
                fprintf(stderr, "Failed to add WFS_SUBAP_GET_CENTROIDS_OFFSET job.\n");
                exit(-8);
        }

	if (!add_message_job(server, WFS_TEXT_MESSAGE,
                message_wfs_text_message))
        {
                fprintf(stderr,"Failed to add WFS_TEXT_MESSAGE job.\n");
                exit(-8);
        }

	if (!add_message_job(server, WFS_SET_CLAMP_FLUXES,
                message_wfs_set_clamp_fluxes))
        {
                fprintf(stderr,"Failed to add WFS_SET_CLAMP_FLUXES job.\n");
                exit(-8);
        }

	if (!add_message_job(server, WFS_TIPTILT_INFO, 
		message_wfs_tiptilt_info))
       {
            fprintf(stderr, "Failed to add WFS_TIPTILT_INFO job.\n");
            exit(-8);
       }

	if (!add_message_job(server, WFS_MEAN_ABERRATIONS, 
		message_wfs_mean_aberrations))
       {
            fprintf(stderr, "Failed to add WFS_MEAN_ABERRATIONS job.\n");
            exit(-8);
       }

	if (!add_message_job(server, WFS_SET_SERVO, 
		message_wfs_set_servo))
        {
            fprintf(stderr, "Failed to add WFS_SET_SERVO job.\n");
            exit(-8);
        }

} /* set_wfs_messages() */

/************************************************************************/
/* message_wfs_andor_setup()   	                                        */
/*                                                                      */
/************************************************************************/

int message_wfs_andor_setup(int server, struct smessage *mess)
{
        if (mess->length != sizeof(struct s_wfs_andor_setup))
        {
                print_status(ERROR,
                "Got WFS_ANDOR_SETUP message with bad data.\n");
                return NOERROR;
        }
        andor_setup = *((struct s_wfs_andor_setup *) mess->data);

        got_andor_setup = TRUE;

	andor_setup_display();

        return NOERROR;

} /* message_wfs_andor_setup() */

/************************************************************************/
/* message_wfs_andor_stop_movie()                                       */
/*                                                                      */
/************************************************************************/

int message_wfs_andor_stop_movie(int server, struct smessage *mess)
{
        if (mess->length != 0)
        {
                print_status(ERROR,
                "Got WFS_ANDOR_STOP_MOVIE message with data.\n");
        }

	if (movie_running) toggle_movie_running_callback(NULL, NULL);

        return NOERROR;

} /* message_wfs_andor_stop_movie() */

/************************************************************************/
/* message_wfs_andor_update_setup()                                     */
/*                                                                      */
/************************************************************************/

int message_wfs_andor_update_setup(int server, struct smessage *mess)
{
	struct s_wfs_andor_setup *new_setup;

        if (mess->length != sizeof(struct s_wfs_andor_setup))
        {
                print_status(ERROR,
                "Got WFS_ANDOR_UPDATE_SETUP message with bad data.\n");
                return NOERROR;
        }
        new_setup = ((struct s_wfs_andor_setup *) mess->data);

	/* Only a the things the server can change need to be updated */

	andor_setup.read_mode = new_setup->read_mode;
	andor_setup.temperature = new_setup->temperature;
	andor_setup.usb_frames_per_second = new_setup->usb_frames_per_second;
	andor_setup.cam_frames_per_second = new_setup->cam_frames_per_second;
	andor_setup.camlink_frames_per_second=
		new_setup->camlink_frames_per_second;
	andor_setup.processed_frames_per_second = 
		new_setup->processed_frames_per_second;
	andor_setup.missed_frames_per_second = 
		new_setup->missed_frames_per_second;
	andor_setup.temperature_status = new_setup->temperature_status;

	/* OK, update what we need to */

	andor_setup_update();

        return NOERROR;

} /* message_wfs_andor_update_setup() */

/************************************************************************/
/* message_wfs_subap_get_centroids_ref()                                */
/*                                                                      */
/************************************************************************/

int message_wfs_subap_get_centroids_ref(int server, struct smessage *mess)
{
        if (mess->length != sizeof(struct s_wfs_subap_centroids))
        {
                print_status(ERROR,
                "Got WFS_SUBAP_GET_CENTROIDS_REF message with bad data.\n");
                return NOERROR;
        }

	subap_centroids_ref = *((struct s_wfs_subap_centroids*)mess->data);

        return NOERROR;

} /* message_wfs_subap_get_centroids_ref() */

/************************************************************************/
/* message_wfs_subap_get_centroids()                                    */
/*                                                                      */
/************************************************************************/

int message_wfs_subap_get_centroids(int server, struct smessage *mess)
{
        if (mess->length != sizeof(struct s_wfs_subap_centroids))
        {
                print_status(ERROR,
                "Got WFS_SUBAP_GET_CENTROIDS message with bad data.\n");
                return NOERROR;
        }

	subap_centroids = *((struct s_wfs_subap_centroids*)mess->data);

        return NOERROR;

} /* message_wfs_subap_get_centroids() */

/************************************************************************/
/* message_wfs_subap_get_centroids_offset()                             */
/*                                                                      */
/************************************************************************/

int message_wfs_subap_get_centroids_offset(int server, struct smessage *mess)
{
        if (mess->length != sizeof(struct s_wfs_subap_centroids))
        {
                print_status(ERROR,
                "Got WFS_SUBAP_GET_CENTROIDS_OFFSET message with bad data.\n");
                return NOERROR;
        }

	subap_centroids_offset = *((struct s_wfs_subap_centroids*)mess->data);

        return NOERROR;

} /* message_wfs_subap_get_centroids_offset() */

/************************************************************************/
/* message_wfs_text_message()                                           */
/*                                                                      */
/************************************************************************/

int message_wfs_text_message(int server, struct smessage *mess)
{
        if (mess->length == 0) return NOERROR;

        gtk_label_set_text((GtkLabel *)message_label, (char *)mess->data);

        return NOERROR;

} /* message_wfs_text_message() */

/************************************************************************/
/* message_wfs_set_clamp_fluxes()                                       */
/*                                                                      */
/************************************************************************/

int message_wfs_set_clamp_fluxes(int server, struct smessage *mess)
{
        if (mess->length != sizeof(struct s_wfs_clamp_fluxes))
        {
                print_status(ERROR,
                "Got WFS_SET_CLAMP_FLUXES message with bad data.\n");
                return NOERROR;
        }

	clamp_fluxes = *((struct s_wfs_clamp_fluxes *)mess->data);

	update_clamp_fluxes();

        return NOERROR;

} /* message_wfs_set_clamp_fluxes() */

/************************************************************************/
/* message_wfs_tiptilt_info                                             */
/*                                                                      */
/************************************************************************/

int message_wfs_tiptilt_info(int server, struct smessage *mess)
{

	if (mess->length != sizeof(struct s_wfs_tiptilt))
	{
      		return print_status(ERROR,
                    "Got WFS_TIPTILT_INFO message with bad data.\n");

	}

	wfs_tiptilt = *((struct s_wfs_tiptilt *) mess->data);

	if (wfs_show_tiptilt_info_flag)
	{
        	do_tt_display();
	}
 	else
	{
        	clear_tt_display();
	}

	return NOERROR;

} /* message_wfs_tiptilt_info() */

/************************************************************************/
/* message_wfs_mean_aberrations                                         */
/*                                                                      */
/************************************************************************/

int message_wfs_mean_aberrations(int server, struct smessage *mess)
{

	if (mess->length != sizeof(struct s_wfs_aberrations))
	{
      		return print_status(ERROR,
                    "Got WFS_MEAN_ABERRATIONS message with bad data.\n");

	}

	wfs_mean_aberrations = *((struct s_wfs_aberrations *) mess->data);

	update_mean_aberrations();

	return NOERROR;

} /* message_wfs_mean_aberrations() */

/************************************************************************/
/* message_wfs_set_servo                                                */
/*                                                                      */
/************************************************************************/

int message_wfs_set_servo(int server, struct smessage *mess)
{

	if (mess->length != sizeof(struct s_wfs_tiptilt_servo))
	{
      		return print_status(ERROR,
                    "Got WFS_SET_SERVO message with bad data.\n");

	}

	wfs_tiptilt_servo = *((struct s_wfs_tiptilt_servo *) mess->data);

	update_tiptilt_servo();

	return NOERROR;

} /* message_wfs_mean_aberrations() */
