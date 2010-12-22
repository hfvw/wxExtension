////////////////////////////////////////////////////////////////////////////////
// Name:      style.h
// Purpose:   Declaration of wxExStyle class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTYLE_H
#define _EXSTYLE_H

#include <set>
#include <wx/xml/xml.h>

class wxStyledTextCtrl;

/// This class defines our scintilla styles. The no as in xml or in the string
/// can be a single style, or several styles separated by a comma.
/// E.g.
/// 1,2,3=fore:light steel blue,italic,size:8
/// 1,2,3 are the scintilla styles numbers, and the rest is spec
class WXDLLIMPEXP_BASE wxExStyle
{
public:
  /// Default constructor.
  wxExStyle();
  
  /// Constructor using xml node.
  wxExStyle(const wxXmlNode* node);

  /// Constructor using no and value.
  wxExStyle(const wxString& no, const wxString& value);

  /// Applies this style to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Does this style concern the default style.
  bool IsDefault() const;

  /// Returns true if this style is valid.
  bool IsOk() const;
private:
  void Set(const wxXmlNode* node);
  void SetNo(const wxString& no);

  std::set <int> m_No;
  wxString m_Value;
};
#endif