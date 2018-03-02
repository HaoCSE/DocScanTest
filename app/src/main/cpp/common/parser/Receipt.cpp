//
//  Receipt.cpp
//  AbaScan
//
//  Created by Lance Watson on 26.01.18.
//  Copyright © 2018 Lance Watson. All rights reserved.
//

#include "Receipt.hpp"
#include "json.h"
#include <regex>
using namespace std::regex_constants;
using namespace std;

const std::string kJsonTagElements("elements");
const std::string kJsonTagFields("fields");
const std::string kJsonTagPerimeter("perimeter");
const std::string kJsonTagElement("element");
const std::string kJsonTagSearchPattern("searchPattern");
const std::string kJsonTagValuePatternIndex("valuePatternIndex");
const std::string kJsonTagValueReplacePatterns("valueReplacePatterns");



const std::string kJsonTagTotal("total");
const std::string kJsonTagDate("date");
const std::string kJsonTagTime("time");
const std::string kJsonTagCurrency("currency");
const std::string kJsonTagVendorName("vendorName");
const std::string kJsonTagVendorVatCode("vatCode");
const std::string kJsonTagVendorTelephone("telephone");
const std::string kJsonTagLocation1("location1");
const std::string kJsonTagLocation2("location2");
const std::string kJsonTagCountry("country");

ReceiptField receiptFieldFromString(std::string string)
{
  
  if (string.compare(kJsonTagTotal) == 0)
    return ReceiptFieldTotal;
  else if (string.compare(kJsonTagTotal) == 0)
    return ReceiptFieldDate;
  else if (string.compare(kJsonTagTime) == 0)
    return ReceiptFieldTime;
  else if (string.compare(kJsonTagDate) == 0)
    return ReceiptFieldDate;
  else if (string.compare(kJsonTagCurrency) == 0)
    return ReceiptFieldCurrency;
  else if (string.compare(kJsonTagVendorName) == 0)
    return ReceiptFieldVendorName;
  else if (string.compare(kJsonTagVendorVatCode) == 0)
    return ReceiptFieldVendorVatCode;
  else if (string.compare(kJsonTagVendorTelephone) == 0)
    return ReceiptFieldTelephone;
  else if (string.compare(kJsonTagLocation1) == 0)
    return ReceiptFieldLocation1;
  else if (string.compare(kJsonTagLocation2) == 0)
    return ReceiptFieldLocation2;
  else if (string.compare(kJsonTagCountry) == 0)
    return ReceiptFieldCountry;
  return ReceiptFieldUnknown;
}

std::string stringFromReceiptField(ReceiptField field)
{
  switch (field) {
    case ReceiptFieldUnknown:
      return "unknown";
    case ReceiptFieldTotal:
      return kJsonTagTotal;
    case ReceiptFieldDate:
      return kJsonTagDate;
    case ReceiptFieldTime:
      return kJsonTagTime;
    case ReceiptFieldCurrency:
      return kJsonTagCurrency;
    case ReceiptFieldVendorName:
      return kJsonTagVendorName;
    case ReceiptFieldVendorVatCode:
      return kJsonTagVendorVatCode;
    case ReceiptFieldTelephone:
      return kJsonTagVendorTelephone;
    case ReceiptFieldLocation1:
      return kJsonTagLocation2;
    case ReceiptFieldLocation2:
      return kJsonTagLocation2;
    case ReceiptFieldCountry:
      return kJsonTagCountry;
  }
}




RegexRecognizer::RegexRecognizer(std::string afield,
                std::string asearchPattern,
                int avaluePatternIndex,
                std::vector<std::pair<std::string, std::string>> avalueReplacePatterns)
{
  field = receiptFieldFromString(afield);
  searchPattern = asearchPattern;
  valuePatternIndex = avaluePatternIndex;
  valueReplacePatterns = avalueReplacePatterns;
}


std::string RegexRecognizer::applyToText(std::string text)
{

  if(text == "03.01.2018 17:54:35")
  {
    int i = 1;
    i++;
  }
  std::regex searchRegex = std::regex(searchPattern, icase );
  std::smatch piecesMatch;
  if (std::regex_match(text, piecesMatch, searchRegex)) {
    if(valuePatternIndex < piecesMatch.size())
    {
      std::string match = piecesMatch[valuePatternIndex].str();
      for (std::pair<std::string, std::string> replacePattern : valueReplacePatterns)
      {
        match = std::regex_replace(match, std::regex(replacePattern.first),replacePattern.second);
      }
      return match;
    }
  }
  return "";
}


Receipt::Receipt(std::string json)
{
  hasPerimeter = false;

  Json::Value root;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse( json.c_str(), root );
  
  if(!parsingSuccessful) return;
  
  
  for (Json::Value::ArrayIndex i = 0; i != root[kJsonTagPerimeter].size(); i++)
  {
    for (Json::Value::ArrayIndex j = 0; j != root[kJsonTagPerimeter][i].size(); j++)
    {
      Json::Value value = root[kJsonTagPerimeter][i][j];
      cv::Point2f perimeterPoint = perimeter[i];
      if(j==0)
        perimeterPoint.x =  value.asFloat();
      else
        perimeterPoint.y =  value.asFloat();
      perimeter[i] = perimeterPoint;
    }
    hasPerimeter = true;
  }
    
  fields =  std::map<ReceiptField, std::string>();
  vector<string> fieldNames = {
    kJsonTagTotal,
    kJsonTagDate,
    kJsonTagTime,
    kJsonTagCurrency,
    kJsonTagVendorName,
    kJsonTagVendorVatCode,
    kJsonTagVendorTelephone,
    kJsonTagLocation1,
    kJsonTagLocation2,
    kJsonTagCountry};
  
    for (auto & fieldName : fieldNames)
    {
  
      string value = root[kJsonTagFields][fieldName].asString();
      ReceiptField field = receiptFieldFromString(fieldName);
      fields.insert(std::pair<ReceiptField, std::string>(field, value));
    }
  
}





bool Receipt::matchesField(ReceiptField field, Receipt& reference)
{
  if (fields.find(field) == fields.end() )
    return false;
  
  if(fields[field] != reference.fields[field])
    return false;
  
  return true;
}


bool Receipt::hasField(ReceiptField field)
{
  if (fields.find(field) == fields.end() )
    return false;
  if (fields[field] == "" )
    return false;
  return true;
}


Receipt::  Receipt(std::string receiptText, std::string configPath)
{

  

  ifstream fin(configPath.c_str());
  
  
  
  Json::Value elements;

  if (!fin) {
    cerr << "error: open file for input failed!" << endl;
    abort();
  }

  fin >> elements;
  for (Json::Value::iterator it = elements[kJsonTagElements].begin(); it != elements[kJsonTagElements].end(); ++it) {
    std::vector<std::pair<std::string, std::string>> replacePatterns;
    Json::Value jValueReplacePatterns = (*it)[kJsonTagValueReplacePatterns];
    for (Json::Value::ArrayIndex i = 0; i != jValueReplacePatterns.size(); i++)
    {
      pair<string, string> pair;
      for (Json::Value::ArrayIndex j = 0; j != jValueReplacePatterns[i].size(); j++)
      {
        if(j==0)
          pair.first =  jValueReplacePatterns[i][j].asString();
        else
        pair.second =  jValueReplacePatterns[i][j].asString();
      }
      replacePatterns.push_back(pair);
    }
    std::vector<std::pair<std::string, std::string>> avalueReplacePatterns;
    RegexRecognizer recognizer ((*it)[kJsonTagElement].asString(),
                                (*it)[kJsonTagSearchPattern].asString(),
                                (*it)[kJsonTagValuePatternIndex].asInt(),
                                replacePatterns);

    recognizers.push_back(recognizer);

  }
  fin.close();
  
  std::string result;
  
  std::istringstream iss(receiptText);
  
  fields =  std::map<ReceiptField, std::string>();
  
  for (std::string line; std::getline(iss, line); )
  {
    line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );
    for (auto & recognizer : recognizers)
    {
     std::string result = recognizer.applyToText(line);
      if((result != "") && (fields.find(recognizer.field) == fields.end()))
      {
        fields.insert(std::pair<ReceiptField, std::string>(recognizer.field, result));
      }
    }
  }
}


std::string Receipt::jsonString()
{
  Json::Value root;
  Json::Value jfields;

  for (int field = ReceiptFieldUnknown+1; field < ReceiptFieldCountry+1; field++)
  {
    std::string fieldName = stringFromReceiptField((ReceiptField)field);
    string value = "";
    if (fields.find((ReceiptField)field) != fields.end() )
      value = fields[(ReceiptField)field];
    jfields[fieldName] = value;
  }
  
  if(hasPerimeter)
  {
    Json::Value jperimeter;
    for(int i=0; i<4;i++)
    {
      Json::Value jpoint;
      jpoint[0] = perimeter[i].x;
      jpoint[1] = perimeter[i].y;
      jperimeter[i] = jpoint;
    }
    root[kJsonTagPerimeter] = jperimeter;
  }
  
  root[kJsonTagFields] = jfields;
  
  Json::StyledWriter writer;
  const string output = writer.write(root);
  return output;
}





