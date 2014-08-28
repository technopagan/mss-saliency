//============================================================================
// Name        : SaliencyDetector.cpp
// Author      : Neil Jedrzejewski
// Version     :
// Copyright   :
// Description :
//============================================================================


#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <getopt.h>
#include <sys/time.h>
#include <Magick++.h>
#include "SaliencyDetector.h"
#include "SymmetricSurroundSaliency.h"
#include "ImageLoader.h"

using namespace std;
using namespace Magick;

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

// Global stuff like command line options.
//

#define VERSIO "0.2 alpha"

typedef unsigned long long timestamp_t;

static timestamp_t get_timestamp ()
{
    struct timeval now;
    gettimeofday (&now, NULL);
    return  now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

CmdOptions opts;

struct option long_options[] =
{
		// These set a flag
		{ "normalize",	no_argument, &opts.normalize_output, 1 },
		{ "threshold",	no_argument, &opts.threshold_bw, 1 },
		{ "blocks", 	no_argument, &opts.chunk_salmap, 1 },
		{ "qblocks", 	no_argument, &opts.quantize_chunks, 1 },
		// These dont.
		{ "verbose",	no_argument, 0, 'v' },
		{ "help",		no_argument, 0, 'h' },
		{ "quiet",		no_argument, 0, 'q' },
		{ "version",	no_argument, 0, 'V' },
		{ "lthresh",	required_argument, 0,'L'},
		{ "uthresh",	required_argument, 0,'U'},
		{0,0,0,0}
};

// Print useage details
void p_usage(void)
{
	if (!opts.quiet_mode) {
		fprintf(stderr,"SaliencyDetector v" VERSIO "  - Copyright (c) Neil 'Jed' Jedrzejewski/Tobias Baldauf 2014\n");

		fprintf(stderr,
	       "Usage: SaliencyDetector [options] <input file> <output file>\n\n"
			"  -h, --help        display this help and exit\n"
			"  -q, --quiet       quiet mode\n"
			"  -v, --verbose     enable verbose mode\n"
			"  -V, --version     print program version\n\n"
			"  --normalize       normalize greyscale output\n\n"
			"  --threshold       convert saliency map to a two colour image\n"
			"  -L<num>           lower threshold (def. 12.5 %%) for black cut-off for two colour conversion.\n"
			"  -U<num>           upper threshold (def. 75%%) for white cut-off for two colour conversion\n"
            "                    Note: setting either of the above two implies --threshold\n\n"
			"  --blocks          convert saliency map to series of averaged blocks\n"
			"  --qblocks         convert saliency map to series of averaged blocks quantized to 4 colours.\n"
			"\n\n");
	}
	exit(1);
}

// Print banner
void p_banner(void)
{
    printf("SaliencyDetector v%s\n", VERSIO);
    printf("Copyright (c)2014 Neil 'Jed' Jedrzejewski/Tobias Baldauf\n\n");
    return;
}

// And so out story begins...
//
int main(int argc, char **argv)
{

	// Process command line.
	//
	if (argc < 2)
	{
		if (!opts.quiet_mode) fprintf( stderr,"SaliencyDetector: file arguments missing\n"
				"Try 'SaliencyDetector --help' for more information.\n");
		exit(1);
	}

	int ch = 0;
	int opt_index = 0;

	while(1)
	{
		opt_index = 0;

	    if ( (ch = getopt_long(argc, argv, "vhqVU:L:", long_options, &opt_index) ) == -1)
	      break;

	    switch (ch)
	    {
			case 'v':
				opts.verbose_mode++; // Every extra v adds a level of verbosity.
				break;

			case 'h':			// Help me!
		    case '?':
				p_usage();
				break;

			case 'q':			// Shhhh!
				opts.quiet_mode = 1;
				break;

		    case 'V':			// Rawr!
		    	p_banner();
		    	exit(0);
		    	break;

		    case 'L':
		    {
		    	float tmpvar;
		        if (sscanf(optarg,"%f", &tmpvar) == 1)
	            {
	            	opts.threshold_low = tmpvar;
	            	if (opts.threshold_low < 1) opts.threshold_low = 1.0f;
	            	if (opts.threshold_low > 99) opts.threshold_low = 99.0f;
	            	opts.threshold_bw = 1;
	            }
	            else
	            {
	            	fprintf(stderr, "invalid argument for -L, --lthresh\n");
		        	exit(1);
	            }
	    	}
	    		break;

		    case 'U':
		    {
		    	float tmpvar;
				if (sscanf(optarg,"%f", &tmpvar) == 1)
				{
					opts.threshold_high = tmpvar;
				    if (opts.threshold_high < 0) opts.threshold_high = 0.0f;
				    if (opts.threshold_high > 100) opts.threshold_high = 100.0f;
				    opts.threshold_bw = 1;
				}
				else
				{
				   	fprintf(stderr, "invalid argument for -U, --uthresh\n");
					exit(1);
				}
			}
				break;

	    }
	}

	// loop through the parameters
	int i=1;
	bool gotInFile = false;
	bool gotOutFile = false;

	do
	{
		/// Skip empty values or anything starting with a hypen.
		if (!argv[i][0]) continue;
		if (argv[i][0]=='-') continue;

		// first arg we hit after the options should be the input file.
		if (gotInFile == false)
		{
			opts.inFile = argv[i];	// Grab it
			gotInFile = true;		// Flag it
			continue;				// Moving on...
		}

		// Ditto for output file
		if (gotOutFile == false)
		{
			opts.outFile = argv[i];
			gotOutFile = true;
			continue;
		}
	} while (++i < argc);

	// Dump some quick verbose info.
	if (opts.verbose_mode)
	{
		p_banner();
		printf("Input file: %s\nOutput file: %s\n", opts.inFile.c_str(), opts.outFile.c_str() );
		printf("Normalize image: %s\n", opts.normalize_output ? "yes" : "no"  );
		printf("Threshold: %s\n", opts.threshold_bw ? "yes" : "no"  );
		if (opts.threshold_bw)
		{
			printf("Lower threshold: %g%%\n", opts.threshold_low);
			printf("Upper threshold: %g%%\n", opts.threshold_high);
		}
		printf("Chunk saliency map: %s\n", opts.chunk_salmap ? "yes" : "no"  );
		printf("Quantize chunks: %s\n", opts.quantize_chunks ? "yes" : "no"  );
		printf("\n");
	};

	// Check we can open the source file.
	if (access(opts.inFile.c_str(), F_OK) == -1)
	{
		fprintf( stderr,"SaliencyDetector: Cannot open input file %s\n", opts.inFile.c_str() );
		exit(1);
	}

	// Tick...
	timestamp_t t0 = get_timestamp();

	// Initialise ImageMagick++
	InitializeMagick(*argv);

	// RGB Vector buffer to hold our source image.
	vector<RGBColour> img(0);
	int width(0);
	int height(0);

	// Load the image into the buffer for processing.
	ImageLoader::loadMagickFile(opts.inFile, img, width, height);

	// Process Saliency Map
	vector<double> salmap(0);

	SymmetricSurroundSaliency sal;
	//true normalizes the saliency map
	sal.ComputeMaximumSymmetricSurroundSaliency(img, width, height, salmap, true);

	// Create a new buffer for the greyscale output image.
	int sz = height * width;
	vector<char> outimg(sz);

	for (int i = 0; i < sz; i++)
	{
		// Round up and put into the buffer array
		outimg[i] = char(salmap[i] + 0.5);
	}

	// Save the resulting image to disk.
	ImageLoader::saveMagickFile(opts.outFile, outimg, width, height);

	// Tock...
	timestamp_t t1 = get_timestamp();

	// Print execution time
	if (!opts.quiet_mode)
	{
			printf("Done!\nExecution time: %.3Lf secs\n\n", (t1 - t0) / 1000000.0L );
	}

	return 0;
}
