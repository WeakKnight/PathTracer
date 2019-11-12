
// cyCodeBase by Cem Yuksel
// [www.cemyuksel.com]
//-------------------------------------------------------------------------------
//!
//! \file       cyIrradianceMap.h 
//! \author     Cem Yuksel
//! \version    0.4
//! \date       August 21, 2019
//!
//! \brief irradiance map class.
//!
//!
//! @copydoc cyIrradianceMap
//!
//! A simple class to store irradiance values for rendering using Monte Carlo
//! sampling for indirect illumination.
//!
//-------------------------------------------------------------------------------
//
// Copyright (c) 2016, Cem Yuksel <cem@cemyuksel.com>
// All rights reserved.
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the "Software"), to deal 
// in the Software without restriction, including without limitation the rights 
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
// copies of the Software, and to permit persons to whom the Software is 
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
// 
//-------------------------------------------------------------------------------
 
 
#ifndef _CY_IRRADIANCE_MAP_H_INCLUDED_
#define _CY_IRRADIANCE_MAP_H_INCLUDED_
 
//-------------------------------------------------------------------------------
 
#include <mutex>
 
//-------------------------------------------------------------------------------
namespace cy {
//-------------------------------------------------------------------------------
 
//! Irradiance map class
template <class T> class IrradianceMap
{
public:
    IrradianceMap() : data(nullptr), width(0), height(0) {}
    virtual ~IrradianceMap() { if ( data ) delete [] data; }
 
    //! Initialize once by providing the image with and height.
    //! The maxSubdiv parameter determines how many computation points will be generated.
    //! The minSubdiv parameter determines the minimum computation resolution.
    //! When minSubdiv is negative, a computation point is generated with 2^(-minSubdiv) pixels apart.
    //! When minSubdiv is positive, 2^(minSubdiv) computation points are generated per pixel.
    //! The total number of computation points is 2^(maxSubdiv) per pixel.
    //! If maxSubdiv is negative, width and height parameters must be an even multiple of 2^(-maxSubdiv);
    //! otherwise, returns false.
    bool Initialize(unsigned int _width, unsigned int _height, int _minSubdiv=-5, int _maxSubdiv=0)
    {
        if ( _maxSubdiv < 0 ) {
            unsigned int mask = (1<<(-_maxSubdiv))-1;
            if ( (_width & mask) > 0 || (_height & mask) > 0 ) return false;
        }
        if ( _maxSubdiv < _minSubdiv ) return false;
        if ( data ) delete [] data;
        width  = _width;
        height = _height;
        minSubdiv = _minSubdiv;
        maxSubdiv = _maxSubdiv;
        widthSampleCount = (width >> -maxSubdiv)+1; 
        heightSampleCount = (height >> -maxSubdiv)+1; 
        unsigned int n = widthSampleCount * heightSampleCount;
        data = new T[n];
        currentSubdiv = minSubdiv;
        currentX = 0;
        currentY = 0;
        currentSkipX = 1<<(-currentSubdiv+maxSubdiv);
        currentSkipY = currentSkipX;
        currentPhase = 0;
        return true;
    }
 
    //! Computes the next data point. If the data point can be estimated using previously
    //! computed points, it is estimated. Otherwise, ComputePoint() is called.
    //! Returns false if no more computation is needed.
    bool ComputeNextPoint(int threadID=0)
    {
        Lock();
        int subdiv = currentSubdiv;
        int phase = currentPhase;
        unsigned int x = currentX;
        unsigned int y = currentY;
        currentX += currentSkipX;
        if ( currentX >= widthSampleCount ) {
            currentY += currentSkipY;
            if ( currentY >= heightSampleCount ) {
                if ( currentPhase == 0 ) currentSubdiv++;
                currentPhase++;
                if ( currentPhase > 2 ) {
                    currentSubdiv++;
                    currentPhase = 1;
                }
                if ( currentPhase == 1 ) {
                    currentSkipX = 1<<(-currentSubdiv+maxSubdiv+1);
                    currentSkipY = currentSkipX;
                    currentX = currentSkipX/2;
                    currentY = currentSkipY/2;
                } else { // currentPhase == 2
                    currentSkipY /= 2;
                    currentX = currentSkipX/2;
                    currentY = 0;
                }
            } else {
                switch ( currentPhase ) {
                case 0:
                    currentX = 0;
                    break;
                case 1:
                    currentX = currentSkipX / 2;
                    break;
                case 2:
                    currentX = (1-((currentY/currentSkipY)&1)) * (currentSkipX/2);
                    break;
                }
            }
        }
        Unlock();
        if ( subdiv > maxSubdiv ) return false;
 
        unsigned int i = y*widthSampleCount + x;
        unsigned int halfSkip = currentSkipX/2;
        unsigned int x2 = x + halfSkip;
        unsigned int y2 = y + halfSkip;
         
        bool estimate = false;
        unsigned int i0, i1, i2, i3;
 
        if ( phase == 1 ) {
            if ( x2 < widthSampleCount && y2 < heightSampleCount ) {
                estimate = true;
                unsigned int x1 = x - halfSkip;
                unsigned int y1 = y - halfSkip;
                i0 = y1*widthSampleCount + x1;
                i1 = y1*widthSampleCount + x2;
                i2 = y2*widthSampleCount + x1;
                i3 = y2*widthSampleCount + x2;
            }
        } else if ( phase == 2 ) {
            if ( x > 0 && y > 0 && x2 < widthSampleCount && y2 < heightSampleCount ) {
                estimate = true;
                unsigned int x1 = x - halfSkip;
                unsigned int y1 = y - halfSkip;
                i0 = y1*widthSampleCount + x;
                i1 = y *widthSampleCount + x1;
                i2 = y *widthSampleCount + x2;
                i3 = y2*widthSampleCount + x;
            }
        }
 
        if ( estimate ) {
            T avrg;
            if ( Estimate( avrg, data[i0], data[i1], data[i2], data[i3] ) ) {
                data[i] = avrg;
                return true;
            }
        }
 
        float sskip = (maxSubdiv<0) ? (1<<-maxSubdiv) : (1.0f/(1<<maxSubdiv));
        float sx = float(x) * sskip;
        float sy = float(y) * sskip;
 
        ComputePoint( data[i], sx, sy, threadID );
        return true;
    }
 
    //! Evaluates the value at a given image position by interpolating
    //! the values of the computation points.
    //! Use this method after the computation is done.
    //! The given val should include information that is necessary
    //! to determine is the interpolation in the Filter call is good enough.
    T Sample(float x, float y) const
    {
        float iskip = (maxSubdiv<0) ? (1.0f/(1<<-maxSubdiv)) : (1<<maxSubdiv);
        float xx = x*iskip;
        float yy = y*iskip;
        unsigned int ix = (unsigned int)xx;
        unsigned int iy = (unsigned int)yy;
        float fx = xx - (float)ix;
        float fy = yy - (float)iy;
        unsigned int ix2 = ix+1;
        unsigned int iy2 = iy+1;
        if ( ix >= widthSampleCount ) ix=ix2=widthSampleCount-1;
        else if ( ix2 >= widthSampleCount ) ix2=widthSampleCount-1;
        if ( iy >= heightSampleCount ) iy=iy2=heightSampleCount-1;
        else if ( iy2 >= heightSampleCount ) iy2=heightSampleCount-1;
        unsigned int i0 = iy *widthSampleCount + ix;
        unsigned int i1 = iy *widthSampleCount + ix2;
        unsigned int i2 = iy2*widthSampleCount + ix;
        unsigned int i3 = iy2*widthSampleCount + ix2;
        T val;
        BilinearFilter( val, data[i0], data[i1], data[i2], data[i3], fx, fy );
        return val;
    }
 
protected:
 
    //! Enters the critical section.
    void Lock() { 
		iterator_mutex.lock(); 
	}
 
    //! Leaves the critical section.
    void Unlock() { 
		iterator_mutex.unlock(); 
	}
 
    //! Computes the given screen position.
    virtual void ComputePoint( T &data, float x, float y, int threadID )=0;
 
    //! Computes the average of the four given data values.
    //! If the average is close to all four data values, returns true.
    //! Otherwise, returns false
    virtual bool Estimate( T &avrg, T const &data0, T const &data1, T const &data2, T const &data3 ) const
    {
        Average( avrg, data0, data1, data2, data3 );
        if (    IsSimilar( avrg, data0 ) &&
                IsSimilar( avrg, data1 ) &&
                IsSimilar( avrg, data2 ) &&
                IsSimilar( avrg, data3 ) ) return true;
        return false;
    }
 
    //! Returns the average of the given four data values.
    virtual void Average( T &avrg, T const &data0, T const &data1, T const &data2, T const &data3 ) const=0;
 
    //! Returns if the given two data values are similar.
    virtual bool IsSimilar( T const &data0, T const &data1 ) const=0;
 
    //! Returns the bilinear interpolation of the given four points.
    virtual void BilinearFilter( T &outVal, T const &dataX0Y0, T const &dataX1Y0, T const &dataX0Y1, T const &dataX1Y1, float fx, float fy ) const
    {
        T vy0, vy1;
        LinearFilter( vy0, dataX0Y0, dataX1Y0, fx );
        LinearFilter( vy1, dataX0Y1, dataX1Y1, fx );
        LinearFilter( outVal, vy0, vy1, fy );
    }
 
    //! Returns the linear interpolation of the given two points.
    virtual void LinearFilter( T &outVal, T const &data0, T const &data1, float f ) const=0;
 
private:
    T *data;
    unsigned int width, height;
    int minSubdiv, maxSubdiv;
    unsigned int widthSampleCount, heightSampleCount;
    int currentSubdiv, currentPhase;
    unsigned int currentX, currentY, currentSkipX, currentSkipY;
    std::mutex iterator_mutex;
};
 
//-------------------------------------------------------------------------------
 
//! Irradiance map for a single floating point value per computation.
//! Uses a threshold value to determine if the interpolation is good enough.
class IrradianceMapFloat : public IrradianceMap<float>
{
public:
    IrradianceMapFloat(float _threshold=1.0e30f) : threshold(_threshold) {}
    void SetThreshold(float t) { threshold=t; }
protected:
    virtual void Average( float &avrg, float const &data0, float const &data1, float const &data2, float const &data3 ) const
    {
        avrg = ( data0 + data1 + data2 + data3 ) * 0.25f;
    }
    virtual bool IsSimilar( float const &data0, float const &data1 ) const { return fabs(data0-data1)<threshold; }
    virtual void LinearFilter( float &outVal, float const &data0, float const &data1, float f ) const
    {
        outVal = data0*(1-f) + data1*f;
    }
private:
    float threshold;
};
 
//-------------------------------------------------------------------------------
 
//! Irradiance map for a single color value per computation.
//! Uses a threshold value to determine if the interpolation is good enough.
class IrradianceMapColor : public IrradianceMap<Color>
{
public:
    IrradianceMapColor(float _threshold=1.0e30f) { SetThreshold(_threshold); }
    IrradianceMapColor(Color _threshold) : threshold(_threshold) {}
    void SetThreshold(float t) { threshold.Set(t,t,t); }
    void SetThreshold(Color const &t) { threshold=t; }
protected:
    virtual void Average( Color &avrg, Color const &data0, Color const &data1, Color const &data2, Color const &data3 ) const
    {
        avrg = ( data0 + data1 + data2 + data3 ) * 0.25f;
    }
    virtual bool IsSimilar( Color const &data0, Color const &data1 ) const
    {
        Color dif = data0-data1;
        return fabs(dif.r)<threshold.r && fabs(dif.g)<threshold.g && fabs(dif.b)<threshold.b;
    }
    virtual void LinearFilter( Color &outVal, Color const &data0, Color const &data1, float f ) const
    {
        outVal = data0*(1-f) + data1*f;
    }
private:
    Color threshold;
};
 
//-------------------------------------------------------------------------------
 
//! Irradiance map for with a color and a z-depth value per computation.
//! Uses a color and a z-depth threshold value to determine if the interpolation is good enough.
class IrradianceMapColorZ : public IrradianceMap<ColorA>
{
public:
    IrradianceMapColorZ(float _thresholdColor=1.0e30f, float _thresholdZ=1.0e30f) : thresholdZ(_thresholdZ) { SetColorThreshold(_thresholdColor); }
    IrradianceMapColorZ(Color _thresholdColor, float _thresholdZ=1.0e30f) : thresholdColor(_thresholdColor), thresholdZ(_thresholdZ) {}
    void SetColorThreshold(float t) { thresholdColor.Set(t,t,t); }
    void SetColorThreshold(Color const &t) { thresholdColor=t; }
    void SetZThreshold(float t) { thresholdZ=t; }
protected:
    virtual void Average( ColorA &avrg, ColorA const &data0, ColorA const &data1, ColorA const &data2, ColorA const &data3 ) const
    {
        avrg = ( data0 + data1 + data2 + data3 ) * 0.25f;
    }
    virtual bool IsSimilar( ColorA const &data0, ColorA const &data1 ) const
    {
        ColorA dif = data0-data1;
        return fabs(dif.r)<thresholdColor.r && fabs(dif.g)<thresholdColor.g && fabs(dif.b)<thresholdColor.b && fabs(dif.a)<thresholdZ;
    }
    virtual void LinearFilter( ColorA &outVal, ColorA const &data0, ColorA const &data1, float f ) const
    {
        outVal = data0*(1-f) + data1*f;
    }
private:
    Color thresholdColor;
    float thresholdZ;
};
 
//-------------------------------------------------------------------------------
 
//! Structure that keep a color, a z-depth, and a normal value.
//! Used in IrradianceMapColorZNormal.
struct ColorZNormal
{
    Color c;
    float z;
    Vec3f N;
    ColorZNormal operator + ( ColorZNormal const &czn ) const { ColorZNormal r; r.c=c+czn.c; r.z=z+czn.z; r.N=N+czn.N; return r; }
    ColorZNormal operator - ( ColorZNormal const &czn ) const { ColorZNormal r; r.c=c-czn.c; r.z=z-czn.z; r.N=N-czn.N; return r; }
    ColorZNormal operator * ( float f ) const { ColorZNormal r; r.c=c*f; r.z=z*f; r.N=N*f; return r; }
};
 
//! Irradiance map for with a color, a z-depth, and a normal value per computation.
//! Uses a color, a z-depth, and a normal threshold value to determine if the interpolation is good enough.
class IrradianceMapColorZNormal : public IrradianceMap<ColorZNormal>
{
public:
    IrradianceMapColorZNormal(float _thresholdColor=1.0e30f, float _thresholdZ=1.0e30f, float _thresholdN=0.7f) : thresholdZ(_thresholdZ), thresholdN(_thresholdN) { SetColorThreshold(_thresholdColor); }
    IrradianceMapColorZNormal(Color _thresholdColor, float _thresholdZ=1.0e30f, float _thresholdN=0.7f) : thresholdColor(_thresholdColor), thresholdZ(_thresholdZ), thresholdN(_thresholdN) {}
    void SetColorThreshold(float t) { thresholdColor.Set(t,t,t); }
    void SetColorThreshold(Color const &t) { thresholdColor=t; }
    void SetZThreshold(float t) { thresholdZ=t; }
    void SetNThreshold(float t) { thresholdN=t; }
protected:
    virtual void Average( ColorZNormal &avrg, ColorZNormal const &data0, ColorZNormal const &data1, ColorZNormal const &data2, ColorZNormal const &data3 ) const
    {
        avrg = ( data0 + data1 + data2 + data3 ) * 0.25f;
    }
    virtual bool IsSimilar( ColorZNormal const &data0, ColorZNormal const &data1 ) const
    {
        ColorZNormal dif = data0-data1;
        return fabs(dif.c.r)<thresholdColor.r && fabs(dif.c.g)<thresholdColor.g && fabs(dif.c.b)<thresholdColor.b && fabs(dif.z)<thresholdZ && (data0.N%data1.N)>thresholdN;
    }
    virtual void LinearFilter( ColorZNormal &outVal, ColorZNormal const &data0, ColorZNormal const &data1, float f ) const
    {
        outVal = data0*(1-f) + data1*f;
    }
private:
    Color thresholdColor;
    float thresholdZ;
    float thresholdN;
};
 
//-------------------------------------------------------------------------------
} // namespace cy
//-------------------------------------------------------------------------------
 
#endif