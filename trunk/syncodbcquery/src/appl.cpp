/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of classes for syncodbcquery
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aboutdlg.h>
#include <wx/regex.h>
#include <wx/tokenzr.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/grid.h>
#include <wx/extension/shell.h>
#include <wx/extension/version.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/stc.h>
#include <wx/extension/report/util.h>
#include "appl.h"

#ifndef __WXMSW__
#include "appl.xpm"
#endif

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
  SetAppName("syncodbcquery");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  MyFrame *frame = new MyFrame();
  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

BEGIN_EVENT_TABLE(MyFrame, wxExFrameWithHistory)
  EVT_CLOSE(MyFrame::OnClose)
  EVT_MENU(wxID_EXECUTE, MyFrame::OnCommand)
  EVT_MENU(wxID_STOP, MyFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND, MyFrame::OnCommand)
  EVT_MENU(ID_SHELL_COMMAND_STOP, MyFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, MyFrame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_PREFERENCES, MyFrame::OnCommand)
  EVT_MENU_RANGE(ID_FIRST, ID_LAST, MyFrame::OnCommand)
  EVT_UPDATE_UI(wxID_SAVE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_SAVEAS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_STOP, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_DATABASE_CLOSE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_DATABASE_OPEN, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(wxID_EXECUTE, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_RECENTFILE_MENU, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_QUERY, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_RESULTS, MyFrame::OnUpdateUI)
  EVT_UPDATE_UI(ID_VIEW_STATISTICS, MyFrame::OnUpdateUI)
END_EVENT_TABLE()

MyFrame::MyFrame()
  : wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppName())
  , m_Running(false)
  , m_Stopped(false)
{
  SetIcon(wxICON(appl));

  wxExMenu* menuFile = new wxExMenu;
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu* menuDatabase = new wxExMenu;
  menuDatabase->Append(ID_DATABASE_OPEN, wxExEllipsed(_("&Open")));
  menuDatabase->Append(ID_DATABASE_CLOSE, _("&Close"));

  wxExMenu* menuQuery = new wxExMenu;
  menuQuery->Append(wxID_EXECUTE);
  menuQuery->Append(wxID_STOP);

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(wxID_PREFERENCES);

  wxMenu* menuView = new wxMenu();
  menuView->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));
  menuView->AppendCheckItem(ID_VIEW_TOOLBAR, _("&Toolbar"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_QUERY, _("Query"));
  menuView->AppendCheckItem(ID_VIEW_RESULTS, _("Results"));
  menuView->AppendCheckItem(ID_VIEW_STATISTICS, _("Statistics"));

  wxMenu* menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuDatabase, _("&Connection"));
  menubar->Append(menuQuery, _("&Query"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  SetMenuBar(menubar);

  m_Query = new wxExSTCWithFrame(this);
  m_Query->SetLexer("sql");

  m_Results = new wxExGrid(this);
  m_Results->CreateGrid(0, 0);
  m_Results->EnableEditing(false); // this is a read-only grid

  m_Shell = new wxExSTCShell(this, ">", ";", true, 50);
  m_Shell->SetFocus();

  GetManager().AddPane(m_Shell,
    wxAuiPaneInfo().
      Name("CONSOLE").
      CenterPane());

  GetManager().AddPane(m_Results,
    wxAuiPaneInfo().
      Name("RESULTS").
      Caption(_("Results")).
      CloseButton(true).
      MaximizeButton(true));

  GetManager().AddPane(m_Query,
    wxAuiPaneInfo().
      Name("QUERY").
      Caption(_("Query")).
      CloseButton(true).
      MaximizeButton(true));

  GetManager().AddPane(m_Statistics.Show(this),
    wxAuiPaneInfo().Left().
      MaximizeButton(true).
      Caption(_("Statistics")).
      Name("STATISTICS"));

  GetManager().LoadPerspective(wxExApp::GetConfig("Perspective"));
  GetManager().GetPane("QUERY").Show(false);

  GetManager().Update();

  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("PaneLines", 100, _("Lines in window")));
  SetupStatusBar(panes);

  CreateToolBar();

  m_ToolBar->AddTool(wxID_NEW);
  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->AddTool(wxID_SAVE);

#ifdef __WXGTK__
  m_ToolBar->AddTool(wxID_EXECUTE);
#endif

  m_ToolBar->Realize();
}

void MyFrame::ConfigDialogApplied(wxWindowID dialogid)
{
  if (dialogid == wxID_PREFERENCES)
  {
    m_Query->ConfigGet();
    m_Shell->ConfigGet();
  }
  else
  {
    wxFAIL;
  }
}

wxExGrid* MyFrame::GetGrid()
{
  if (m_Results->IsShown())
  {
    return m_Results;
  }
  else
  {
    const wxExGrid* grid = m_Statistics.GetGrid();

    if (grid != NULL && grid->IsShown())
    {
      return (wxExGrid*)grid;
    }
    else
    {
      return NULL;
    }
  }
}

wxExSTC* MyFrame::GetSTC()
{
  if (m_Query->IsShown())
  {
    return m_Query;
  }
  else if (m_Shell->IsShown())
  {
    return m_Shell;
  }

  return NULL;
}

void MyFrame::OnClose(wxCloseEvent& event)
{
  wxExFileDialog dlg(this, m_Query);

  if (!dlg.Continue())
  {
    return;
  }

  wxConfigBase::Get()->Write("Perspective", GetManager().SavePerspective());

  event.Skip();
}

void MyFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetDescription(_("This program offers a general ODBC query."));
    info.SetVersion("v1.0.0");
    info.SetCopyright("(c) 2008-2009, Anton van Wezenbeek");
    info.AddDeveloper(wxVERSION_STRING);
    info.AddDeveloper(wxEX_VERSION_STRING);
    info.AddDeveloper(wxExOTL::Version());
    wxAboutBox(info);
    }
    break;

  case wxID_EXECUTE:
    m_Stopped = false;
    RunQueries(m_Query->GetText());
    break;

  case wxID_EXIT:
    Close(true);
    break;

  case wxID_NEW:
    m_Query->FileNew();
    m_Query->SetLexer("sql");
    m_Query->SetFocus();
    GetManager().GetPane("QUERY").Show();
    GetManager().Update();
    break;

  case wxID_OPEN:
    wxExOpenFilesDialog(this, wxFD_OPEN | wxFD_CHANGE_DIR, "sql files (*.sql) | *.sql", true);
    break;

  case wxID_PREFERENCES:
    event.Skip();
    break;

  case wxID_SAVE:
    m_Query->FileSave();
    break;

  case wxID_SAVEAS:
    {
      wxExFileDialog dlg(this, m_Query, _("File Save As"), wxFileSelectorDefaultWildcardStr, wxFD_SAVE);
      if (dlg.ShowModal(false) == wxID_OK)
      {
         m_Query->FileSave(dlg.GetPath());
      }
    }
    break;

  case wxID_STOP:
    m_Running = false;
    m_Stopped = true;
    break;

  case ID_DATABASE_CLOSE:
    m_otl.Logoff();
    m_Shell->SetPrompt(">");
    break;

  case ID_DATABASE_OPEN:
    m_otl.Logon(this, wxConfigBase::Get());
    m_Shell->SetPrompt((m_otl.IsConnected() ? wxExApp::GetConfig(_("Datasource")): "") + ">");
    break;

  case ID_SHELL_COMMAND:
    if (m_otl.IsConnected())
    {
      try
      {
        const wxString query = event.GetString().substr(
          0,
          event.GetString().length() - 1);

        m_Stopped = false;
        RunQuery(query, true);
      }
      catch (otl_exception& p)
      {
        if (m_Results->IsShown())
        {
          m_Results->EndBatch();
        }

        m_Shell->AppendText(_("\nerror: ") + wxExQuoted(p.msg));
      }
    }
    else
    {
      m_Shell->AppendText(_("\nnot connected"));
    }

    m_Shell->Prompt();
    break;

  case ID_SHELL_COMMAND_STOP:
    m_Stopped = true;
    m_Shell->Prompt(_("cancelled"));
    break;

  case ID_VIEW_QUERY: TogglePane("QUERY"); break;
  case ID_VIEW_RESULTS: TogglePane("RESULTS"); break;
  case ID_VIEW_STATISTICS: TogglePane("STATISTICS"); break;

  default: 
    wxFAIL;
  }
}

void MyFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
  case wxID_EXECUTE:
    // If we have a query, you can hide it, but still run it.
    event.Enable(m_Query->GetLength() > 0 && m_otl.IsConnected());
    break;

  case wxID_SAVE:
    event.Enable(m_Query->GetModify());
    break;

  case wxID_SAVEAS:
    event.Enable(m_Query->GetLength() > 0);
    break;

  case wxID_STOP:
    event.Enable(m_Running);
    break;

  case ID_DATABASE_CLOSE:
    event.Enable(m_otl.IsConnected());
    break;

  case ID_DATABASE_OPEN:
    event.Enable(!m_otl.IsConnected());
    break;

  case ID_RECENTFILE_MENU:
    event.Enable(!GetRecentFile().empty());
    break;

  case ID_VIEW_QUERY:
    event.Check(GetManager().GetPane("QUERY").IsShown());
    break;

  case ID_VIEW_RESULTS:
    event.Check(GetManager().GetPane("RESULTS").IsShown());
    break;

  case ID_VIEW_STATISTICS:
    event.Check(GetManager().GetPane("STATISTICS").IsShown());
    break;

  default:
    wxFAIL;
  }
}

bool MyFrame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  GetManager().GetPane("QUERY").Show(true);
  GetManager().Update();

  // Take care that wxExOpenFilesDialog always results in opening in the query.
  // Otherwise if results are focused, the file is opened in the results.
  return m_Query->Open(filename, line_number, match, flags);
}

void MyFrame::RunQuery(const wxString& query, bool empty_results)
{
  wxStopWatch sw;

  const wxString query_lower = query.Lower();

  // Query functions supported by ODBC
  // $SQLTables, $SQLColumns, etc.
  // $SQLTables $1:'%'
  // allow you to get database schema.
  if (query_lower.StartsWith("select") ||
      query_lower.StartsWith("describe") ||
      query_lower.StartsWith("show") ||
      query_lower.StartsWith("explain") ||
      query_lower.StartsWith("$sql"))
  {
    long rpc;

    if (m_Results->IsShown())
    {
      rpc = m_otl.Query(query, m_Results, m_Stopped, empty_results);
    }
    else
    {
      rpc = m_otl.Query(query, m_Shell, m_Stopped);
    }

    sw.Pause();

    UpdateStatistics(sw, rpc);
  }
  else
  {
    const long rpc = m_otl.Query(query);

    sw.Pause();

    UpdateStatistics(sw, rpc);
  }

  m_Shell->DocumentEnd();
}

void MyFrame::RunQueries(const wxString& text)
{
  if (m_Results->IsShown())
  {
    m_Results->ClearGrid();
  }

  // Skip sql comments.
  wxString output = text;
  wxRegEx("--.*$", wxRE_NEWLINE).ReplaceAll(&output, "");

  // Queries are seperated by ; character.
  wxStringTokenizer tkz(output, ";");
  int no_queries = 0;

  wxStopWatch sw;
  m_Running = true;

  // Run all queries.
  while (tkz.HasMoreTokens() && !m_Stopped)
  {
    wxString query = tkz.GetNextToken();
    query.Trim(true);
    query.Trim(false);

    if (!query.empty())
    {
      try
      {
        RunQuery(query, no_queries == 0);
        no_queries++;
        wxTheApp->Yield();
      }
      catch (otl_exception& p)
      {
        m_Statistics.Inc(_("Number of query errors"));
        m_Shell->AppendText(
          _("\nerror: ") +  wxExQuoted(p.msg) + 
          _(" in: ") + wxExQuoted(query));
      }
    }
  }

  m_Shell->Prompt(wxString::Format(_("\n%d queries (%.3f seconds)"),
    no_queries,
    (float)sw.Time() / (float)1000));

  m_Running = false;
}

void MyFrame::UpdateStatistics(const wxStopWatch& sw, long rpc)
{
  m_Shell->AppendText(wxString::Format(_("\n%d rows processed (%.3f seconds)"),
    rpc,
    (float)sw.Time() / (float)1000));

  m_Statistics.Set(_("Rows processed"), rpc);
  m_Statistics.Set(_("Query runtime"), sw.Time());

  m_Statistics.Inc(_("Total number of queries run"));
  m_Statistics.Inc(_("Total query runtime"), sw.Time());
  m_Statistics.Inc(_("Total rows processed"), rpc);
}
