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
	float	xtilt = 0.0, ytilt = 0.0;
	float	dx, dy;
	int	subap;
	static struct  s_wfs_aberrations calc_aberrations;
	static int count_aberrations = 32700;
	static float	xtilt2 = 0.0, ytilt2 = 0.0;
	static float xmirr = 0.0, ymirr = 0.0, xmirr2 = 0.0, ymirr2 = 0.0;
	float	mir_var = 0.0;
	float	rad, num = 0.0;

	/* Find the pupil center */

	for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
        {
		x_mean_offset += subap_centroids_ref.x[subap];
		y_mean_offset += subap_centroids_ref.y[subap];
	}
	x_mean_offset /= WFS_DFT_SUBAP_NUMBER;
	y_mean_offset /= WFS_DFT_SUBAP_NUMBER;
	total_flux = 0.0;

	/* Now do the remaining calculations */

	rad = max_radius * max_radius;
	num = 0.0;
	for(subap = 0; subap < WFS_DFT_SUBAP_NUMBER; subap++)
        {
		x_offset = subap_centroids_ref.x[subap] - x_mean_offset;
		y_offset = subap_centroids_ref.y[subap] - y_mean_offset;

		if (rad > 0 && rad < x_offset*x_offset + y_offset*y_offset)
			continue;

		num += 1.0;

		if (fabs(x_offset) > max_offset) max_offset = fabs(x_offset);
		if (fabs(y_offset) > max_offset) max_offset = fabs(y_offset);

		xtilt += (dx = subap_centroids.x[subap] - 
				subap_centroids_ref.x[subap]);
		ytilt += (dy = subap_centroids.y[subap] - 
				subap_centroids_ref.y[subap]);

		total_flux += subap_centroids.inten[subap];

		if (subap_centroids.inten[subap] > max_flux)
			max_flux = subap_centroids.inten[subap];

		focus += (dx * x_offset + dy * y_offset);
		a1 += (dx * x_offset - dy * y_offset);
		a2 += (dy * x_offset + dx * y_offset);
	}

	/* Tilt needs to go between -1 and 1 */

	xtilt /= (float)(num*(subap_centroids_ref.size/2.0));
	ytilt /= (float)(num*(subap_centroids_ref.size/2.0));
	focus /= (float)num;
	a1 /= (float)num;
	a2 /= (float)num;

#warning We may also need to multiply by arcsecs per pixel

	/* 
	 * So that all terms are at least in units of "pixels" 
	 * (i.e. max tilt per sub-aperture), we divide by the
         * radius of the pupil.
	 */

        focus /= max_offset;
        a1 /= max_offset;
        a2 /= max_offset;

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

	/* Note that we are using these for mirror position. */

	wfs_aberrations.det_stddev = wfs_tiptilt.correctx;
	wfs_aberrations.mir_stddev = wfs_tiptilt.correcty;
	wfs_aberrations.r0 = wfs_mean_aberrations.r0;
	wfs_aberrations.seeing = wfs_mean_aberrations.seeing;

	/* Now some means */

	calc_aberrations.xtilt += xtilt;
	calc_aberrations.ytilt += ytilt;
	calc_aberrations.focus += focus;
	calc_aberrations.a1 += a1;
	calc_aberrations.a2 += a2;

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
				sin(wfs_tiptilt_modulation.cycle) -
			wfs_tiptilt.correcty;

	    wfs_tiptilt_modulation.cycle += wfs_tiptilt_modulation.delta_cycle;
	}
	else if (wfs_tiptilt_servo.on)
	{
	    /* Servo is on */

	    delta_x = wfs_tiptilt_servo.gain_x * wfs_tiptilt.offsetx
			- wfs_tiptilt_servo.damp_x * delta_x;
	
	    delta_y = wfs_tiptilt_servo.gain_y * wfs_tiptilt.offsety
			- wfs_tiptilt_servo.damp_y * delta_y;
	}
	else
	{
	    /* Force it to zero */

	    delta_x = -1.0 * wfs_tiptilt.correctx;
	    delta_y = -1.0 * wfs_tiptilt.correcty;
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

#warning X and Y were reversed on old server

	if (wfs_tiptilt_servo.send)
		send_tiptilt_data(wfs_tiptilt.correctx, wfs_tiptilt.correcty);

} /* servo_tiptilt() */
