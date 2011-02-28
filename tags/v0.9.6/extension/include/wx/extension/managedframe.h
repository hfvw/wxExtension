////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Created:   2010-04-11
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMANAGEDFRAME_H
#define _EXMANAGEDFRAME_H

#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxStaticText;
class wxExTextCtrl;
class wxExToolBar;
class wxExVi;

#if wxUSE_AUI
/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar, findbar and vibar support.
class WXDLLIMPEXP_BASE wxExManagedFrame : public wxExFrame
{
public:
  /// Constructor, registers the aui manager, and creates the bars.
  wxExManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE);

  /// Destructor, uninits the aui manager.
 ~wxExManagedFrame();

  // Interface for notebooks.
  /// Returns true if the page can be closed.
  virtual bool AllowClose(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {return true;}

  /// Called if the notebook changed page.
  virtual void OnNotebook(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {;};

  /// Called after all pages from the notebooks are deleted.
  virtual void SyncCloseAll(wxWindowID WXUNUSED(id)) {;};

  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};
  
  /// Gets a command line vi command.
  void GetViCommand(wxExVi* vi, const wxString& command);
  
  /// Hides the vi bar.
  void HideViBar();

  /// Shows text in vi bar.
  void ShowViMessage(const wxString& text);
  
  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  void TogglePane(const wxString& pane);
protected:
  /// Add controls to specified toolbar.
  virtual void DoAddControl(wxExToolBar*) {;};

  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  bool AddToolBarPane(wxWindow* window, const wxString& name, const wxString& caption);
  void CreateViPanel(
    wxStaticText*& statictext,
    wxExTextCtrl*& text,
    const wxString& name);
  void GetViPaneCommand(
    wxStaticText* statictext,
    wxExTextCtrl* text,
    const wxString& pane,
    wxExVi* vi,
    const wxString& command);
  
  wxAuiManager m_Manager;

  wxExTextCtrl* m_viCommand;
  wxStaticText* m_viCommandPrefix;
  wxExTextCtrl* m_viFind;
  wxStaticText* m_viFindPrefix;
  
  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_AUI
#endif // wxUSE_GUI
#endif