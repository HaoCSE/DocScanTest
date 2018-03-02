//
//  LWHomographyExtractor.cpp
//  AbaScan
//
//  Created by Lance Watson on 20/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "LWHomographyExtractor.hpp"
#include "niblack_thresholding.hpp"

using namespace std;
using namespace cv;

template <typename T> int sgn(T val) {
  return (T(0) < val) - (val < T(0));
}

static void annotateQuadWeight(const cv::Scalar &color, Quad &quad, cv::Mat &rgbImage) {
  float perimeterScore = 0;
  float areaConsidered = 0;
  cv::Rect imageRect = cv::Rect(cv::Point2f(0,0),cv::Point2f(rgbImage.cols,rgbImage.rows));
  
  cv::Mat image = rgbImage;
  
  for (int side = 0; side < 4; side++)
  {
    for (int i = 0; i < quad.pointsOnSide(side); i+= 5)
    {
      cv::Point2f pointOnPerimeter = quad.pointOnPerimeter(side,i);
      
      cv::Point2f tl = pointOnPerimeter - cv::Point2f(2,2);
      cv::Point2f br = pointOnPerimeter + cv::Point2f(2,2);
      cv::Rect rect = cv::Rect(tl,br);
      rect &= imageRect;
      cv::Mat roi = image(rect);
      areaConsidered += rect.area();
      perimeterScore += cv::sum(roi)[0];
      cv::rectangle(rgbImage, tl, br, color);
    }
  }
}

void LWHomographyExtractor::pannotateQuad(const cv::Scalar &color, int index,  cv::Mat rgbImage, float scale)
{
  if(quads.size() > index)
  {
    Quad quad = quads[index];
    annotateQuadWeight(color, quad, rgbImage);
  }
}

void LWHomographyExtractor::annotateQuad( Quad& quad, cv::Mat rgbImage, const cv::Scalar &color)
{
  cv::Point2f* perimeter = quad.perimeter;
  cv::line(rgbImage,perimeter[0],perimeter[1],color,4);
  cv::line(rgbImage,perimeter[1],perimeter[2],color,4);
  cv::line(rgbImage,perimeter[2],perimeter[3],color,4);
  cv::line(rgbImage,perimeter[3],perimeter[0],color,4);
}


cv::Vec4f lineOfMaxum(cv::Point3f maximum, cv::Point2f center)
{
  float angle = fmod(maximum.x,360) / 180.0 * CV_PI;
  float mag   = maximum.y + 5;
  float cos = std::cos(angle);
  float sin = std::sin(angle);

  
  cv::Point2f cartMaximum = mag*cv::Point2f(cos,sin);
  cv::Point2f pVector = cv::Point2f(sin,-cos)*1000;
  
  cv::Point2f origin= center+cartMaximum;
  cv::Point2f pInf = origin+pVector;;
  cv::Point2f nInf = origin-pVector;
  
  return(cv::Vec4f(nInf.x, nInf.y,pInf.x,pInf.y));
}

static std::array<float, 3> cross(const std::array<float, 3> &a,
                                  const std::array<float, 3> &b)
{
  std::array<float, 3> result;
  result[0] = a[1] * b[2] - a[2] * b[1];
  result[1] = a[2] * b[0] - a[0] * b[2];
  result[2] = a[0] * b[1] - a[1] * b[0];
  return result;
}

bool get_intersection(cv::Vec4f line_a,
                      cv::Vec4f line_b,
                      cv::Point2f & intersection)
{
  std::array<float, 3> pa{ { line_a[0], line_a[1], 1 } };
  std::array<float, 3> pb{ { line_a[2], line_a[3], 1 } };
  std::array<float, 3> la = cross(pa, pb);
  pa[0] = line_b[0], pa[1] = line_b[1], pa[2] = 1;
  pb[0] = line_b[2], pb[1] = line_b[3], pb[2] = 1;
  std::array<float, 3> lb = cross(pa, pb);
  std::array<float, 3> inter = cross(la, lb);
  if (inter[2] == 0) return false; // two lines are parallel
  else {
    intersection.x = inter[0] / inter[2];
    intersection.y = inter[1] / inter[2];
    return true;
  }
}

float differenceInDegrees(float targetA, float sourceA)
{
  float a = fabs(targetA - sourceA);
  
  if(a > 180)
    a = 180-a;
  
  return a;
}

Corner::Corner()
{
  
}

Corner::Corner(cv::Point3f maximum1, cv::Point3f maximum2, cv::Point2f cen)
{
  max1 = maximum1;
  max2 = maximum2;
  center = cen;
  score = calcScore();
}

Corner::~Corner()
{
  
}



bool Corner::operator==(const Corner &other) const {
  if ((max1 == other.max1) && (max2 == other.max2))
    return true;

  if ((max2 == other.max1) && (max1 == other.max2))
    return true;

  return false;
}

bool Corner::hasMaximaInCommonWith(Corner other)
{
    if ((max1 == other.max1) || (max2 == other.max2))
    return true;

  if ((max2 == other.max1) || (max1 == other.max2))
    return true;

  return false;
}


static float dotProduct(const cv::Vec4f &line1, const cv::Vec4f &line2) {
  Point2f vec1 = Point2f(line1[2]-line1[0],line1[3]-line1[1]);
  Point2f vec2 = Point2f(line2[2]-line2[0],line2[3]-line2[1]);
  
  vec1 /= norm(vec1);
  vec2 /= norm(vec2);
  
  float dot = vec1.x*vec2.x + vec1.y*vec2.y;
  return dot;
}

static bool perpIntersect(cv::Point2f &intersect, cv::Point2f center, const cv::Vec4f &line1, const cv::Vec4f &line2) {
  float dot = fabs(dotProduct(line1, line2));
  
  if(dot > 0.3)
    return false;
  
  bool crosses = get_intersection(line1,line2,intersect);
  if(!crosses)
    return false;
  
  if(fabs(intersect.x-center.x)/center.x > 1.2)
    return false;
  
  if(fabs(intersect.y-center.y)/center.y > 1.2)
    return false;
  
  
  return true;
}

float Corner::calcScore()
{
  if(max1 == max2)
    return 0;

  line1 = lineOfMaxum(max1, center);
  line2 = lineOfMaxum(max2, center);
  cv::Point2f intersect = cv::Point2f();

  if (!perpIntersect(intersect, center, line1, line2))
    return 0;

  return max1.z * max2.z;
}


const cv::Point2f Corner::intersect()
{
  cv::Point2f intersect = cv::Point2f();
  bool crosses = get_intersection(line1,line2,intersect);
  if(!crosses)
    return center;
  
  return intersect;
}

Quad::Quad(Corner corn1, Corner corn2, cv::Mat image)
{
  corner1 = corn1;
  corner2 = corn2;
  _perimeterScore = -1;
  _orthogonalityScore = -1;
  cv::Point2f center = cv::Point2f(image.cols,image.rows)/2;
  if (get_corner_points(center))
  {
    score = calcScore(image);
  }
  else score = 0;
}

bool Quad::operator==(const Quad &other) const {
  if ((corner1.max1 != other.corner1.max1) &&
      (corner1.max1 != other.corner1.max2) &&
      (corner1.max1 != other.corner2.max1) &&
      (corner1.max1 != other.corner2.max2))
    return false;

  if ((corner1.max2 != other.corner1.max1) &&
      (corner1.max2 != other.corner1.max2) &&
      (corner1.max2 != other.corner2.max1) &&
      (corner1.max2 != other.corner2.max2))
    return false;

  if ((corner2.max1 != other.corner1.max1) &&
      (corner2.max1 != other.corner1.max2) &&
      (corner2.max1 != other.corner2.max1) &&
      (corner2.max1 != other.corner2.max2))
    return false;

  if ((corner2.max2 != other.corner1.max1) &&
      (corner2.max2 != other.corner1.max2) &&
      (corner2.max2 != other.corner2.max1) &&
      (corner2.max2 != other.corner2.max2))
    return false;

  return true;
}

cv::Point2f Quad::pointOnPerimeter(int side,int position)
{
  int startIndex = side;
  int endIndex = (startIndex+1) % 4;
  Point2f startPoint = perimeter[startIndex];
  Point2f endPoint = perimeter[endIndex];
  Point2f vector = (endPoint-startPoint);
  float arg = atan2(vector.y, vector.x);
  float mod;
  if(abs(vector.x) > abs(vector.y))
  {
    mod = sgn(vector.x)*(float)position / cos(arg);
  } else {
    mod = sgn(vector.y)*(float)position / sin(arg);
  }
  return startPoint+vector*mod/norm(vector);
}

int Quad::pointsOnSide(int side)
{
  int startIndex = side;
  int endIndex = (startIndex+1) % 4;
  Point2f startPoint = perimeter[startIndex];
  Point2f endPoint = perimeter[endIndex];
  Point2f vector = (endPoint-startPoint);
  return max(abs(vector.x),abs(vector.y));
}

float Quad::perimeterScore(const cv::Mat &image)
{
  if(_perimeterScore >= 0)
    return _perimeterScore;
  
  float perimeterScore = 0;
  float areaConsidered = 0;
  cv::Rect imageRect = cv::Rect(cv::Point2f(0,0),cv::Point2f(image.cols,image.rows));

  for (int side = 0; side < 4; side++)
  {
    for (int i = 0; i < pointsOnSide(side); i+= 10)
    {
      cv::Point2f pointOnPerimeter = this->pointOnPerimeter(side,i);
      cv::Point2f tl = pointOnPerimeter - cv::Point2f(5,5);
      cv::Point2f br = pointOnPerimeter + cv::Point2f(5,5);
      cv::Rect rect = cv::Rect(tl,br);
      rect &= imageRect;
      cv::Mat roi = image(rect);
      areaConsidered += rect.area();
      float scoreIncrement = cv::sum(roi)[0];
      perimeterScore += scoreIncrement;
    }
  }
  _perimeterScore = perimeterScore/areaConsidered;
  return _perimeterScore;
}


float Quad::orthogonalityScore()
{
  if(_orthogonalityScore != -1)
    return _orthogonalityScore;

  _orthogonalityScore = 4;

  cv::Vec4f line1 = corner1.line1;
  cv::Vec4f line2 = corner1.line2;
  cv::Vec4f line3 = corner2.line1;
  cv::Vec4f line4 = corner2.line2;
  _orthogonalityScore -= fabs(dotProduct(line1,line2));
  _orthogonalityScore -= fabs(dotProduct(line2, line3));
  _orthogonalityScore -= fabs(dotProduct(line3, line4));
  _orthogonalityScore -= fabs(dotProduct(line4, line1));
  
  return _orthogonalityScore;
}

float Quad::calcScore(cv::Mat image)
{
  if(corner1 == corner2) {
    return 0;
  }
  float area = contourArea(contour);
  if(area == 0)
    return 0;
  
  float imageArea = (float)(image.cols*image.rows);
  float areaScore = sqrt(area/imageArea);
  return areaScore;
}

void  Quad::addIntersection(cv::Point2f center, const cv::Vec4f &line1, const cv::Vec4f &line2)
{
  cv::Point2f intersect = cv::Point2f();
  if (perpIntersect(intersect, center, line1, line2))
    contour.push_back(intersect);
}

bool Quad::get_corner_points( cv::Point2f center)
{
  contour = std::vector<cv::Point2f>();
  
  cv::Vec4f line1 = corner1.line1;
  cv::Vec4f line2 = corner1.line2;
  cv::Vec4f line3 = corner2.line1;
  cv::Vec4f line4 = corner2.line2;

  addIntersection(center, line1, line2);
  addIntersection(center, line1, line3);
  addIntersection(center, line1, line4);
  addIntersection(center, line2, line3);
  addIntersection(center, line2, line4);
  addIntersection(center, line3, line4);

  if(contour.size() != 4)
    return false;
  
  std::sort(contour.begin(),contour.end(),[](cv::Point elt1, cv::Point elt2) { return (elt1.x+elt1.y) < (elt2.x+elt2.y); });
  
  perimeter[0]=contour.front();
  perimeter[2]=contour.back();
  
  // compute the difference between the contour -- the top-right
  // will have the minumum difference and the bottom-left will
  // have the maximum difference
  
  std::sort(contour.begin(),contour.end(),[](cv::Point elt1, cv::Point elt2) { return (-elt1.x+elt1.y) < (-elt2.x+elt2.y); });
  
  perimeter[1]=contour.front();
  perimeter[3]=contour.back();
  
  contour = std::vector<cv::Point2f>();
  contour.push_back(perimeter[0]);
  contour.push_back(perimeter[1]);
  contour.push_back(perimeter[2]);
  contour.push_back(perimeter[3]);

  return true;
  
}

LWHomographyExtractor::LWHomographyExtractor(cv::Mat img)
{

  // Open input image with leptonica library
  this->sourceImg = img;
  this->annotatedImg = Mat();
  this->processedImg = Mat();
}


LWHomographyExtractor::LWHomographyExtractor()
{
  
  // Open input image with leptonica library
  this->sourceImg = NULL;
  this->annotatedImg = Mat();
  this->processedImg = Mat();
}


static void extractTransform(const cv::Mat &image, cv::Mat &transformImage) {
  Mat grad_x, grad_y;
  int ddepth = CV_16S;
  int scale = 1;
  
  int flowResolution = 1;
  
  Mat gradient_field = Mat::zeros(image.rows,image.cols,CV_32FC2);
  Scharr(image,grad_x,ddepth,1,0,scale);
  Scharr(image,grad_y,ddepth,0,1,scale);
  GaussianBlur (grad_x, grad_x, cv::Size(3,3), 6,0);
  GaussianBlur(grad_y, grad_y, cv::Size(3,3),  6,0);
  
  for (int i = 0 ; i < image.rows ; i ++){
    for (int j = 0 ; j < image.cols ; j ++){
      short xval = grad_x.at<short>(i,j);
      short yval = grad_y.at<short>(i,j);
      gradient_field.at<cv::Point2f>(i,j) = cv::Point2f(xval,yval);
    }
  }
  
  cv::Point2f center = cv::Point2f(image.cols,image.rows)/2;
  float maxDistanceFromCenter = norm(center)+1;
  transformImage = cv::Mat::zeros(360,360,CV_32FC1);
  
  for (int i = 0 ; i < image.rows ; i+= flowResolution){
    for (int j = 0 ; j < image.cols ; j+= flowResolution){
      cv::Point2f p(j,i);
      cv::Point2f p2(gradient_field.at<cv::Point2f>(p));
      if(norm(p2) == 0)
        continue;
      
      cv::Point2f vectorFromOrigin = p-center;
      cv::Point2f normalizedGrad =  p2/norm(p2);
      float distanceToTangentCircle = normalizedGrad.dot(vectorFromOrigin);
      cv::Point2f correctlySignedVector = normalizedGrad;
      if(distanceToTangentCircle<0)  {
        correctlySignedVector*=-1;
        distanceToTangentCircle*=-1;
        if(distanceToTangentCircle>5)
        {
          distanceToTangentCircle-=5;
        }
      }
      if(distanceToTangentCircle>5)
      {
        distanceToTangentCircle-=2;
      }
      float angle =  fmod(fastAtan2(correctlySignedVector.y,correctlySignedVector.x),360);
      float val = (transformImage.at<float>(cv::Point2f(angle,distanceToTangentCircle*360/maxDistanceFromCenter)));
      val += norm(p2);
      transformImage.at<float>(cv::Point2f(angle,distanceToTangentCircle*360/maxDistanceFromCenter)) = val;
    }
  }
  double min, max;
  cv::minMaxLoc(transformImage, &min, &max);
  transformImage -= min;
  transformImage *= 255/(max-min);
  transformImage.convertTo(transformImage, CV_8U);
}


void LWHomographyExtractor::extractMaxima(const cv::Mat &image, cv::Mat &transformImage)
{
  
  // Detect blobs.
  std::vector<KeyPoint> keypoints;

  double min, max;
  cv::Point2i minLoc,maxLoc;

  while(keypoints.size() < 8) {

    cv::minMaxLoc(transformImage, &min, &max, &minLoc, &maxLoc);

    if(max <= 8)
      break;
    
    cv::KeyPoint keypoint = cv::KeyPoint();
    keypoint.size = 0;
    for (int x=maxLoc.x -35 ; x <= (maxLoc.x +35) ; x++) {
      for(int y=maxLoc.y -10; y <= (maxLoc.y +10);y++) {
        Point2i point((x+transformImage.cols)%transformImage.cols,(y+transformImage.rows)%transformImage.rows);
        keypoint.size += transformImage.at<unsigned char>(point);
        transformImage.at<unsigned char>(point) = 0;
      }
    }
    keypoint.pt = maxLoc;
    keypoint.size /= 200;
    keypoints.push_back(keypoint);
  }
  
//  drawKeypoints( annotatedImg, keypoints, annotatedImg, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
  
  cv::Point2f center = cv::Point2f(image.cols,image.rows)/2;
  float maxDistanceFromCenter = norm(center)+1;

  for (auto & keypoint : keypoints) {
    maxima.push_back(cv::Point3f(keypoint.pt.x, keypoint.pt.y*maxDistanceFromCenter/360, keypoint.size));
  }

/*  maxima.push_back(cv::Point3f(90, center.x, 32));
  maxima.push_back(cv::Point3f(270, center.x, 32));
  maxima.push_back(cv::Point3f(0, center.y, 32));
  maxima.push_back(cv::Point3f(180, center.y, 32));*/

}



void LWHomographyExtractor::extractCorners(const cv::Mat &image) {
  cv::Point2f center = cv::Point2f(image.cols,image.rows)/2;
  corners = std::vector<Corner>();
  
  for(size_t i = 0; i < maxima.size(); i++)
  {
    for(size_t j = i+1; j < maxima.size(); j++)
    {
      Point3f maximum = maxima[i];
      Point3f other = maxima[j];
      
      Corner corner = Corner(maximum,other,center);
      if(corner.score != 0)
      {
        corners.push_back(corner);
      }
    }
  }
  std::sort(corners.begin(), corners.end(), [&image](Corner &a, Corner &b) {
    return (a.score > b.score);
  });
  while(corners.size() > 12)
  {
    corners.pop_back();
  }
}

void LWHomographyExtractor::extractQuads(const cv::Mat& image)
{
  quads = std::vector<Quad>();
  
  for(size_t i = 0; i < corners.size(); i++)
  {
    for(size_t j = i+1; j < corners.size(); j++)
    {
      Corner corner= corners[i];
      Corner other = corners[j];
      if(corner.hasMaximaInCommonWith(other))
        continue;
      Quad quad = Quad(corner,other, sourceImg);
      for(auto &duplicateQuadTest : quads)  {
        if(duplicateQuadTest == quad)
          quad.score = 0;
      }
      if(quad.score != 0)
      {
//        annotateQuad(quad, image, Scalar(255));
        quads.push_back(quad);
      }
    }
  }
  
  
  std::sort(quads.begin(), quads.end(), [&image](Quad &a, Quad &b) {
    return (a.perimeterScore(image) > b.perimeterScore(image));
  });
}

void LWHomographyExtractor::extractHomography()
{
  cv::Mat image =  sourceImg;
  
  cv::Mat transformImage;
  extractTransform(image, transformImage);
  transformImage.convertTo(annotatedImg, CV_8U);
  cv::cvtColor(annotatedImg, annotatedImg, CV_GRAY2RGB);
  extractMaxima(image,transformImage);
  extractCorners(image);
  extractQuads(image);
  cv::Mat unprocImg = cv::Mat();
  transformImage.convertTo(processedImg, CV_8U);
}


