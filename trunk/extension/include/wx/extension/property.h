////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wxExProperty class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXPROPERTY_H
#define _EXPROPERTY_H

#include <wx/xml/xml.h>

class wxStyledTextCtrl;

class wxExProperty
{
public:
  /// Default constructor.
  wxExProperty(const wxXmlNode* node = NULL);
private:
  void Set(wxXmlNode* node);
  
  wxString m_Key;
  wxString m_Value;
};
#endif
