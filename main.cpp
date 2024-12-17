// Kanaan Sullivan CS_4420 
// Project 4
// Last edit 11/4/2024

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <fstream>

void computeHistogram(const cv::Mat& image, std::vector<int>& histogram) {
    histogram.assign(256, 0);
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            int intensity = image.at<uchar>(r, c);
            histogram[intensity]++;
        }
    }
}

cv::Mat histogramEqualization(const cv::Mat& image) {
    std::vector<int> histogram;
    computeHistogram(image, histogram);

    // Compute cumulative distribution function (CDF)
    std::vector<float> cdf(256, 0);
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Normalize CDF
    float cdfMin = *std::min_element(cdf.begin(), cdf.end());
    int totalPixels = image.rows * image.cols;
    for (int i = 0; i < 256; i++) {
        cdf[i] = (cdf[i] - cdfMin) / (totalPixels - cdfMin) * 255;
    }

    // Map the original image intensities to equalized intensities
    cv::Mat equalizedImage = image.clone();
    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {
            equalizedImage.at<uchar>(r, c) = static_cast<uchar>(cdf[image.at<uchar>(r, c)]);
        }
    }
    return equalizedImage;
}

cv::Mat histogramMatching(const cv::Mat& source, const cv::Mat& reference) {
    std::vector<int> histSource, histReference;
    computeHistogram(source, histSource);
    computeHistogram(reference, histReference);

    // Compute CDF for source
    std::vector<float> cdfSource(256, 0);
    cdfSource[0] = histSource[0];
    for (int i = 1; i < 256; i++) {
        cdfSource[i] = cdfSource[i - 1] + histSource[i];
    }

    // Compute CDF for reference
    std::vector<float> cdfReference(256, 0);
    cdfReference[0] = histReference[0];
    for (int i = 1; i < 256; i++) {
        cdfReference[i] = cdfReference[i - 1] + histReference[i];
    }

    // Normalize CDF
    float cdfSourceMin = *std::min_element(cdfSource.begin(), cdfSource.end());
    float cdfReferenceMin = *std::min_element(cdfReference.begin(), cdfReference.end());
    int totalSourcePixels = source.rows * source.cols;
    int totalReferencePixels = reference.rows * reference.cols;

    for (int i = 0; i < 256; i++) {
        cdfSource[i] = (cdfSource[i] - cdfSourceMin) / (totalSourcePixels - cdfSourceMin) * 255;
        cdfReference[i] = (cdfReference[i] - cdfReferenceMin) / (totalReferencePixels - cdfReferenceMin) * 255;
    }

    // Map intensities from source to reference
    cv::Mat matchedImage = source.clone();
    for (int r = 0; r < source.rows; r++) {
        for (int c = 0; c < source.cols; c++) {
            int srcIntensity = static_cast<int>(cdfSource[source.at<uchar>(r, c)]);
            // Find closest match in reference CDF
            int matchedIntensity = 0;
            float minDiff = std::abs(cdfReference[0] - srcIntensity);
            for (int j = 1; j < 256; j++) {
                float diff = std::abs(cdfReference[j] - srcIntensity);
                if (diff < minDiff) {
                    minDiff = diff;
                    matchedIntensity = j;
                }
            }
            matchedImage.at<uchar>(r, c) = static_cast<uchar>(matchedIntensity);
        }
    }
    return matchedImage;
}

cv::Mat histogramMatchingWithFile(const cv::Mat& source, const std::string& filePath) {
    std::ifstream file(filePath);
    std::vector<float> desiredHistogram(256);
    for (int i = 0; i < 256; i++) {
        file >> desiredHistogram[i];
    }
    
    // Convert the normalized histogram to a cumulative distribution function
    std::vector<float> cdf(256, 0);
    cdf[0] = desiredHistogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + desiredHistogram[i];
    }

    // Scale to 255
    for (int i = 0; i < 256; i++) {
        cdf[i] = cdf[i] * 255;
    }

    // Map intensities
    cv::Mat matchedImage = source.clone();
    for (int r = 0; r < source.rows; r++) {
        for (int c = 0; c < source.cols; c++) {
            int intensity = source.at<uchar>(r, c);
            matchedImage.at<uchar>(r, c) = static_cast<uchar>(cdf[intensity]);
        }
    }
    return matchedImage;
}

int main(int argc, char** argv) {
    cv::CommandLineParser parser(argc, argv, "{h help||}{m|1|Method: 1=Histogram Equalization, 2=Histogram Matching with Image, 3=Histogram Matching with File}{@imagefile||Input Image}{@histogram_file||Histogram File for Matching}");
    
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    std::string imageFile = parser.get<std::string>("@imagefile");
    int method = parser.get<int>("m");
    std::string histogramFile = parser.get<std::string>("@histogram_file");

    cv::Mat image = cv::imread(imageFile, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Could not open or find the image!" << std::endl;
        return -1;
    }
    
    if (image.type() != CV_8UC1) {
        std::cerr << "Input image is not a grayscale image!" << std::endl;
        return -1;
    }

    cv::Mat outputImage;
    switch (method) {
        case 1:
            outputImage = histogramEqualization(image);
            break;
        case 2:
            if (histogramFile.empty()) {
                std::cerr << "Reference image file is required for histogram matching!" << std::endl;
                return -1;
            }
            {
                cv::Mat referenceImage = cv::imread(histogramFile, cv::IMREAD_GRAYSCALE);
                if (referenceImage.empty()) {
                    std::cerr << "Could not open or find the reference image!" << std::endl;
                    return -1;
                }
                outputImage = histogramMatching(image, referenceImage);
            }
            break;
        case 3:
            if (histogramFile.empty()) {
                std::cerr << "Histogram file is required for histogram matching!" << std::endl;
                return -1;
            }
            outputImage = histogramMatchingWithFile(image, histogramFile);
            break;
        default:
            std::cerr << "Invalid method selection!" << std::endl;
            return -1;
    }

    cv::imshow("Original Image", image);
    cv::imshow("Enhanced Image", outputImage);
    cv::waitKey(0);

    return 0;
}
