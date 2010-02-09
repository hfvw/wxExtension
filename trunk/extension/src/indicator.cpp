////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.cpp
// Purpose:   Implementation of class 'wxExIndicator'
// Author:    Anton van Wezenbeek
// Created:   2010-02-09
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stc/stc.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexers.h>

wxExIndicator::wxExIndicator(const wxXmlNode* node)
  : m_Number(-1)
  , m_Style(-1)
{
  if (node != NULL)
  {
    Set(node);
  }
}

void wxExIndicator::Apply(wxStyledTextCtrl* stc) const
{
  stc->IndicatorSetStyle(m_Number, m_Style);
}

void wxExIndicator::Set(const wxXmlNode* node)
{
  m_Number = atoi(
    wxExLexers::Get()->ApplyMacro(node->GetAttribute("no", "0")).c_str());
  m_Style = atoi(node->GetNodeContent().Strip(wxString::both).c_str());
}
