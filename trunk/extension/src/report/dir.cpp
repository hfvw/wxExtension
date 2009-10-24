/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxExtension report utility functions and classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: util.cpp 1958 2009-10-24 08:59:50Z antonvw $
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/configdlg.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/report/report.h>

bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2)
{
  if (wxConfigBase::Get()->Read(_("Comparator")).empty())
  {
    return false;
  }

  const wxString arguments =
     (file1.GetModificationTime() < file2.GetModificationTime()) ?
       "\"" + file1.GetFullPath() + "\" \"" + file2.GetFullPath() + "\"":
       "\"" + file2.GetFullPath() + "\" \"" + file1.GetFullPath() + "\"";

  if (wxExecute(wxConfigBase::Get()->Read(_("Comparator")) + " " + arguments) == 0)
  {
    return false;
  }

  const wxString msg = _("Compared") + ": " + arguments;
  wxExApp::Log(msg);
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif

  return true;
}

void wxExFindInFiles(bool replace)
{
  if (wxExDir::GetIsBusy())
  {
    wxExDir::Cancel();
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Cancelled previous find files"));
#endif
  }

  std::vector<wxExConfigItem> v;
  v.push_back(
    wxExConfigItem(wxExApp::GetFindReplaceData()->GetTextFindWhat(), 
    CONFIG_COMBOBOX, 
    wxEmptyString, 
    true));

  if (replace) 
  {
    v.push_back(wxExConfigItem(
      wxExApp::GetFindReplaceData()->GetTextReplaceWith(), 
      CONFIG_COMBOBOX));
  }

  v.push_back(wxExConfigItem(_("In files"), CONFIG_COMBOBOX, wxEmptyString, true));
  v.push_back(wxExConfigItem(_("In folder"), CONFIG_COMBOBOXDIR, wxEmptyString, true));
  v.push_back(wxExConfigItem());
  v.push_back(wxExConfigItem(wxExApp::GetFindReplaceData()->GetInfo()));

  if (wxExConfigDialog(NULL,
    v,
    (replace ? _("Replace In Files"): _("Find In Files"))).ShowModal() == wxID_CANCEL)
  {
    return;
  }

  const wxExTool tool =
    (replace ?
       ID_TOOL_REPORT_REPLACE:
       ID_TOOL_REPORT_FIND);

  if (!wxExTextFileWithListView::SetupTool(tool))
  {
    return;
  }

  wxExApp::Log(wxExApp::GetFindReplaceData()->GetText(replace));

  wxExDirWithListView dir(
    tool,
    wxConfigBase::Get()->Read(_("In folder")),
    wxConfigBase::Get()->Read(_("In files")));

  dir.FindFiles();
  dir.GetStatistics().Log(tool);
}

bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListViewFile* listview,
  wxFileName* lastfile)
{
  /* Add the base version if present. E.g.
  fullpath: F:\CCIS\v990308\com\atis\atis-ctrl\atis-ctrl.cc
  base:  F:\CCIS\
  append:   \com\atis\atis-ctrl\atis-ctrl.cc
  */
  const wxString fullpath = filename.GetFullPath();

  wxRegEx reg("[/|\\][a-z-]*[0-9]+\\.?[0-9]*\\.?[0-9]*\\.?[0-9]*");

  if (!reg.Matches(fullpath.Lower()))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("No version information found"));
#endif
    return false;
  }

  size_t start, len;
  if (!reg.GetMatch(&start, &len))
  {
    wxFAIL;
    return false;
  }

  wxString base = fullpath.substr(0, start);
  if (!wxEndsWithPathSeparator(base))
  {
    base += wxFileName::GetPathSeparator();
  }

  wxDir dir(base);

  if (!dir.IsOpened())
  {
    wxFAIL;
    return false;
  }

  wxString filename_string;
  bool cont = dir.GetFirst(&filename_string, wxEmptyString, wxDIR_DIRS); // only get dirs

  wxDateTime lastmodtime((time_t)0);
  const wxString append = fullpath.substr(start + len);

  bool found = false;

  // Readme: Maybe use a thread for this.
  while (cont)
  {
    wxFileName fn(base + filename_string + append);

    if (fn.FileExists() &&
        fn.GetPath().CmpNoCase(filename.GetPath()) != 0 &&
        fn.GetModificationTime() != filename.GetModificationTime())
    {
      found = true;

      if (listview == NULL && lastfile == NULL)
      {
        // We are only interested in return value, so speed it up.
        return true;
      }

      if (listview != NULL)
      {
        wxExListItemWithFileName item(listview, fn.GetFullPath());
        item.Insert();
      }

      if (lastfile != NULL)
      {
        if (fn.GetModificationTime() > lastmodtime)
        {
          lastmodtime = fn.GetModificationTime();
          *lastfile = fn;
        }
      }
    }

    cont = dir.GetNext(&filename_string);

    if (wxTheApp != NULL)
    {
      wxTheApp->Yield();
    }
  }

  if (!found && (listview != NULL || lastfile != NULL))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("No files found"));
#endif
  }

  return found;
}

bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
    int page = notebook->GetPageCount() - 1;
    page >= 0;
    page--)
  {
    wxExListViewWithFrame* lv = (wxExListViewWithFrame*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxFAIL;
      return false;
    }

    if (id >= wxID_VIEW_DETAILS &&  id <= wxID_VIEW_LIST)
    {
      long view = 0;
      switch (id)
      {
      case wxID_VIEW_DETAILS: view = wxLC_REPORT; break;
      case wxID_VIEW_LIST: view = wxLC_LIST; break;
      case wxID_VIEW_SMALLICONS: view = wxLC_SMALL_ICON; break;
      default: wxFAIL;
      }

      lv->SetSingleStyle(view);
      wxConfigBase::Get()->Write("List/Style", view);
    }
    else
    {
    switch (id)
    {
    case ID_LIST_ALL_ITEMS:
      {
        if (font.IsOk())
        {
          lv->SetFont(font);
        }

        lv->ItemsUpdate();
      }
      break;

    case ID_LIST_ALL_CLOSE:
      {
      wxExFileDialog dlg(notebook, lv);
      if (!dlg.Continue()) return false;
      if (!notebook->DeletePage(page)) return false;
      }
      break;

    default: wxFAIL;
    }
    }
  }

  return true;
}

bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile)
{
  const wxString cwd = wxGetCwd();

  wxSetWorkingDirectory(makefile.GetPath());

  const bool ret = frame->ProcessRun(
    wxConfigBase::Get()->Read("Make", "make") + " " +
    wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
    makefile.GetFullPath());

  wxSetWorkingDirectory(cwd);

  return ret;
}

void wxExOpenFiles(
  wxExFrameWithHistory* frame,
  const wxArrayString& files,
  long file_flags,
  int dir_flags)
{
  for (size_t i = 0; i < files.GetCount(); i++)
  {
    wxString file = files[i]; // cannot be const because of file = later on

    if (file.Contains("*") || file.Contains("?"))
    {
      wxExDirWithListView dir(frame, wxGetCwd(), file, file_flags, dir_flags);
      dir.FindFiles();
    }
    else
    {
      int line = 0;

      if (file.Contains(":"))
      {
        line = atoi(files[i].AfterFirst(':').c_str());

        if (line != 0) // this indicates an error in the number
        {
          file = file.BeforeFirst(':');
        }
      }

      frame->OpenFile(file, line, wxEmptyString, file_flags);
    }
  }
}

void wxExOpenFilesDialog(
  wxExFrameWithHistory* frame,
  long style,
  const wxString wildcards,
  bool ask_for_continue)
{
  wxExSTC* stc = frame->GetSTC();
  wxArrayString files;

  if (stc != NULL)
  {
    wxExFileDialog dlg(frame,
      stc,
      _("Select Files"),
      wxFileSelectorDefaultWildcardStr,
      style);

    if (dlg.ShowModal(ask_for_continue) == wxID_CANCEL) return;
    dlg.GetPaths(files);
  }
  else
  {
    wxFileDialog dlg(frame,
      _("Select Files"),
      wxEmptyString,
      wxEmptyString,
      wildcards,
      style);
    if (dlg.ShowModal() == wxID_CANCEL) return;
    dlg.GetPaths(files);
  }

  wxExOpenFiles(frame, files);
}

wxExDirWithListView::wxExDirWithListView(const wxExTool& tool,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(NULL)
  , m_Flags(0)
  , m_Tool(tool)
{
}

wxExDirWithListView::wxExDirWithListView(wxExListViewFile* listview,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics(fullpath)
  , m_Frame(NULL)
  , m_ListView(listview)
  , m_Flags(0)
  , m_Tool(ID_TOOL_LOWEST)
{
}

wxExDirWithListView::wxExDirWithListView(wxExFrame* frame,
  const wxString& fullpath, 
  const wxString& filespec, 
  long file_flags,
  int dir_flags)
  : wxExDir(fullpath, filespec, dir_flags)
  , m_Statistics(fullpath)
  , m_Frame(frame)
  , m_ListView(NULL)
  , m_Flags(file_flags)
  , m_Tool(ID_TOOL_LOWEST)
{
}

void wxExDirWithListView::OnDir(const wxString& dir)
{
  if (m_ListView != NULL)
  {
    wxExListItemWithFileName(m_ListView, dir, GetFileSpec()).Insert();
  }
}

void wxExDirWithListView::OnFile(const wxString& file)
{
  if (m_Frame == NULL && m_ListView == NULL)
  {
    const wxExFileName filename(file);

    if (filename.GetStat().IsOk())
    {
      wxExTextFileWithListView report(filename, m_Tool);
      report.RunTool();
      m_Statistics += report.GetStatistics();
    }
  }
  else
  {
    if (m_Frame != NULL)
    {
      m_Frame->OpenFile(file, 0, wxEmptyString, m_Flags);
    }
    else if (m_ListView != NULL)
    {
      wxExListItemWithFileName item(m_ListView, file, GetFileSpec());
      item.Insert();

      // Don't move next code into insert, as it itself inserts!
      if (m_ListView->GetType() == wxExListViewWithFrame::LIST_VERSION)
      {
        wxExListItemWithFileName item(m_ListView, m_ListView->GetItemCount() - 1);

        wxExTextFileWithListView report(item.m_Statistics, ID_TOOL_REVISION_RECENT);
        if (report.SetupTool(ID_TOOL_REVISION_RECENT))
        {
          report.RunTool();
          item.UpdateRevisionList(report.GetRCS());
        }
      }
    }
  }
}
