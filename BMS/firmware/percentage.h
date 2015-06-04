#ifndef PERCENTAGE_H
#define PERCENTAGE_H

#define p1         0.0668
#define p2         0.3141
#define p3         0.3531
#define p4        -0.2128
#define p5        -0.3324
#define p6         0.6422
#define p7          2.936
#define p8          4.184

#define p10        0.5245
#define p20        -1.722
#define p30        -1.428
#define p40         7.848
#define p50       -0.7242     
#define p60        -13.69
#define p70          29.7
#define p80         60.95

#define transition 11.014
#define maximum      12.3
#define minimum       9.6

float Percentage(float x)
{
  float percent;
  
  if ((x < transition) && (x>minimum))
  {
    x=(x-10.64)/0.314;
    percent = p1*pow(x,7) + p2*pow(x,6) + p3*pow(x,5) + p4*pow(x,4) + p5*pow(x,3) + 
    p6*pow(x,2) + p7*x + p8;
    
    if (percent < 0)
    {
      percent=0;
    }
  }
  
  else if ((x >= transition) && (x<maximum))
  {
    x=(x-11.49)/0.3152;
    percent = p10*pow(x,7) + p20*pow(x,6) + p30*pow(x,5) + p40*pow(x,4) + p50*pow(x,3) + 
    p60*pow(x,2) + p70*x + p80;
    
    if (percent > 100)
    {
      percent=100;
    }
  }
  
  else if (x>=maximum)
  {
    percent=100;
  }
  
  else
  {
    percent=0;
  }
  
  return percent;
}

#endif
