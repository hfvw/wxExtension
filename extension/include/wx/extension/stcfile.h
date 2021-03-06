////////////////////////////////////////////////////////////////////////////////
// Name:      stcfile.h
// Purpose:   Declaration of class wxExSTCFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTCFILE_H
#define _EXSTCFILE_H

#include <wx/extension/file.h> // for wxExFile

class wxExSTC;

#if wxUSE_GUI
/// Adds file read and write to wxExSTC.
class WXDLLIMPEXP_BASE wxExSTCFile: public wxExFile
{
public:
  /// Constructor.
  /// Does not open the file.
  wxExSTCFile(
    /// the stc component
    wxExSTC* stc,
    /// the filename to be assigned if not empty
    const wxString& filename = wxEmptyString);
  
  /// Override virtual methods.
  virtual bool GetContentsChanged() const;
  virtual void ResetContentsChanged();
  
  /// Reads other file and adds contents to STC.
  /// Returns true if file was read.
  bool Read(const wxString& file) const;
protected:
  virtual bool DoFileLoad(bool synced = false);
  virtual void DoFileNew();
  virtual void DoFileSave(bool save_as = false);
private:
  void ReadFromFile(bool get_only_new_data);

  wxExSTC* m_STC;
  wxFileOffset m_PreviousLength;
};
#endif // wxUSE_GUI
#endif
