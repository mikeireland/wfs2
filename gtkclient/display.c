/************************************************************************/
/* display.c 		                                                */
/*                                                                      */
/* Hanldes messages for displaying pictures.				*/
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
/* Date   : April 2008                                                  */
/************************************************************************/

#include "wfsgtk.h"

static XImage *picture_image = NULL;
static char picture_window_up = FALSE;
static Window picture_window;
static float **values = NULL;
static int last_npixx = -1;
static int last_npixy = -1;

static char     tt_display_window_up = FALSE;
static int      tt_display_length = 20;
static XColor   det_color;
static XColor   mir_color;
static int      tt_display_pos;
static struct tt_display_struct {
        unsigned char   *picture;
        int             *di,*dj,*mi,*mj;
        Window          theWindow;
                             } tt_display;

/************************************************************************/
/* message_wfs_andor_current_frame()                                    */
/*                                                                      */
/************************************************************************/

int message_wfs_andor_current_frame(int server, struct smessage *mess)
{
        unsigned short int  *compressed_values;
	float *uncompressed_values;
	char *picture;
	uLongf  len, clen;
	int	i,j;
	float   *pf1;
	char	*pc1, *pc2;
	float	x, y, max, min;
	int	pixmult = 1;

	/* Do we do anything at all? */

        if (!do_local_display || !movie_running) return NOERROR;

	/* Is there any data? */

        if (mess->length == 0)
        {
                print_status(ERROR,
			"Got WFS_ANDOR_CURRENT_FRAME with no data.\n");
		send_ready_for_display = TRUE;
                return TRUE;
        }

	/* Do we know how big the image is suppose to be? */

	if (andor_setup.npixx <= 0 || andor_setup.npixy <= 0)
	{
		print_status(ERROR,"Got image without status!\n");
		send_ready_for_display = TRUE;
		return FALSE;
	}

	clen = mess->length;
	len = andor_setup.npixx * andor_setup.npixy * sizeof(float) * 2;
		
	/* Allocate memory */

	if ((uncompressed_values = malloc(len)) == NULL)
	{
		print_status(ERROR,"Ran out of memory!\n");
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	if ((compressed_values = malloc(len)) == NULL)
	{
		print_status(ERROR,"Ran out of memory!\n");
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	/* Copy the compressed stuff */

	pc1 = (char *)mess->data;
	pc2 = (char *)compressed_values;

	for(i=0; i<clen; i++) *pc2++ = *pc1++;

	/* Uncompress the data */

	if (uncompress((unsigned char *)uncompressed_values, &len, 
			(unsigned char *)compressed_values, clen) != Z_OK)
	{
		print_status(ERROR,"Uncompress failure on image.\n");
		free(compressed_values);
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}
	
	/* Does it come out right? */

	if (len != andor_setup.npix * sizeof(float))
	{
		print_status(ERROR,
			"Uncompressed size %d does not match frame size %d.\n",
			len, andor_setup.npix * sizeof(unsigned short int));
		free(compressed_values);
		free(uncompressed_values);
		send_ready_for_display = TRUE;
		return FALSE;
	}

	/*
	 * Have the dimmensions changed?
	 * NOTE: This uses NRC, matrices go from 1 not 0.
	 */

	if (andor_setup.npixx != last_npixx || andor_setup.npixy != last_npixy)
	{
		if (values != NULL) 
			free_matrix(values, 1, last_npixx, 1, last_npixy);
		values = matrix(1, andor_setup.npixx, 1, andor_setup.npixy);

		if (picture_window_up)
		{
			XDestroyWindow(theDisplay, picture_window);
			picture_window_up = FALSE;
		}
		last_npixx = andor_setup.npixx;
		last_npixy = andor_setup.npixy;
	}

	/* Copy the data, J inverted for display reasons */

	for(pf1 = uncompressed_values, i = 1; i <= andor_setup.npixx; i++)
	for(j = 1; j <= andor_setup.npixy; j++) values[i][j] = *pf1++;

	/* OK, finished with the memory */

	free(compressed_values);
	free(uncompressed_values);

	/* Work out a decent pixel multiplier */

	pixmult = andor_setup.width / andor_setup.npixx; 
	if (pixmult < 1) pixmult = 1;

	/* Put up the window if we have to */

	if (!picture_window_up)
	{
		picture_window = openWindow(server_name,
			theWidth-andor_setup.npixx*pixmult-30 ,5,
			andor_setup.npixx*pixmult, andor_setup.npixy*pixmult);
		picture_window_up = TRUE;
	}

	/* Display it */

	max = 0.0;
	min = 0.0;
	picture = make_scaled_picture(andor_setup.npixx,andor_setup.npixy,
			pixmult, values,LIN, &min, &max);
	picture_image = XCreateImage(theDisplay,theVisual,
			theDepth,ZPixmap,0, picture,
			andor_setup.npixx*pixmult,andor_setup.npixy*pixmult,
			theBitmapPad, 0);

	/* We need to add boxes */

	if (show_boxes)
	{
	    for(i=0; i<subap_centroids_ref.num; i++)
	    {
		min = (float)subap_centroids_ref.size/2.0;

		x = subap_centroids_ref.x[i] + subap_centroids_offset.x[i];
		y = subap_centroids_ref.y[i] + subap_centroids_offset.y[i];

		overlay_rectangle(andor_setup.npixx*pixmult,
			andor_setup.npixy*pixmult, picture, 
			(x - min - 0.5)*pixmult,
                        (andor_setup.npixy - y - min + 0.5)*pixmult,
                        (x + min - 0.5)*pixmult,
                        (andor_setup.npixy - y + min + 0.5)*pixmult,
			-1,PLOT_GREEN);

		overlay_rectangle(andor_setup.npixx*pixmult,
			andor_setup.npixy*pixmult, picture, 
			(subap_centroids.x[i] - 1.0)*pixmult,
                        (andor_setup.npixy - subap_centroids.y[i])
			*pixmult,
                        subap_centroids.x[i]*pixmult,
                        (andor_setup.npixy - subap_centroids.y[i] + 
			1.0)*pixmult, -1,PLOT_RED);
	    }
	}

	XPutImage(theDisplay,picture_window,theGC, picture_image,0,0,0,0,
			andor_setup.npixx*pixmult,andor_setup.npixy*pixmult);
	XMapWindow(theDisplay, picture_window);
	XFlush(theDisplay);
	if (picture_image != NULL) XDestroyImage(picture_image);

	/* We must be ready for another image */

	send_ready_for_display = TRUE;

        return TRUE;

} /* message_wfs_andor_current_frame() */

/************************************************************************/
/* clear_picture_callback()                                             */
/*                                                                      */
/************************************************************************/

void clear_picture_callback(GtkButton *button, gpointer user_data)
{
	if (picture_window_up)
        {
                XDestroyWindow(theDisplay, picture_window);
		XFlush(theDisplay);
                picture_window_up = FALSE;
		send_ready_for_display = TRUE;
        }       

} /* clear_picture_callback() */

/************************************************************************/
/* toggle_movie_running_callback()                                      */
/*                                                                      */
/************************************************************************/

void toggle_movie_running_callback(GtkButton *button, gpointer user_data)
{
	if (movie_running)
	{
		movie_running = FALSE;
		send_ready_for_display = FALSE;
		clear_picture_callback(button, user_data);
	}
	else
	{
		movie_running = TRUE;
		send_ready_for_display = TRUE;
	}

} /* toggle_movie_running_callback() */

/************************************************************************/
/* tiptilt_display()							*/
/* 									*/
/* Deals with incoming display information.				*/
/************************************************************************/

void do_tt_display(void)
{
        unsigned char   *p8;
        unsigned short int   *p16;
        unsigned int   *p32;
        int             ii,jj;
	static XImage  	*theImage = NULL;
	unsigned char	*disp_picture;
	unsigned char	*p8_1,*p8_2;
	unsigned short int	*p16_1,*p16_2;
	unsigned int	*p32_1,*p32_2;
	float rot_x, rot_y, Az, El;
	struct timezone tz;
        struct timeval stime; /* System and RT time. */
        long  time_now_ms = 0;
        static long start_time_sec = 0;

	/* What time is it now? */

        gettimeofday(&stime,&tz);
        if (start_time_sec == 0) start_time_sec = stime.tv_sec;
        stime.tv_sec -= start_time_sec;

        time_now_ms = stime.tv_sec*1000 + stime.tv_usec/1000;

	/* Need we do anything at all ? */

	if (!do_local_display) return;

	/* Set things up if we have to */

	if (!tt_display_window_up)
	{
		/* Alloctae our own colors */

		det_color.red = 65535;
		det_color.green = 65535;
		det_color.blue = 65535;
		det_color.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(theDisplay,theCmap,&det_color))
		{
			create_message_window(
				"Could not allocate WHITE for detector.\n");
			det_color.pixel = GreyScale[NUM_COLORS-1];
		}

		mir_color.red = 0;
		mir_color.green = 65535;
		mir_color.blue = 0;
		mir_color.flags = DoRed | DoGreen | DoBlue;
		if (!XAllocColor(theDisplay,theCmap,&mir_color))
		{
			create_message_window(
			"Could not allocate GREEN for mirror, using white\n");
			mir_color.pixel = GreyScale[NUM_COLORS-1];
		}

		/* Allocate memory */

		if ((tt_display.picture = 
			malloc(16384*theBitmapBytes)) == NULL)
		{
			fprintf(stderr,
			"Not enough memory for display.\n");
			fflush(stderr);
			exit(-1);
		}

		if ((tt_display.di=
				malloc(sizeof(int)*tt_display_length)) == NULL)
		{
			fprintf(stderr,"Not enough memory.\n");
			fflush(stderr);
			exit(-1);
		}

		if ((tt_display.dj=
				malloc(sizeof(int)*tt_display_length)) == NULL)
		{
			fprintf(stderr,"Not enough memory.\n");
			fflush(stderr);
			exit(-1);
		}

		if ((tt_display.mi=
				malloc(sizeof(int)*tt_display_length)) == NULL)
		{
			fprintf(stderr,"Not enough memory.\n");
			fflush(stderr);
			exit(-1);
		}

		if ((tt_display.mj=
			malloc(sizeof(int)*tt_display_length)) == NULL)
		{
			fprintf(stderr,"Not enough memory.\n");
			fflush(stderr);
			exit(-1);
		}

		/* Fill with dumby values */

		for(ii=0;ii<tt_display_length;ii++)
		{
			tt_display.di[ii] = tt_display.dj[ii] =
			tt_display.mi[ii] = tt_display.mj[ii] = -1;
		}

		/* Make picture all white */

		switch(theDepthBytes)
		{
			case 1: p8 = tt_display.picture;
				for(ii=0;ii<128;ii++)
				for(jj=0;jj<128;jj++)
				{
					*p8++ = GreyScale[0];
				}
				break;

			case 2: p16 = (unsigned short int *)
					tt_display.picture;
				for(ii=0;ii<128;ii++)
				for(jj=0;jj<128;jj++)
				{
					*p16++ = GreyScale[0];
				}
				break;

			default: p32 = (unsigned int *)
					tt_display.picture;
				for(ii=0;ii<128;ii++)
				for(jj=0;jj<128;jj++)
				{
					*p32++ = GreyScale[0];
				}
				break;
		}

		/* OK now we start and setup for the display */

		tt_display.theWindow = 
		openWindow(scope_name,10+140, theHeight-150, 128,128);

		/* That is all for now */

		tt_display_pos = 0;
		tt_display_window_up = TRUE;

	}

	/* Get the information for this detector */

	rot_x = wfs_tiptilt.offsetx;
	rot_y = wfs_tiptilt.offsety;
	Az = wfs_tiptilt.correctx;
	El = wfs_tiptilt.correcty;

	/* Now wipe the old value */

	switch(theDepthBytes)
	{
		case 1: p8_1 = tt_display.picture;
			if (tt_display.di[tt_display_pos] != -1 && 
				tt_display.dj[tt_display_pos] != -1)
			{
				*(p8_1+128*tt_display.dj[tt_display_pos]+
				    tt_display.di[tt_display_pos]) = 
					GreyScale[0];
			}

			if (tt_display.mi[tt_display_pos] != -1 && 
				tt_display.mj[tt_display_pos] != -1)
			{
				*(p8_1+128*tt_display.mj[tt_display_pos]+
				    tt_display.mi[tt_display_pos]) = 
					GreyScale[0];
			}
			break;

		case 2: p16_1=(unsigned short int *)tt_display.picture;
			if (tt_display.di[tt_display_pos] != -1 && 
				tt_display.dj[tt_display_pos] != -1)
			{
				*(p16_1+128*tt_display.dj[tt_display_pos]+
				    tt_display.di[tt_display_pos]) = 
					GreyScale[0];
			}

			if (tt_display.mi[tt_display_pos] != -1 && 
				tt_display.mj[tt_display_pos] != -1)
			{
				*(p16_1+128*tt_display.mj[tt_display_pos]+
				    tt_display.mi[tt_display_pos]) =
					GreyScale[0];
			}
			break;

		default: p32_1 = (unsigned int *)tt_display.picture;
			if (tt_display.di[tt_display_pos] != -1 && 
				tt_display.dj[tt_display_pos] != -1)
			{
				*(p32_1+128*tt_display.dj[tt_display_pos]+
				    tt_display.di[tt_display_pos]) =
					GreyScale[0];
			}

			if (tt_display.mi[tt_display_pos] != -1 && 
				tt_display.mj[tt_display_pos] != -1)
			{
				*(p32_1+128*tt_display.mj[tt_display_pos]+
				    tt_display.mi[tt_display_pos]) =
					GreyScale[0];
			}
			break;

	}

	/* Calculate new positions */

	tt_display.di[tt_display_pos]=(int)(64.0*rot_x+64.5);
	tt_display.dj[tt_display_pos]=(int)(64.0*rot_y+64.5);

	if (tt_display.di[tt_display_pos] < 0) 
		tt_display.di[tt_display_pos] = 0;
	else if (tt_display.di[tt_display_pos]> 127)
		tt_display.di[tt_display_pos] = 127;
	
	if (tt_display.dj[tt_display_pos] < 0) 
		tt_display.dj[tt_display_pos] = 0;
	else if (tt_display.dj[tt_display_pos]> 127)
		tt_display.dj[tt_display_pos] = 127;

	tt_display.mi[tt_display_pos] = (int)(64.0*Az + 64.5);
	tt_display.mj[tt_display_pos] = (int)(64.0*El + 64.5);

	if (tt_display.mi[tt_display_pos] < 0) 
	tt_display.mi[tt_display_pos] = 0;
	else if (tt_display.mi[tt_display_pos]> 127)
		tt_display.mi[tt_display_pos] = 127;
		
	if (tt_display.mj[tt_display_pos] < 0) 
		tt_display.mj[tt_display_pos] = 0;
	else if (tt_display.mj[tt_display_pos]> 127)
		tt_display.mj[tt_display_pos] = 127;

	/* Allocate display memory */

	if ((disp_picture = malloc(16384*theBitmapBytes)) == NULL)
	{
		fprintf(stderr,"Not enough memory for display.\n");
		fflush(stderr);
		exit(-1);
	}

	/* Now set the new colors and copy the picture for display */

	switch(theDepthBytes)
	{
		case 1: p8_1 = tt_display.picture;
			p8_2 = disp_picture;
			*(p8_1+128*tt_display.dj[tt_display_pos]+
				tt_display.di[tt_display_pos]) = 
					det_color.pixel;
			*(p8_1+128*tt_display.mj[tt_display_pos]+
				tt_display.mi[tt_display_pos]) = 
					mir_color.pixel;
			for(ii=0;ii<128;ii++)
			for(jj=0;jj<128;jj++)
			{
				*p8_2++ = *p8_1++;
			}
			break;

		case 2: p16_1 = 
			    (unsigned short int *)tt_display.picture;
			p16_2 = (unsigned short int *)disp_picture;
			*(p16_1+128*tt_display.dj[tt_display_pos]+
				tt_display.di[tt_display_pos]) = 
					det_color.pixel;
			*(p16_1+128*tt_display.mj[tt_display_pos]+
				tt_display.mi[tt_display_pos]) = 
					mir_color.pixel;
			for(ii=0;ii<128;ii++)
			for(jj=0;jj<128;jj++)
			{
				*p16_2++ = *p16_1++;
			}
			break;

		default: p32_1 = (unsigned int *)tt_display.picture;
			p32_2 = (unsigned int *)disp_picture;
			*(p32_1+128*tt_display.dj[tt_display_pos]+
				tt_display.di[tt_display_pos]) = 
					det_color.pixel;
			*(p32_1+128*tt_display.mj[tt_display_pos]+
				tt_display.mi[tt_display_pos]) = 
					mir_color.pixel;
			for(ii=0;ii<128;ii++)
			for(jj=0;jj<128;jj++)
			{
				*p32_2++ = *p32_1++;
			}
			break;
	}

	/* Incriment the position in the arrays */

	if (++tt_display_pos >= tt_display_length) tt_display_pos = 0;

	/* And update the image */

	if (theImage != NULL) XDestroyImage(theImage);
	theImage = XCreateImage(theDisplay,theVisual,theDepth,ZPixmap,0,
		(char *)disp_picture,128,128, theBitmapPad, 0);
	XPutImage(theDisplay,tt_display.theWindow,theGC,
		theImage,0,0,0,0,128,128);
	XMapWindow(theDisplay, tt_display.theWindow);

	/* Flush the display */

	XFlush(theDisplay);

	/* Let the background know we are ready to display things */

        send_ready_for_display = TRUE;

	return;

} /* do_tt_display() */

/************************************************************************/
/* clear_tt_display()                                                   */
/*                                                                      */
/* Clears up all the current tt_display.                                */
/************************************************************************/

void clear_tt_display(void)
{
        if (tt_display_window_up)
        {
                XDestroyWindow(theDisplay, tt_display.theWindow);
                free(tt_display.di);
                free(tt_display.dj);
                free(tt_display.mi);
                free(tt_display.mj);
                free(tt_display.picture);
		XFlush(theDisplay);
	}
        tt_display_window_up = FALSE;

} /* clear_tt_display() */
