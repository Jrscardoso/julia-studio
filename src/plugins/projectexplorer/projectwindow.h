/***************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Qt Software Information (qt-info@nokia.com)
**
**
** Non-Open Source Usage
**
** Licensees may use this file in accordance with the Qt Beta Version
** License Agreement, Agreement version 2.2 provided with the Software or,
** alternatively, in accordance with the terms contained in a written
** agreement between you and Nokia.
**
** GNU General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the packaging
** of this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
**
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt GPL Exception
** version 1.3, included in the file GPL_EXCEPTION.txt in this package.
**
***************************************************************************/

#ifndef PROJECTWINDOW_H
#define PROJECTWINDOW_H

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE
class QModelIndex;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;
QT_END_NAMESPACE

namespace ProjectExplorer {

class Project;
class PropertiesPanel;
class ProjectExplorerPlugin;
class SessionManager;

namespace Internal {

class ProjectWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectWindow(QWidget *parent = 0);
    ~ProjectWindow();

private slots:
    void showProperties(ProjectExplorer::Project *project, const QModelIndex &subIndex);
    void restoreStatus();
    void saveStatus();

    void updateTreeWidget();
    void handleItem(QTreeWidgetItem *item, int column);
    void handleCurrentItemChanged(QTreeWidgetItem *);

private:
    SessionManager *m_session;
    ProjectExplorerPlugin *m_projectExplorer;

    QTreeWidget* m_treeWidget;
    QTabWidget *m_panelsTabWidget;

    QList<PropertiesPanel*> m_panels;

    Project *findProject(const QString &path) const;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // PROJECTWINDOW_H
