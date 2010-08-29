////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.cpp
// Purpose:   Implementation of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Created:   2010-08-27
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/menu.h>
#include <wx/extension/vcsentry.h>
#include <wx/extension/defs.h>
#include <wx/extension/util.h>

int wxExVCSCommand::m_Instances = 0;

wxExVCSCommand::wxExVCSCommand()
  : m_Command()
  , m_No(0)
  , m_Type(VCS_COMMAND_IS_UNKNOWN) 
{
}

wxExVCSCommand::wxExVCSCommand(
  const wxString& command,
  const wxString& type)
  : m_Command(command)
  , m_No(m_Instances++)
  , m_Type(From(type))
{
}
  
int wxExVCSCommand::From(const wxString& type) const
{
  if (type.IsEmpty())
  {
    return VCS_COMMAND_IS_BOTH;
  }
  else if (type == "popup")
  {
    return VCS_COMMAND_IS_POPUP;
  }
  else if (type == "main")
  {
    return VCS_COMMAND_IS_MAIN;
  }
  else
  {
    return VCS_COMMAND_IS_UNKNOWN;
  }
}

bool wxExVCSCommand::IsAdd() const
{
  return 
    m_Command == "add";
}

bool wxExVCSCommand::IsCommit() const
{
  return 
    m_Command == "commit" ||
    m_Command == "co";
}

bool wxExVCSCommand::IsDiff() const
{
  return 
    m_Command == "diff";
}

bool wxExVCSCommand::IsHelp() const
{
  return 
    m_Command == "help";
}

bool wxExVCSCommand::IsOpen() const
{
  return
    m_Command == "blame" ||
    m_Command == "cat" ||
    m_Command == "diff";
}

bool wxExVCSCommand::IsUpdate() const
{
  return 
    m_Command == "update";
}

int wxExVCSEntry::m_Instances = 2; // TODO: VCS_AUTO + 1;

wxExVCSEntry::wxExVCSEntry()
  : m_No(-1)
  , m_Name()
{
}

wxExVCSEntry::wxExVCSEntry(const wxXmlNode* node)
  : m_No(m_Instances++)
  , m_Name(node->GetAttribute("name"))
{
  if (m_Name.empty())
  {
    wxLogError(_("Missing vcs on line: %d"), node->GetLineNumber());
  }
  else
  {
    wxXmlNode *child = node->GetChildren();

    while (child)
    {
      if (child->GetName() == "commands")
      {
        m_Commands = ParseNodeCommands(child);
      }
      else if (child->GetName() == "comment")
      {
        // Ignore comments.
      }
      else
      {
        wxLogError(_("Undefined tag: %s on line: %d"),
          child->GetName().c_str(),
          child->GetLineNumber());
      }

      child = child->GetNext();
    }
  }
}

#if wxUSE_GUI
void wxExVCSEntry::BuildMenu(int base_id, wxMenu* menu, bool is_popup) const
{
  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    ++it)
  {
    bool add = false;

    switch (it->GetType())
    {
      case wxExVCSCommand::VCS_COMMAND_IS_BOTH: add = true; break;
      case wxExVCSCommand::VCS_COMMAND_IS_POPUP: add = is_popup; break;
      case wxExVCSCommand::VCS_COMMAND_IS_MAIN: add = !is_popup; break;
      case wxExVCSCommand::VCS_COMMAND_IS_UNKNOWN: add = false; break;
      default: wxFAIL;
    }

    if (add)
    {
      const wxString text(wxExEllipsed("&" + it->GetCommand()));
      menu->Append(base_id + it->GetNo(), text);
    }
  }
}
#endif

const wxString wxExVCSEntry::GetCommand(int command_id) const
{
  return m_Commands.at(command_id).GetCommand();
}
  
const std::vector<wxExVCSCommand> wxExVCSEntry::ParseNodeCommands(
  const wxXmlNode* node) const
{
  std::vector<wxExVCSCommand> v;
  
  wxExVCSCommand::ResetInstances();

  wxXmlNode* child = node->GetChildren();

  while (child)
  {
    if (child->GetName() == "command")
    {
      if (v.size() == VCS_MAX_COMMANDS)
      {
        wxLogError(_("Reached commands limit: %d"), VCS_MAX_COMMANDS);
      }
      else
      {
        const wxString content = child->GetNodeContent().Strip(wxString::both);
        const wxString attrib = child->GetAttribute("type");
        v.push_back(wxExVCSCommand(content, attrib));
      }
    }
    else if (child->GetName() == "comment")
    {
      // Ignore comments.
    }
    else
    {
      wxLogError(_("Undefined tag: %s on line: %d"),
        child->GetName().c_str(),
        child->GetLineNumber());
    }

    child = child->GetNext();
  }

  return v;
}