/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-20012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef cmDependInfo_h
#define cmDependInfo_h

#include <map>
#include <vector>

#include "cmStandardIncludes.h"
#include "cmDepends.h"

//class cmGlobalGenerator;
class cmGeneratedFileStream;
class cmLocalGenerator;
class cmGeneratorTarget;

// class cmDependInfoManifest
// {
// public:
//   static const char* DEPEND_INFO_MANIFEST_FILE;
// 
//   /// 
//   /// 
//   static void Write(cmGeneratedFileStream& os, cmGlobalGenerator& generator);
// 
// private:
//   cmDependInfoManifest();
//   ~cmDependInfoManifest();
// };

class cmDependInfo
{
public:
  cmDependInfo(cmLocalGenerator* generator);
  virtual ~cmDependInfo();

  void Read(const char* depInfoFile);

  void Scan(const char* directory);

  /// @description Write information (in DependInfo.cmake style) needed
  ///  to scan target components for implicit dependencies.
  void Write(cmGeneratedFileStream& os, cmGeneratorTarget& target);

  static const char* DEPEND_INFO_FILE;
private:
  cmLocalGenerator* LocalGenerator;
  std::map<std::string, cmDepends::DependencyVector> FoundDependencies;
};

#endif // include guard
