#include <cmath>
#include <complex>
#include "nrutil.h"
// Given a real vector of data[1..n], and given m, this routine returns m linear prediction coefficients as d[1..m], 
//     and returns the mean square discrepancy as xms.
void memcof(float data[], int n, int m, float *xms, float d[]) {
	int k,j,i;
	float p=0.0,*wk1,*wk2,*wkm;
	wk1=vector(1,n);
	wk2=vector(1,n);
	wkm=vector(1,m);
	for (j=1;j<=n;j++) p += SQR(data[j]);
	*xms=p/n;
	wk1[1]=data[1];
	wk2[n-1]=data[n];
	for (j=2;j<=n-1;j++) {
		wk1[j]=data[j];
		wk2[j-1]=data[j];
	}
	for (k=1;k<=m;k++) {
		float num=0.0,denom=0.0;
		for (j=1;j<=(n-k);j++) {
			num += wk1[j]*wk2[j];
			denom += SQR(wk1[j])+SQR(wk2[j]);
		}
		d[k]=2.0*num/denom;
		*xms *= (1.0-SQR(d[k]));
		for (i=1;i<=(k-1);i++)
			d[i]=wkm[i]-d[k]*wkm[k-i];
			// The algorithm is recursive, building up the answer for larger and larger values of m
			//     until the desired value is reached. At this point in the algorithm, one could return
			//     the vector d and scalar xms for a set of LP coefficients with k (rather than m) terms.
		if (k == m) {
			free_vector(wkm,1,m);
			free_vector(wk2,1,n);
			free_vector(wk1,1,n);
			return;
		}
		for (i=1;i<=k;i++) wkm[i]=d[i];
		for (j=1;j<=(n-k-1);j++) {
			wk1[j] -= wkm[k]*wk2[j];
			wk2[j]=wk2[j+1]-wkm[k]*wk1[j+1];
		}
	}
	nrerror("never get here in memcof.");
}

#define NMAX 100 // Largest expected value of m.
// #define ZERO Complex(0.0,0.0)
// #define ONE Complex(1.0,0.0)

// Given the LP coefficients d[1..m], this routine finds all roots of the characteristic polynomial
//     (13.6.14), reflects any roots that are outside the unit circle back inside, and then returns a
//     modified set of coefficients d[1..m].
void fixrts(float d[], int m) {
	void zroots(fcomplex a[], int m, fcomplex roots[], int polish);
	int i,j,polish;
	fcomplex a[NMAX],roots[NMAX];
	a[m]=ONE;
	for (j=m-1;j>=0;j--) Set up complex coefficients for polynomial root
	a[j]=Complex(-d[m-j],0.0); finder.
	polish=1;
	zroots(a,m,roots,polish); Find all the roots.
	for (j=1;j<=m;j++) Look for a...
	if (Cabs(roots[j]) > 1.0) root outside the unit circle,
	roots[j]=Cdiv(ONE,Conjg(roots[j])); and reflect it back inside.
	a[0]=Csub(ZERO,roots[1]); Now reconstruct the polynomial coefficients,
	a[1]=ONE;
	for (j=2;j<=m;j++) { by looping over the roots
	a[j]=ONE;
	for (i=j;i>=2;i--) and synthetically multiplying.
	a[i-1]=Csub(a[i-2],Cmul(roots[j],a[i-1]));
	a[0]=Csub(ZERO,Cmul(roots[j],a[0]));
	}
	for (j=0;j<=m-1;j++) The polynomial coefficients are guaranteed to be
	real, so we need only return the real part as
	new LP coefficients.
	d[m-j] = -a[j].r;
}