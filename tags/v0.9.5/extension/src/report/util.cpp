/******************************************************************************\
* File:          util.cpp
* Purpose:       Implementation of wxExtension report utility functions
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/regex.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/util.h>
#include <wx/extension/report/util.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/frame.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listviewfile.h>

bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListView* listview)
{
  /* Add the base version if present. E.g.
  fullpath: F:\CCIS\v990308\com\atis\atis-ctrl\atis-ctrl.cc
  base:  F:\CCIS\
  append:   \com\atis\atis-ctrl\atis-ctrl.cc
  */
  const wxString fullpath = filename.GetFullPath();

  const wxRegEx reg("[/|\\][a-z-]*[0-9]+\\.?[0-9]*\\.?[0-9]*\\.?[0-9]*");

  if (!reg.Matches(fullpath.Lower()))
  {
    wxLogStatus(_("No version information found"));
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
      wxExListItem(listview, fn).Insert();
    }

    cont = dir.GetNext(&filename_string);

    if (wxTheApp != NULL)
    {
      wxTheApp->Yield();
    }
  }

  if (!found)
  {
    wxLogStatus(_("No files found"));
  }

  return found;
}

bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font)
{
  for (
	// no auto, should be int
    int page = notebook->GetPageCount() - 1;
    page >= 0;
    page--)
  {
    // Required by wxExFileDialog.
    wxExListViewFile* lv = (wxExListViewFile*)notebook->GetPage(page);

    if (lv == NULL)
    {
      wxFAIL;
      return false;
    }

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
      if (dlg.ShowModalIfChanged() == wxID_CANCEL) return false;
      if (!notebook->DeletePage(page)) return false;
      }
      break;

    default: wxFAIL;
    }
  }

  return true;
}

int wxExGetIconID(const wxExFileName& filename)
{
  if (filename.GetStat().IsOk())
  {
    if (filename.DirExists(filename.GetFullPath()))
    {
      return wxFileIconsTable::folder;
    }
    else if (!filename.GetExt().empty())
    {
      return wxTheFileIconsTable->GetIconID(filename.GetExt());
    }
    else
    {
      return wxFileIconsTable::file;
    }
  }
  else
  {
    return wxFileIconsTable::computer;
  }
}

bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile)
{
  const wxString cwd = wxGetCwd();

  if (!wxSetWorkingDirectory(makefile.GetPath()))
  {
    wxLogError(_("Cannot set working directory"));
    return false;
  }

  const bool ret = frame->ProcessRun(
    wxConfigBase::Get()->Read("Make", "make") + " " +
    wxConfigBase::Get()->Read("MakeSwitch", "-f") + " " +
    makefile.GetFullPath());

  wxSetWorkingDirectory(cwd);

  return ret;
}