//
//  LWHomographyExtractor.hpp
//  AbaScan
//
//  Created by Lance Watson on 20/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef LWHomographyExtractor_hpp
#define LWHomographyExtractor_hpp

#ifndef _EiC
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <opencv2/opencv.hpp>
#endif

cv::Vec4f lineOfMaxum(cv::Point3f maximum, cv::Point2f center);

struct Corner
{
  cv::Point3f max1;
  cv::Point3f max2;
  cv::Point2f center;
  float score;
  Corner();
  Corner(cv::Point3f maximum1, cv::Point3f maximum2, cv::Point2f cen);
  ~Corner();
  bool operator==(const Corner &other) const;
  bool isCloseTo(Corner &other);
  bool hasMaximaInCommonWith(Corner other);
  float calcScore();
  const cv::Point2f intersect();
  cv::Vec4f line1;
  cv::Vec4f line2;

};

struct Quad
{
  Corner corner1;
  Corner corner2;
  float score;
  Quad(Corner corn1, Corner corn2, cv::Mat img);
  float calcScore(cv::Mat img);
  bool get_corner_points( cv::Point2f center);
  std::vector<cv::Point2f> contour;
  bool operator==(const Quad &other) const;
  cv::Point2f pointOnPerimeter(int side,int position);
  int pointsOnSide(int side);
  cv::Point2f perimeter[4];
  void addIntersection(cv::Point2f center, const cv::Vec4f &line1, const cv::Vec4f &line2);
  float perimeterScore(const cv::Mat &image);
  float _perimeterScore;
  float orthogonalityScore();
  float _orthogonalityScore;

};

class LWHomographyExtractor
{
public:
  cv::Mat sourceImg;
  cv::Mat processedImg;
  cv::Mat annotatedImg;

  LWHomographyExtractor();
  LWHomographyExtractor(cv::Mat img);
  void extractHomography();
  
//private:
    void extractMaxima(const cv::Mat &image, cv::Mat &transformImage);
  void extractCorners(const cv::Mat &image);
  void extractQuads(const cv::Mat &image);
  std::vector<cv::Point3f> maxima;
  std::vector<Corner> corners;
  std::vector<Quad> quads;
  cv::Point2f perimeter[4];
  void annotateQuad( Quad& quad, cv::Mat rgbImage, const cv::Scalar &color);
  void pannotateQuad(const cv::Scalar &color, int index,  cv::Mat rgbImage, float scale);

};


#endif /* LWHomographyExtractor_hpp */

