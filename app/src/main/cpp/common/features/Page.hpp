//
//  Page.hpp
//  AbaScan
//
//  Created by Lance Watson on 24.11.17.
//  Copyright © 2017 Lance Watson. All rights reserved.
//



#ifndef Page_hpp
#define Page_hpp

#ifndef _EiC
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#endif

#import "Element.hpp"
#import "Group.hpp"
#import "Contour.hpp"
#import "Line.hpp"
//#include <TesseractOCR/baseapi.h>

class Page {
  
public:
  
  cv::Mat sourceImg;
  cv::Mat processedImg;
  cv::Mat annotatedImg;
  cv::Mat characterTrainingImage;
  cv::Mat wordTrainingImage;
  std::set<Element*>* contours;
  std::set<Group*>* groups;
  std::set<Line*>* lines;
  std::set<Para*>* paras;

  cv::Rect typRect;
  //tesseract::TessBaseAPI *api;
  
  void sortByMaxDimension();
  void sortByArea();
  Page(cv::Mat img, const char* directoryPath);
  ~Page();
  void processSourceImage();
  void findContours ();
  void removeAnomolousContours();
  void removeAnomolousLines();
  void generateGroups();
  void generateLines();
  void generateParas();
  void removeAnomolousGroups();
  Element* n_element_by_area(int n);
  void getContourStatistics(float percentile);
  void drawContours();
  void drawBoundingRects();
  void drawLines();
  void drawParas();
  void ocrGroups();
  std::string ocrRect(cv::Rect boundingRect, cv::Mat sourceImg);
  void addElementToTrainingData(Element* element, cv::Mat source, cv::Mat &mat, int xpos, int ypos);
  void generateCharacterTrainingData();
  std::string runTesseract(Element* element);
  std::string text();
  int addWordToTrainingData(Element* element,
                            cv::Mat source,
                            cv::Mat &target,
                            int xpos, int ypos);
  void generateWordTrainingData();
  std::vector<Para*> sortedParas();
  
  void writeWordSamplesToDirectory(std::string directory);
  
};

#endif /* Page_hpp */
