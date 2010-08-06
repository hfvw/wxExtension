/******************************************************************************\
* File:          frame.h
* Purpose:       Declaration of class 'Frame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FRAME_H
#define _FRAME_H

#include <wx/generic/dirctrlg.h>
#include <wx/extension/notebook.h>
#include "support.h"

/// Adds event handling and implements virtual methods.
class Frame : public DecoratedFrame
{
public:
  /// Constructor.
  Frame(bool open_recent);
  
  /// Opens the file.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);
protected:
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnTree(wxTreeEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  virtual wxExListViewStandard* Activate(
    wxExListViewStandard::ListType type, 
    const wxExLexer* lexer = NULL);
  wxExListViewWithFrame* AddPage(
    wxExListViewStandard::ListType type, 
    const wxExLexer* lexer = NULL);
  bool AllowCloseAll(wxWindowID id);
  bool DialogProjectOpen();
  virtual wxExListView* GetListView();
  virtual wxExListViewFile* GetProject();
  virtual wxExSTC* GetSTC();
  void NewFile(bool as_project = false);
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxString& unique,
    const wxString& contents,
    long flags = 0);
  virtual void SyncCloseAll(wxWindowID id);

  int m_NewFileNo;
  int m_NewProjectNo;

  wxGenericDirCtrl* m_DirCtrl;
  wxExListViewWithFrame* m_History;
  wxExNotebook* m_NotebookWithEditors;
  wxExNotebook* m_NotebookWithLists;
  wxExNotebook* m_NotebookWithProjects;

  const wxString m_ProjectWildcard;

  DECLARE_EVENT_TABLE()
};
#endif
