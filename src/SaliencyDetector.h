/*
 * CustomTypes.h
 *
 *  Created on: Mar 24, 2014
 *      Author: root
 */

#ifndef CUSTOMTYPES_H_
#define CUSTOMTYPES_H_

#include <string>

// RGBA Colour structure.
struct RGBColour
{
    unsigned char r, b, g, a;
};

// Command line options
struct CmdOptions
{
	CmdOptions(): quiet_mode(0),verbose_mode(0),normalize_output(0),
				threshold_bw(0),threshold_low(12.5f), threshold_high(75.0f),
				chunk_salmap(0),quantize_chunks(0){}

	int 	quiet_mode;				// Shhh!
	int 	verbose_mode;			// Rawr!
	int 	normalize_output;		// Normalize greyscale values in the saliency map.
	int 	threshold_bw;			// Convert to a 2 colour threshold map.
	float	threshold_low;			// Black threshold cut-off (percent)
	float	threshold_high;			// White threshold cut-off (percent)
	int 	chunk_salmap;			// Break the saliency map down into averaged blocks.
	int 	quantize_chunks;		// Quantize the blocks to 4 levels

	std::string inFile;
	std::string outFile;
};


#endif /* CUSTOMTYPES_H_ */
