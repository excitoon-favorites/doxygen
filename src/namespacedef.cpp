/******************************************************************************
 *
 * 
 *
 * Copyright (C) 1997-2000 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include "qtbc.h"
#include "namespacedef.h"
#include "outputlist.h"
#include "util.h"
#include "doc.h"
#include "language.h"
#include "classdef.h"
#include "classlist.h"
#include "memberlist.h"
#include "doxygen.h"
#include "message.h"

NamespaceDef::NamespaceDef(const char *df,int dl,
                           const char *name,const char *lref) : 
   Definition(df,dl,name)
{
  fileName="namespace_"+nameToFile(name);
  classList = new ClassList;
  classDict = new ClassDict(1009);
  //memList = new MemberList;
  usingDirList = 0;
  usingDeclList = 0;
  setReference(lref);
  memberGroupList = new MemberGroupList;
  memberGroupList->setAutoDelete(TRUE);
  memberGroupDict = new MemberGroupDict(1009);
  defFileName = df;
  defLine = dl;
}

NamespaceDef::~NamespaceDef()
{
  delete classList;
  delete classDict;
  delete usingDirList;
  delete usingDeclList;
  delete memberGroupList;
  delete memberGroupDict;
}

void NamespaceDef::distributeMemberGroupDocumentation()
{
  MemberGroupListIterator mgli(*memberGroupList);
  MemberGroup *mg;
  for (;(mg=mgli.current());++mgli)
  {
    mg->distributeMemberGroupDocumentation();
  }
}
void NamespaceDef::insertUsedFile(const char *f)
{
  if (files.find(f)==-1) 
  {
    if (Config::sortMembersFlag)
      files.inSort(f);
    else
      files.append(f);
  }
}

void NamespaceDef::insertClass(ClassDef *cd)
{
  if (classDict->find(cd->name())==0)
  {
    if (Config::sortMembersFlag)
      classList->inSort(cd);
    else
      classList->append(cd);
    classDict->insert(cd->name(),cd);
  }
}

void NamespaceDef::addMemberListToGroup(MemberList *ml,
                                        bool (MemberDef::*func)() const)
{
  MemberListIterator mli(*ml);
  MemberDef *md;
  for (;(md=mli.current());++mli)
  {
    int groupId=md->getMemberGroupId();
    if ((md->*func)() && groupId!=-1)
    {
      QCString *pGrpHeader = memberHeaderDict[groupId];
      QCString *pDocs      = memberDocDict[groupId];
      if (pGrpHeader)
      {
        MemberGroup *mg = memberGroupDict->find(groupId);
        if (mg==0)
        {
          mg = new MemberGroup(groupId,*pGrpHeader,pDocs ? pDocs->data() : 0);
          memberGroupDict->insert(groupId,mg);
          memberGroupList->append(mg);
        }
        mg->insertMember(md);
        md->setMemberGroup(mg);
      }
    }
  }
}

void NamespaceDef::addMembersToMemberGroup()
{
  addMemberListToGroup(&allMemberList,&MemberDef::isTypedef);
  addMemberListToGroup(&allMemberList,&MemberDef::isEnumerate);
  addMemberListToGroup(&allMemberList,&MemberDef::isEnumValue);
  addMemberListToGroup(&allMemberList,&MemberDef::isFunction);
  addMemberListToGroup(&allMemberList,&MemberDef::isVariable);
}

void NamespaceDef::insertMember(MemberDef *md)
{
  //memList->append(md);
  allMemberList.append(md); 
  switch(md->memberType())
  {
    case MemberDef::Variable:     
      if (Config::sortMembersFlag)
        varMembers.inSort(md); 
      else
        varMembers.append(md);
      break;
    case MemberDef::Function: 
      if (Config::sortMembersFlag)    
        funcMembers.inSort(md); 
      else
        funcMembers.append(md);
      break;
    case MemberDef::Typedef:      
      if (Config::sortMembersFlag)
        typedefMembers.inSort(md); 
      else
        typedefMembers.append(md);
      break;
    case MemberDef::Enumeration:  
      if (Config::sortMembersFlag)
        enumMembers.inSort(md); 
      else
        enumMembers.append(md);
      break;
    case MemberDef::EnumValue:    
      if (Config::sortMembersFlag)
        enumValMembers.inSort(md); 
      else
        enumValMembers.append(md);
      break;
    case MemberDef::Prototype:    
      if (Config::sortMembersFlag)
        protoMembers.inSort(md); 
      else
        protoMembers.append(md);
      break;
    case MemberDef::Define:       
      if (Config::sortMembersFlag)
        defineMembers.inSort(md); 
      else
        defineMembers.append(md);
      break;
    default:
       err("NamespaceDef::insertMembers(): unexpected member inserted in namespace!\n");
  }
  //addMemberToGroup(md,groupId);
}

void NamespaceDef::computeAnchors()
{
  setAnchors('a',&allMemberList);
  //MemberGroupListIterator mgli(*memberGroupList);
  //MemberGroup *mg;
  //for (;(mg=mgli.current());++mgli)
  //{
  //  mg->setAnchors();
  //}
}

void NamespaceDef::writeDocumentation(OutputList &ol)
{
  QCString pageTitle=name()+" Namespace Reference";
  startFile(ol,fileName,pageTitle);
  startTitle(ol,getOutputFileBase());
  //ol.docify(pageTitle);
  parseText(ol,theTranslator->trNamespaceReference(name()));
  endTitle(ol,getOutputFileBase(),name());
  
  if (!Config::genTagFile.isEmpty()) tagFile << "%" << name() << ":\n";
  
  ol.startTextBlock();
    
  OutputList briefOutput(&ol); 
  if (!briefDescription().isEmpty()) 
  {
    parseDoc(briefOutput,defFileName,defLine,name(),0,briefDescription());
    ol+=briefOutput;
    ol.writeString(" \n");
    ol.pushGeneratorState();
    ol.disableAllBut(OutputGenerator::Html);
    ol.startTextLink(0,"_details");
    parseText(ol,theTranslator->trMore());
    ol.endTextLink();
    ol.popGeneratorState();
  }
  ol.disable(OutputGenerator::Man);
  ol.newParagraph();
  ol.enable(OutputGenerator::Man);
  ol.writeSynopsis();

  ol.endTextBlock();
  
  ol.startMemberSections();
  classList->writeDeclaration(ol);

  /* write user defined member groups */
  MemberGroupListIterator mgli(*memberGroupList);
  MemberGroup *mg;
  for (;(mg=mgli.current());++mgli)
  {
    mg->writeDeclarations(ol,0,this,0,0);
  }
  
  allMemberList.writeDeclarations(ol,0,this,0,0,0,0);
  ol.endMemberSections();
  
  if ((!briefDescription().isEmpty() && Config::repeatBriefFlag) || 
      !documentation().isEmpty())
  {
    ol.writeRuler();
    ol.pushGeneratorState();
    ol.disableAllBut(OutputGenerator::Html);
    //bool latexOn = ol.isEnabled(OutputGenerator::Latex);
    //if (latexOn) ol.disable(OutputGenerator::Latex);
    ol.writeAnchor(0,"_details"); 
    //if (latexOn) ol.enable(OutputGenerator::Latex);
    ol.popGeneratorState();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trDetailedDescription());
    ol.endGroupHeader();
    ol.startTextBlock();
    if (!briefDescription().isEmpty() && Config::repeatBriefFlag)
    {
      ol+=briefOutput;
    }
    if (!briefDescription().isEmpty() && Config::repeatBriefFlag &&
        !documentation().isEmpty())
    {
      ol.newParagraph();
    }
    if (!documentation().isEmpty())
    {
      parseDoc(ol,defFileName,defLine,name(),0,documentation()+"\n");
      ol.newParagraph();
    }
    ol.endTextBlock();
  }

  //memList->countDocMembers();
  defineMembers.countDocMembers();
  if ( defineMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trDefineDocumentation());
    ol.endGroupHeader();
    defineMembers.writeDocumentation(ol,name(),this);
  }
  
  protoMembers.countDocMembers(); 
  if ( protoMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trFunctionPrototypeDocumentation());
    ol.endGroupHeader();
    protoMembers.writeDocumentation(ol,name(),this);
  }

  typedefMembers.countDocMembers();
  if ( typedefMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trTypedefDocumentation());
    ol.endGroupHeader();
    typedefMembers.writeDocumentation(ol,name(),this);
  }
  
  enumMembers.countDocMembers();
  if ( enumMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trEnumerationTypeDocumentation());
    ol.endGroupHeader();
    enumMembers.writeDocumentation(ol,name(),this);
  }

  //enumValMembers.countDocMembers();
  //if ( enumValMembers.totalCount()>0 )
  //{
  //  ol.writeRuler();
  //  ol.startGroupHeader();
  //  parseText(ol,theTranslator->trEnumerationValueDocumentation());
  //  ol.endGroupHeader();
  //  enumValMembers.writeDocumentation(ol,name());
  //}

  funcMembers.countDocMembers();
  if ( funcMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trFunctionDocumentation());
    ol.endGroupHeader();
    funcMembers.writeDocumentation(ol,name(),this);
  }
  
  varMembers.countDocMembers();
  if ( varMembers.totalCount()>0 )
  {
    ol.writeRuler();
    ol.startGroupHeader();
    parseText(ol,theTranslator->trVariableDocumentation());
    ol.endGroupHeader();
    varMembers.writeDocumentation(ol,name(),this);
  }

  // write Author section (Man only)
  ol.pushGeneratorState();
  ol.disableAllBut(OutputGenerator::Man);
  ol.startGroupHeader();
  parseText(ol,theTranslator->trAuthor());
  ol.endGroupHeader();
  parseText(ol,theTranslator->trGeneratedAutomatically(Config::projectName));
  //ol.enableAll();
  ol.popGeneratorState();
  endFile(ol);
}

int NamespaceDef::countMembers()
{
  allMemberList.countDocMembers();
  return allMemberList.totalCount()+classList->count();
}

void NamespaceDef::addUsingDirective(NamespaceDef *nd)
{
  if (usingDirList==0)
  {
    usingDirList = new NamespaceList;
  }
  usingDirList->append(nd);
}

void NamespaceDef::addUsingDeclaration(ClassDef *cd)
{
  if (usingDeclList==0)
  {
    usingDeclList = new ClassList;
  }
  usingDeclList->append(cd);
}
