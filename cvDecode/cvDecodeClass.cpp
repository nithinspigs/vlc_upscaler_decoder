#include "cvDecodeClass.hpp"
#include "opencv2/opencv.hpp"
#include <iostream>

void CvDecode::cvDecode(unsigned char *p_buffer, int i_buffer) {

	/* p_buffer contains the encoded image information that came from p_dec -> p_buffer
	 * I need to see link on how to convert this into a cv::Mat without data copying
	 * I will need to deal with memory deallocation on my own
	 * Should I just set p_buffer to decoded data like chatgpt example?
	 * Or I can return decoded data as a char array, which will be fed into p_pic
	 * HOW will decoded data be converted into p_pic for VLC?
	 */

	// std::cout << p_buffer[0] << ", "  << i_buffer << std::endl;
	std::cerr << "cvDecode function" << std::endl;

}
