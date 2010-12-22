/******************************************************************************\
* File:          defs.h
* Purpose:       Constant definitions
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _SYNCDEFS_H
#define _SYNCDEFS_H

// Command id's used.
enum
{
  ID_APPL_LOWEST = ID_EXTENSION_REPORT_HIGHEST + 1,
  wxID_EXECUTE,
  ID_EDIT_HEX_MODE,
  ID_SYNC_MODE,
  ID_OPEN_LEXERS,
  ID_OPEN_LOGFILE,
  ID_OPTION_EDITOR,
  ID_OPTION_SVN_AND_COMPARATOR,
  // wxID_SORT_ASCENDING already used by wxex
  ID_OPTION_LIST_SORT_ASCENDING,
  ID_OPTION_LIST_SORT_DESCENDING,
  ID_OPTION_LIST_SORT_TOGGLE,
  ID_SVN_ADD,
  ID_SVN_COMMIT,
  ID_SVN_DIFF,
  ID_SVN_HELP,
  ID_SVN_INFO,
  ID_SVN_LOG,
  ID_SVN_LS,
  ID_SVN_STAT,
  ID_SVN_UPDATE,
  ID_PROCESS_SELECT,
  ID_PROJECT_NEW,
  ID_PROJECT_OPEN,
  ID_PROJECT_OPENTEXT,
  ID_PROJECT_CLOSE,
  ID_PROJECT_SAVEAS,
  ID_RECENT_FILE_MENU,
  ID_RECENT_PROJECT_MENU,
  ID_SORT_SYNC,
  ID_SPLIT,
  ID_TREE_OPEN,
  ID_TREE_RUN_MAKE,
  ID_VIEW_PANE_FIRST, // put all your panes from here
  ID_VIEW_FILES,
  ID_VIEW_PROJECTS,
  ID_VIEW_OUTPUT,
  ID_VIEW_DIRCTRL,
  ID_VIEW_FINDBAR,
  ID_VIEW_HISTORY,
  ID_VIEW_ASCII_TABLE,
  ID_VIEW_PANE_LAST,
  ID_APPL_HIGHEST,
};

// wxWidget id's.
enum
{
  NOTEBOOK_EDITORS,
  NOTEBOOK_LISTS,
  NOTEBOOK_PROJECTS,
};
#endif