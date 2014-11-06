// Copyright (c) 2014 Stanislas Polu.
// Copyright (c) 2012 The Chromium Authors.
// See the LICENSE file.

#include "src/browser/dialog/web_dialog_helper.h"

#include <vector>

#include "base/bind.h"
#include "base/files/file_enumerator.h"
#include "base/strings/utf_string_conversions.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "ui/shell_dialogs/selected_file_info.h"

#include "src/browser/dialog/file_dialog.h"

namespace thrust_shell {

ThrustShellWebDialogHelper::ThrustShellWebDialogHelper(
    ThrustWindow* window)
    : window_(window),
      weak_factory_(this) {
}

ThrustShellWebDialogHelper::~ThrustShellWebDialogHelper() 
{
}

void 
ThrustShellWebDialogHelper::RunFileChooser(
    content::WebContents* web_contents,
    const content::FileChooserParams& params) 
{
  std::vector<ui::SelectedFileInfo> result;
  if (params.mode == content::FileChooserParams::Save) {
    base::FilePath path;
    if (file_dialog::ShowSaveDialog(window_,
                                    base::UTF16ToUTF8(params.title),
                                    params.default_file_name,
                                    file_dialog::Filters(),
                                    &path))
      result.push_back(ui::SelectedFileInfo(path, path));
  } else {
    int flags = file_dialog::FILE_DIALOG_CREATE_DIRECTORY;
    switch (params.mode) {
      case content::FileChooserParams::OpenMultiple:
        flags |= file_dialog::FILE_DIALOG_MULTI_SELECTIONS;
      case content::FileChooserParams::Open:
        flags |= file_dialog::FILE_DIALOG_OPEN_FILE;
        break;
      case content::FileChooserParams::UploadFolder:
        flags |= file_dialog::FILE_DIALOG_OPEN_DIRECTORY;
        break;
      default:
        NOTREACHED();
    }

    std::vector<base::FilePath> paths;
    if (file_dialog::ShowOpenDialog(window_,
                                    base::UTF16ToUTF8(params.title),
                                    params.default_file_name,
                                    file_dialog::Filters(),
                                    flags,
                                    &paths))
      for (auto& path : paths)
        result.push_back(ui::SelectedFileInfo(path, path));
  }

  web_contents->GetRenderViewHost()->FilesSelectedInChooser(
      result, params.mode);
}

void 
ThrustShellWebDialogHelper::EnumerateDirectory(
    content::WebContents* web_contents,
    int request_id,
    const base::FilePath& dir) 
{
  int types = base::FileEnumerator::FILES |
              base::FileEnumerator::DIRECTORIES |
              base::FileEnumerator::INCLUDE_DOT_DOT;
  base::FileEnumerator file_enum(dir, false, types);

  base::FilePath path;
  std::vector<base::FilePath> paths;
  while (!(path = file_enum.Next()).empty())
    paths.push_back(path);

  web_contents->GetRenderViewHost()->DirectoryEnumerationFinished(
      request_id, paths);
}

}  // namespace thrust_shell
