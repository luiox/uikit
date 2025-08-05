#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

// 全局变量用于存储参数
int lowThreshold = 50;
int maxThreshold = 150;
int blurSize = 5;
const int max_lowThreshold = 200;
const int max_maxThreshold = 300;
const int max_blurSize = 20;

Mat image, gray, blurred, edged;

// 回调函数，用于处理滑动条变化
void processImage(int, void*)
{
    // 确保模糊核大小为奇数
    if (blurSize % 2 == 0) blurSize++;
    if (blurSize < 3) blurSize = 3;
    
    // 应用高斯模糊
    GaussianBlur(gray, blurred, Size(blurSize, blurSize), 0);
    
    // Canny边缘检测
    Canny(blurred, edged, lowThreshold, maxThreshold);
    
    // 显示结果
    namedWindow("2. after GaussianBlur", WINDOW_NORMAL);
    imshow("2. after GaussianBlur", blurred);
    namedWindow("3. after Canny", WINDOW_NORMAL);
    imshow("3. after Canny", edged);
    
    // 寻找轮廓
    vector<vector<Point>> contours;
    findContours(edged, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> outerRect, innerRect;
    double maxArea = 0, minArea = 1e6;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < 500)
            continue;   // 过滤小噪声

        vector<Point> approx;
        double peri = arcLength(cnt, true);
        // 提高轮廓逼近精度
        approxPolyDP(cnt, approx, 0.015 * peri, true);

        if (approx.size() == 4) {   // 判断是否为四边形
            if (area > maxArea) {
                maxArea = area;
                outerRect = approx;
            }
            if (area < minArea) {
                minArea = area;
                innerRect = approx;
            }
        }
    }

    // 绘制结果
    Mat resultImage = image.clone();
    if (!outerRect.empty()) {
        polylines(resultImage, outerRect, true, Scalar(0, 255, 0), 3);   // 绿色为外框
    }
    if (!innerRect.empty()) {
        polylines(resultImage, innerRect, true, Scalar(0, 0, 255), 3);   // 红色为内框
    }

    // 显示结果
    namedWindow("Detected Rectangles", WINDOW_NORMAL);
    imshow("Detected Rectangles", resultImage);
}

int main()
{
    // 读取图像
    image = imread("D:/desktop/rect_demo.jpg");
    if (image.empty()) {
        cerr << "无法加载图像！" << endl;
        return -1;
    }

    // 转为灰度图
    cvtColor(image, gray, COLOR_BGR2GRAY);
    namedWindow("1. after gray", WINDOW_NORMAL);
    imshow("1. after gray", gray);

    // 创建控制窗口
    namedWindow("Controls", WINDOW_NORMAL);
    
    // 创建滑动条
    createTrackbar("Low Threshold:", "Controls", &lowThreshold, max_lowThreshold, processImage);
    createTrackbar("Max Threshold:", "Controls", &maxThreshold, max_maxThreshold, processImage);
    createTrackbar("Blur Size:", "Controls", &blurSize, max_blurSize, processImage);
    
    // 初始化处理
    processImage(0, 0);

    waitKey(0);
    destroyAllWindows();

    return 0;
}
