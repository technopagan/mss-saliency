/*
 * ImageLoader.h
 *
 *  Created on: Mar 24, 2014
 *      Author: root
 */

#ifndef IMAGELOADER_H_
#define IMAGELOADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SaliencyDetector.h"
#include "SymmetricSurroundSaliency.h"

using namespace std;

class ImageLoader
{
	public:
		ImageLoader();
		virtual 		~ImageLoader();

		static bool		loadMagickFile(
							string& 				fileName,
							vector<RGBColour>&		buffer,
							int&					width,
							int&					height );

		static bool		saveMagickFile(
							string& 				fileName,
							vector<char>&			buffer,
							int&					width,
							int&					height );
};

#endif /* IMAGELOADER_H_ */
