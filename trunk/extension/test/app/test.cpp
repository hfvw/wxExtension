/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <TestCaller.h>
#include <wx/extension/header.h>
#include <wx/extension/tool.h>
#include "test.h"
#define TEST_FILE "../test.h"

void wxExAppTestFixture::setUp()
{
  m_Grid = new wxExGrid(wxTheApp->GetTopWindow());
  m_ListView = new wxExListView(wxTheApp->GetTopWindow());
  m_Notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
  m_STC = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  m_STCShell = new wxExSTCShell(wxTheApp->GetTopWindow());
  m_SVN = new wxExSVN(SVN_INFO, TEST_FILE);
}

void wxExAppTestFixture::testConstructors()
{
}

void wxExAppTestFixture::testMethods()
{
  // test wxExApp
  CPPUNIT_ASSERT(wxExApp::GetConfig() != NULL);
  CPPUNIT_ASSERT(wxExApp::GetLexers() != NULL);
  CPPUNIT_ASSERT(wxExApp::GetPrinter() != NULL);
  CPPUNIT_ASSERT(!wxExTool::GetToolInfo().empty());

  // test wxExGrid
  CPPUNIT_ASSERT(m_Grid->CreateGrid(5, 5));
  m_Grid->SelectAll();
  m_Grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test");
  m_Grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test1");

  // test wxExListView
  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn("String", wxExColumn::COL_STRING);
  m_ListView->InsertColumn("Number", wxExColumn::COL_INT);
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") == 1);

  // test wxExNotebook (parent should not be NULL)
  wxWindow* page1 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page2 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);

  CPPUNIT_ASSERT(m_Notebook->AddPage(page1, "key1") != NULL);
  CPPUNIT_ASSERT(m_Notebook->AddPage(page2, "key2") != NULL);
  CPPUNIT_ASSERT(m_Notebook->AddPage(page1, "key1") == NULL);
  CPPUNIT_ASSERT(m_Notebook->GetKeyByPage(page1) == "key1");
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("key1") == page1);
  CPPUNIT_ASSERT(m_Notebook->SetPageText("key1", "keyx", "hello"));
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("keyx") == page1);
  CPPUNIT_ASSERT(m_Notebook->DeletePage("keyx"));
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("keyx") == NULL);

  // test wxExSTC
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(m_STC->Open(wxExFileName("../test.bin")));
  CPPUNIT_ASSERT(m_STC->GetFlags() == 0);
  const wxCharBuffer& buffer = m_STC->GetTextRaw();
  wxLogMessage(buffer.data());
  CPPUNIT_ASSERT(buffer.length() == 40);
  CPPUNIT_ASSERT(!m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded());
  m_STC->StartRecord();
  CPPUNIT_ASSERT( m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded());
  m_STC->StopRecord();
  CPPUNIT_ASSERT(!m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded()); // still no macro

  // test wxExSTCShell
  m_STCShell->Prompt("test1");
  m_STCShell->Prompt("test2");
  m_STCShell->Prompt("test3");
  m_STCShell->Prompt("test4");
  // Prompting does not add a command to history...
  // TODO: Make a better test.
  CPPUNIT_ASSERT(!m_STCShell->GetHistory().Contains("test4"));

  // test wxExSVN
  CPPUNIT_ASSERT(m_SVN->Execute() == 0); // do not use a dialog
  CPPUNIT_ASSERT(!m_SVN->GetOutput(wxTheApp->GetTopWindow().empty()));

  // test util
  CPPUNIT_ASSERT(wxExClipboardAdd("test"));
  CPPUNIT_ASSERT(wxExClipboardGet() == "test");
  CPPUNIT_ASSERT(wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(wxExGetLineNumberFromText("test on line: 1200") == 1200);

  // Only usefull if the lexers file was present
  if (wxExApp::GetLexers()->Count() > 0)
  {
    wxExApp::GetConfig()->Set(_("Purpose"), "hello test");
    const wxExFileName fn(TEST_FILE);
    const wxString header = wxExHeader(wxExApp::GetConfig()).Get(&fn);
    CPPUNIT_ASSERT(header.Contains("hello test"));
  }
  else
  {
    wxLogMessage("No lexers available");
  }

  CPPUNIT_ASSERT(wxExLog("hello from wxextension test"));
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(wxExSkipWhiteSpace("t     es   t") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExAppTestFixture::tearDown()
{
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxextension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testConstructors",
    &wxExAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testMethods",
    &wxExAppTestFixture::testMethods));
}
