/******************************************************************************\
* File:          textfile.cpp
* Purpose:       Implementation of wxExTextFile class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <algorithm>
#include <ctype.h> // for isspace
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/extension/textfile.h>
#include <wx/extension/frd.h>
#include <wx/extension/util.h>

long wxExFileStatistics::Get(const wxString& key) const
{
  std::map<wxString, long>::const_iterator it = m_Elements.GetItems().find(key);

  if (it != m_Elements.GetItems().end())
  {
    return it->second;
  }
  else
  {
    std::map<wxString, long>::const_iterator it = m_Keywords.GetItems().find(key);

    if (it != m_Keywords.GetItems().end())
    {
      return it->second;
    }
  }

  return 0;
}

const wxString REV_DATE_FORMAT = "%y%m%d";

wxExRCS::wxExRCS()
  : m_RevisionFormat(REV_DATE_FORMAT)
{
}

const wxString wxExRCS::GetRevision() const
{
  wxString logtext;

  if (!m_RevisionNumber.empty()) logtext << m_RevisionNumber << ' ';
  if (m_RevisionTime.IsValid()) logtext << m_RevisionTime.Format(m_RevisionFormat) << ' ';
  if (!m_User.empty()) logtext << m_User << ' ';
  logtext << m_Description;

  return logtext;
}

bool wxExRCS::SetRevision(wxString& text)
{
  // ClassBuilder lines start with '* ', these characters are skipped here.
  wxRegEx("^\\* ").ReplaceFirst(&text, wxEmptyString);
  // If there is a revision in the first word, store it.
  wxString word = wxExGetWord(text);
  if (word.find('.') != wxString::npos)
  {
    m_RevisionNumber = word;
  }
  else
  {
    m_RevisionNumber.clear();
    text = word + " " + text; // put back the word!
  }

  const wxString REV_CBD_FORMAT = "%B %d, %Y %H:%M";
  const wxString REV_TIMESTAMP_FORMAT = "%y%m%d %H%M%S";

  m_RevisionFormat = REV_TIMESTAMP_FORMAT;

  return true;
}

bool wxExTextFile::m_Cancelled = false;

wxExTextFile::wxExTextFile(
  const wxExFileName& filename,
  const wxExTool& tool)
  : m_FileName(filename)
  , m_LastSyntaxType(SYNTAX_NONE)
  , m_SyntaxType(SYNTAX_NONE)
  , m_Tool(tool)
{
  m_AllowAction = false;
  m_EmptyLine = false;
  m_FinishedAction = false;
  m_IsCommentStatement = false;
  m_IsString = false;
  m_Modified = false;
  m_RevisionActive = false;
  m_LineMarker = 0;
  m_LineMarkerEnd = 0;
  m_VersionLine = 0;
}

wxExTextFile::wxExCommentType wxExTextFile::CheckCommentSyntax(
  const wxString& syntax_begin,
  const wxString& syntax_end,
  const wxString& text) const
{
  if (syntax_begin == text)
  {
    return (syntax_end == text) ? COMMENT_BOTH: COMMENT_BEGIN;
  }
  else
  {
    if (syntax_end == text ||
       (syntax_end.empty() && text.empty()))
    {
      return COMMENT_END;
    }
  }

  if ( syntax_begin.StartsWith(text) || 
      (!syntax_end.empty() && syntax_end.StartsWith(text)))
  {
    return COMMENT_INCOMPLETE;
  }

  return COMMENT_NONE;
}

wxExTextFile::wxExCommentType wxExTextFile::CheckForComment(
  const wxString& text)
{
  if (m_FileName.GetLexer().GetCommentBegin2().empty())
  {
    return CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin(),
      m_FileName.GetLexer().GetCommentEnd(), text);
  }

  wxExCommentType comment_type1 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_ONE)
  {
    if ((comment_type1 = CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin(),
      m_FileName.GetLexer().GetCommentEnd(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_ONE;
  }

  wxExCommentType comment_type2 = COMMENT_NONE;

  if (m_SyntaxType == SYNTAX_NONE || m_SyntaxType == SYNTAX_TWO)
  {
    if ((comment_type2 = CheckCommentSyntax(
      m_FileName.GetLexer().GetCommentBegin2(),
      m_FileName.GetLexer().GetCommentEnd2(), text)) == COMMENT_BEGIN)
      m_SyntaxType = SYNTAX_TWO;
  }

  wxExCommentType comment_type = COMMENT_NONE;

  switch (comment_type1)
  {
  case COMMENT_NONE:  comment_type = comment_type2; break;
  case COMMENT_BEGIN: comment_type = COMMENT_BEGIN; break;
  case COMMENT_END:   comment_type = COMMENT_END; break;
  case COMMENT_BOTH:  comment_type = COMMENT_BOTH; break;
  case COMMENT_INCOMPLETE:
    comment_type = (comment_type2 == COMMENT_NONE) ? COMMENT_INCOMPLETE: comment_type2;
    break;
  default: wxFAIL;
  }

  if (comment_type == COMMENT_END)
  {
    // E.g. we have a correct /* */ comment, with */ at the end of the line.
    // Then the end of line itself should not generate a COMMENT_END.
    if (m_SyntaxType == SYNTAX_NONE) comment_type = COMMENT_NONE;
    // Keep the syntax type.
    m_LastSyntaxType = m_SyntaxType;
    m_SyntaxType = SYNTAX_NONE;
  }

  return comment_type;
}

void wxExTextFile::CommentStatementEnd()
{
  m_IsCommentStatement = false;

  // Remove the end of comment characters (as last used) from the buffer.
  m_Comments = m_Comments.Left(
    m_Comments.length() - CommentEnd().length());
}

void wxExTextFile::CommentStatementStart()
{
  m_IsCommentStatement = true;

  m_Stats.m_Elements.Inc(_("Comments"));
  m_Stats.m_Elements.Inc(
    _("Comment Size"),
    CommentBegin().length());
}

void wxExTextFile::EndCurrentRevision()
{
  if (m_RevisionActive)
  {
    m_Stats.m_Elements.Inc(_("Actions Completed"));

    if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
    {
      Report(GetCurrentLine() - 1);
    }

    m_RevisionActive = false;
  }
}

void wxExTextFile::InsertLine(const wxString& line)
{
  if (GetCurrentLine() == GetLineCount())
  {
    AddLine(line);
  }
  else
  {
    wxTextFile::InsertLine(line, GetCurrentLine());
  }

  m_Modified = true;

  GoToLine(GetCurrentLine() + 1);
}

bool wxExTextFile::MatchLine(wxString& line)
{
  bool match = false;

  wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  if (!frd->IsRegularExpression())
  {
    const wxString search_line = frd->MatchCase() ? line: line.Upper();
    const size_t start = search_line.find(frd->GetFindStringNoCase());

    if (start != wxString::npos)
    {
      if (frd->MatchWord() && m_Tool.GetId() == ID_TOOL_REPORT_FIND)
      {
        if (( start == 0 ||
             (start > 0 && !IsWordCharacter(search_line[start - 1]))) &&
            !IsWordCharacter(search_line[start + frd->GetFindStringNoCase().length()]))
        {
          match = true;
        }
      }
      else
      {
        match = true;
      }

      if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
      {
        const size_t count = line.Replace(frd->GetFindString(), frd->GetReplaceString());
        m_Stats.m_Elements.Inc(_("Actions Completed"), count);
        m_Modified = true;
      }
    }
  }
  else
  {
    match = frd->GetRegularExpression().Matches(line);

    if (match && m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
    {
      const int count = frd->GetRegularExpression().ReplaceAll(&line, frd->GetReplaceString());
      m_Stats.m_Elements.Inc(_("Actions Completed"), count);
      m_Modified = true;
    }
  }

  if (match)
  {
    m_RCS.m_Description = line;

    if (m_Tool.GetId() != ID_TOOL_REPORT_REPLACE)
    {
      m_Stats.m_Elements.Inc(_("Actions Completed"));
    }
  }

  return match;
}

bool wxExTextFile::Parse()
{
  if (m_Tool.GetId() == ID_TOOL_REPORT_REPLACE)
  {
    if (m_FileName.GetStat().IsReadOnly())
    {
      return false;
    }
  }

  if (m_Tool.IsFindType())
  {
    if (wxExFindReplaceData::Get()->GetFindString().empty())
    {
      wxFAIL;
      return false;
    }
  }

  for (size_t i = 0; i < GetLineCount() && !Cancelled() && !m_FinishedAction; i++)
  {
    wxString& line = GetLine(i);

    if (m_Tool.IsFindType())
    {
      if (MatchLine(line))
      {
        Report(i);
      }
    }
    else
    {
      GoToLine(i);
      
      if (!ParseLine(line))
      {
        return false;
      }
    }

    if (wxIsMainThread())
    {
      if (wxTheApp != NULL)
      {
        wxTheApp->Yield();
      }
    }
    else
    {
      wxThread::This()->Yield();
    }
  }

  return !Cancelled();
}

bool wxExTextFile::ParseComments()
{
  if (m_Tool.IsRCSType())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION || m_VersionLine <= 1)
    {
      // this is the minimal prefix
      if (m_Comments.length() >= 3)
      {
        const bool insert = 
          (wxRegEx("^[\\*| ]   *").ReplaceFirst(&m_Comments, wxEmptyString) == 0);

        m_Comments.Trim(); // from right, default

        if (insert)
        {
          EndCurrentRevision();

          if (!PrepareRevision())
          {
            m_Comments.clear();
            return true;
          }

          m_RevisionActive = true;
        }
        else
        {
          if (m_VersionLine >= 1 && GetCurrentLine() == m_LineMarkerEnd + 1)
          {
            m_LineMarkerEnd = GetCurrentLine();
            m_RCS.m_Description += 
              (!m_RCS.m_Description.empty() ? wxString(" "): wxString(wxEmptyString)) + m_Comments;
          }
          else
          {
            m_Comments.clear();
            return true;
          }
        }
      }
    }

    if (  m_AllowAction ||
         // In case there is one revision comment, this is ended by an empty line.
        (m_Tool.GetId() == ID_TOOL_REVISION_RECENT &&
         !m_FinishedAction && m_VersionLine == 1 && m_EmptyLine)
       )
    {
      if (m_Tool.GetId() == ID_TOOL_REVISION_RECENT)
      {
        m_AllowAction = false;
        // We are not yet finished, there might still be RCS keywords somewhere!
        // m_FinishedAction = true;
      }
    }

    m_Comments.clear();
  }

  return true;
}

bool wxExTextFile::ParseLine(const wxString& line)
{
  bool line_contains_code = false, sequence = false;
  wxString codeword;

  for (size_t i = 0; i < line.length(); i++)
  {
    if (m_IsCommentStatement)
    {
      if (m_Tool.IsCount())
      {
        m_Stats.m_Elements.Inc(_("Comment Size"));
      }

      m_Comments += line[i];
    }
    else if (line[i] == '"')
    {
      m_IsString = !m_IsString;
    }

    // Comments and codewords only appear outside strings.
    if (!m_IsString)
    {
      if (line.length() == 0) continue;

      if (i == 0) 
      {
        if (!isspace(line[0]))
        {
          codeword = line[i];
        }

        continue;
      }

      const size_t max_check_size = 
        m_FileName.GetLexer().GetCommentBegin().Length();
      const size_t check_size = (i > max_check_size ? max_check_size: i + 1);

      const wxString text = line.substr(i + 1 - check_size, check_size);

      switch (CheckForComment(text))
      {
      case COMMENT_BEGIN:
        if (!m_IsCommentStatement) CommentStatementStart();
        break;

      case COMMENT_END:
        CommentStatementEnd();
        break;

      case COMMENT_BOTH:
        !m_IsCommentStatement ? CommentStatementStart(): CommentStatementEnd();
        break;

      case COMMENT_NONE:
        if (!isspace(line[i]) && !m_IsCommentStatement)
        {
          line_contains_code = true;

          if (!IsCodewordSeparator(line[i]))
          {
            if (!sequence)
            {
              if (m_Tool.IsCount())
              {
                m_Stats.m_Elements.Inc(_("Words Of Code"));
              }

              sequence = true;
            }

            codeword += line[i];
          }
        }
      break;

      case COMMENT_INCOMPLETE:
        break;

      default: 
        wxFAIL;
        break;
      }

      if ( sequence && 
          (IsCodewordSeparator(line[i]) || i ==0 || i == line.length() - 1))
      {
        if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
        {
          if (m_FileName.GetLexer().IsKeyword(codeword))
          {
            m_Stats.m_Keywords.Inc(codeword);
          }
        }

        sequence = false;
        codeword.clear();
      }
    }
    else
    {
      line_contains_code = true;
    }
  }

  if (CheckForComment(wxEmptyString) == COMMENT_END)
  {
    CommentStatementEnd();
  }

  if (line_contains_code)
  {
    m_Stats.m_Elements.Inc(_("Lines Of Code"));

    if (m_Tool.GetId() == ID_TOOL_LINE || m_Tool.GetId() == ID_TOOL_LINE_CODE)
    {
      ReportLine(line);
    }

    // Finish action.
    // However, some sources might contain revisions at the end of the file, 
    // so these are not reported.
    if (m_Stats.m_Elements.Get(_("Lines Of Code")) > 5 &&
        m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
    {
      m_FinishedAction = true;
    }
  }

  m_EmptyLine = (line.length() == 0);

  if (m_EmptyLine)
  {
    if (m_Tool.GetId() == ID_TOOL_LINE && !m_IsCommentStatement)
    {
      ReportLine(line);
    }

    m_Stats.m_Elements.Inc(_("Empty Lines"));
  }

  if (m_IsCommentStatement && GetCurrentLine() < GetLineCount() - 1)
  {
    if (m_Tool.GetId() == ID_TOOL_LINE_COMMENT)
    {
      ReportLine(line);
    }

    // End of lines are included in comment size as well.
    if (m_Tool.IsCount())
    {
      m_Stats.m_Elements.Inc(_("Comment Size"), wxString(GetEOL()).length());
    }
  }

  return ParseComments();
}

bool wxExTextFile::PrepareRevision()
{
  if (m_Tool.GetId() == ID_TOOL_REVISION_RECENT ||
      wxConfigBase::Get()->Read("RCS/RecentOnly", 0L))
  {
    if (m_VersionLine == 1)
    {
      if (!m_FinishedAction)
      {
        m_AllowAction = true;
      }

      return false;
    }
  }

  if (!m_RCS.SetRevision(m_Comments))
  {
    return false;
  }

  if (m_LineMarker == 0)
  {
    m_LineMarker = GetCurrentLine();
    m_LineMarkerEnd = GetCurrentLine();
  }

  if (m_Tool.GetId() == ID_TOOL_REPORT_REVISION)
  {
    m_LineMarkerEnd = GetCurrentLine();
  }

  if (m_Tool.GetId() != ID_TOOL_REVISION_RECENT)
  {
    m_AllowAction = true;
  }

  m_VersionLine++;

  return true;
}

bool wxExTextFile::RunTool()
{
  if (!wxTextFile::Open(m_FileName.GetFullPath()))
  {
    return false;
  }

  m_Stats.m_Elements.Set(_("Files"), 1);

  if (m_Tool.IsCount())
  {
    m_Stats.m_Elements.Inc(_("Total Size"), m_FileName.GetStat().st_size);
    m_Stats.m_Elements.Inc(_("Lines"), GetLineCount());
  }

  if (GetLineCount() > 0)
  {
    if (m_FileName.GetLexer().GetScintillaLexer().empty())
    {
      m_FileName.SetLexer(wxEmptyString, GetLine(0));
    }

    if (!Parse())
    {
      Close();

      return false;
    }
  }

  m_Stats.m_Elements.Set(_("Files Passed"), 1);

  if (m_Tool.IsStatisticsType())
  {
    if (m_Tool.GetId() == ID_TOOL_REPORT_KEYWORD)
    {
      if (!m_FileName.GetLexer().GetKeywordsString().empty())
      {
        m_Stats.m_Elements.Inc(_("Actions Completed"));
      }

    }

    ReportStatistics();
  }

  if (m_Modified && !m_FileName.GetStat().IsReadOnly())
  {
    if (!Write())
    {
      Close();

      return false;
    }
  }

  Close();

  return true;
}