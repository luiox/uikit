#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

// 全局变量用于存储参数
int       lowThreshold     = 50;
int       maxThreshold     = 150;
int       blurSize         = 5;
int       morphSize        = 3;
int       preprocessMethod = 0;   // 0: GaussianBlur, 1: MedianBlur, 2: BilateralFilter
int       edgeMethod       = 0;   // 0: Canny, 1: Sobel, 2: Scharr, 3: Laplacian, 4: Auto-Canny
int       cannyRatio       = 3;
int       minRectArea      = 1000;   // 新增：矩形最小面积阈值
const int max_lowThreshold = 200;
const int max_maxThreshold = 300;
const int max_blurSize     = 20;
const int max_morphSize    = 10;
const int max_cannyRatio   = 10;

Mat image, gray, processed, edged;

// 自动计算Canny阈值
void autoCanny(Mat& src, Mat& dst, double sigma = 0.33)
{
    // 计算图像的中值
    Scalar median = cv::mean(src);
    double v      = median.val[0];

    // 计算高低阈值
    double lower = std::max(0.0, (1.0 - sigma) * v);
    double upper = std::min(255.0, (1.0 + sigma) * v);

    Canny(src, dst, lower, upper);
}

// 回调函数，用于处理滑动条变化
void processImage(int, void*)
{
    // 确保模糊核大小为奇数
    if (blurSize % 2 == 0)
        blurSize++;
    if (blurSize < 3)
        blurSize = 3;

    if (morphSize < 1)
        morphSize = 1;

    // 根据选择的方法进行预处理
    switch (preprocessMethod) {
    case 0:   // 高斯模糊
        GaussianBlur(gray, processed, Size(blurSize, blurSize), 0);
        break;
    case 1:   // 中值滤波
        medianBlur(gray, processed, blurSize);
        break;
    case 2:   // 双边滤波
        bilateralFilter(gray, processed, blurSize, 80, 80);
        break;
    }

    // 应用形态学操作来减少噪声并连接边缘
    if (morphSize > 1) {
        Mat kernel = getStructuringElement(MORPH_RECT, Size(morphSize, morphSize));
        morphologyEx(processed, processed, MORPH_CLOSE, kernel);
    }

    // 根据选择的方法进行边缘检测
    switch (edgeMethod) {
    case 0:   // Canny
        Canny(processed, edged, lowThreshold, lowThreshold * cannyRatio);
        break;
    case 1:   // Sobel
    {
        Mat grad_x, grad_y;
        Sobel(processed, grad_x, CV_16S, 1, 0, 3);
        Sobel(processed, grad_y, CV_16S, 0, 1, 3);
        convertScaleAbs(grad_x, grad_x);
        convertScaleAbs(grad_y, grad_y);
        addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edged);
    } break;
    case 2:   // Scharr
    {
        Mat grad_x, grad_y;
        Scharr(processed, grad_x, CV_16S, 1, 0);
        Scharr(processed, grad_y, CV_16S, 0, 1);
        convertScaleAbs(grad_x, grad_x);
        convertScaleAbs(grad_y, grad_y);
        addWeighted(grad_x, 0.5, grad_y, 0.5, 0, edged);
    } break;
    case 3:   // Laplacian
        Laplacian(processed, edged, CV_16S, 3);
        convertScaleAbs(edged, edged);
        break;
    case 4:   // Auto-Canny
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
    double        maxArea = 0, minArea = 1e6;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < 500)
            continue;   // 过滤小噪声

        vector<Point> approx;
        double        peri = arcLength(cnt, true);
        // 提高轮廓逼近精度
        approxPolyDP(cnt, approx, 0.02 * peri, true);

        // 轮廓筛选条件增强
        if (approx.size() == 4) {   // 判断是否为四边形
            // 计算轮廓的边界框
            Rect   bbox        = boundingRect(approx);
            double aspectRatio = (double)bbox.width / bbox.height;

            // 检查长宽比是否在合理范围内（这里假设合理的长宽比范围为0.5到2.0）
            if (aspectRatio > 0.5 && aspectRatio < 2.0) {
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

int method1()
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
    createTrackbar(
        "Preprocess: 0=Gauss,1=Median,2=Bilateral", "Controls", &preprocessMethod, 2, processImage);
    createTrackbar("Edge Method: 0-4", "Controls", &edgeMethod, 4, processImage);
    createTrackbar(
        "Min Rect Area:", "Controls", &minRectArea, 5000, processImage);   // 添加最小矩形面积控制

    // 初始化处理
    processImage(0, 0);

    waitKey(0);
    destroyAllWindows();

    return 0;
}

void method2()
{
    // 转为灰度图
    cvtColor(image, gray, COLOR_BGR2GRAY);

    // 1. 图像预处理：灰度 + 反转二值化（因为目标是黑色）
    Mat gray, binary;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    threshold(gray, binary, 120, 255, THRESH_BINARY_INV | THRESH_OTSU);

    // 2. 闭运算消除孔洞（核大小增大）
    Mat morphKernel = getStructuringElement(MORPH_RECT, Size(50, 50));
    morphologyEx(binary, binary, MORPH_CLOSE, morphKernel);

    // 3. 查找并筛选矩形轮廓
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<vector<Point>> rectContours;
    vector<double>        rectAreas;

    for (const auto& cnt : contours) {
        double area = contourArea(cnt);
        if (area < 500)
            continue;   // 过滤小噪声

        vector<Point> approx;
        double        peri = arcLength(cnt, true);
        approxPolyDP(cnt, approx, 0.02 * peri, true);

        // 允许 4 或 5 个顶点，并检查凸性
        if ((approx.size() >= 4 && approx.size() <= 5) && isContourConvex(approx)) {
            Rect   bbox        = boundingRect(approx);
            double aspectRatio = (double)bbox.width / bbox.height;

            if (aspectRatio > 0.2 && aspectRatio < 5.0) {
                rectContours.push_back(approx);
                rectAreas.push_back(area);
            }
        }
    }

    // 4. 如果检测到多个矩形，找出面积最大的（假设是外框）
    vector<vector<Point>> finalRectContours;
    if (!rectContours.empty()) {
        auto maxIt    = max_element(rectAreas.begin(), rectAreas.end());
        int  maxIndex = distance(rectAreas.begin(), maxIt);
        finalRectContours.push_back(rectContours[maxIndex]);
    }

    // 5. 解算中心点
    vector<Point2f> centers;
    for (const auto& rect : finalRectContours) {
        Moments M = moments(rect);
        if (M.m00 != 0) {
            Point2f center(M.m10 / M.m00, M.m01 / M.m00);
            centers.push_back(center);
        }
    }

    // 6. 结果绘制
    Mat resultImage = image.clone();
    for (size_t i = 0; i < finalRectContours.size(); i++) {
        polylines(resultImage, finalRectContours[i], true, Scalar(0, 255, 0), 2);
    }
    for (const auto& center : centers) {
        circle(resultImage, center, 5, Scalar(255, 0, 0), -1);
    }

    // 显示中间结果
    namedWindow("1. Original Image", WINDOW_NORMAL);
    imshow("1. Original Image", image);
    namedWindow("2. Binary Image", WINDOW_NORMAL);
    imshow("2. Binary Image", binary);

    // 显示最终结果
    namedWindow("Method2 Result", WINDOW_NORMAL);
    imshow("Method2 Result", resultImage);
}


void detectBlackTapeRectangle(Mat& image)
{
    // 1. 转为灰度图
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);

    // 2. 反转二值化（目标黑色变白色）
    Mat binary;
    threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    namedWindow("threshold", WINDOW_NORMAL);
    imshow("threshold", binary);

    // 3. 形态学闭运算（连接断裂区域）
    Mat morphKernel = getStructuringElement(MORPH_RECT, Size(50, 50));
    morphologyEx(binary, binary, MORPH_CLOSE, morphKernel);
    namedWindow("morphologyEx", WINDOW_NORMAL);
    imshow("morphologyEx", binary);

    // 4. 查找轮廓（带层次结构）
    vector<vector<Point>> contours;
    vector<Vec4i>         hierarchy;
    findContours(binary, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    // 5. 遍历轮廓，记录符合条件的内外框
    int    maxOuterIndex = -1;
    int    maxInnerIndex = -1;
    double maxOuterArea  = 0;
    double maxInnerArea  = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area < 100)
            continue;   // 跳过小轮廓

        bool isInner = hierarchy[i][3] != -1;

        // 近似多边形
        vector<Point> approx;
        double        peri = arcLength(contours[i], true);
        approxPolyDP(contours[i], approx, 0.02 * peri, true);

        // 筛选条件：4个顶点 + 凸性
        if (approx.size() == 4 && isContourConvex(approx)) {
            if (isInner) {
                if (area > maxInnerArea) {
                    maxInnerArea  = area;
                    maxInnerIndex = i;
                }
            }
            else {
                if (area > maxOuterArea) {
                    maxOuterArea  = area;
                    maxOuterIndex = i;
                }
            }
        }
    }

    // 6. 只绘制最大的一组内外框
    if (maxOuterIndex != -1) {
        drawContours(image, contours, maxOuterIndex, Scalar(0, 0, 255), 2);   // 红色外框

        // 计算外接矩形
        Rect  boundRect = boundingRect(contours[maxOuterIndex]);
        Point topLeft   = boundRect.tl();                         // 左上角
        Point topRight(topLeft.x + boundRect.width, topLeft.y);   // 右上角

        // 为了让文字不贴边，可以稍微偏移
        Point textPos(topRight.x - 50, topRight.y - 15);   // 向左偏移50，向下偏移20

        // 绘制中心点
        Moments m = moments(contours[maxOuterIndex]);
        Point   center(m.m10 / m.m00, m.m01 / m.m00);
        circle(image, center, 5, Scalar(255, 0, 0), -1);

        // 在右上角绘制文字
        putText(image, "Outer", textPos, FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
    }

    if (maxInnerIndex != -1) {
        drawContours(image, contours, maxInnerIndex, Scalar(0, 255, 0), 2);   // 绿色内框

        // 计算外接矩形
        Rect  boundRect = boundingRect(contours[maxInnerIndex]);
        Point topLeft   = boundRect.tl();                         // 左上角
        Point topRight(topLeft.x + boundRect.width, topLeft.y);   // 右上角

        // 为了让文字不贴边，可以稍微偏移
        Point textPos(topRight.x - 50, topRight.y + 20);   // 向左偏移50，向下偏移20

        // 绘制中心点
        Moments m = moments(contours[maxInnerIndex]);
        Point   center(m.m10 / m.m00, m.m01 / m.m00);
        circle(image, center, 5, Scalar(255, 0, 0), -1);

        // 在右上角绘制文字
        putText(image, "Inner", textPos, FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 255), 2);
    }

    // 显示结果
    namedWindow("Result", WINDOW_NORMAL);
    imshow("Result", image);
}

int main()
{
    // 读取图像
    image = imread("D:/desktop/rect_demo.jpg");
    if (image.empty()) {
        cerr << "无法加载图像！" << endl;
        return -1;
    }

    detectBlackTapeRectangle(image);

    waitKey(0);
    destroyAllWindows();

    return 0;
}