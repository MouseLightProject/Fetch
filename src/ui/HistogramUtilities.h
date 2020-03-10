#pragma once
#include "common.h"
namespace mylib {
#include <array.h>
}
#include "qcustomplot.h"
#include <algorithm>

//DGA: Moved this from HistogramDockWidget to its own header file.

#ifdef _MSC_VER
#define restrict __restrict
#else
#define restrict __restrict__ 
#endif

#define FAIL     do{         qFatal("%s(%d)"ENDL "\tExecution should not reach here."ENDL,__FILE__,__LINE__);                                 }while(0)

// histogram utilities START  
#define TYPECASES(ARRAYARG) do {\
    switch(ARRAYARG->type)  \
    { CASE( UINT8_TYPE ,u8); \
      CASE( UINT16_TYPE,u16);\
      CASE( UINT32_TYPE,u32);\
      CASE( UINT64_TYPE,u64);\
      CASE(  INT8_TYPE ,i8); \
      CASE(  INT16_TYPE,i16);\
      CASE(  INT32_TYPE,i32);\
      CASE(  INT64_TYPE,i64);\
      CASE(FLOAT32_TYPE,f32);\
      CASE(FLOAT64_TYPE,f64);\
      default: \
        FAIL;  \
    }}while(0)

  static double amin(mylib::Array *a)
  { double out; 
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]<m)?d[i]:m; out=(double)m;} break;
    TYPECASES(a);
#undef CASE
    return out;
  }

  static double amax(mylib::Array *a)
  { double out;
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]>m)?d[i]:m; out=(double)m;} break;    
    TYPECASES(a);
#undef CASE
    return out;    
  }


  static void percentiles(mylib::Array *a, unsigned int * _pixelValueCounts, double minPercent, double maxPercent, double &minValueOut, double &maxValueOut) //DGA: Function to calculate percentiles
  { int totalCount=0, currentValue=0; //DGA: totalCount for pdfMethod and currentValue for incrementing through PDF
	double minCount = minPercent * a->size, maxCount = maxPercent * a->size; //DGA: minCount and maxCount used for determining when correct value is reached
	minValueOut=-1; //DGA: Initialize minValueOut to -1
	//DGA: Define PDFMETHOD as a way to calculate the minimum and maximum cutoff values for image a when a has int data types that are less than 32 bits. 
	//Set d equal to the data from a,hen set _pixelValueCounts (unsigned int array) to 0.
	//Bin values of d and increment corresponding bins to build up a PDF of pixel values.
	//Loop through _pixelValueCounts, incrementing the total count, thereby creating a CDF.
	//When totalCount >= minCount, then minValueOut should equal currentValue.
	//When totalCount >= maxCount, then maxValueOut should equal currentValue.
#define PDFMETHOD(T) do{ const T *d = (T*)a->data; memset(_pixelValueCounts,0,65536*sizeof(unsigned int)); for(int i=0;i<a->size;i++) _pixelValueCounts[d[i]]++;\
						 while (totalCount<maxCount){\
							    totalCount+=_pixelValueCounts[currentValue];\
		                        if (minValueOut==-1 && totalCount>=minCount) minValueOut=currentValue;\
		                        currentValue++;}\
				         maxValueOut=currentValue;}while(0) 
	//DGA: For data types requiring 32 or more bits, use SORTMETHOD to calculate minimum and maximum cutoff values for image a. First copy data from a into new dataArray.
	//Then sort data array. minValueOut will be the value at the ceil(minCount) location, maxValueOut will be the value at ceil(maxCount) location.
#define SORTMETHOD(T) do{T * dataArray = new T [a->size];  memcpy(dataArray,a->data,a->size*sizeof(T)); std::sort(dataArray,dataArray+a->size); minValueOut=dataArray[int(ceil(minCount))]; maxValueOut=dataArray[int(ceil(maxCount))];}while(0)
    //DGA: Use the PDF method if type is less than 32 bits, toherwise use the sort method
    if (a->type == mylib::UINT8_TYPE) PDFMETHOD(u8);
	else if (a->type == mylib::UINT16_TYPE) PDFMETHOD(u16);
	else if (a->type == mylib::INT8_TYPE) PDFMETHOD(i8);
	else if (a->type == mylib::INT16_TYPE) PDFMETHOD(i16);
	else if (a->type == mylib::UINT32_TYPE) SORTMETHOD(u32);
	else if (a->type == mylib::INT32_TYPE) SORTMETHOD(i32);
	else if (a->type == mylib::UINT64_TYPE) SORTMETHOD(u64);
	else if (a->type == mylib::INT64_TYPE) SORTMETHOD(i64);
	else if (a->type == mylib::FLOAT32_TYPE) SORTMETHOD(f32);
	else if (a->type == mylib::FLOAT64_TYPE) SORTMETHOD(f64);
#undef PDFMETHOD;
#undef SORTMETHOD;
  }

  static void determineMaximumValueForDataType(mylib::Value_Type type, f64 &maximumValueForDataType){ //DGA: Determine the maximum value for the data type
	  if (type == mylib::UINT8_TYPE) maximumValueForDataType = UCHAR_MAX;
	  else if (type == mylib::UINT16_TYPE) maximumValueForDataType = USHRT_MAX;
	  else if (type == mylib::UINT32_TYPE)  maximumValueForDataType = UINT_MAX;
	  else if (type == mylib::UINT64_TYPE)  maximumValueForDataType = ULLONG_MAX;
	  else if (type == mylib::INT8_TYPE) maximumValueForDataType = CHAR_MAX;
	  else if (type == mylib::INT16_TYPE) maximumValueForDataType = SHRT_MAX;
	  else if (type == mylib::INT32_TYPE)  maximumValueForDataType = INT_MAX;
	  else if (type == mylib::INT64_TYPE)  maximumValueForDataType = LLONG_MAX;
	  else if (type == mylib::FLOAT32_TYPE)  maximumValueForDataType = FLT_MAX;
	  else if (type == mylib::FLOAT64_TYPE)  maximumValueForDataType = DBL_MAX;
  }

  static unsigned nbins(mylib::Array *a,double min, double max)
  { unsigned n,lim = 1<<12; // max number of bins
    switch(a->type)
    { case mylib::UINT8_TYPE:
      case mylib::INT8_TYPE:
        lim=1<<8;
      case mylib::UINT16_TYPE:
      case mylib::UINT32_TYPE:
      case mylib::UINT64_TYPE:
      case mylib::INT16_TYPE:
      case mylib::INT32_TYPE:
      case mylib::INT64_TYPE:
        n=max-min+1;
        return (n<lim)?n:lim;      
      case mylib::FLOAT32_TYPE:
      case mylib::FLOAT64_TYPE:
        return lim;
      default:
        FAIL;
    }
  }
  
  static double binsize(unsigned n, double min, double max)
  { return (n==0)?0:(((double)n)-1)/(max-min);
  }
  
  template<class T>
  static void count(double *restrict pdf, size_t nbins, T *restrict data, size_t nelem, T min, double dy)
  { for(size_t i=0;i<nelem;++i) pdf[ (size_t)((data[i]-min)*dy) ]++;      
    for(size_t i=0;i<nbins;++i) pdf[i]/=(double)nelem;
  }
  static void scan(double *restrict cdf, double *restrict pdf, size_t n)
  { memcpy(cdf,pdf,n*sizeof(double));
    for(size_t i=1;i<n;++i) cdf[i]+=cdf[i-1];
  }
  static void setbins(double *x, size_t n, double min, double dy)
  { for(size_t i=0;i<n;++i) x[i] = i/dy+min;
  }
  	
  static size_t findIndex(QVector<double> &cdf, double perct)
  {
    size_t index;
    double * cdfdata = cdf.data();
    double diff=1.0, minDiff = 1.0;

    for (size_t i=0; i<cdf.size(); ++i)
    {
      diff = abs(cdfdata[i] - perct);
      if (diff <= minDiff) 
      {
        minDiff = diff;
        index = i;
      }
    }

    return index;
  }
      
  static void histogram(QVector<double> &x, QVector<double> &pdf, QVector<double> &cdf, mylib::Array *a)
  { double min,max,dy;
    unsigned n;
    min=amin(a);
    max=amax(a);
#if 0
    HERE;
    mylib::Write_Image("histogram.tif",a,mylib::DONT_PRESS);    
#endif
//    debug("%s(%d) %s"ENDL "\tmin %6.1f\tmax %6.1f"ENDL,__FILE__,__LINE__,__FUNCTION__,min,max);
    n=nbins(a,min,max);
    dy=binsize(n,min,max); // value transform is  (data[i]-min)*dy --> for max this is (max-min)*(nbins-1)/(max-min)

    x.resize(n);
    pdf.resize(n);
    cdf.resize(n);
    
#define CASE(ID,T) case mylib::ID: count<T>(pdf.data(),n,(T*)a->data,a->size,min,dy); break;
    TYPECASES(a);
#undef CASE
    scan(cdf.data(),pdf.data(),n);
    setbins(x.data(),n,min,dy);
  }
 // histogram utilities END