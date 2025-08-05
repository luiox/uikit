#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

// 全局变量用于存储参数
int lowThreshold = 50;
int maxThreshold = 150;
int blurSize = 5;
int morphSize = 3;
int preprocessMethod = 0; // 0: GaussianBlur, 1: MedianBlur, 2: BilateralFilter
int edgeMethod = 0; // 0: Canny, 1: Sobel, 2: Scharr, 3: Laplacian, 4: Auto-Canny
int cannyRatio = 3;
const int max_lowThreshold = 200;
const int max_maxThreshold = 300;
const int max_blurSize = 20;
const int max_morphSize = 10;
const int max_cannyRatio = 10;

Mat image, gray, processed, edged;

// 自动计算Canny阈值
void autoCanny(Mat& src, Mat& dst, double sigma = 0.33) {
    // 计算图像的中值
    Scalar median = cv::mean(src);
    double v = median.val[0];
    
    // 计算高低阈值
    double lower = std::max(0.0, (1.0 - sigma) * v);
    double upper = std::min(255.0, (1.0 + sigma) * v);
    
    Canny(src, dst, lower, upper);
}

// 回调函数，用于处理滑动条变化
void processImage(int, void*)
{
    // 确保模糊核大小为奇数
    if (blurSize % 2 == 0) blurSize++;
    if (blurSize < 3) blurSize = 3;
    
    if (morphSize < 1) morphSize = 1;
    
    // 根据选择的方法进行预处理
    switch(preprocessMethod) {
        case 0: // 高斯模糊
            GaussianBlur(gray, processed, Size(blurSize, blurSize), 0);
            break;
        case 1: // 中值滤波
            medianBlur(gray, processed, blurSize);
            break;
        case 2: // 双边滤波
            bilateralFilter(gray, processed, blurSize, 80, 80);
            break;
    }
    
    // 应用形态学操作来减少噪声并连接边缘
    if (morphSize > 1) {
        Mat kernel = getStructuringElement(MORPH_RECT, Size(morphSize, morphSize));
        morphologyEx(processed, processed, MORPH_CLOSE, kernel);
    }
    
    // 根据选择的方法进行边缘检测
    switch(edgeMethod) {
        case 0: // Canny
            Canny(processed, edged, lowThreshold, lowThreshold * cannyRatio);
            break;
        case 1: // Sobel
            {
                Mat grad_x, grad_y;
                Sobel(processed, grad_x, CV_16S, 1, 0, 3);
                Sobel(processed, grad_y, CV_16S, 0, 1, 3);
                convertScaleAbs(grad_x, grad_x);
                convertScaleAbs(grad_y, grad_y);
                addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edged);
            }
            break;
        case 2: // Scharr
            {
                Mat grad_x, grad_y;
                Scharr(processed, grad_x, CV_16S, 1, 0);
                Scharr(processed, grad_y, CV_16S, 0, 1);
                convertScaleAbs(grad_x, grad_x);
                convertScaleAbs(grad_y, grad_y);
                addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edged);
            }
            break;
        case 3: // Laplacian
            Laplacian(processed, edged, CV_16S, 3);
            convertScaleAbs(edged, edged);
            break;
        case 4: // Auto-Canny
            autoCanny(processed, edged);
            break;
    }
    
    // 显示结果
    namedWindow("2. after Preprocess", WINDOW_NORMAL);
    imshow("2. after Preprocess", processed);
    namedWindow("3. after Edge Detection", WINDOW_NORMAL);
    imshow("3. after Edge Detection", edged);
    
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
    createTrackbar("Canny Ratio:", "Controls", &cannyRatio, max_cannyRatio, processImage);
    createTrackbar("Blur Size:", "Controls", &blurSize, max_blurSize, processImage);
    createTrackbar("Morph Size:", "Controls", &morphSize, max_morphSize, processImage);
    createTrackbar("Preprocess: 0=Gauss,1=Median,2=Bilateral", "Controls", &preprocessMethod, 2, processImage);
    createTrackbar("Edge Method: 0-4", "Controls", &edgeMethod, 4, processImage);
    
    // 初始化处理
    processImage(0, 0);

    waitKey(0);
    destroyAllWindows();

    return 0;
}