//
// Created by benedetti on 2/26/2018.
//
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <jni.h>
#include <string>
#include <structured_edge_detection.hpp>



using namespace cv;

extern "C"
JNIEXPORT void JNICALL Java_ch_abacus_test_docscantest_MainActivity_returnEdges (JNIEnv* env, jobject obj, jstring path, jlong imgPtr, jlong resPtr) {
    std::string myPath = env->GetStringUTFChars(path, 0);
    Ptr<ximgproc::StructuredEdgeDetection> pDollar = ximgproc::createStructuredEdgeDetection(myPath);

    Mat &img = *(Mat *) imgPtr;
    Mat &res = *(Mat *) resPtr;
    cvtColor(img, img, COLOR_RGBA2BGR);
    img.convertTo(img, DataType<float>::type, 1/255.0);
    Mat edges(img.size(), img.type());
    pDollar->detectEdges(img, edges);
    edges.convertTo(edges, cv::DataType<unsigned char>::type, 255);
    res = edges.clone();
    cvtColor(res,res,CV_GRAY2RGBA);

//    int type = res.type();
//    uchar depth = type & CV_MAT_DEPTH_MASK;
//    uchar chans = 1 + (type >> CV_CN_SHIFT);

}

extern "C"
JNIEXPORT jstring JNICALL Java_ch_abacus_test_docscantest_MainActivity_test (JNIEnv* env, jobject obj, jstring msg) {
    std::string myPath = env->GetStringUTFChars(msg, 0);
    return env->NewStringUTF(myPath.append("_XXX").c_str());
}

