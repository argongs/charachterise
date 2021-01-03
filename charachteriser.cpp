//Base C++ headers
#include <iostream>

//Base OpenCV headers
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

int main (int argc, char** argv )
{
    	//Check image
    	if ( argc != 2 )
    	{
        	std::cout<<"usage: exec_binary <Image_Path>\n";
        	return -1;
    	}
    
    	//Read input image
	Mat image = imread ( argv[1], IMREAD_COLOR );

    	if ( !image.data )
    	{
        	std::cout<<"No main image data found\n";
        	return -1;
    	}

	//Matrix for anded image
	Mat anded_output = Mat::zeros (image.size (), CV_8UC3);
	//Create pattern image
	//Define how the pattern string should look like
	std::string pattern_str ("u");
	int font = FONT_HERSHEY_PLAIN;

	//First get a precise idea of how much area will the string(that'll be used to create the pattern) will take	
	int baseline = 0;
	Size textSize = getTextSize (pattern_str, font, 1.0, 1, &baseline);

	//Create the bottom left point. It'll be used later to place the pattern string accordingly
	Point bottom_left (0, textSize.height);
	//Create a full black image, which has the same dimensions as the source image. This image will be modified into a mask image full of patterns
	Mat pattern = Mat::zeros (image.size(), CV_8UC1);
	
	//Count no. of pattern blocks (basically the area occupied by the pattern) that can be placed horizontally and vertically
	int h_count = image.cols/textSize.width;
	int v_count = image.rows/textSize.height;

	//Place 'h_count x v_count' pattern strings
	int i, j;
	for ( i = 0; i <= v_count; i++ )
	{	
		for ( j = 0; j <= h_count; j++)
		{	
			putText (pattern, pattern_str, bottom_left, font, 1.0, Scalar (255), 1);
			//Increment the 'x' coordinate of the bottom left point location by width of the pattern string in order to allow putText to place the pattern string in the next column
			bottom_left.x += textSize.width;
		}
		//Reset the 'x' coordinate to 0 and increment the 'y' coordinate of the bottom left point location in order to allow putText function to fill the pattern string in the next row
		bottom_left.x = 0;
		bottom_left.y += 5+textSize.height;//We add extra 1 to the y coordinate of the bottom left point to ensure that there's atleast 1 pixel space in b/w. 2 pattern strings vertically and thus they do not 'stick' to each other.
	}

	//Perform the bitwise ANDing operation:
	bitwise_and (image, image, anded_output, pattern);

	//Matrix for storing the mask for region of interest(i.e. the regions where the pattern is present in it's entririty)
	Mat roi_mask = Mat::zeros(anded_output.size(), CV_8UC1);
	//Now create a template using the pattern string
	bottom_left.x = 0;
	bottom_left.y = textSize.height;
	Mat template_img = Mat::zeros (textSize, CV_8UC1);
	putText (template_img, pattern_str, bottom_left, font, 1.0, Scalar (255), 1);

	//Prepare a matrix to store the result of template matching
	int result_cols = image.cols-template_img.cols+1;
	int result_rows = image.rows-template_img.rows+1;
	Mat result = Mat::zeros(result_rows, result_cols, CV_32FC1);
	
	//Convert the anded image result to a grayscale image so that template matching can happen. (Template matching should be done with a grayscale template and a grayscale only)
	Mat gray_image = Mat::zeros (image.rows, image.cols, CV_8UC1);
	cvtColor (anded_output, gray_image, COLOR_BGR2GRAY);
	
	//Now perform template matching onto the grayscale anded image
	matchTemplate (gray_image, template_img, result, TM_SQDIFF_NORMED);
	
	//Now create a mask based on the the regions of minima obtained from the result. This mask can later be anded with the anded output image to obtain the desired image
	int element_count = 0, row = 0, col = 0;
	float threshold = 0.00625; //Anything darker than the thresold is the point of our interest
	////Check if the result matrix has been continuously allocated.
	if (result.isContinuous ())
		element_count = result.rows*result.cols;

	////Continuously allocated matrix can be accessed as single big one dimensional array
	const float* res_ptr = result.ptr<float>(0);//since we have only one channeled image (grayscale image) therefore the first index/channel i.e '0' should be the only one whose elements should be taken out
	for (int i = 0; i < element_count; i++)
	{
		//If the intensity value is higher than the threshold then fill that region with full black in the copy of the anded image and then add a simple dot in that black rectangle (to give a much more complete feeling to the picture)
		if (res_ptr[i] < threshold)
		{
			//There's an important point to notice over here : rows change vertically and columns change horizontally. Hence, columns correspond to 'x' coordinate and rows correspond to 'y' coordinate. And we're accessing the 'result' obtained from template matching in result[row][column] manner. Hence, we need to keep in mind that result[row][column] actually corresponds to the (x = column, y = row) coordinate of the image. This is the reason why we have mentioned Point(col, row) and not Point(row, col) below.
			rectangle (roi_mask, Point (col, row), Point (col+textSize.height, row+textSize.width), Scalar (255), FILLED);
		}
	
		//Update the row, col value depending upon the index of the current element that is being accessed
		col = (col+1)%result.cols;
		if (col == 0)
			row++;
	}

	Mat final_image;
	//Now use the ROI mask to perform bitwise anding with the anded output image so that we can obtain the fine tuned image.
	bitwise_and (anded_output, anded_output, final_image, roi_mask);

    	//Display image
    	namedWindow ("Main Image", WINDOW_NORMAL );
    	imshow ("Main Image", image); 
    	waitKey (0);

	//Display pattern
	namedWindow ("Pattern Image", WINDOW_NORMAL );
    	imshow ("Pattern Image", pattern); 
    	waitKey (0);

	//Display result of bitwise anding
	namedWindow ("Bitwise ANDing result", WINDOW_NORMAL );
	imshow ("Bitwise ANDing result", anded_output);
	waitKey(0);	

	namedWindow ("Template matching", WINDOW_NORMAL);
	imshow ("Template matching", result);
	waitKey(0);

	namedWindow("ROI mask", WINDOW_NORMAL );
	imshow("ROI mask", roi_mask);
	waitKey(0);

	namedWindow("Final result", WINDOW_NORMAL );
	imshow("Final result", final_image);
	waitKey(0);

	//Write the final image to a file
	imwrite("result.png", final_image);
	//TODO
	//1. Create a mask composed of a text string of choice which will be repeated to create a mask image of dimension 'row x col', if 'row x col' is also the dimension for the main image.
	//2. Now perform masking of the main image using the mask created above and the Open CV's bitwise and operation
	//3. The above operation will probably lead to creation of an image which will be filled with the text string we used in step 1. Now note that the resulting image will contain a few cropped out charachters as well. Now we need to get rid of such charachters
	//4.1 Approach 1: Scan along the edges of the images obtained in step 3. If there's an object (pixel with value > 0) detected then initiate flood fill operation to cover that object's entirety with black color. This will ensure that all the cropped out charachters will be removed. The reason why this procedure should work : The objects will share the pixels with the image boundary if they were the ones to be cropped out or affected due to masking.
	//4.2 Approach 2: Use template matching and threshold out those objects which are fully matched. Those regions where matching score is less should be turned to black. 	
    	return 0;
}
