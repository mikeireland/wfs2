/************************************************************************/
/* wfs_server.c								*/
/*                                                                      */
/* Server program for WFSs, including telescope axis motion.		*/
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY SERVER LIB   				*/
/*                 Based on the CHARA User Interface			*/
/*                 Based on the SUSI User Interface			*/
/*		In turn based on the CHIP User interface		*/
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA			*/
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu			                */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar 		                        */
/* Date   : Dec 2010							*/
/************************************************************************/

#include "wfs_server.h"

/* Globals */

bool verbose = FALSE;
char wfs_name[256];
int  scope_number = S1;
struct s_wfs_andor_setup andor_setup;
struct s_wfs_status wfs_status;
at_u16 *image_data = NULL;
bool save_fits = FALSE;
bool use_cameralink = TRUE;
int number_of_processed_frames = 0;
float **data_frame = NULL;
float **dark_frame = NULL;
float data_threshhold = 0;

int main(int argc, char **argv)
{
	int	i = 0;
	char	s[80], *p = NULL;
	struct  s_process_sockets process;
	char	*progname = NULL; 
	struct s_wfs_andor_image image;
	int	preamp_gain;
	int	vertical_speed;
	int	ccd_horizontal_speed;
	int	em_horizontal_speed;
	int	port;

	/* Create the restart_command */

	strcpy(process.restart_command,"/usr/local/bin/wfs_server");
	for(i=1; i<argc; i++)
	{
		strcat(process.restart_command," ");
		strcat(process.restart_command,argv[i]);
	}

	/* Setup the defaults */

	image.hbin = DFT_ANDOR_HBIN;
	image.vbin = DFT_ANDOR_VBIN;
	image.hstart = DFT_ANDOR_HSTART;
	image.hend = DFT_ANDOR_HEND;
	image.vstart = DFT_ANDOR_VSTART;
	image.vend = DFT_ANDOR_VEND;
	preamp_gain = DFT_ANDOR_PREAMP_GAIN;
	vertical_speed = DFT_ANDOR_VERTICAL_SPEED;
	ccd_horizontal_speed = DFT_ANDOR_CCD_HORIZONTAL_SPEED;
	em_horizontal_speed = DFT_ANDOR_EMCCD_HORIZONTAL_SPEED;
	port = -1;

	/* Check the command line */

	progname = argv[0];
        p = s;
        while(--argc > 0 && (*++argv)[0] == '-')
        {
             for(p = argv[0]+1; *p != '\0'; p++)
             {
                switch(*p)
                {
			case 'B': if (sscanf(p+1,"%d,%d",
					&(image.hbin), &(image.vbin)) != 2)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

                        case 'C': use_cameralink = !use_cameralink;
				  break;

			case 'G': if (sscanf(p+1,"%d", &preamp_gain) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

                        case 'h': print_usage_message(progname);
                                  exit(0);

			case 'H': if (sscanf(p+1,"%d,%d", 
					&ccd_horizontal_speed,
					&em_horizontal_speed) != 2)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

			case 'R': if (sscanf(p+1,"%d,%d,%d,%d", 
					&(image.hstart),
					&(image.hend),
					&(image.vstart),
					&(image.vend)) != 4)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
				  error(MESSAGE,"Got image %d,%d,%d,%d.",
					image.hstart,
					image.hend,
					image.vstart,
					image.vend);
			
                                  while(*p != '\0') p++; p--;
                                  break;

			case 's': if (sscanf(p+1,"%d", &port) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

                        case 'v': verbose = !verbose;
				  break;

			case 'V': if (sscanf(p+1,"%d", &vertical_speed) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

			default: fprintf(stderr,"Unknown flag %c.\n",*p);
                                 print_usage_message(progname);
                                 exit(-1);
                                 break;
		}
	     }
	}

	if (argc != 1)
	{
		print_usage_message(progname);
		exit(-1);
	}

	if (verbose)
	{
		if (use_cameralink)
			error(MESSAGE,"Camera Link Enabled.");
		else
			error(MESSAGE,"Camera Link Disabled.");
	}
	
	/* Create the server neame */

	sprintf(process.name,"wfs_%s", argv[0]);
	strcat(wfs_name, process.name);

	/* Which scope is this? */

	for (scope_number = S1; scope_number < NUM_SCOPES; scope_number++)
		if (strcmp(scope_types[scope_number], argv[0]) == 0) break;

	if (scope_number >= NUM_SCOPES)
	{
		error(FATAL,"Failed to identify SCOPE %s\n", argv[0]);
	} 
	/* Open server port */

	if (port < 0)
	{
	    if (auto_open_message_socket(process.name, 
			process.restart_command) != NOERROR)
	    {
		error(FATAL,"Failed to connect process %s.",
			process.name);
	    }
	}
	else
	{
	    if (open_message_socket(port) != NOERROR)
	    {
		error(FATAL,"Failed to connect to port %d.", port);
	    }
	}

	/* This is a stand alone server */

	set_standalone(TRUE);

	/* Initialise the jobs */

	server_init_jobs();

	/* Setup close function */

	setup_close_down(close_function);

	/* Add any local jobs */

	setup_wfs_messages();

	/* Set the name */

	sprintf(s,"%s Server",process.name);
	set_server_name(s);

	/* Set our jobs */

	//add_top_job(wfs_top_job); /* Doesn't do anything anymore */
	add_background_job(wfs_periodic_job);

	/* OK, we try and open a connection to the camera */

	if (andor_open(WFS_CAMERA, image, preamp_gain, 
	  vertical_speed, ccd_horizontal_speed, em_horizontal_speed) != NOERROR)
	{
		error(ERROR, "Failed ot open Andor connection.");
	}

	/*
	 * There is no CHARA time card in here, so we will fake it out
  	 * using the local clock, assumed to be using NTP.
	 */

	set_bypass_clock_driver(TRUE);
	if (!open_clock_device()) error(FATAL,"Failed to open Clock Device");
	setup_standard_clock_messages();

	/* Create the USB thread */

	andor_start_usb_thread();

	/* Create the Cameralink thread */

	if (use_cameralink)
	{
		andor_start_camlink_thread(); 
	}
	else
	{
		/* We send Camera Link data anyway */

		andor_set_camera_link(1);
		error(MESSAGE,"Camera Link Turned on.");
	}

	/* Had over control */

	process_jobs();

	/* Should never reach here */

	exit(0);

} /* main() */

/************************************************************************/
/* close_function()							*/
/*									*/
/* Final closing down function.						*/
/************************************************************************/

void close_function(void)
{
	/* Stop the USB thread */

	andor_stop_usb_thread();

	/* Stop the CAMERA LINK thread */

	if (use_cameralink) andor_stop_camlink_thread();

	/* Now stop talking to the camera */

	if (andor_close() != NOERROR)
	{
		error(ERROR, "Failed ot close Andor connection.");
	}

} /* close_function() */

/************************************************************************/
/* print_usage_message()                                                */
/*                                                                      */
/* Prints a usage message for this program.                             */
/************************************************************************/

void print_usage_message(char *name)
{

        fprintf(stderr,"usage: %s [-flags] SCOPE\n",name);
        fprintf(stderr,"Flags:\n");
        fprintf(stderr,"-B\t[Hbin,Vbin] Set binning (%d,%d)\n",
		DFT_ANDOR_HBIN, DFT_ANDOR_VBIN);
        fprintf(stderr,"-C\tToggle camera link (ON)\n");
        fprintf(stderr,"-G\t[Gain] Set gain (%d)\n", DFT_ANDOR_PREAMP_GAIN);
        fprintf(stderr,"-h\tPrint this message\n");
        fprintf(stderr,"-H\t[CCD,EMCCD] Set horizontal speeds (%d,%d)\n",
		DFT_ANDOR_CCD_HORIZONTAL_SPEED,
		DFT_ANDOR_EMCCD_HORIZONTAL_SPEED);
        fprintf(stderr,"-R\t[Hstart,Hend,Vstart,Vend] Set ROI (%d,%d,%d,%d)\n",
		DFT_ANDOR_HSTART, DFT_ANDOR_HEND,
		DFT_ANDOR_VSTART, DFT_ANDOR_VEND);
        fprintf(stderr,"-s\t[port] Bypass socket manager (FALSE)\n");
        fprintf(stderr,"-v\tToggle vergose mode (OFF)\n");
        fprintf(stderr,"-V\t[VSpeed] Set vertical speed (%d)\n",
		DFT_ANDOR_VERTICAL_SPEED);

} /* print_usage_message() */

/************************************************************************/
/* wfs_top_job()							*/
/*									*/
/* Routine that does all the work.					*/
/* Actually it's all part of a separrate thread now.			*/
/************************************************************************/

int wfs_top_job(void)
{
	return NOERROR;

} /* wfs_top_job() */

/************************************************************************/
/* wfs_periodic_job()							*/
/*									*/
/* ROutine that does things every now and then.				*/
/************************************************************************/

int wfs_periodic_job(void)
{
	static time_t last_time = 0;
	struct smessage mess;

	/* Is it time to do this? */

	if (time(NULL) < last_time + WFS_PERIODIC) return NOERROR;
	last_time = time(NULL);

	/* Is the camera working? */

	if (andor_setup.running)
	{
		/* Yes it is */

		if (verbose)
		{
			error(MESSAGE,"USB FPS = %.2f.",
				andor_setup.usb_frames_per_second);
			error(MESSAGE,"CL  FPS = %.2f.",
				andor_setup.camlink_frames_per_second);
			error(MESSAGE,"CAM FPS = %.2f.",
				andor_setup.cam_frames_per_second);
		}
	}

	/* Get the temperature */

	andor_get_temperature();

	/* Update the current setup information */

        mess.type = WFS_ANDOR_UPDATE_SETUP;
        mess.data = (unsigned char *)&(andor_setup);
        mess.length = sizeof(andor_setup);

        if (server_send_message_all(&mess) != NOERROR)
                return error(ERROR,"Failed to send andor setup.");

	/* That should be all */

	return NOERROR;

} /* wfs_periodic_job() */
