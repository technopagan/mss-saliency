/*
 * ImageLoader.cpp
 *
 *  Created on: Mar 24, 2014
 *      Author: Jed
 */

#include <string.h>
#include <Magick++.h>
#include "SaliencyDetector.h"
#include "ImageLoader.h"

extern CmdOptions opts;

using namespace std;
using namespace Magick;

ImageLoader::ImageLoader()
{
}
ImageLoader::~ImageLoader()
{
}

//===========================================================================
//	loadMagickFile
//
//	Loads the given image filename from disk and pulls pixels into our
//	RGB Colours vector.
//===========================================================================
bool ImageLoader::loadMagickFile(string& fileName, vector<RGBColour>& buffer,
		int& width, int& height)
{

	// Load image from disk to IM
	// ToDo: Add error handling.
	Image srcImage(fileName.c_str());

	// Get image info
	width = srcImage.columns();
	height = srcImage.rows();
	int pixCount = width * height;

	// re-size the storage vector to suit.
	buffer.resize(pixCount);

	// Get access to our raw pixels
	Pixels pixel_cache(srcImage);
	PixelPacket* pixels = pixel_cache.get(0, 0, width, height);

	// Pull the raw pixels into our buffer.
	int i = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			buffer[i].r = char(pixels[width * y + x].red);
			buffer[i].g = char(pixels[width * y + x].green);
			buffer[i].b = char(pixels[width * y + x].blue);
			buffer[i].a = char(pixels[width * y + x].opacity);
			i++;
		}
	}

	return 0;
}
;

//===========================================================================
//	saveMagickFile
//
//	Saves our char vector array of greyscale values to disk as the given
//	image filename.
//===========================================================================
bool ImageLoader::saveMagickFile(string& fileName, vector<char>& buffer,
		int& width, int& height)
{

	// Create our destination image
	Image dstImage(Geometry(width, height), Color("black"));
	dstImage.modifyImage();

	Pixels pixel_cache(dstImage);
	PixelPacket* pixels = pixel_cache.get(0, 0, width, height);

	// Push the raw pixels into our buffer.
	int i = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// Normalise greyscale value to between 0-1.0 and write the pixels.
			float gV = (1.0f / 255.0f) * buffer[i];
			pixels[width * y + x] = ColorRGB(gV, gV, gV);
			i++;
		}
	}

	// Sync pixel buffer and write to disk
	dstImage.syncPixels();

	if (opts.normalize_output == 1)
	{
		// Normalise levels
		dstImage.normalize();
	}

	if ((opts.chunk_salmap == 1 || opts.quantize_chunks == 1) && opts.threshold_bw == 0)
	{
		// Calculate average grey value for given block size.
		int blockSize = 8; // 8x8

		// How many whole blocks width and height do we need to cover the whole image in both axis?
		int blocksWidth = (width / blockSize) + (width % blockSize == 0 ? 0 : 1);
		int blocksHeight = (height / blockSize) + (height % blockSize == 0 ? 0 : 1);

		float downScale = (100.0f/width)* blocksWidth;
		float upScale = 100 * (width/blocksWidth);

		//TODO: Yeah this needs to work with images of any size
		dstImage.scale(Geometry("64x64"));
		dstImage.scale(Geometry("512x512"));

		if (opts.quantize_chunks == 1)
		{
			// Quantize to 4 colour if desired
			dstImage.level(MaxRGB / 8.0f, MaxRGB, 1);

			// Convert to monochrome
			dstImage.quantizeColorSpace(GRAYColorspace);
			dstImage.quantizeColors(4);
			dstImage.quantizeDither(false);
			dstImage.quantize();
		}
	}

	if (opts.threshold_bw == 1)
	{
		//const double kernel[] = { 0, 1, 0, 1, 2, 1, 0, 1, 0 };
		//dstImage.convolve( 3, kernel );

		// Filter out black darker than 12.5% and boost white >75%
		dstImage.level(MaxRGB / 8.0f, MaxRGB / 4.0f, 1);

		// Convert to monochrome
		dstImage.quantizeColorSpace(GRAYColorspace);
		dstImage.quantizeColors(2);
		dstImage.quantizeDither(false);
		dstImage.quantize();
	}

	dstImage.write(fileName.c_str());

	return 0;
}
