/******************************************************************************\
* File:          app.h
* Purpose:       Declaration of sample classes for wxExtension
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/app.h>
#include <wx/extension/frame.h>
#include <wx/extension/statistics.h>
#include <wx/extension/dir.h>
#include <wx/extension/grid.h>
#include <wx/extension/listview.h>
#include <wx/extension/notebook.h>
#include <wx/extension/stc.h>
#include <wx/extension/shell.h>

/// Derive your application from wxExApp.
class wxExSampleApp: public wxExApp
{
public:
  /// Constructor.
  wxExSampleApp() {}
private:
  /// Override the OnInit.
  virtual bool OnInit();
  DECLARE_NO_COPY_CLASS(wxExSampleApp)
};

#if wxUSE_GRID
/// Use wxExDir.
class wxExSampleDir: public wxExDir
{
public:
  /// Constructor.
  wxExSampleDir(
    const wxString& fullpath, 
    const wxString& findfiles, 
    wxExGrid* grid);
private:
  /// Override the OnFile.
  virtual void OnFile(const wxString& file);
  wxExGrid* m_Grid; ///< put it in a grid
};
#endif

/// Use wxExManagedFrame.
class wxExSampleFrame: public wxExManagedFrame
{
public:
  /// Constructor.
  wxExSampleFrame();
protected:
  virtual void OnCommandConfigDialog(
    wxWindowID id, 
    int commandid = wxID_APPLY);
  /// Do something.
  void OnCommand(wxCommandEvent& event);
private:
  virtual wxExGrid* GetGrid();
  virtual wxExListView* GetListView();
  virtual wxExSTC* GetSTC();
  void ShowConfigItems();
  
#if wxUSE_GRID
  wxExGrid* m_Grid;         ///< a grid
#endif
  wxExListView* m_ListView; ///< a listview
  wxExNotebook* m_Notebook; ///< a notebook
  wxExSTCShell* m_STCShell; ///< an stc shell
  wxExSTC* m_STC;           ///< an stc
  wxExSTC* m_STCLexers;     ///< an stc

  long m_FlagsSTC;          ///< keep current flags
  wxExStatistics <long> m_Statistics; ///< keep some statistics

  DECLARE_NO_COPY_CLASS(wxExSampleFrame)
  DECLARE_EVENT_TABLE()
};
