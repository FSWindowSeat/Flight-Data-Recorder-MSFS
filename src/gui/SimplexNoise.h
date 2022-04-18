#ifndef SimplexNoise_H
#define SimplexNoise_H

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <random>

/**
* Simplex Noise Implementation C++ (1D, 2D, 3D)
*
* Revised C++ implementation of Simplex Noise (Ken Perlin, 2002) by Sebastien Rombauts (2014). 
* Published under the MIT License (MIT) https://github.com/SRombauts/SimplexNoise
*
* Original speed-improved Java version by Stefan Gustavson (2012)
* http://webstaff.itn.liu.se/~stegu/simplexnoise/SimplexNoise.java
*
* @author muppetlabs@fswindowseat.com
*/
class SimplexNoise {

private:
    float mFrequency;   
    float mAmplitude;   
    float mLacunarity;  
    float mPersistence;

    // Default permutation table
    uint8_t perm[256] = {
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
    8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 
    35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 
    139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 
    40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
    135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
    5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
    223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
    129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
    251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
    49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
    138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
    };

public:
	/**
    * Constructor to initialize a fractal noise summation
    *
    * @param frequency    Frequency of the first octave of noise (default to 1.0)
    * @param amplitude    Amplitude of the first octave of noise (default to 1.0)
    * @param lacunarity   Lacunarity specifies the frequency multiplier between successive octaves (default 2.0).
    * @param persistence  Persistence is the loss of amplitude between successive octaves (usually 1/lacunarity)
    */
    explicit SimplexNoise(float frequency = 1.0f, float amplitude = 1.0f, float lacunarity = 2.0f, float persistence = 0.50f) :
        mFrequency(frequency),
        mAmplitude(amplitude),
        mLacunarity(lacunarity),
        mPersistence(persistence) {
    }    
	
    /** 
    * Function to randomize permutation table based on seed
    * 
    * @param[in]   seed    Unsigned integer seed value
    * @return      Randomizes permutation table
    */
    void ReSeed(unsigned int seed);

    /**
    * 1D / 2D / 3D Perlin simplex noise
    *
    * @param[in] x float coordinate
    * @param[in] y float coordinate
    * @param[in] z float coordinate
    * @return    Noise value in the range[-1; 1], value of 0 on all integer coordinates.
    */
	float Noise(float x);
	float Noise(float x, float y);
	float Noise(float x, float y, float z);

    /**
    * 1D / 2D / 3D Fractal/Fractional Brownian Motion (fBm) summation of Perlin Simplex noise
    *
    * @param[in] octaves   number of fraction of noise to sum
    * @param[in] x         x float coordinate
    * @param[in] y         y float coordinate
    * @param[in] z         z float coordinate
    * @return Noise value in the range[-1; 1], value of 0 on all integer coordinates.
    */
    float Fractal(size_t octaves, float x);
    float Fractal(size_t octaves, float x, float y);
    float Fractal(size_t octaves, float x, float y, float z);

private:
    /**
    * Function to hash an integer using the permutation table
    *
    * @param[in] i Integer value to hash
    * @return    8-bits hashed value
    */
    uint8_t Hash(int32_t i);

    /**
    * Function to compute the largest integer value not greater than the float one
    *
    * @param[in]   fp    float input value
    * @return      largest integer value not greater than fp
    */
    int32_t FastFloor(float fp);

    /**
    * Function to compute gradients-dot-residual vectors (1D)
    *
    * @param[in] hash  hash value
    * @param[in] x     distance to the corner
    * @return    gradient value
    */
    float Grad(int32_t hash, float x);

    /**
    * Function to compute gradients-dot-residual vectors (2D)
    *
    * @param[in] hash  hash value
    * @param[in] x     x coord of the distance to the corner
    * @param[in] y     y coord of the distance to the corner
    * @return    gradient value
    */
    float Grad(int32_t hash, float x, float y);

    /**
    * Function to compute gradients-dot-residual vectors (3D)
    *
    * @param[in] hash  hash value
    * @param[in] x     x coord of the distance to the corner
    * @param[in] y     y coord of the distance to the corner
    * @param[in] z     z coord of the distance to the corner
    * @return    gradient value
    */
    float Grad(int32_t hash, float x, float y, float z);
};
#endif