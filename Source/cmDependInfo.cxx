/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-20012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include <string>
#include <vector>
#include <map>

#include "cmDependInfo.h"
#include "cmGlobalGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"
#include "cmSystemTools.h"
#include "cmComputeLinkInformation.h"

//////////////////////////////////////////////////////////////////////////
// cmDependInfoManifest
//////////////////////////////////////////////////////////////////////////

const char* cmDependInfoManifest::DEPEND_INFO_MANIFEST_FILE = "Makefile.cmake";

cmDependInfoManifest::cmDependInfoManifest()
{
}

cmDependInfoManifest::~cmDependInfoManifest()
{
}

void cmDependInfoManifest::Write(cmGeneratedFileStream& os, cmGlobalGenerator& generator)
{
  const std::vector<cmLocalGenerator*>& localGenerators = generator.GetLocalGenerators();

  // now list all the target info files
  os << "# Dependency information for all targets:\n";
  os << "SET(CMAKE_DEPEND_INFO_FILES\n";
  for (unsigned int i = 0; i < localGenerators.size(); ++i)
  {
    cmLocalGenerator* lg = localGenerators[i];

    // for all of out targets
    for (cmTargets::iterator l = lg->GetMakefile()->GetTargets().begin();
      l != lg->GetMakefile()->GetTargets().end(); l++)
    {
      if((l->second.GetType() == cmTarget::EXECUTABLE) ||
        (l->second.GetType() == cmTarget::STATIC_LIBRARY) ||
        (l->second.GetType() == cmTarget::SHARED_LIBRARY) ||
        (l->second.GetType() == cmTarget::MODULE_LIBRARY) ||
        (l->second.GetType() == cmTarget::OBJECT_LIBRARY) ||
        (l->second.GetType() == cmTarget::UTILITY))
      {
        std::string tname = lg->GetRelativePath(lg->GetTargetDirectory(l->second));
        tname += "/";
        tname += cmDependInfo::DEPEND_INFO_FILE;
        cmSystemTools::ConvertToUnixSlashes(tname);
        os << "  \"" << tname.c_str() << "\"\n";
      }
    }
  }
  os << "  )\n";
}

//////////////////////////////////////////////////////////////////////////
// cmDependInfo
//////////////////////////////////////////////////////////////////////////

const char* cmDependInfo::DEPEND_INFO_FILE = "DependInfo.cmake";

cmDependInfo::cmDependInfo()
{
}

cmDependInfo::~cmDependInfo()
{
}

void cmDependInfo::Write(cmGeneratedFileStream& os, cmGeneratorTarget& generatorTarget)
{
  const char* configurationName = generatorTarget.Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  const cmTarget::LinkImplementation* linkImplementation = 
    generatorTarget.Target->GetLinkImplementation(configurationName); 

  //
  // Languages that are part of this target
  //
  // set (CMAKE_DEPENDS_LANGUAGES
  //   "C"
  //   "CXX"
  //   ..
  // )
  os << "# The set of languages for which implicit dependencies are needed:\n";
  os << "SET(CMAKE_DEPENDS_LANGUAGES\n";

  for (std::vector<std::string>::const_iterator
    language = linkImplementation->Languages.begin(); 
    language != linkImplementation->Languages.end(); 
    language++)
  {
    os << "  \"" << *language << "\"\n";
  }

  os << "  )\n";

  // 
  // For each language, list of files to scan:
  //
  // set (CMAKE_DEPENDS_CHECK_<Lang>
  //    "<Source File>" "<Object File>"
  //    ...
  // )
  //

  // Categorize source file dependencies by language
  const std::vector<cmSourceFile*>& allSources = generatorTarget.Target->GetSourceFiles();
  std::map<std::string, std::vector<cmSourceFile*> > sourcesByLanguage;

  for (std::vector<cmSourceFile*>::const_iterator
    source = allSources.begin();
    source != allSources.end();
    source++)
  {
    // Only interested in sources that have a known language association
    if ((*source)->GetLanguage() != NULL)
    {
      std::string language = (*source)->GetLanguage();

      sourcesByLanguage[language];
      sourcesByLanguage[language].push_back(*source);
    }
  }

  os << "# The set of files for implicit dependencies of each language:\n";

  // Write out language-to-source-to-output mappings, and compiler IDs
  for (std::vector<std::string>::const_iterator
      language = linkImplementation->Languages.begin(); 
      language != linkImplementation->Languages.end(); 
    language++)
  {
    // Source to object mappings
    os << "SET(CMAKE_DEPENDS_CHECK_" << *language << "\n";

    std::vector<cmSourceFile*> &sources = sourcesByLanguage[*language];
    for (std::vector<cmSourceFile*>::iterator
      source = sources.begin();
      source != sources.end();
    source++)
    {
      if (*source)
      {
        // <InputPath> <PrimaryOutputPath>
        std::string objectPath = generatorTarget.GetObjectFilePath(*source);

        if (objectPath.size() > 0)
        {
          //TODO: HACK... How are you supposed to do this?
          std::string fullObjectPath = generatorTarget.LocalGenerator->Convert(
            cmLocalGenerator::HOME_OUTPUT,
            objectPath.c_str());
          
          fullObjectPath += "/" + objectPath;

          os << "  \"" << (*source)->GetFullPath() << "\""
            << " \"" << fullObjectPath << "\"\n";
        }

        // <InputPath> <OptionalAdditionalPath>
        if(const char* objectOutputs = 
          (*source)->GetProperty("OBJECT_OUTPUTS")) 
        {
          std::vector<std::string> outputList;
          cmSystemTools::ExpandListArgument(objectOutputs, outputList);

          for(std::vector<std::string>::iterator
            objectPath = outputList.begin();
            objectPath != outputList.end();
            objectPath++)
          {
            os << "  \"" << (*source)->GetFullPath() << "\""
              " \"" << *objectPath << "\"\n";
          }
        }
      }
    }

    os << "  )\n";

    //
    // Compiler ID
    //
    std::string compilerIdVar = "CMAKE_";
    compilerIdVar += *language + "_COMPILER_ID";

    const char* compilerIdVal = 
      generatorTarget.Makefile->GetDefinition(compilerIdVar.c_str());
    if (compilerIdVal && *compilerIdVal)
    {
      os << "SET(" << compilerIdVar << " \"" << compilerIdVal << "\")\n";
    }
  }

  //
  // Compiler definitions
  //
  // set (CMAKE_TARGET_DEFINITIONS
  //   "-Dblah"
  //   ...
  // )
  //
  std::vector<std::string> defines;

  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += cmSystemTools::UpperCase(configurationName);

  if(const char* globalDefs = generatorTarget.Makefile->GetProperty("COMPILE_DEFINITIONS"))
  {
    cmSystemTools::ExpandListArgument(globalDefs, defines);
  }
  if(const char* targetDefs = generatorTarget.Target->GetProperty("COMPILE_DEFINITIONS"))
  {
    cmSystemTools::ExpandListArgument(targetDefs, defines);
  }
  if(const char* globalBuildTypeDefs = generatorTarget.Makefile->GetProperty(defPropName.c_str()))
  {
    cmSystemTools::ExpandListArgument(globalBuildTypeDefs, defines);
  }
  if(const char* targetBuildTypeDefs = generatorTarget.Target->GetProperty(defPropName.c_str()))
  {
    cmSystemTools::ExpandListArgument(targetBuildTypeDefs, defines);
  }
  
  if(!defines.empty())
  {
    os 
      << "\n"
      << "# Preprocessor definitions for this target.\n"
      << "SET(CMAKE_TARGET_DEFINITIONS\n";

    for(std::vector<std::string>::const_iterator di = defines.begin();
      di != defines.end(); ++di)
    {
      os
        << "  " << generatorTarget.LocalGenerator->EscapeForCMake(di->c_str()) << "\n";
    }

    os << "  )\n";
  }


  //
  // Implicit dependency include transformations
  //
  // set (CMAKE_INCLUDE_TRANSFORMS
  //   "xform1"
  //   ...
  // )
  //
  std::vector<std::string> transformRules;

  if(const char* xform =
    generatorTarget.Makefile->GetProperty("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM"))
  {
    cmSystemTools::ExpandListArgument(xform, transformRules);
  }
  if(const char* xform =
    generatorTarget.Target->GetProperty("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM"))
  {
    cmSystemTools::ExpandListArgument(xform, transformRules);
  }

  if(!transformRules.empty())
  {
    os << "SET(CMAKE_INCLUDE_TRANSFORMS\n";

    for(std::vector<std::string>::const_iterator tri = transformRules.begin();
      tri != transformRules.end(); ++tri)
    {
      os << "  " << generatorTarget.LocalGenerator->EscapeForCMake(tri->c_str()) << "\n";
    }

    os << "  )\n";
  }

  //
  // Multiple output pairs?
  //
  // TODO:
  //

  //
  // List of targets linked directly or transitively
  //
  std::set<cmTarget const*> emitted;

  os
    << "\n"
    << "# Targets to which this target links.\n"
    << "SET(CMAKE_TARGET_LINKED_INFO_FILES\n";

  if(cmComputeLinkInformation* cli = generatorTarget.Target->GetLinkInformation(configurationName))
  {
    cmComputeLinkInformation::ItemVector const& items = cli->GetItems();
    for(cmComputeLinkInformation::ItemVector::const_iterator
      i = items.begin(); i != items.end(); ++i)
    {
      cmTarget const* linkee = i->Target;
      if(linkee && !linkee->IsImported() && emitted.insert(linkee).second)
      {
        cmMakefile* mf = linkee->GetMakefile();
        cmLocalGenerator* lg = mf->GetLocalGenerator();
        std::string di = mf->GetStartOutputDirectory();
        di += "/";
        di += lg->GetTargetDirectory(*linkee);
        di += "/";
        di += DEPEND_INFO_FILE;
        os << "  \"" << di << "\"\n";
      }
    }
  }
  os << "  )\n";

  //
  // Fortran module dir: Not Supported!?
  //
  // TODO: ?
  //

  //
  // Target-specific include directories
  //
  std::vector<std::string> includes;

  generatorTarget.LocalGenerator->GetIncludeDirectories(
    includes,
    &generatorTarget, 
    "C", 
    configurationName);

  os
    << "\n"
    << "# The include file search paths:\n"
    << "SET(CMAKE_C_TARGET_INCLUDE_PATH\n";

  for(std::vector<std::string>::iterator i = includes.begin();
    i != includes.end(); ++i)
  {
    os
      << "  \""
      << generatorTarget.LocalGenerator->Convert(
          i->c_str(),
          cmLocalGenerator::HOME_OUTPUT)
      << "\"\n";
  }

  os << "  )\n";

  //
  // Use include path for all languages
  // 

  os
    << "SET(CMAKE_CXX_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";
  os
    << "SET(CMAKE_Fortran_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";
  os
    << "SET(CMAKE_ASM_TARGET_INCLUDE_PATH "
    << "${CMAKE_C_TARGET_INCLUDE_PATH})\n";
}
