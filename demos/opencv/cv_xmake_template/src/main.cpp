#include <iostream>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

int main(int argc, char **argv) {
  cv::Mat image = cv::imread("D:/desktop/demo.jpg");
  // 检查图像是否加载成功
  if (image.empty()) {
    std::cerr << "Error: Image cannot be loaded." << std::endl;
    return -1;
  }

  // 定义缩放因子
  double scaleWidth = 0.5;  // 宽度缩放为原来的一半
  double scaleHeight = 0.5; // 高度缩放为原来的一半

  // 计算新的尺寸
  cv::Size newSize(static_cast<int>(image.cols * scaleWidth),
                   static_cast<int>(image.rows * scaleHeight));

  // 创建目标图像
  cv::Mat resizedImage;

  // 使用resize函数缩放图像
  cv::resize(image, resizedImage, newSize, 0, 0, cv::INTER_LINEAR);

  // 显示原始图像和缩放后的图像
  //   cv::imshow("Original Image", image);
  cv::imshow("Resized Image", resizedImage);
  cv::waitKey(0);
  return 0;
}
