/************************************************************************/
/* wfs_tiptilt.c                                                        */
/*                                                                      */
/* Does the tiptilt and a few other simple calculations.		*/
/************************************************************************/
/*                                                                      */
/*     Center for High Angular Resolution Astronomy                     */
/* Georgia State University, Atlanta GA 30303-3083, U.S.A.              */
/*                                                                      */
/*                                                                      */
/* Telephone: 1-404-651-1882                                            */
/* Fax      : 1-404-651-1389                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu/~theo/theo.html                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo							*/
/* Date   : Sep 2014                                                    */
/************************************************************************/

#include "./wfs_server.h"

#define MAX_ABERRATIONS_RECORD      100000
static int aberrations_record_num = 0;
static int aberrations_record_count = 0;
static struct  s_wfs_aberrations aberrations_record[MAX_ABERRATIONS_RECORD];

/************************************************************************/
/* calculate_tiptilt							*/
/*									*/
/* Works out the mean tiptilt, focus and astigmatism terms. Also will 	*/
/* will perform tiptilt servo calculations.				*/
/************************************************************************/

void calculate_tiptilt(void)
{
	float	x_mean_offset = 0.0, y_mean_offset = 0.0;
	float	max_offset = 0.0;
	float	x_offset = 0.0, y_offset = 0.0, total_flux = 0.0;
	float	max_flux = -1e32;
	float	a1 = 0.0, a2 = 0.0, focus = 0.0;
	float	c1 = 0.0, c2 = 0.0;
	float	xtilt = 0.0, ytilt = 0.0;
	float	dx, dy;
	int	subap;
	static struct  s_wfs_aberrations calc_aberrations;
	static int count_aberrations = 32700;
	static float	xtilt2 = 0.0, ytilt2 = 0.0;
	static float xmirr = 0.0, ymirr = 0.0, xmirr2 = 0.0, ymirr2 = 0.0;
	float	mir_var = 0.0;
	float	rad, num = 0.0;
	float	x[WFS_DFT_SUBAP_NUMBER];
	float	y[WFS_DFT_SUBAP_NUMBER];

	/* Find the pupil center */

	for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
        {
		x_mean_offset += (x[subap] = subap_centroids_ref.x[subap] +
			subap_centroids_offset.x[subap]);
		y_mean_offset =  (y[subap] = subap_centroids_ref.y[subap] +
			subap_centroids_offset.y[subap]);
	}
	x_mean_offset /= WFS_DFT_SUBAP_NUMBER;
	y_mean_offset /= WFS_DFT_SUBAP_NUMBER;
	total_flux = 0.0;

	/* Now do the remaining calculations */

	rad = max_radius * max_radius;
	num = 0.0;
	for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
        {
		x_offset = x[subap] - x_mean_offset;
		y_offset = y[subap] - y_mean_offset;

		if (rad > 0 && rad < x_offset*x_offset + y_offset*y_offset)
			continue;

		num += 1.0;

		if (fabs(x_offset) > max_offset) max_offset = fabs(x_offset);
		if (fabs(y_offset) > max_offset) max_offset = fabs(y_offset);

		xtilt += (dx = subap_centroids.x[subap] - x[subap]);
		ytilt += (dy = subap_centroids.y[subap] - y[subap]);

		total_flux += subap_centroids.inten[subap];

		if (subap_centroids.inten[subap] > max_flux)
			max_flux = subap_centroids.inten[subap];

		focus += (dx * x_offset + dy * y_offset);
		a1 += (dx * x_offset - dy * y_offset);
		a2 += (dy * x_offset + dx * y_offset);
		c1 +=  dx*(9*x_offset*x_offset + 3*y_offset*y_offset-2)/9.0 + 
                        dy*2*x_offset*y_offset/3.0;
                c2 +=  dy*(9*y_offset*y_offset + 3*x_offset*x_offset-2)/9.0 + 
                        dx*2*x_offset*y_offset/3.0;
	}

	/* Tilt needs to go between -1 and 1 */

	xtilt /= (float)(num*(subap_centroids_ref.size/2.0));
	ytilt /= (float)(num*(subap_centroids_ref.size/2.0));
	focus /= (float)num;
	a1 /= (float)num;
	a2 /= (float)num;
	c1 /= (float)num;
	c2 /= (float)num;

#warning We may also need to multiply by arcsecs per pixel

	/* 
	 * So that all terms are at least in units of "pixels" 
	 * (i.e. max tilt per sub-aperture), we divide by the
         * radius of the pupil.
	 */

        focus /= max_offset;
        a1 /= max_offset;
        a2 /= max_offset;
        c1 /= (max_offset * max_offset);
        c2 /= (max_offset * max_offset);

	/* Now are we adding modulation to this signal? */

	if (wfs_tiptilt_modulation.which == WFS_MODULATION_SQUARE_WAVE)
	{
	    /* Is it time to do something? */

	    if (time(NULL) > wfs_tiptilt_modulation.last_change)
 	    {
	        wfs_tiptilt_modulation.last_change = time(NULL);
    
	        if (wfs_tiptilt_modulation.amplitude_x > 0.0)
	        {
		    if (wfs_tiptilt_modulation.current_offset_x <= 0.0)
		    {
		        wfs_tiptilt_modulation.current_offset_x = 
			    wfs_tiptilt_modulation.amplitude_x;
		    }
		    else
		    {
		        wfs_tiptilt_modulation.current_offset_x = 
			    -1.0 * wfs_tiptilt_modulation.amplitude_x;
		    }
		}
    
	        if (wfs_tiptilt_modulation.amplitude_y > 0.0)
	        {
		    if (wfs_tiptilt_modulation.current_offset_y <= 0.0)
		    {
		        wfs_tiptilt_modulation.current_offset_y = 
			    wfs_tiptilt_modulation.amplitude_y;
		    }
		    else
		    {
		        wfs_tiptilt_modulation.current_offset_y = 
			    -1.0 * wfs_tiptilt_modulation.amplitude_y;
		    }
	        }
	    }
	}
	else
	{
	    wfs_tiptilt_modulation.current_offset_x = 0.0;
	    wfs_tiptilt_modulation.current_offset_y = 0.0;
	}

	/* Put these into some global variables */

	wfs_tiptilt.offsetx = xtilt + wfs_tiptilt_modulation.current_offset_x;
	wfs_tiptilt.offsety = ytilt + wfs_tiptilt_modulation.current_offset_y;
	wfs_tiptilt.totinten = (int)(total_flux + 0.5);
	wfs_tiptilt.maxinten = (int)(max_flux + 0.5);

	wfs_aberrations.xtilt = wfs_tiptilt.offsetx;
	wfs_aberrations.ytilt = wfs_tiptilt.offsety;
	wfs_aberrations.focus = focus;
	wfs_aberrations.a1 = a1;
	wfs_aberrations.a2 = a2;
	wfs_aberrations.c1 = c1;
	wfs_aberrations.c2 = c2;
	wfs_aberrations.time_stamp = chara_time_now();

	/* Note that we are using these for mirror position. */

	wfs_aberrations.det_stddev = wfs_tiptilt.correctx;
	wfs_aberrations.mir_stddev = wfs_tiptilt.correcty;
	wfs_aberrations.labao_rate =  andor_setup.processed_frames_per_second;
	wfs_aberrations.r0 = wfs_mean_aberrations.r0;
	wfs_aberrations.seeing = wfs_mean_aberrations.seeing;

	/* Are we recording these? */

	if (aberrations_record_num > 0 &&
            aberrations_record_count < aberrations_record_num)
        {
                aberrations_record[aberrations_record_count] = 
			wfs_aberrations;

		aberrations_record_count++;
	}

	/* Now some means */

	calc_aberrations.xtilt += xtilt;
	calc_aberrations.ytilt += ytilt;
	calc_aberrations.focus += focus;
	calc_aberrations.a1 += a1;
	calc_aberrations.a2 += a2;
	calc_aberrations.c1 += c1;
	calc_aberrations.c2 += c2;
	calc_aberrations.flux += total_flux;

	/* 
	 * For the r0 and residual calculations 
	 * Note that this assumes that the mirror positions
	 * have been set elsewhere, most likely the next function.
	 */

	xtilt2 += (xtilt * xtilt);
	ytilt2 += (ytilt * ytilt);
	xmirr  += wfs_tiptilt.correctx;
	ymirr  += wfs_tiptilt.correcty;
	xmirr2  += (wfs_tiptilt.correctx * wfs_tiptilt.correctx);
	ymirr2  += (wfs_tiptilt.correcty * wfs_tiptilt.correcty);

	if (++count_aberrations >= num_mean_aberrations)
	{	
		/* Mean aberrations */

		wfs_mean_aberrations.xtilt = 
			calc_aberrations.xtilt/num_mean_aberrations;
		wfs_mean_aberrations.ytilt = 
			calc_aberrations.ytilt/num_mean_aberrations;
		wfs_mean_aberrations.focus = 
			calc_aberrations.focus/num_mean_aberrations;
		wfs_mean_aberrations.a1 = 
			calc_aberrations.a1/num_mean_aberrations;
		wfs_mean_aberrations.a2 = 
			calc_aberrations.a2/num_mean_aberrations;
		wfs_mean_aberrations.c1 = 
			calc_aberrations.c1/num_mean_aberrations;
		wfs_mean_aberrations.c2 = 
			calc_aberrations.c2/num_mean_aberrations;
		wfs_mean_aberrations.flux = 
			calc_aberrations.flux/num_mean_aberrations;

		/* R0 and residual degtector */

		xtilt2 /= num_mean_aberrations;
		ytilt2 /= num_mean_aberrations;
		xmirr  /= num_mean_aberrations;
		ymirr  /= num_mean_aberrations;
		xmirr2 /= num_mean_aberrations;
		ymirr2 /= num_mean_aberrations;

		wfs_mean_aberrations.det_stddev = sqrt(xtilt2 + ytilt2) *
						DET_CALIB;

		mir_var = (xmirr2 - xmirr * xmirr + ymirr2 - ymirr * ymirr) *
				MIR_CALIB * MIR_CALIB;

		wfs_mean_aberrations.mir_stddev = sqrt(mir_var);
		
		/* 
		 * Now, for the r0 calculation we need the mean 
		 * power in one axis, not over both (See Log Book 4 Page 334).
 		 * So we now divide by two
		 */

		mir_var /= 2.0;

#warning This does not do zenith corrected seeing
		if (mir_var > 0.0)
		{
		    /* Convert rad^2 */

		    mir_var *= 2.35044E-11;

		    /* Get r0 and seeing disk size */

		    wfs_mean_aberrations.r0 = 
				100.0 * pow((4.575E-14/mir_var),0.6);

		    if (wfs_mean_aberrations.r0 > 0.0)
		    {
			wfs_mean_aberrations.seeing = (0.71e-6/
				 pow(wfs_mean_aberrations.r0*0.01,0.833333))
					/4.85e-6;
		    }
		    else
		    {
			wfs_mean_aberrations.seeing = 0.0;
		    }
		}
		else
		{
		    wfs_mean_aberrations.r0 = 0.0;
		    wfs_mean_aberrations.seeing = 0.0;
		}

		/* Reset mean calculations */

		calc_aberrations.xtilt = 0.0;
		calc_aberrations.ytilt = 0.0;
		calc_aberrations.focus = 0.0;
		calc_aberrations.a1 = 0.0;
		calc_aberrations.a2 = 0.0;
		calc_aberrations.c1 = 0.0;
		calc_aberrations.c2 = 0.0;
		calc_aberrations.flux = 0.0;

		xtilt2 = 0.0;
		ytilt2 = 0.0;
		xmirr  = 0.0;
		ymirr  = 0.0;
		xmirr2 = 0.0;
		ymirr2 = 0.0;

		count_aberrations = 0;
		new_mean_aberrations = TRUE;
	}

} /* calculate_tiptilt() */

/************************************************************************/
/* servo_tiptilt()							*/
/*									*/
/* Actually performs the tiptilt servo part.				*/
/************************************************************************/

void servo_tiptilt(void)
{
	static float	delta_x = 0.0;
	static float	delta_y = 0.0;
	float		az = 0.0, el = 0.0;
	float	gain = 0.0;

	/* Now, are we faking out the servo? */

	if (fake_mirror)
	{
		gain = MIR_CALIB / DET_CALIB;

		wfs_tiptilt.offsetx -= gain * wfs_tiptilt.correctx;
		wfs_tiptilt.offsety -= gain * wfs_tiptilt.correcty;
	}

	/* Are we sending a sine wave or running a servo? */

	if (wfs_tiptilt_modulation.which == WFS_MODULATION_SIN_WAVE)
	{
	    /* Modulation is on */

	    delta_x = wfs_tiptilt_modulation.amplitude_x * 
				sin(wfs_tiptilt_modulation.cycle) -
			wfs_tiptilt.correctx;
	    delta_y = wfs_tiptilt_modulation.amplitude_y * 
				cos(wfs_tiptilt_modulation.cycle) -
			wfs_tiptilt.correcty;

	    wfs_tiptilt_modulation.cycle += wfs_tiptilt_modulation.delta_cycle;
	}
	else if (wfs_tiptilt_servo.on)
	{
	    /* Servo is on, do we have enough flux? */

	    if (wfs_mean_aberrations.flux < wfs_tiptilt_servo.min_flux)
	    {
		wfs_tiptilt_servo.on = FALSE;
	        delta_x = 0.0;
	        delta_y = 0.0;
	        wfs_tiptilt.correctx = 0.0;
	        wfs_tiptilt.correcty = 0.0;
		send_wfs_text_message("Too little flux. Tiptilt SERVO off.");
		send_tiptilt_servo = TRUE;
	    } 
	    else
	    {
		/* Anything from labao? */

		current_labao_tiptilt(&az, &el);

	        delta_x = wfs_tiptilt_servo.gain_x * wfs_tiptilt.offsetx +
#ifndef MOVE_BOXES_FOR_LABAO_TIPTILT
                        wfs_tiptilt_servo.labao_x * az -
#endif
			wfs_tiptilt_servo.damp_x * delta_x;
	
	        delta_y = wfs_tiptilt_servo.gain_y * wfs_tiptilt.offsety +
#ifndef MOVE_BOXES_FOR_LABAO_TIPTILT
                        wfs_tiptilt_servo.labao_y * el -
#endif
			wfs_tiptilt_servo.damp_y * delta_y;
	    }
	}
	else
	{
	    /* Force it to zero */

	    delta_x = 0.0;
	    delta_y = 0.0;
	    wfs_tiptilt.correctx = 0.0;
	    wfs_tiptilt.correcty = 0.0;
	}
	   
	/* We only alow a certain maximum change in mirror position */

	if (delta_x > MAX_MIRROR_DELTA)
		delta_x = MAX_MIRROR_DELTA;
	else if (delta_x < -1.0 * MAX_MIRROR_DELTA)
		delta_x = -1.0 * MAX_MIRROR_DELTA;

	if (delta_y > MAX_MIRROR_DELTA)
		delta_y = MAX_MIRROR_DELTA;
	else if (delta_y < -1.0 * MAX_MIRROR_DELTA)
		delta_y = -1.0 * MAX_MIRROR_DELTA;

	/* We only allow a certain range of motion */

	if  ((wfs_tiptilt.correctx += delta_x) > MAX_MIRROR)
		wfs_tiptilt.correctx = MAX_MIRROR;
	else if  (wfs_tiptilt.correctx < -1.0* MAX_MIRROR)
		wfs_tiptilt.correctx = -1.0 * MAX_MIRROR;

	if  ((wfs_tiptilt.correcty += delta_y) > MAX_MIRROR)
		wfs_tiptilt.correcty = MAX_MIRROR;
	else if  (wfs_tiptilt.correcty < -1.0* MAX_MIRROR)
		wfs_tiptilt.correcty = -1.0 * MAX_MIRROR;

	/* Send this to the scope if we can */

	if (wfs_tiptilt_servo.send)
		send_tiptilt_data(wfs_tiptilt.correctx, wfs_tiptilt.correcty);

} /* servo_tiptilt() */

/************************************************************************/
/* message_wfs_save_aberrations()					*/
/*									*/
/* Allocate memory, setup globals for recording aberrations data.	*/
/************************************************************************/

int message_wfs_save_aberrations(struct smessage *mess)
{
	int	num = 0;

        if (mess->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of bytes in WFS_SETUP_ABERRATIONS_RECORD.");
        }

	/* Are we already recording data? */

	if (aberrations_record_num != 0)
	{
		send_wfs_text_message(
			"We are already recording aberrations data.");
		return NOERROR;
	}

	num = *((int *)mess->data);

	if (num <= 0) return NOERROR;
	if (num > MAX_ABERRATIONS_RECORD) num = MAX_ABERRATIONS_RECORD;

	aberrations_record_count = 0;
	aberrations_record_num = num;

	send_wfs_text_message(
		"Trying to save %d aberration measurements", num);

	return NOERROR;

}  /* message_wfs_save_aberrations() */

/************************************************************************/
/* complete_aberrations_record()					*/
/*									*/
/* Should be called periodically to see if there is a complete set	*/
/* of aberrations data recorded we need to save.			*/
/************************************************************************/

void complete_aberrations_record(void)
{
	int	year, month, day;
	char	filename[345];
	char	s[345];
	FILE	*fp;
	time_t	now;
	struct tm *gmt_now;
	int	i;

	/* Have we finished? */

	if (aberrations_record_num == 0 ||
	    aberrations_record_count < aberrations_record_num)
	{
		return;
	}

	/* We have data, let's find a good filename */

	time(&now);
	gmt_now = gmtime(&now);
	year = gmt_now->tm_year + 1900;
	month = gmt_now->tm_mon+1;
	day = gmt_now->tm_mday;
	
	if (year < 1950) year+=100; /* Y2K.... */

	for(i=1; i<1000; i++)
	{
		sprintf(filename,
			"%s/%4d_%02d_%02d_%s_wfs_aberrations_%03d.dat",
			get_data_directory(s), year, month, day, wfs_name, i);

		if ((fp = fopen(filename, "r")) == NULL) break;
		fclose(fp);
	}

	if (i > 999) 
	{
		error(ERROR,"Too many aberrations files.");
		return;
	}

	/* OK, save the data */

	if ((fp = fopen(filename, "w")) == NULL)
	{
		error(ERROR, "Failed to open file %s", filename );
		return;
	}

	fprintf(fp,"# FILENAME        : %s\n",filename);
	fprintf(fp,"# GMT YEAR        : %d\n",year);
	fprintf(fp,"# GMT MONTH       : %d\n",month);
	fprintf(fp,"# GMT DAY         : %d\n",day);
	fprintf(fp,
	"# Time     Tilt X  Tilt Y   Focus    A1      A2      C1     C2\n");
	
	for(i=0; i< aberrations_record_num; i++)
		fprintf(fp,
		"%9d %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
			aberrations_record[i].time_stamp,			
			aberrations_record[i].xtilt,			
			aberrations_record[i].ytilt,			
			aberrations_record[i].focus,
			aberrations_record[i].a1,
			aberrations_record[i].a2,
			aberrations_record[i].c1,
			aberrations_record[i].c2);

	fclose(fp);
	aberrations_record_num = 0;
	aberrations_record_count = 0;

	send_wfs_text_message("Saved file %s", filename);

} /* complete_aberrations_record() */
