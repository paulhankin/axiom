/* m_near.c - nearbyint (C17 7.12.9.3): rint, with the "inexact" flag put
   back as it was (F.10.6.3). */
#if __STDC_VERSION__ >= 199901L
double rint(double x);
float rintf(float x);
unsigned long __axc_fe_flags(void);
int __axc_fe_set_flags(unsigned long f, unsigned long mask);

#define INEXACT 0x20

double nearbyint(double x)
{
    unsigned long f = __axc_fe_flags();
    double r = rint(x);
    (void)__axc_fe_set_flags(f, INEXACT);
    return r;
}

float nearbyintf(float x)
{
    unsigned long f = __axc_fe_flags();
    float r = rintf(x);
    (void)__axc_fe_set_flags(f, INEXACT);
    return r;
}
#else
typedef int __axc_near_c89;
#endif
