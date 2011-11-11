
#include "exr_test.h"

void normalize(float * array, int size) {
	float sum = 0;
	for(int i = 0; i < size; i++) {
		sum += array[i];
	}
	for(int i = 0; i < size; i++) {
		array[i] = array[i] / sum;
	}
}

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


float gauss(float i, float sigma) {
	return (1/sqrt(2*pi*(pow(sigma,2)))) *
						pow(e,(-(pow(i,2))/(2*(pow(sigma,2)))));
}

Array2D<Rgba> * gaussian_blur(float sigma, int width, int height, const Array2D<Rgba> &image) {


	Array2D<Rgba> firstPass(height, width);
	firstPass.resizeErase(height, width);
	//Six sigmas until things trail off, so three sigmas in each direction
	float maxRadius = 3 * sigma;
	int kernelRadius = ceil(maxRadius);
	int kernelWidth = (1 + 2*kernelRadius);
	float * kernel = new float[kernelWidth];

	//This kernel can be used in both directions
	for(int i = -kernelRadius; i <= kernelRadius; i++) {
			//for sum, and also helps debugging massively
			kernel[i + kernelRadius] = gauss(i, sigma);
	}

	normalize(kernel, kernelWidth);

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

/* None of these refs are constant, all will change unless scale is 1 */
Array2D<Rgba> * scaleImage(const Array2D<Rgba> & image, float scale, int & width, int & height, Header & h) {

	/* The data is a little unweildy */
	int newWidth = round(scale * width);
	int newHeight = round(scale * height);
	Array2D<Rgba> firstPass(height, newWidth);
	firstPass.resizeErase(height, newWidth);
	Array2D<Rgba> * secondPass = new Array2D<Rgba>(newHeight, newWidth);
	secondPass->resizeErase(newHeight, newWidth);

	/*
	 * I'm going to use the Gaussian filter for resampling, with a sigma of 0.5
	 * scaled to the proper values
	 */
	float sigma = 0.5 / scale;
	float maxRadius = 3 * sigma;
	int kernelRadius = ceil(maxRadius);
	int kernelWidth = (1 + 2*kernelRadius);
	float * kernel = new float[kernelWidth];

	//Let's do X first
	for(int y = 0; y < height; y++) {
		for(int newX = 0; newX < newWidth; newX++) {
			half * rValue = new half(0);
			half * gValue = new half(0);
			half * bValue = new half(0);
			half * aValue = new half(0);
			for(int i = 0; i < kernelWidth; i++) {
				float val = (float)newX/scale - kernelRadius + i;
				kernel[i] = gauss(val,sigma);
			}
			normalize(kernel,kernelWidth);
			for(int i = 0; i < kernelWidth; i++) {
				float val = (float)newX/scale - kernelRadius + i;
				float weight = kernel[i];
				int xIndex = CLAMP(round(val), 0, (width - 1));
				Rgba pixel = image[y][xIndex];
				*rValue += pixel.r * weight;
				*gValue += pixel.g * weight;
				*bValue += pixel.g * weight;
				*aValue += pixel.a * weight;
			}
			firstPass[y][newX] = *(new Rgba(*rValue, *gValue, *bValue, *aValue));
		}
	}


	//Now let's do the Y's.
	for(int newX = 0; newX < newWidth; newX++) {
		for(int newY = 0; newY < newHeight; newY++) {
			half * rValue = new half(0);
			half * gValue = new half(0);
			half * bValue = new half(0);
			half * aValue = new half(0);
			for(int i = 0; i < kernelWidth; i++) {
				float val = newY / scale - kernelRadius + i;
				kernel[i] = gauss(val,sigma);
			}
			normalize(kernel,kernelWidth);
			for(int i = 0; i < kernelWidth; i++) {
				float val = newY / scale - kernelRadius + i;
				int yIndex = CLAMP(round(val), 0, (height-1));
				Rgba pixel = firstPass[yIndex][newX];
				*rValue += pixel.r * kernel[i];
				*gValue += pixel.g * kernel[i];
				*bValue += pixel.b * kernel[i];
				*aValue += pixel.a * kernel[i];
			}
			(*secondPass)[newY][newX] = *(new Rgba(*rValue, *gValue, *bValue, *aValue));
		}
	}

	/* Don't forget all the other info that has to be updated before we can write */
	height = newHeight;
	width = newWidth;
	//h.setMaxImageSize(newWidth,newHeight);
	return secondPass;
}

int main (int argc, char *argv[])
{
    try
    {

    	char * inputFile = argv[1];
    	char * outputFile = argv[2];

        int width = 0;
        int height = 0;
        Array2D<Rgba> p;
        RgbaInputFile * input = readRgba (inputFile, p, width, height);
        Header h = input->header();


        if((argc > 4) && (0 == (strcmp(argv[3],"-b")))) {
        	float radius;
        	sscanf(argv[4], "%f", &radius);
        	Array2D<Rgba> * blurred = gaussian_blur(radius, width, height, p);
        	writeRgba(outputFile, &((*blurred)[0][0]), width, height, h);
        	delete(blurred);
        }

        else if((argc > 4) && (0 == strcmp(argv[3],"-s"))) {
        	float scale;
        	sscanf(argv[4], "%f", &scale);
        	/* Width and height will be changed by function */
        	Array2D<Rgba> * scaled = scaleImage(p, scale, width, height, h);
        	//@todo: change the header to fit the width and height infos
        	writeRgba(outputFile, &((*scaled)[0][0]), width, height, h);
        	delete(scaled);
        }

        delete(input);
    }

    catch (const std::exception &exc) {
        std::cerr << exc.what() << std::endl;
        return 1;
    }

    return 0;
}




