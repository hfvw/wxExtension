/******************************************************************************\
* File:          lexers.cpp
* Purpose:       Implementation of wxExLexers classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <wx/stc/stc.h>
#include <wx/textfile.h>
#include <wx/extension/lexers.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h> // for wxExMatchesOneOf

wxExLexers* wxExLexers::m_Self = NULL;

wxExLexers::wxExLexers(const wxFileName& filename)
  : m_FileName(filename)
{
}

const wxString wxExLexers::BuildWildCards(const wxFileName& filename) const
{
  const wxString allfiles_wildcard =
    _("All Files") + wxString::Format(" (%s)|%s",
      wxFileSelectorDefaultWildcardStr,
      wxFileSelectorDefaultWildcardStr);

  wxString wildcards = allfiles_wildcard;

  // Build the wildcard string using all available lexers.
  for (
    std::vector<wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->GetAssociations().empty())
    {
      const wxString wildcard =
        it->GetScintillaLexer() +
        " (" + it->GetAssociations() + ") |" +
        it->GetAssociations();

      if (wxExMatchesOneOf(filename, it->GetAssociations()))
      {
        wildcards = wildcard + "|" + wildcards;
      }
      else
      {
        wildcards = wildcards + "|" + wildcard;
      }
    }
  }

  return wildcards;
}

const wxExLexer wxExLexers::FindByFileName(const wxFileName& filename) const
{
  if (!filename.IsOk())
  {
    return wxExLexer();
  }

  for (
    std::vector<wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (wxExMatchesOneOf(filename, it->GetAssociations()))
    {
      return *it;
    }
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByName(
  const wxString& name,
  bool fail_if_not_found) const
{
  for (
    std::vector<wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (name == it->GetScintillaLexer())
    {
      return *it;
    }
  }

  if (!m_Lexers.empty() && fail_if_not_found)
  {
    wxFAIL;
  }

  return wxExLexer();
}

const wxExLexer wxExLexers::FindByText(const wxString& text) const
{
  // Add automatic lexers if text starts with some special tokens.
  const wxString text_lowercase = text.Lower();

  if (text_lowercase.StartsWith("#") ||
      // .po files that do not have comment headers, start with msgid, so set them
      text_lowercase.StartsWith(wxT("msgid")))
  {
    return FindByName(wxT("bash"), false); // don't show error
  }
  else if (text_lowercase.StartsWith("<html>") ||
           text_lowercase.StartsWith("<?php"))
  {
    return FindByName(wxT("hypertext"), false); // don't show error
  }
  else if (text_lowercase.StartsWith("<?xml"))
  {
    return FindByName(wxT("xml"), false); // don't show error
  }
  // cpp files like #include <map> really do not have a .h extension (e.g. /usr/include/c++/3.3.5/map)
  // so add here.
  else if (text_lowercase.StartsWith("//"))
  {
    return FindByName(wxT("cpp"), false); // don't show error
  }

  return wxExLexer();
}

wxExLexers* wxExLexers::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExLexers(wxFileName(
#ifdef wxExUSE_PORTABLE
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
      wxStandardPaths::Get().GetUserDataDir()
#endif
      + wxFileName::GetPathSeparator() + "lexers.xml")
    );

    m_Self->Read();
  }

  return m_Self;
}

const wxString wxExLexers::GetLexerAssociations() const
{
  wxString text;

  for (
    std::vector<wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    if (!it->GetAssociations().empty())
    {
      if (!text.empty())
      {
        text += wxExFindReplaceData::Get()->GetFieldSeparator();
      }

      text += it->GetAssociations();
    }
  }

  return text;
}

const wxString wxExLexers::ParseTagColourings(const wxXmlNode* node) const
{
  wxString text;

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == wxT("colouring"))
    {
      text +=
        child->GetPropVal(wxT("no"), "0") + "=" +
        child->GetNodeContent().Strip(wxString::both) + wxTextFile::GetEOL();
    }
    else if (child->GetName() == wxT("comment"))
    {
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined colourings tag: %s",
        child->GetName().c_str());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexers::ParseTagGlobal(const wxXmlNode* node)
{
  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == wxT("comment"))
    {
      // Ignore comments.
    }
    else if (child->GetName() == wxT("hex"))
    {
      m_StylesHex.push_back(
        child->GetPropVal(wxT("no"), "0") + "=" +
        child->GetNodeContent().Strip(wxString::both));
    }
    else if (child->GetName() == wxT("indicator"))
    {
      m_Indicators.insert(std::make_pair(
        atoi(child->GetPropVal(wxT("no"), "0").c_str()),
        atoi(child->GetNodeContent().Strip(wxString::both).c_str())));
    }
    else if (child->GetName() == wxT("marker"))
    {
      const wxExMarker marker(ParseTagMarker(
        child->GetPropVal(wxT("no"), "0"),
        child->GetNodeContent().Strip(wxString::both)));

      if (marker.GetMarkerNumber() < wxSTC_STYLE_MAX &&
          marker.GetMarkerSymbol() < wxSTC_STYLE_MAX)
      {
        m_Markers.push_back(marker);
      }
      else
      {
        wxLogError("Illegal marker number: %d or symbol: %d",
          marker.GetMarkerNumber(),
          marker.GetMarkerSymbol());
      }
    }
    else if (child->GetName() == wxT("style"))
    {
      m_Styles.push_back(
        child->GetPropVal(wxT("no"), "0") + "=" +
        child->GetNodeContent().Strip(wxString::both));
    }
    else
    {
      wxLogError("Undefined global tag: %s",
        child->GetName().c_str());
    }

    child = child->GetNext();
  }
}

const wxExLexer wxExLexers::ParseTagLexer(const wxXmlNode* node) const
{
  wxExLexer lexer;
  lexer.m_ScintillaLexer = node->GetPropVal(wxT("name"), wxT(""));
  lexer.m_Associations = node->GetPropVal(wxT("extensions"), wxT(""));

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == wxT("colourings"))
    {
      lexer.m_Colourings = ParseTagColourings(child);
    }
    else if (child->GetName() == wxT("keywords"))
    {
      if (!lexer.SetKeywords(child->GetNodeContent().Strip(wxString::both)))
      {
        wxLogError("Keywords could not be set");
      }
    }
    else if (child->GetName() == wxT("properties"))
    {
      lexer.m_Properties = ParseTagProperties(child);
    }
    else if (child->GetName() == wxT("comments"))
    {
      lexer.m_CommentBegin = child->GetPropVal("begin1", wxT(""));
      lexer.m_CommentEnd = child->GetPropVal("end1", wxT(""));
      lexer.m_CommentBegin2 = child->GetPropVal("begin2", wxT(""));
      lexer.m_CommentEnd2 = child->GetPropVal("end2", wxT(""));
    }
    else if (child->GetName() == wxT("comment"))
    {
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined lexer tag: %s",
        child->GetName().c_str());
    }

    child = child->GetNext();
  }

  return lexer;
}

const wxExMarker wxExLexers::ParseTagMarker(
  const wxString& number,
  const wxString& props) const
{
  wxStringTokenizer prop_fields(props, ",");

  const wxString symbol = prop_fields.GetNextToken();

  wxColour foreground;
  wxColour background;

  if (prop_fields.HasMoreTokens())
  {
    foreground = prop_fields.GetNextToken();

    if (prop_fields.HasMoreTokens())
    {
      background = prop_fields.GetNextToken();
    }
  }

  const int no = atoi(number.c_str());
  const int symbol_no = atoi(symbol.c_str());

  if (no <= wxSTC_MARKER_MAX && symbol_no <= wxSTC_MARKER_MAX)
  {
    return wxExMarker(no, symbol_no, foreground, background);
  }
  else
  {
    wxLogError("Illegal marker number: %d or symbol: %d", no, symbol_no);
    return wxExMarker(0, 0, foreground, background);
  }
}

const wxString wxExLexers::ParseTagProperties(const wxXmlNode* node) const
{
  wxString text;

  wxXmlNode *child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == wxT("property"))
    {
      text +=
        child->GetPropVal(wxT("name"), "0") + "=" +
        child->GetNodeContent().Strip(wxString::both) + wxTextFile::GetEOL();
    }
    else if (child->GetName() == wxT("comment"))
    {
      // Ignore comments.
    }
    else
    {
      wxLogError("Undefined properties tag: %s",
        child->GetName().c_str());
    }

    child = child->GetNext();
  }

  return text;
}

void wxExLexers::Read()
{
  // This test is to prevent showing an error if the lexers file does not exist,
  // as this is not required.
  if (!m_FileName.FileExists())
  {
    return;
  }

  wxXmlDocument doc;

  if (!doc.Load(m_FileName.GetFullPath()))
  {
    return;
  }

  // Initialize members.
  m_Indicators.clear();
  m_Lexers.clear();
  m_Markers.clear();
  m_Styles.clear();
  m_StylesHex.clear();

  wxXmlNode* child = doc.GetRoot()->GetChildren();

  while (child)
  {
    if (child->GetName() == wxT("global"))
    {
      ParseTagGlobal(child);
    }
    else if (child->GetName() == wxT("lexer"))
    {
      const wxExLexer& lexer = ParseTagLexer(child);

      if (!lexer.GetScintillaLexer().empty())
      {
        if (lexer.GetScintillaLexer() == wxT("hypertext"))
        {
          // As our lexers.xml files cannot use xml comments,
          // add them here.
          wxExLexer l(lexer);
          l.m_CommentBegin = "<!--";
          l.m_CommentEnd = "-->";
          m_Lexers.push_back(l);
        }
        else
        {
          m_Lexers.push_back(lexer);
        }
      }
    }

    child = child->GetNext();
  }

  if (!wxConfigBase::Get()->Exists(_("In files")))
  {
    wxConfigBase::Get()->Write(_("In files"), GetLexerAssociations());
  }

  if (!wxConfigBase::Get()->Exists(_("Add what")))
  {
    wxConfigBase::Get()->Write(_("Add what"), GetLexerAssociations());
  }
}

wxExLexers* wxExLexers::Set(wxExLexers* lexers)
{
  wxExLexers* old = m_Self;
  m_Self = lexers;
  return old;
}

bool wxExLexers::ShowDialog(
  wxWindow* parent,
  wxExLexer& lexer,
  const wxString& caption) const
{
  wxArrayString aChoices;
  int choice = -1;
  int index = 0;

  for (
    std::vector<wxExLexer>::const_iterator it = m_Lexers.begin();
    it != m_Lexers.end();
    ++it)
  {
    aChoices.Add(it->GetScintillaLexer());

    if (lexer.GetScintillaLexer() == it->GetScintillaLexer())
    {
      choice = index;
    }

    index++;
  }

  // Add the <none> lexer (index is already incremented).
  const wxString no_lexer = _("<none>");

  aChoices.Add(no_lexer);

  // And set the choice if we do not have a lexer.
  if (lexer.GetScintillaLexer().empty())
  {
    choice = index;
  }

  wxSingleChoiceDialog dlg(parent, _("Input") + wxString(":"), caption, aChoices);

  if (choice != -1)
  {
    dlg.SetSelection(choice);
  }

  if (dlg.ShowModal() == wxID_CANCEL)
  {
    return false;
  }

  const wxString sel = dlg.GetStringSelection();

  if (sel == no_lexer)
  {
    lexer = wxExLexer();
  }
  else
  {
    lexer = FindByName(sel);
  }

  return true;
}