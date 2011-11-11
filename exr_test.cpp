
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
          int & width,
          int & height)
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


Array2D<Rgba> * guassian_blur(float sigma, int width, int height, const Array2D<Rgba> &image) {


	Array2D<Rgba> firstPass(height, width);
	firstPass.resizeErase(height, width);
	//Six sigmas until things trail off, so three sigmas in each direction
	float maxRadius = 3 * sigma;
	int kernelRadius = ceil(maxRadius);
	int kernelWidth = (1 + 2*kernelRadius);
	float * kernel = new float[kernelWidth];
	float sum = 0;

	//This kernel can be used in both directions
	for(int i = -kernelRadius; i <= kernelRadius; i++) {
			//for sum, and also helps debugging massively
			float val = (1/sqrt(2*pi*(pow(sigma,2)))) *
					pow(e,(-(pow(i,2))/(2*(pow(sigma,2)))));
			kernel[i + kernelRadius] = val;
			sum += val;
	}

	//normalize
	for(int i = 0; i < kernelWidth; i++) {
		kernel[i] = kernel[i] / sum;
	}

	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			half * rValue = new half(0);
			half * gValue = new half(0);
			half * bValue = new half(0);
			half * aValue = new half(0);
			for(int i = 0; i < kernelWidth; i++) {
				int xIndex = CLAMP(x-kernelRadius + i, 0, width-1);
				float weight = kernel[i];
				Rgba pixel = image[y][xIndex];
				*rValue += pixel.r * weight;
				*gValue += pixel.g * weight;
				*bValue += pixel.b * weight;
				*aValue += pixel.a * weight;
			}
			firstPass[y][x] = *(new Rgba(*rValue, *gValue, *bValue, *aValue));
		}
	}


	Array2D<Rgba> * secondPass = new Array2D<Rgba>(height,width);
	secondPass->resizeErase(height, width);
	//Yay, now do the Y's! Isn't this fun?
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			half rValue = 0;
			half gValue = 0;
			half bValue = 0;
			half aValue = 0;
			for(int i = 0; i < kernelWidth; i++) {
				int yIndex = CLAMP(y-kernelRadius + i, 0, height-1);
				float weight = kernel[i];
				Rgba pixel = firstPass[yIndex][x];
				rValue += pixel.r * weight;
				gValue += pixel.g * weight;
				bValue += pixel.b * weight;
				aValue += pixel.a * weight;
			}
			(*secondPass)[y][x] = *(new Rgba(rValue, gValue, bValue, aValue));
		}

	}

	delete(kernel);
	return secondPass;
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
        Header h = input->header();

        float radius = 0;
        //at three sigma can stop caring
        if((argc > 3) && (0 == (strcmp(argv[2],"-b")))) {
        	radius = sscanf(argv[3], "%f", &radius);
        	Array2D<Rgba> * blurred = guassian_blur(radius, width, height, p);
        	string prefixed = "blurred_";
        	string filename(argv[1]);
        	string outputFileName = prefixed + filename;
        	writeRgba(outputFileName.c_str(), &((*blurred)[0][0]), width, height, h);
        }

        delete(input);
    }

    catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}




