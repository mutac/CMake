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

#include "cmStandardIncludes.h"
#include "cmDepends.h"

class cmGlobalGenerator;
class cmGeneratedFileStream;
class cmGeneratorTarget;

class cmDependInfoManifest
{
public:
  static const char* DEPEND_INFO_MANIFEST_FILE;

  /// 
  /// 
  static void Write(cmGeneratedFileStream& os, cmGlobalGenerator& generator);

private:
  cmDependInfoManifest();
  ~cmDependInfoManifest();
};

class cmDependInfo
{
public:
  static const char* DEPEND_INFO_FILE;

  /// @description Write information (in DependInfo.cmake style) needed
  ///  to scan target components for implicit dependencies.
  static void Write(cmGeneratedFileStream& os, cmGeneratorTarget& target);

private:
  cmDependInfo();
  virtual ~cmDependInfo();
};

#endif // include guard
