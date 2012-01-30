////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTESTUNIT_H
#define _EXTESTUNIT_H

#ifdef __WXGTK__

#include <TestCaller.h>
#include <TestFixture.h>
#include <TestSuite.h>
#include <wx/extension/extension.h>

/// CppUnit test fixture.
/// These classes require either an wxExApp object, or wx to be initialized.
class wxExTestFixture : public CppUnit::TestFixture
{
public:
  /// Default constructor.
  wxExTestFixture() : TestFixture() {}; 
  
  /// Destructor.
 ~wxExTestFixture() {};
 
  /// Adds text to report.
  void Report(const wxString& text);
 
  /// Prints out report.
  static void PrintReport();
private:
  static wxString m_Report;  
};
#endif
#endif