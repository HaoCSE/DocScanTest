//
//  Receipt.hpp
//  AbaScan
//
//  Created by Lance Watson on 26.01.18.
//  Copyright © 2018 Lance Watson. All rights reserved.
//

#ifndef Receipt_hpp
#define Receipt_hpp

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

typedef enum {
  ReceiptFieldUnknown,
  ReceiptFieldTotal,
  ReceiptFieldDate,
  ReceiptFieldTime,
  ReceiptFieldCurrency,
  ReceiptFieldVendorName,
  ReceiptFieldVendorVatCode,
  ReceiptFieldTelephone,
  ReceiptFieldLocation1,
  ReceiptFieldLocation2,
  ReceiptFieldCountry,
} ReceiptField;

ReceiptField receiptFieldFromString(std::string string);
std::string stringFromReceiptField(ReceiptField field);


struct RegexRecognizer
{
  
  RegexRecognizer(std::string afield,
                std::string asearchPattern,
                int avaluePatternIndex,
                  std::vector<std::pair<std::string, std::string> > valueReplacePatterns);
  
  ReceiptField field;
  std::string searchPattern;
  int valuePatternIndex;
  std::vector<std::pair<std::string, std::string> > valueReplacePatterns;
  
  std::string applyToText(std::string text);
};


struct Receipt {
  bool hasPerimeter;
  cv::Point2f perimeter[4];
  std::map<ReceiptField, std::string> fields;
  Receipt(std::string json);
  Receipt(std::string receiptText, std::string configPath);
  bool matchesField(ReceiptField field, Receipt& other);
  bool hasField(ReceiptField field);
  std::string jsonString();
private:

  std::vector<RegexRecognizer> recognizers;

  
};


#endif /* Receipt_hpp */
