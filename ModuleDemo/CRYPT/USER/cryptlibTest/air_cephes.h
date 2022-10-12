#ifndef __AIR_CEPHES_H
#define __AIR_CEPHES_H

double air_cephes_igamc(double a, double x);
double air_cephes_igam(double a, double x);
double air_cephes_lgam(double x);
double air_cephes_p1evl(double x, double *coef, int N);
double air_cephes_polevl(double x, double *coef, int N);
double air_cephes_erf(double x);
double air_cephes_erfc(double x);
double air_cephes_normal(double x);

#endif /*  __AIR_CEPHES_H  */
