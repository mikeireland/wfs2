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
at_u16 *image_data = NULL;
bool save_fits = FALSE;
bool use_cameralink = FALSE;
int number_of_processed_frames = 0;
float **data_frame = NULL;
float **dark_frame = NULL;
int dark_frame_num = 0;
float dark_frame_mean = 0.0;
float dark_frame_stddev = 0.0;
float **calc_dark_frame = NULL;
float **raw_frame = NULL;
float **sum_frame = NULL;
int sum_frame_num = 1;
float data_threshold = -1e32;
struct s_wfs_tdc_status tdc_status;
struct s_wfs_subap_centroids subap_centroids_mean;
struct s_wfs_subap_centroids subap_centroids_ref;
struct s_wfs_subap_centroids subap_centroids;
struct s_wfs_clamp_fluxes clamp_fluxes;
struct s_wfs_tiptilt wfs_tiptilt;
int num_mean_aberrations = 100;
struct s_wfs_aberrations wfs_mean_aberrations;
struct s_wfs_aberrations wfs_aberrations;
bool set_subap_centroids_ref = FALSE;
struct s_wfs_tiptilt_modulation wfs_tiptilt_modulation;
struct s_wfs_tiptilt_servo wfs_tiptilt_servo;
bool fake_mirror = FALSE;
float max_radius = 0.0;
bool new_mean_aberrations = FALSE;

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

			case 'm': if (sscanf(p+1,"%f", &max_radius) != 1)
                                  {
                                    print_usage_message(progname);
                                    exit(-1);
                                  }
                                  while(*p != '\0') p++; p--;
                                  break;

			case 'M': fake_mirror = !fake_mirror;
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
	setup_tdc_messages();

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

	/* Open connection to SH rotation stage */

	tdc_open();
	tdc_initialize();

	/* Create the USB thread */

	andor_start_usb_thread();

	/* We always send Camera Link data anyway */

	andor_set_camera_link(1);
	error(MESSAGE,"Camera Link Turned on.");

	/* Create the Cameralink thread */

	if (use_cameralink) andor_start_camlink_thread(); 

	/* Start the labao tiptilt thread */

	initialize_labao_tiptilt();

	/* Read in the last positions for the reference centroids */

	wfs_load_ref_centroids(NULL);

	/* Setup messages */

	setup_text_message();

	/* Set tiptilt and so on to zero */

	wfs_tiptilt.offsetx = 0.0;
	wfs_tiptilt.offsety = 0.0;
	wfs_tiptilt.correctx = 0.0;
	wfs_tiptilt.correcty = 0.0;
	wfs_tiptilt.totinten = 0;
	wfs_tiptilt.maxinten = 0;

	wfs_tiptilt_modulation.amplitude_x = 0.0;
	wfs_tiptilt_modulation.current_offset_x = 0.0;
	wfs_tiptilt_modulation.amplitude_y = 0.0;
	wfs_tiptilt_modulation.current_offset_y = 0.0;
	wfs_tiptilt_modulation.last_change = 0;
	wfs_tiptilt_modulation.cycle = 0;
	wfs_tiptilt_modulation.delta_cycle = 0;
	wfs_tiptilt_modulation.which = WFS_MODULATION_NONE;

	wfs_tiptilt_servo.gain_x = DEFAULT_GAIN_X;
	wfs_tiptilt_servo.labao_x = 0.0;
	wfs_tiptilt_servo.damp_x = DEFAULT_DAMP_X;
	wfs_tiptilt_servo.gain_y = DEFAULT_GAIN_Y;
	wfs_tiptilt_servo.labao_y = 0.0;
	wfs_tiptilt_servo.damp_y = DEFAULT_DAMP_Y;
	wfs_tiptilt_servo.on = FALSE;
	wfs_tiptilt_servo.send = FALSE;

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
	/* Stop the labao tiptilt thread */

	stop_labao_tiptilt();

	/* Stop the USB thread */

	andor_stop_usb();
	usleep(100000);
	andor_stop_usb_thread();

	/* Stop the CAMERA LINK thread */

	if (use_cameralink) andor_stop_camlink_thread();

	/* Now stop talking to the camera */

	if (andor_close() != NOERROR)
	{
		error(ERROR, "Failed ot close Andor connection.");
	}

	/* Close connection to SH rotation stage */

	tdc_close();

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
        fprintf(stderr,"-C\tToggle camera link (OFF)\n");
        fprintf(stderr,"-G\t[Gain] Set gain (%d)\n", DFT_ANDOR_PREAMP_GAIN);
        fprintf(stderr,"-h\tPrint this message\n");
        fprintf(stderr,"-H\t[CCD,EMCCD] Set horizontal speeds (%d,%d)\n",
		DFT_ANDOR_CCD_HORIZONTAL_SPEED,
		DFT_ANDOR_EMCCD_HORIZONTAL_SPEED);
        fprintf(stderr,"-m\tSet maximum radius of subap (OFF)\n");
        fprintf(stderr,"-M\tToggle fake mirror for tests (OFF)\n");
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
	float	data_mean = 0.0;
	float	data2_mean = 0.0;
	float	min = 1e32;
	float 	max = -1e32;
	int	minx = 0, miny = 0, maxx = 0, maxy = 0;
	float	n = 0.0;
	int	i, j;
	static long last_tiptil_totinten = 0.0;

	/* Send any messages we might need to send */

	broadcast_text_message();

	/* Send the tiptilt data if it is new. */

	if (last_tiptil_totinten != wfs_tiptilt.totinten)
	{
		last_tiptil_totinten = wfs_tiptilt.totinten;

		mess.type = WFS_TIPTILT_INFO;
                mess.length = sizeof(struct s_wfs_tiptilt);
                mess.data = (unsigned char *)&wfs_tiptilt;

                if (server_send_message_all(&mess) != NOERROR)
                  return error(ERROR,"Failed to send WFS_TIPTILT_INFO.");
	}

	/* Send the current aberrations if they are new */

	if (new_mean_aberrations)
	{
		mess.type = WFS_MEAN_ABERRATIONS;
		mess.data = (unsigned char *)&(wfs_mean_aberrations);
		mess.length = sizeof(wfs_mean_aberrations);

		if (server_send_message_all(&mess) != NOERROR)
			return error(ERROR,"Failed to send mean aberrations.");

		new_mean_aberrations = FALSE;
	}

	/* Is it time to do this? */

	if (time(NULL) < last_time + WFS_PERIODIC) return NOERROR;
	last_time = time(NULL);

	/* Is the camera working? */

	if (andor_setup.running)
	{
	    /* OK, work out the minimum, maximum and mean etc. */

	    for(i = 1; i <= andor_setup.npixx; i++)
	    for(j = 1; j <= andor_setup.npixy; j++)
	    {
		    data_mean += data_frame[i][j];
		    data2_mean += (data_frame[i][j]*data_frame[i][j]);
		    if (data_frame[i][j] > max)
		    {
			max = data_frame[i][j];
			maxx = i;
			maxy = j;
		    }
		    else if (data_frame[i][j] < min)
		    {
			min = data_frame[i][j];
			minx = i;
			miny = j;
		    }
		    n += 1.0;
	    }
	    data_mean /= n;
	    data2_mean /= n;

	    send_wfs_text_message(
		"(%.1f,%.1f) (%.1f,%.1f) Data mean = %.1f+-%.1f Max = %.1f Min = %.1f", 
			subap_centroids.x[0],
			subap_centroids.y[0],
			subap_centroids_ref.x[0],
			subap_centroids_ref.y[0],
			data_mean,
			sqrt(data2_mean - data_mean*data_mean),
			max, maxx, maxy, min, minx, miny);

	    /* Yes it is */

	    if (verbose)
	    {
		error(MESSAGE,"USB FPS = %.2f.",
				andor_setup.usb_frames_per_second);
		error(MESSAGE,"CL  FPS = %.2f.",
				andor_setup.camlink_frames_per_second);
		error(MESSAGE,"CAM FPS = %.2f.",
				andor_setup.cam_frames_per_second);
		error(MESSAGE,"PRC FPS = %.2f.",
				andor_setup.processed_frames_per_second);
		error(MESSAGE,"Data mean = %.1f+-%.1f", data_mean,
			sqrt(data2_mean - data_mean*data_mean));
		error(MESSAGE,"Max = %.1f (%d, %d) Min = %.1f (%d, %d)",
			max, maxx, maxy, min, minx, miny);
		error(MESSAGE,"LABAO tiptilt receive rate = %.1f", 
			current_labao_receive_rate());
	    }
	}

	/* Get the temperature */

	andor_get_temperature();

	/* Is it time to save some tiptilt data? */

	complete_tiptilt_record();

	/* Or indeed some raw data? */

	complete_data_record();

	/* Update the current setup information */

        mess.type = WFS_ANDOR_UPDATE_SETUP;
        mess.data = (unsigned char *)&(andor_setup);
        mess.length = sizeof(andor_setup);

        if (server_send_message_all(&mess) != NOERROR)
                return error(ERROR,"Failed to send andor setup.");

	/* That should be all */

	return NOERROR;

} /* wfs_periodic_job() */

/************************************************************************/
/* wfs_write_ref_centroids()                                            */
/*                                                                      */
/* Save the reference centroids.					*/
/************************************************************************/

int wfs_write_ref_centroids(char *file)
{

	char s[256],filename[256];
	FILE *f;
	int ii = 0;

	if (file == NULL)
	{
		sprintf(filename,"%s%s_%s",
		    get_etc_directory(s), wfs_name, REF_CENTROID_FILENAME);
	}
	else
	{
		sprintf(filename,"%s%s", get_etc_directory(s), file);
	}

	if ((f = fopen(filename,"w")) == NULL)
	{
		return error(ERROR, "Failed to write to %s",filename);
	}

	/* First the fluxes for clamping and so on */

	fprintf(f,"#  denom_clamp  min_flux clamp_flux\n");

	fprintf(f,"%10.1f %10.1f %10.1f\n", clamp_fluxes.denom_clamp_subap,
					  clamp_fluxes.min_flux_subap,
					  clamp_fluxes.clamp_flux_subap);

	/* Now the centroids */

	fprintf(f,"%10s %10s %10s %10s %10s\n", 
		"# subapID","centroidx","centroidy","pixelx","pixely");

	for (ii = 0; ii< WFS_DFT_SUBAP_NUMBER ; ii++)
		fprintf(f,"%10d %10.2f %10.2f %10d %10d\n", 
			ii,subap_centroids_ref.x[ii],subap_centroids_ref.y[ii],
			subap_centroids_ref.xp[ii],subap_centroids_ref.yp[ii]);

	fclose(f);

	error(MESSAGE,
		"New reference centroids has been written to %s",filename);
	send_wfs_text_message(
		"New reference centroids has been written to %s",filename);
	
	return NOERROR;

}/* wfs_write_ref_centroids() */

/************************************************************************/
/* wfs_load_ref_centroids()                                             */
/*                                                                      */
/* Read the reference centroids.					*/
/************************************************************************/

int wfs_load_ref_centroids(char *file)
{

	char s[256],filename[256];
	FILE *f;
	float centerx=0,centery=0;
	int pixelx=0,pixely=0,index=0;
	int	i;

	if (file == NULL)
	{
		sprintf(filename,"%s%s_%s",
		    get_etc_directory(s), wfs_name, REF_CENTROID_FILENAME);
	}
	else
	{
		sprintf(filename,"%s%s", get_etc_directory(s), file);
	}

	if ((f = fopen(filename,"r")) == NULL)
	{
		return error(ERROR, "Failed to load to %s",filename);
	}

	/* Get information about fluxes */

	if (getline_cs(s, 256, f) == (char *)EOF || sscanf(s,"%f %f %f",
		&clamp_fluxes.denom_clamp_subap,
		&clamp_fluxes.min_flux_subap,
		&clamp_fluxes.clamp_flux_subap) != 3)
		return error(ERROR,"Failed to get clamp_fluxes");

	if (verbose) 
	{
		error(MESSAGE,"Denom clamp subap = %.1f",
			clamp_fluxes.denom_clamp_subap);
		error(MESSAGE,"Minimum subaperture flux = %.1f",
			clamp_fluxes.min_flux_subap);
		error(MESSAGE,"Clamp flux = %.1f",
			clamp_fluxes.clamp_flux_subap);
	}

	/* Now get the centroids themselves */

	while(getline_cs(s, 256, f) != (char *)EOF)
	{
	    if (sscanf(s,"%10d %10f %10f %10d %10d\n",
			&index,&centerx,&centery,&pixelx,&pixely) == 5)
	    {
		subap_centroids_ref.x[index] = centerx;
		subap_centroids_ref.y[index] = centery;
		subap_centroids_ref.xp[index] = pixelx;
		subap_centroids_ref.yp[index] = pixely;
		if (verbose) error(MESSAGE,"%f, %f, %d, %d", 
			subap_centroids_ref.x[index], 
			subap_centroids_ref.y[index],
			subap_centroids_ref.xp[index],
			subap_centroids_ref.yp[index]);
	    }
	}

	fclose(f);

	subap_centroids_ref.num = index+1;
	subap_centroids_ref.size = WFS_DFT_SUBAP_SIZE;
	for(i=0; i < WFS_DFT_SUBAP_NUMBER; i++)
		subap_centroids_ref.inten[i] = 0.0;

	subap_centroids = subap_centroids_ref;

	if(index == WFS_DFT_SUBAP_NUMBER-1)
	{

		subap_calc_pitch();
		subap_calc_pix_mask();

		//wfs_calc_aperture_info();
		//wfs_zernphase_init(WFS_DFT_ZERNIKE_MODE_NUMBER);

		send_wfs_text_message(
			"New reference centroids has been loaded from %s",
			filename);
		return error(NOERROR,
			"New reference centroids has been loaded from %s",
			filename);
	}
	else
	{
		send_wfs_text_message(
		   "%s has been corrupted, reference centroids not loaded",
			filename);
		return error(ERROR,
		   "%s has been corrupted, reference centroids not loaded",
			filename);
	}

	/* Tell users about the fluxes */

	send_fluxes();

	return NOERROR;

}/* wfs_load_ref_centroids() */
