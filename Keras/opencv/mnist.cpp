#include <opencv2/opencv.hpp>
#include <iostream>

#ifndef _DEBUG
#pragma comment(lib, "opencv_world3411.lib")
#else
#pragma comment(lib, "opencv_world3411d.lib")
#endif

int main()
{
    cv::Mat image = cv::imread("image.png");
    cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

    cv::namedWindow("image");
    cv::imshow("image", image);
    cv::waitKey(1000);

    cv::resize(image, image, cv::Size(28, 28));
    image = 0xff - image;

    cv::dnn::Net net = cv::dnn::readNet("model.pb");
    cv::Mat input = cv::dnn::blobFromImage(image, 1.0/255.0, cv::Size(28, 28));
    net.setInput(input);
    cv::Mat result = net.forward();
    
    std::size_t offset = result.cols;
    float *data = reinterpret_cast<float *>(result.data), max = 0.0;
    for (std::size_t i = 0; i < result.cols; i++) {
        if ((offset >= result.cols) || (data[i] > max)) {
            offset = i;
            max = data[i];
        }
    }

    std::cout << offset << std::endl;

    return EXIT_SUCCESS;
}