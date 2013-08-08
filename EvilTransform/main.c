//
//  main.c
//  EvilTransform
//
//  Created by Leo.Chen on 13-7-29.
//  Copyright (c) 2013年 Leo.Chen. All rights reserved.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static double Bd_pi = M_PI * 3000.0 / 180.0;
static double a = 6378245.0;
static double ee = 0.00669342162296594323;

int outOfChina(double lat, double lon)
{
	if (lon < 72.004 || lon > 137.8347)
		return 1;
	if (lat < 0.8293 || lat > 55.8271)
		return 1;
	return 0;
}

double transformLat(double x, double y)
{
	double ret = -100.0 + 2.0*x + 3.0*y + 0.2*y*y + 0.1*x*y + 0.2*sqrt(abs(x));
	ret += (20.0 * sin(6.0*x*M_PI) + 20.0*sin(2.0*x*M_PI)) * 2.0/3.0;
	ret += (20.0 * sin(y*M_PI) + 40.0*sin(y/3.0*M_PI))*2.0/3.0;
	ret += (160.0 * sin(y/12.0*M_PI) + 320*sin(y*M_PI/30.0)) * 2.0/3.0;
	return ret;
	
}

double transformLon(double x, double y)
{
	double ret = 300.0 + x + 2.0*y + 0.1*x*x + 0.1*x*y + 0.1*sqrt(abs(x));
	ret += (20.0 * sin(6.0*x*M_PI) + 20.0*sin(2.0*x*M_PI)) * 2.0/3.0;
	ret += (20.0 * sin(x*M_PI) + 40.0*sin(x/3.0*M_PI)) * 2.0/3.0;
	ret += (150.0 * sin(x/12.0*M_PI) + 300.0*sin(x/30.0*M_PI)) * 2.0/3.0;
	return ret;
}

/**gps转火星坐标*/
int wgs2gcj(double wgLat, double wgLon, double *mgLat, double *mgLon)
{
	if (outOfChina(wgLat, wgLon)) {
		*mgLat = wgLat;
		*mgLon = wgLon;
	}
	
	double dLat = transformLat(wgLon - 105.0, wgLat - 35.0);
	double dLon = transformLon(wgLon - 105.0, wgLat - 35.0);
	
	double radLat = wgLat/180.0*M_PI;
	double magic = sin(radLat);
	magic = 1-ee * magic*magic;
	
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a* (1-ee)) / (magic*sqrtMagic) * M_PI);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);
	
	*mgLat = wgLat + dLat;
	*mgLon = wgLon + dLon;
	
	return 0;
}

/**火星转GPS坐标, 反推,误差1－2米*/
int gcj2wgs(double mgLat, double mgLon, double *wgLat, double *wgLon)
{
	double dLat = transformLat(mgLon - 105.0, mgLat - 35.0);
	double dLon = transformLon(mgLon - 105.0, mgLat - 35.0);
	
	double radLat = mgLat/180.0*M_PI;
	double magic = sin(radLat);
	magic = 1-ee * magic*magic;
	
	double sqrtMagic = sqrt(magic);
	dLat = (dLat * 180.0) / ((a* (1-ee)) / (magic*sqrtMagic) * M_PI);
	dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * M_PI);
	
	*wgLat = mgLat - dLat;
	*wgLon = mgLon - dLon;
	
	return 0;
}

/**火星转百度*/
int gcj2bd (double mgLat, double mgLon, double *bgLat, double *bgLon)
{
    double x = mgLon, y = mgLat;
    double z = sqrt(x * x + y * y) + 0.00002 * sin(y * Bd_pi);
    double theta = atan2(y, x) + 0.000003 * cos(x * Bd_pi);
	*bgLat= z * sin(theta) + 0.006;
    *bgLon = z * cos(theta) + 0.0065;
	
	return 0;
}

/**百度转火星*/
int bd2gcj (double bgLat, double bgLon, double *mgLat, double *mgLon)
{
    double x = bgLon - 0.0065, y = bgLat - 0.006;
    double z = sqrt(x * x + y * y) - 0.00002 * sin(y * Bd_pi);
    double theta = atan2(y, x) - 0.000003 * cos(x * Bd_pi);
    *mgLon = z * cos(theta);
    *mgLat = z * sin(theta);
	
	return 0;
}

int main(int argc, const char * argv[])
{
	if (argc < 4) {
		printf("Usage: wgLat wgLon type\n");
		return 0;
	}
	double lat = 0.0, lon = 0.0;
	double wgLat = strtod(argv[1], NULL);
	double wgLon = strtod(argv[2], NULL);
	
	do {
		if (strcmp(argv[3], "wgs2gcj") == 0) {
			wgs2gcj(wgLat, wgLon, &lat, &lon);
			break;
		}
		if (strcmp(argv[3], "gcj2wgs") == 0) {
			gcj2wgs(wgLat, wgLon, &lat, &lon);
			break;
		}
		if (strcmp(argv[3], "gcj2bd") == 0) {
			gcj2bd(wgLat, wgLon, &lat, &lon);
			break;
		}
		if (strcmp(argv[3], "bd2gcj") == 0) {
			bd2gcj(wgLat, wgLon, &lat, &lon);
			break;
		}
		if (strcmp(argv[3], "bd2wgs") == 0) {
			double tmpLat, tmpLon;
			bd2gcj(wgLat, wgLon, &tmpLat, &tmpLon);
			gcj2wgs(tmpLat, tmpLon, &lat, &lon);
			break;
		}
		if (strcmp(argv[3], "wgs2bd") == 0) {
			double tmpLat, tmpLon;
			wgs2gcj(wgLat, wgLon, &tmpLat, &tmpLon);
			gcj2bd(tmpLat, tmpLon, &wgLat, &wgLon);
			break;
		}
	} while (0);
	
	printf("{\"rc\":0,\"lat\":%f,\"lon\":%f}",lat, lon);
    return 0;
}




