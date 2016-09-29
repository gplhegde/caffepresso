// 8 bit image multiplier
void IMG_mul_8
(
char * restrict imgR1,
/* Image 1 read pointer */
char * restrict imgR2,
/* Image 2 read pointer */
short * restrict imgW,
/* Output image pointer */
int count
/* Number of samples in image */
)
{
int i;
long long img1_p7_p6_p5_p3_p2_p1_p0, img2_p7_p6_p5_p3_p2_p1_p0;
int img1_p7_p6_p5_p4, img1_p3_p2_p1_p0, img2_p7_p6_p5_p4, img2_p3_p2_p1_p0;
double r3_r2_r1_r0, r7_r6_r5_r4;
for (i = 0; i < count >> 3; i += 8) {
img1_p7_p6_p5_p3_p2_p1_p0 = _amem8(imgR1);
img1_p7_p6_p5_p4 = _hill (img1_p7_p6_p5_p3_p2_p1_p0);
img1_p3_p2_p1_p0 = _loll (img1_p7_p6_p5_p3_p2_p1_p0);
imgR1 += 8;
img2_p7_p6_p5_p3_p2_p1_p0 = _amem8(imgR2);
img2_p7_p6_p5_p4 = _hill (img2_p7_p6_p5_p3_p2_p1_p0);
img2_p3_p2_p1_p0 = _loll (img2_p7_p6_p5_p3_p2_p1_p0);
imgR2 += 8;
r3_r2_r1_r0 = _mpyu4 (img1_p3_p2_p1_p0, img2_p3_p2_p1_p0);
r7_r6_r5_r4 = _mpyu4 (img1_p7_p6_p5_p4, img2_p7_p6_p5_p4);
*((double *)imgW) = r3_r2_r1_r0;
imgW += 4;
*((double *)imgW) = r7_r6_r5_r4;
imgW += 4;
}
}

//8 bit image adder
void IMG_add_8
(
char * restrict imgR1, /* Image 1 read pointer */
char * restrict imgR2, /* Image 2 read pointer */
char * restrict imgW, /* Output image pointer */
int count
/* Number of samples in image */
)
{
int i;
long long im1_p7_p6_p5_p4_p3_p2_p1_p0, im2_p7_p6_p5_p4_p3_p2_p1_p0;
int im1_p7_p6_p5_p4, im1_p3_p2_p1_p0, im2_p7_p6_p5_p4, im2_p3_p2_p1_p0;
int res_p7_p6_p5_p4, res_p3_p2_p1_p0;
for (i = 0; i < count >> 4; i += 16) {
im1_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR1);
im2_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR2);
imgR1 += 8;
imgR2 += 8;
im1_p3_p2_p1_p0 = _loll (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im1_p7_p6_p5_p4 = _hill (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p3_p2_p1_p0 = _loll (im2_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p7_p6_p5_p4 = _hill (im2_p7_p6_p5_p4_p3_p2_p1_p0);
res_p3_p2_p1_p0 = _add4 (im1_p3_p2_p1_p0, im2_p3_p2_p1_p0);
res_p7_p6_p5_p4 = _add4 (im1_p7_p6_p5_p4, im2_p7_p6_p5_p4);
_amem8(imgW) = _itoll (res_p7_p6_p5_p4, res_p3_p2_p1_p0);
imgW += 8;
im1_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR1);
im2_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR2);
imgR1 += 8;
imgR2 += 8;
im1_p3_p2_p1_p0 = _loll (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im1_p7_p6_p5_p4 = _hill (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p3_p2_p1_p0 = _loll (im2_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p7_p6_p5_p4 = _hill (im2_p7_p6_p5_p4_p3_p2_p1_p0);
res_p3_p2_p1_p0 = _add4 (im1_p3_p2_p1_p0, im2_p3_p2_p1_p0);
res_p7_p6_p5_p4 = _add4 (im1_p7_p6_p5_p4, im2_p7_p6_p5_p4);
_amem8(imgW) = _itoll (res_p7_p6_p5_p4, res_p3_p2_p1_p0);
imgW += 8;
}
}

//8 bit subtractor
void IMG_sub_8
(
char * restrict imgR1,
/* Image 1 read pointer */
char * restrict imgR2,
/* Image 2 read pointer */
char * restrict imgW,
/* Output image pointer */
int count
/* Number of samples in image */
)
{
int i;
long long im1_p7_p6_p5_p4_p3_p2_p1_p0, im2_p7_p6_p5_p4_p3_p2_p1_p0;
int im1_p7_p6_p5_p4, im1_p3_p2_p1_p0, im2_p7_p6_p5_p4, im2_p3_p2_p1_p0;
int res_p7_p6_p5_p4, res_p3_p2_p1_p0;
for (i = 0; i < count >> 4; i += 16) {
im1_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR1);
im2_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR2);
imgR1 += 8;
imgR2 += 8;
im1_p3_p2_p1_p0 = _loll (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im1_p7_p6_p5_p4 = _hill (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p3_p2_p1_p0 = _loll (im2_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p7_p6_p5_p4 = _hill (im2_p7_p6_p5_p4_p3_p2_p1_p0);
res_p3_p2_p1_p0 = _sub4 (im1_p3_p2_p1_p0, im2_p3_p2_p1_p0);
res_p7_p6_p5_p4 = _sub4 (im1_p7_p6_p5_p4, im2_p7_p6_p5_p4);
_amem8(imgW) = _itoll (res_p7_p6_p5_p4, res_p3_p2_p1_p0);
imgW += 8;
im1_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR1);
im2_p7_p6_p5_p4_p3_p2_p1_p0 = _amem8(imgR2);
imgR1 += 8;
imgR2 += 8;
im1_p3_p2_p1_p0 = _loll (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im1_p7_p6_p5_p4 = _hill (im1_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p3_p2_p1_p0 = _loll (im2_p7_p6_p5_p4_p3_p2_p1_p0);
im2_p7_p6_p5_p4 = _hill (im2_p7_p6_p5_p4_p3_p2_p1_p0);
res_p3_p2_p1_p0 = _sub4 (im1_p3_p2_p1_p0, im2_p3_p2_p1_p0);
res_p7_p6_p5_p4 = _sub4 (im1_p7_p6_p5_p4, im2_p7_p6_p5_p4);
_amem8(imgW) = _itoll (res_p7_p6_p5_p4, res_p3_p2_p1_p0);
imgW += 8;
}
}

