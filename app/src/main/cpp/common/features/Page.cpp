//
//  Page.cpp
//  AbaScan
//
//  Created by Lance Watson on 24.11.17.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Page.hpp"
#include "Para.hpp"
#include "niblack_thresholding.hpp"
//#include <TesseractOCR/tesseractclass.h>
//#include <uuid/uuid.h>

using namespace std;
using namespace cv;

Page::Page(cv::Mat img, const char* directoryPath)
{
  /*
  api = new tesseract::TessBaseAPI();
  // Initialize tesseract-ocr with English, without specifying tessdata path
  
  if (api->Init(directoryPath, "deu")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }
  */

  groups = new std::set<Group*>();
  lines = new std::set<Line*>();
  paras = new std::set<Para*>();
  
//  api->SetVariable("tessedit_char_whitelist", "+,.:-%€/0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
/*  api->SetVariable("segment_penalty_garbage", "10");
  api->SetVariable("segment_penalty_dict_nonword", "1");
  api->SetVariable("segment_penalty_dict_frequent_word", "1");
  api->SetVariable("segment_penalty_dict_case_ok", "1");
  api->SetVariable("segment_penalty_dict_case_bad", "1");
  api->SetVariable("numeric_punctuation", ",.:/-");
*/
  this->sourceImg = img;
  this->annotatedImg = sourceImg.clone();
    //this->processedImg = cv::Mat::Mat();
    this->processedImg = Mat();
}


Page::~Page()
{
  // Destroy used object and release memory
//  api->End();
}


void binarizeImage(Mat & srcImg, Mat & dstImg)
{
  
  cv::Mat equalized = srcImg.clone();
  
  double min, max;
  cv::Point2i minLoc,maxLoc;
  cv::minMaxLoc(srcImg, &min, &max, &minLoc, &maxLoc);
  
  equalized -= min;
  equalized *= (255.0/((float)max-(float)min));
  
  
  cv::ximgproc::niBlackThreshold(equalized, dstImg,
                                 255, THRESH_BINARY + THRESH_OTSU, 35, -0.22, cv::ximgproc::BINARIZATION_NICK);
  
}


void Page::processSourceImage()
{
  Mat graySource =  Mat();
  cv::cvtColor(sourceImg, graySource, CV_BGR2GRAY);

  double min, max;
  cv::Point2i minLoc,maxLoc;
  cv::minMaxLoc(graySource, &min, &max, &minLoc, &maxLoc);

  graySource -= min;
  graySource *= (255.0/((float)max-(float)min));
  
  int dimension = std::min(graySource.rows, graySource.cols);
  dimension /= 38;
  dimension *= 2;
  dimension += 1;
  
  cv::ximgproc::niBlackThreshold(graySource, processedImg,
                                 255, THRESH_BINARY_INV + THRESH_OTSU, dimension, -0.18, cv::ximgproc::BINARIZATION_NICK);
  
}

void Page::findContours ()
{
  
  this->contours = new set<Element*>();
  
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  
  /// Find contours
  cv:: findContours(processedImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS, cv::Point(0, 0) );
  
  /// Approximate contours to polygons + get bounding rects and circles
  vector<vector<cv::Point> > contours_poly( contours.size() );
  vector<Point2f>center( contours.size() );
  vector<float>radius( contours.size() );
  
  //Get poly contours
  for( int i = 0; i < contours.size(); i++ )
  {
    approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
  }
  
  for (int i=0;i<contours_poly.size();i++){
    
    cv::Rect r = boundingRect(Mat(contours_poly[i]));
    if(r.area()<5)continue;
    bool inside = false;
    for(int j=0;j<contours_poly.size();j++){
      if(j==i)continue;
      
      cv::Rect r2 = boundingRect(Mat(contours_poly[j]));
      if(r2.area()<100||r2.area()<r.area())continue;
      if(r.x>r2.x&&r.x+r.width<r2.x+r2.width&&
         r.y>r2.y&&r.y+r.height<r2.y+r2.height){
        
        inside = true;
      }
    }
    //if(inside)continue;
    Contour* element = new Contour(contours_poly[i]);
    this->contours->insert(element);
  }
}

void Page::removeAnomolousContours()
{
  // using function as comp
  if(this->contours->empty())
  {
    return;
  }
  set<Element*> elements = *(this->contours);
  std::set<Element*>::iterator  it;
  
  int maxHeight = ((float)typRect.height)*4;
  int minHeight = ((float)typRect.height)*0.05;
  int typWidth = ((float)typRect.width)*0.67;
  int maxArea = ((float)typRect.area())*14;;
  
  float margin = 4;
  
  cv::Rect innerRect = cv::Rect(margin,margin,sourceImg.cols-2*margin,sourceImg.rows-2*margin);
  
  
  for (it = elements.begin(); it != elements.end(); it++)
  {
    cv::Rect boundingRect = (*it)->boundingRect;
    
    
    if ((boundingRect != (boundingRect & innerRect)) || ((boundingRect.height > maxHeight) ||(boundingRect.area() > maxArea)) || ((boundingRect.height < minHeight) && (boundingRect.width > typWidth )))
    {
      this->contours->erase(*it);
    }
  }
}

void Page::removeAnomolousLines()
{
  // using function as comp
  if(this->lines->empty())
  {
    return;
  }
  set<Line*> elements = *(this->lines);
  std::set<Line*>::iterator  it;
  
  int minHeight = ((float)typRect.height)*0.5;
  
  
  for (it = elements.begin(); it != elements.end(); it++)
  {
  }
}



void Page::generateGroups()
{
  int minProximity = ((float)typRect.width);
  Group::populateSetWithGroupedElements(groups,contours,minProximity);
}


void Page::generateLines()
{
  Line::populateSetWithAlignedGroups(lines,groups,0.25);
}

void Page::generateParas()
{
  Para::populateSetWithLines(paras,lines,0.5);
}


void Page::drawContours()
{
  
  //  Scalar color = Scalar(0,255,0);
  for( int i = 0; i< this->contours->size(); i++ )
  {
    //  cv::drawContours( annotatedImg, this->contours, i, color, 1, 8, vector<Vec4i>(), 0, cv::Point() );
  }
}





void Page::getContourStatistics(float percentile)
{
  int i = percentile * contours->size()/100;
  vector<Element*> vtr = vector<Element*>(contours->begin(), contours->end());
  std::sort(vtr.begin(), vtr.end(), [](Element* elt1, Element* elt2) { return elt1->boundingRect.area() <  elt2->boundingRect.area(); });
  Element* element = vtr.at(i);
  
  typRect = element->boundingRect;
  for (auto &element : *(this->contours)) {
    element->minimumDimension = cv::Point(((float)typRect.width),0);
  }
}

void Page::drawLines()
{
  Scalar color;
  color = Scalar((rand() % 255),(rand() % 255),(rand() % 255));
  
  for (auto &line : *(this->lines)) {
    rectangle(annotatedImg, line->minimalRect().tl(), line->minimalRect().br(),color, 2, 8, 0 );
  }
}

void Page::drawParas()
{
  
  Scalar color;
  color = Scalar((rand() % 255),(rand() % 255),(rand() % 255));
  
  for (auto &para : *(this->paras)) {
    rectangle(annotatedImg, para->minimalRect().tl(), para->minimalRect().br(),color, 2, 8, 0 );
  }
}

void Page::drawBoundingRects()
{
  
  
  Scalar color;
  color = Scalar((rand() % 255),(rand() % 255),(rand() % 255));
  
  for (auto &element : *(this->contours)) {
    if(element->boundingRect.area()<5)continue;
    //   rectangle(annotatedImg, element->minimalRect().tl(), element->minimalRect().br(),color, 2, 8, 0 );
  }
  //    rectangle(annotatedImg, typRect.tl(), typRect.br(), color, CV_FILLED, 8, 0 );
}

static void annotateImageWithText(const cv::Rect &boundingRect, cv::Mat &imageToAnnotate, const string &text) {
  Scalar color;
  
  Scalar white = Scalar(255,255,255);
  
  int fontFace = FONT_HERSHEY_SIMPLEX;
  double fontScale = 0.7;
  int thickness = 2;
  int baseline=2;
  
  rectangle(imageToAnnotate, boundingRect.tl(), boundingRect.br(),color, 2, 8, 0 );
  
  cv::Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
  baseline += thickness;
  
  // center the text
  cv::Point textOrg = cv::Point(boundingRect.tl().x, boundingRect.tl().y-4);
  
  cv::Rect textRect = cv::Rect(textOrg-cv::Point(0, textSize.height+2),textSize+cv::Size(0,4));
  textRect = textRect &  cv::Rect(cv::Point(0,0), imageToAnnotate.size());
  // draw the box
  rectangle(imageToAnnotate,textRect.tl(),textRect.br(), color, CV_FILLED, 8, 0);
  
  // then put the text itself
  putText(imageToAnnotate, text, textOrg, fontFace, fontScale,  white, thickness, 8);
}

static cv::Mat ocrPreparedImage(const cv::Rect &boundingRect, const cv::Mat &sourceImg)
{
  int d = 8;
  
  cv::Rect expandedRect=cv::Rect(boundingRect.tl()+cv::Point(-d,-d),boundingRect.size()+cv::Size(2*d,2*d));
  expandedRect = expandedRect & cv::Rect(cv::Point(0,0), sourceImg.size());
  cv::Mat roi = sourceImg(expandedRect);
  Mat graySource =  Mat();
  cv::cvtColor(roi, graySource, CV_BGR2GRAY);
  return graySource;
}

string Page::ocrRect(cv::Rect boundingRect, cv::Mat sourceImg)
{
    /*
  cv::Mat roi = ocrPreparedImage(boundingRect, sourceImg) ;
  cv::Mat imageToProcess = Mat();
  binarizeImage(roi, imageToProcess);
  api->SetImage((uchar*)imageToProcess.data, imageToProcess.size().width, imageToProcess.size().height, imageToProcess.channels(), (int)imageToProcess.step1());
  char* ocrWordResult = api->GetUTF8Text();
  string text = string(ocrWordResult);
  text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
  
  return text;
     */
    return 0;
}

void Page::ocrGroups()
{
//  api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
  
  for (auto &group : *this->groups){
    cv::Rect boundingRect = group->boundingRect;
    group->text = ocrRect(boundingRect, sourceImg);
    boundingRect = group->boundingRect;
  }
}


void Page::addElementToTrainingData(Element* element,
                                    Mat source,
                                    Mat &target,
                                    int xpos, int ypos)
{
  int xSize = 32;
  int ySize = 32;
  int xPadding = 2;
  int yPadding = 2;
  
  cv::Rect frame = cv::Rect(1, 1, 29, 29);
  frame.x = 1+xpos*xSize+xPadding;
  frame.y = 1+ypos*ySize+yPadding;
  rectangle(target, frame.tl(), frame.br(),Scalar(128,128,128), 1 );

  cv::Rect boundingRect = element->boundingRect;

  boundingRect = boundingRect & cv::Rect(cv::Point(0,0), source.size());
  
  if(boundingRect.area() == 0.0)
    return;
  
  Mat sourceEltImg = source(boundingRect);

  cv::Rect targetRect;
  
  if((boundingRect.width > xSize-2*xPadding)
    ||(boundingRect.height > ySize-2*yPadding)) {
  
    float scale = 28.0f/((float)std::max(boundingRect.width,boundingRect.height));
    float width = scale*(float)boundingRect.width;
    float height = scale*(float)boundingRect.height;
    cv::Size size = cv::Size(width,height);
    cv::Mat scaledImg = cv::Mat();
    cv::resize(sourceEltImg, scaledImg, size);
    sourceEltImg=scaledImg;
    targetRect = cv::Rect(cv::Point(0,0), scaledImg.size());
    
  } else {
    targetRect = boundingRect;
  }
  targetRect.x = xpos*xSize+xPadding;
  targetRect.x += (xSize - targetRect.width)/2;
  targetRect.y = ypos*ySize+yPadding;
  targetRect.y += ySize - targetRect.height-yPadding;
  sourceEltImg.copyTo(target(targetRect));

}




void Page::generateCharacterTrainingData()
{
  int xpos = 0;
  int ypos = 0;
  
  cv::Mat training=cv::Mat::zeros(1024, 1024, CV_8U);

  training += 255;
  
  int itemsPerLine = 32;
  
  Mat graySource = Mat();
  
  cv::cvtColor(sourceImg, graySource, CV_BGR2GRAY);
  
  for(auto &group : *this -> groups)
  {
    for (auto &element : *group->elements){
      cv::Rect boundingRect = element->boundingRect;
      addElementToTrainingData(element,graySource,training,xpos,ypos);
      boundingRect &= cv::Rect(0,0,graySource.cols,graySource.rows);
      Mat sourceEltImg = graySource(boundingRect);
      xpos++;
      xpos %= itemsPerLine;
      if(!xpos)
        ypos++;
    }
  }
  characterTrainingImage = training;
}


void Page::generateWordTrainingData()
{

  int xpos = 0;
  int ypos = 0;
  
  cv::Mat training=cv::Mat::zeros(2048, 1024, CV_8U);
  training += 255;
  
  for (auto para : sortedParas())
  {
    for (auto line : para->sortedLines())
    {
      for (auto group : line->sortedGroups())
      {
        cv::Rect boundingRect = group->boundingRect;
        boundingRect &= cv::Rect(0,0,sourceImg.cols,sourceImg.rows);
        cv::Mat imageToProcess = ocrPreparedImage(boundingRect, sourceImg) ;
        cv::Mat scaledImg = cv::Mat();
        cv::resize(imageToProcess, scaledImg, imageToProcess.size()/4);
        cv::Rect targetRect = cv::Rect(xpos,ypos, scaledImg.cols, scaledImg.rows);
        targetRect &= cv::Rect(0,0,training.cols,training.rows);
        boundingRect = targetRect;
        boundingRect.x = 0;
        boundingRect.y = 0;

        cv::Mat sourceEltImg=scaledImg(boundingRect);
        sourceEltImg.copyTo(training(targetRect));
        xpos += boundingRect.width+8;
      }
      ypos += line->boundingRect.height+8;
      xpos = 0;
    }
    ypos += 32;
  }
  wordTrainingImage = training;
}

void Page::writeWordSamplesToDirectory(std::string directory)
{
  for (auto group : *this->groups)
  {
    cv::Rect boundingRect = group->boundingRect;
    boundingRect &= cv::Rect(0,0,sourceImg.cols,sourceImg.rows);
    cv::Mat imageToProcess = ocrPreparedImage(boundingRect, sourceImg);
//    uuid_t uuid;
//    uuid_generate_random ( uuid );
    char s[37];
//    uuid_unparse ( uuid, s );
    char filePath[256];
    sprintf(filePath,"%s/%s.png",directory.c_str(),s);
    cv::imwrite(std::string(filePath), imageToProcess);
  }
}




std::vector<Para*> Page::sortedParas()
{


  std::vector<Para*> _sortedParas = std::vector<Para*>( this->paras->begin(), this->paras->end() );

  if(_sortedParas.size() == 0)
    return std::vector<Para*>();
  
  std::sort(_sortedParas.begin(), _sortedParas.end(), [](Para* a, Para* b) {
    return (a->boundingRect.tl().y < b->boundingRect.tl().y);
  });
  return _sortedParas;
}

std::string Page::text() {

  
  std::string textString = std::string("");
  
  bool first = true;
  
  for (auto para : sortedParas())
  {
    if(!first)
      textString.append("\n\r");
    
    textString.append(para->text());
    
    first = false;
    
  }
  return textString;
}

