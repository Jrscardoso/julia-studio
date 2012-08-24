/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 AudioCodes Ltd.
**
** Author: Orgad Shaneh <orgad.shaneh@audiocodes.com>
**
** Contact: http://www.qt-project.org/
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**************************************************************************/

#ifndef CLEARCASE_CONSTANTS_H
#define CLEARCASE_CONSTANTS_H

#include <QtGlobal>

namespace ClearCase {
namespace Constants {

const char VCS_ID_CLEARCASE[] = "E.ClearCase";
const char CLEARCASE_ROOT_FILE[] = "view.dat";
const char CLEARCASE_SUBMIT_MIMETYPE[] = "application/vnd.audc.text.clearcase.submit";
const char CLEARCASECHECKINEDITOR[] = "ClearCase Check In Editor";
const char CLEARCASECHECKINEDITOR_ID[] = "ClearCase Check In Editor";
const char CLEARCASECHECKINEDITOR_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("VCS", "ClearCase Check In Editor");
const char CHECKIN_SELECTED[] = "ClearCase.CheckInSelected";
const char DIFF_SELECTED[] = "ClearCase.DiffSelected";
const char TASK_INDEX[] = "ClearCase.Task.Index";
const char KEEP_ACTIVITY[] = "__KEEP__";
enum { debug = 0 };

} // namespace Constants
} // namespace ClearCase

#endif // CLEARCASE_CONSTANTS_H