#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
    // 读取图像
    Mat image = imread("D:/desktop/rect_demo.jpg");
    if (image.empty()) {
        cerr << "无法加载图像！" << endl;
        return -1;
    }

    Mat gray, blurred, edged;

    // 转为灰度图
    cvtColor(image, gray, COLOR_BGR2GRAY);
    namedWindow("1. after gray", WINDOW_NORMAL);
    imshow("1. after gray", gray);

    // 高斯模糊去噪
    GaussianBlur(gray, blurred, Size(5, 5), 0);
    namedWindow("2. after GaussianBlur", WINDOW_NORMAL);
    imshow("2. after GaussianBlur", blurred);

    // 边缘检测
    Canny(blurred, edged, 50, 150);
    namedWindow("3. after Canny", WINDOW_NORMAL);
    imshow("3. after Canny", edged);

    // 寻找轮廓
    vector<vector<Point>> contours;
    findContours(edged, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Point> outerRect, innerRect;
    double        maxArea = 0, minArea = 1e6;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < 500)
            continue;   // 过滤小噪声

        vector<Point> approx;
        double        peri = arcLength(cnt, true);
        approxPolyDP(cnt, approx, 0.02 * peri, true);

        if (approx.size() == 4) {   // 判断是否为四边形
            if (area > maxArea) {
                maxArea   = area;
                outerRect = approx;
            }
            if (area < minArea) {
                minArea   = area;
                innerRect = approx;
            }
        }
    }

    // 绘制结果
    if (!outerRect.empty()) {
        polylines(image, outerRect, true, Scalar(0, 255, 0), 3);   // 绿色为外框
    }
    if (!innerRect.empty()) {
        polylines(image, innerRect, true, Scalar(0, 0, 255), 3);   // 红色为内框
    }

    // 显示结果
    namedWindow("Detected Rectangles", WINDOW_NORMAL);
    imshow("Detected Rectangles", image);

    waitKey(0);
    destroyAllWindows();

    return 0;
}
