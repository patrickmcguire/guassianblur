
#include "exr_test.h"

void
writeRgba (const char * fileName,
           const Rgba *pixels,
           int width,
           int height,
           Header & h)
{
    //
    // Write an RGBA image using class RgbaOutputFile.
    //
    //	- open the file
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //

	RgbaOutputFile file(fileName, h, WRITE_RGBA, 1);
	file.setFrameBuffer(pixels, 1, width);
    file.writePixels (height);
}

RgbaInputFile *
readRgba (const char fileName[],
          Array2D<Rgba> &pixels,
          int width,
          int height)
{
    //
    // Read an RGBA image using class RgbaInputFile:
    //
    //	- open the file
    //	- allocate memory for the pixels
    //	- describe the memory layout of the pixels
    //	- read the pixels from the file
    //

	RgbaInputFile * file = new RgbaInputFile(fileName,1);
    Box2i dw = file->dataWindow();
    
    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    pixels.resizeErase (height, width);
    file->setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file->readPixels (dw.min.y, dw.max.y);
    return file;
}


void guassian_blur(float sigma, int width, int height, const Array2D<Rgba> &image, Array2D<Rgba> &newImage) {

	//Six sigmas until things trail off, so three sigmas in each direction
	float maxRadius = 3 * sigma;
	int kernelRadius = ceil(maxRadius);
	int kernelWidth = (1 + 2*kernelRadius);
	float * kernel = new float[kernelWidth^2];
	float sum = 0;
	for(int i = -kernelRadius; i <= kernelRadius; i++) {
		for(int j = -kernelRadius; j <= kernelRadius; j++) {
			//for sum, and also helps debugging massively
			float val = (1/(2*pi*(sigma^2))) *
					e^(-(i^2 + j^2)/(2*(sigma^2)));
			kernel[(i + kernelRadius)*kernelWidth + (j + kernelRadius)] = val;
			sum += val;
		}
	}

	//normalize
	for(int i = 0; i < kernelWidth; i++) {
		for(int j = 0; j < kernelWidth; j++) {
			kernel[i*kernelWidth+j] = kernel[i*kernelWidth+j] / sum;
		}
	}

	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
		}
	}

	//Take advantage of the fact that it's linearly seperable, do lines first, then
}

int main (int argc, char *argv[])
{
    try
    {
    	/* Will be changed at a later point */
        int width = 0;
        int height = 0;
        Array2D<Rgba> p;
        RgbaInputFile * input = readRgba (argv[1], p, width, height);
        /*
         * Tomfoolery and shennanigans go here
         */


        //copy the image over first....
        Array2D<Rgba> copy(width,height);

        float radius = 0;
        //at three sigma can stop caring
        string two(argv[2]);
        if((0 == (strcmp(argv[2],"-b"))) && (argc > 3)){
        	radius = sscanf(argv[3], "%f", &radius);
        }



        Header h = input->header();
        writeRgba ("allred.exr", &p[0][0], width, height, h);
        delete(input);
    }

    catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}




