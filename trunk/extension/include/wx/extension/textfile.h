/******************************************************************************\
* File:          textfile.h
* Purpose:       Declaration of exTextFile class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTEXTFILE_H
#define _EXTEXTFILE_H

#include <wx/datetime.h>
#include <wx/textfile.h>
#include <wx/extension/statistics.h>

/// Adds file tool methods to wxTextFile.
/// In your derived class just implement the Report or ReportStatistics, and take
/// care that the strings are added to your component.
class exTextFile : public wxTextFile
{
public:
  /// Constructor.
  exTextFile(const exFileName& filename);

  /// Gets the description.
  const wxString& GetDescription() const {return m_Description;};

  /// Gets the filename.
  const exFileName& GetFileName() const {return m_FileNameStatistics;};

  /// Gets the revision format.
  const wxString& GetRevisionFormat() const {return m_RevisionFormat;};

  /// Gets the revision number.
  const wxString& GetRevisionNumber() const {return m_RevisionNumber;};

  /// Gets the revision time.
  const wxDateTime& GetRevisionTime() const {return m_RevisionTime;};

  /// Gets the statistics.
  const exFileNameStatistics& GetStatistics() const {return m_FileNameStatistics;}

  /// Gets the tool.
  static exTool& GetTool() {return m_Tool;};

  /// Gets the user.
  const wxString& GetUser() const {return m_User;};

  /// Inserts a line at current line (or at end if at end),
  /// make that line current and sets modified.
  void InsertLine(const wxString& line);

  /// Opens the file and runs the tool.
  bool RunTool();

  /// Sets the tool as specified and calls a dialog if necessary, afterwards
  /// you have to call RunTool. If you use tool find or replace in files,
  /// be sure to set the exFindReplaceData.
  static bool SetupTool(const exTool& tool);

  /// Writes a comment to the current line.
  void WriteComment(
    const wxString& text,
    const bool fill_out = false,
    const bool fill_out_with_space = false);
protected:
  // Interface.
  /// If it returns true, the operation is cancelled.
  virtual bool Cancelled() {return false;};

  /// Called after comments have been found.
  virtual bool ParseComments();

  // Virtual report generators.
  // The default reports use a wxLogMessage.
  /// This one is invoked during parsing of lines.
  virtual void Report();

  /// This one is invoked for ID_TOOL_LINE_CODE,
  /// for each line that contains code, or
  /// for ID_TOOL_LINE_COMMENTS for each line that contains a comment.
  /// Default reports the lines using a wxLogMessage.
  virtual void ReportLine(const wxString& line);

  /// Sets the tool directly (without calling any dialogs).
  static void SetTool(const exTool& tool) {m_Tool = tool;};
public:
  /// This one is invoked at the end, when statistics are completed.
  /// It is made public, as it can be useful from outside.
  virtual void ReportStatistics();
protected:
  /// Clears the comments.
  void ClearComments() {m_Comments.clear();}

  /// Your derived class is allowed to update statistics.
  exStatistics<long>& GetStatisticElements() {return m_FileNameStatistics.GetElements();};

  /// Your derived class is allowed to update statistics.
  exStatistics<long>& GetStatisticKeywords() {return m_FileNameStatistics.GetKeywords();};

  /// Gets the current comments.
  const wxString& GetComments() const {return m_Comments;};

  /// Parses the specified line, and invokes actions depending on the tool,
  /// and fills the comments if any on the line.
  /// At the end it calls ParseComments.
  bool ParseLine(const wxString& line);
private:
  void CommentStatementEnd();
  void CommentStatementStart();
  void EndCurrentRevision();
  const wxString GetNextRevisionNumber();
  bool HeaderDialog();
  void Initialize();
  bool MatchLine(wxString& line);
  bool ParseForHeader();
  bool ParseForOther();
  void ParseHeader();
  bool PrepareRevision();
  void ProcessFormattedText(
    const wxString& lines,
    const wxString& header,
    bool is_comment);
  void ProcessUnFormattedText(
    const wxString& lines,
    const wxString& header,
    bool is_comment);
  void RevisionAddComments(const wxString& m_FileNameStatistics);
  bool RevisionDialog();
  bool WriteFileHeader();
  void WriteTextWithPrefix(
    const wxString& text,
    const wxString& prefix,
    bool is_comment = true);

  bool m_AllowAction;
  bool m_DialogShown;
  bool m_EmptyLine;
  bool m_FinishedAction;
  bool m_IsCommentStatement;
  bool m_IsString;
  bool m_Modified;
  bool m_RevisionActive;

  static exTool m_Tool;

  exFileNameStatistics m_FileNameStatistics;

  size_t m_LineMarker;
  size_t m_LineMarkerEnd;
  size_t m_VersionLine;

  wxDateTime m_RevisionTime;

  wxString m_Author;
  wxString m_Comments;
  wxString m_Description;
  wxString m_RevisionFormat;
  wxString m_RevisionNumber;
  wxString m_User;
};
#endif
