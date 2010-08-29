////////////////////////////////////////////////////////////////////////////////
// Name:      vcsentry.h
// Purpose:   Declaration of wxExVCSEntry class
// Author:    Anton van Wezenbeek
// Created:   2010-08-27
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVCSENTRY_H
#define _EXVCSENTRY_H

#include <vector>
#include <wx/xml/xml.h>

class wxMenu;

/// This class contains a single vcs command.
class wxExVCSCommand
{
public:
  enum
  {
    VCS_COMMAND_IS_BOTH,
    VCS_COMMAND_IS_POPUP,
    VCS_COMMAND_IS_MAIN,
    VCS_COMMAND_IS_UNKNOWN,
  };
  
  /// Default constructor.
  wxExVCSCommand();

  /// Constructor.
  wxExVCSCommand(
    const wxString& command,
    const wxString& type = wxEmptyString);

  /// Gets the command.
  const wxString GetCommand() const {return m_Command;};
  
  /// Gets the no.
  int GetNo() const {return m_No;};

  /// Gets the type.
  int GetType() const {return m_Type;};
  
  /// Returns true if this is a add like command.
  bool IsAdd() const;

  /// Returns true if this is a commit like command.
  bool IsCommit() const;

  /// Returns true if this is a diff like command.
  bool IsDiff() const;

  /// Returns true if this is a help like command.
  bool IsHelp() const;

  /// Returns true if this command can behave like
  /// opening a file.
  bool IsOpen() const;

  /// Returns true if this is a update like command.
  bool IsUpdate() const;
  
  /// Resets the number of instances, so the no
  /// will start from 0 again.
  static void ResetInstances() {m_Instances = 0;};

  /// Sets the command and type.
  void SetCommand(
    const wxString& command,
    const wxString& type = wxEmptyString) {
    m_Command = command;
    m_Type = From(type);};
private:
  int From(const wxString& type) const;
  wxString m_Command;
  
  static int m_Instances;
  
  int m_No;
  int m_Type;
};

/// This class collects a single vcs.
class wxExVCSEntry
{
public:
  /// Default constructor.
  wxExVCSEntry();
  
  /// Constructor using xml node.
  wxExVCSEntry(const wxXmlNode* node);

#if wxUSE_GUI
  /// Builds a menu, default assumes it is a popup menu.
  void BuildMenu(int base_id, wxMenu* menu, bool is_popup = true) const;
#endif
  
  /// Gets the command.
  const wxString GetCommand(int command_id) const;

  /// Gets the name.
  const wxString& GetName() const {return m_Name;};

  /// Gets the no.
  long GetNo() const {return m_No;};
private:
  const std::vector<wxExVCSCommand> ParseNodeCommands(
    const wxXmlNode* node) const;

  static int m_Instances;

  const wxString m_Name;
  const long m_No;

  std::vector<wxExVCSCommand> m_Commands;
};
#endif