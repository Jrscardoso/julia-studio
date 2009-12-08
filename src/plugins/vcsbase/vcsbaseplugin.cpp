/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "vcsbaseplugin.h"
#include "vcsbasesubmiteditor.h"
#include "vcsplugin.h"
#include "corelistener.h"

#include <coreplugin/icore.h>
#include <coreplugin/ifile.h>
#include <coreplugin/iversioncontrol.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/vcsmanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <utils/qtcassert.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QSharedData>

#include <QtGui/QAction>

enum { debug = 0 };

namespace VCSBase {

namespace Internal {

// Internal state created by the state listener and
// VCSBasePluginState.

struct State {
    void clearFile();
    void clearProject();
    inline void clear();

    bool equals(const State &rhs) const;

    inline bool hasFile() const     { return !currentFile.isEmpty(); }
    inline bool hasProject() const  { return !currentProjectPath.isEmpty(); }
    inline bool isEmpty() const     { return !hasFile() && !hasProject(); }

    QString currentFile;
    QString currentFileName;
    QString currentFileDirectory;
    QString currentFileTopLevel;

    QString currentProjectPath;
    QString currentProjectName;
    QString currentProjectTopLevel;
};

void State::clearFile()
{
    currentFile.clear();
    currentFileName.clear();
    currentFileDirectory.clear();
    currentFileTopLevel.clear();
}

void State::clearProject()
{
    currentProjectPath.clear();
    currentProjectName.clear();
    currentProjectTopLevel.clear();
}

void State::clear()
{
    clearFile();
    clearProject();
}

bool State::equals(const State &rhs) const
{
    return currentFile == rhs.currentFile
            && currentFileName == rhs.currentFileName
            && currentFileTopLevel == rhs.currentFileTopLevel
            && currentProjectPath == rhs.currentProjectPath
            && currentProjectName == rhs.currentProjectName
            && currentProjectTopLevel == rhs.currentProjectTopLevel;
}

QDebug operator<<(QDebug in, const State &state)
{
    QDebug nospace = in.nospace();
    nospace << "State: ";
    if (state.isEmpty()) {
        nospace << "<empty>";
    } else {
        if (state.hasFile()) {
            nospace << "File=" << state.currentFile
                    << ',' << state.currentFileTopLevel;
        } else {
            nospace << "<no file>";
        }
        nospace << '\n';
        if (state.hasProject()) {
            nospace << "       Project=" << state.currentProjectName
            << ',' << state.currentProjectPath
            << ',' << state.currentProjectTopLevel;

        } else {
            nospace << "<no project>";
        }
        nospace << '\n';
    }
    return in;
}

// StateListener: Connects to the relevant signals, tries to find version
// controls and emits signals to the plugin instances.

class StateListener : public QObject {
    Q_OBJECT
public:
    explicit StateListener(QObject *parent);

signals:
    void stateChanged(const VCSBase::Internal::State &s, Core::IVersionControl *vc);

private slots:
    void slotStateChanged();
};

StateListener::StateListener(QObject *parent) :
        QObject(parent)
{
    Core::ICore *core = Core::ICore::instance();
    connect(core, SIGNAL(contextChanged(Core::IContext *)),
            this, SLOT(slotStateChanged()));
    connect(core->fileManager(), SIGNAL(currentFileChanged(QString)),
            this, SLOT(slotStateChanged()));

    if (ProjectExplorer::ProjectExplorerPlugin *pe = ProjectExplorer::ProjectExplorerPlugin::instance())
        connect(pe, SIGNAL(currentProjectChanged(ProjectExplorer::Project*)),
                this, SLOT(slotStateChanged()));
}

void StateListener::slotStateChanged()
{
    const ProjectExplorer::ProjectExplorerPlugin *pe = ProjectExplorer::ProjectExplorerPlugin::instance();
    const Core::ICore *core = Core::ICore::instance();
    Core::VCSManager *vcsManager = core->vcsManager();

    // Get the current file. Are we on a temporary submit editor or something? Ignore.
    State state;
    state.currentFile = core->fileManager()->currentFile();
    if (!state.currentFile.isEmpty() && state.currentFile.startsWith(QDir::tempPath()))
        state.currentFile.clear();
    // Get the file and its control. Do not use the file unless we find one
    Core::IVersionControl *fileControl = 0;
    if (!state.currentFile.isEmpty()) {
        const QFileInfo fi(state.currentFile);
        state.currentFileDirectory = fi.absolutePath();
        state.currentFileName = fi.fileName();
        fileControl = vcsManager->findVersionControlForDirectory(state.currentFileDirectory,
                                                                 &state.currentFileTopLevel);
        if (!fileControl)
            state.clearFile();
    }
    // Check for project, find the control
    Core::IVersionControl *projectControl = 0;
    if (const ProjectExplorer::Project *currentProject = pe->currentProject()) {
        state.currentProjectPath = QFileInfo(currentProject->file()->fileName()).absolutePath();
        state.currentProjectName = currentProject->name();
        projectControl = vcsManager->findVersionControlForDirectory(state.currentProjectPath,
                                                                    &state.currentProjectTopLevel);
        if (projectControl) {
            // If we have both, let the file's one take preference
            if (fileControl && projectControl != fileControl) {
                state.clearProject();
            }
        } else {
            state.clearProject(); // No control found
        }
    }
    // Assemble state and emit signal.
    Core::IVersionControl *vc = state.currentFile.isEmpty() ? projectControl : fileControl;
    if (debug)
        qDebug() << state << (vc ? vc->name() : QString(QLatin1String("No version control")));
    emit stateChanged(state, vc);
}

} // namespace Internal

class VCSBasePluginStateData : public QSharedData {
public:
    Internal::State m_state;
};

VCSBasePluginState::VCSBasePluginState() : data(new VCSBasePluginStateData)
{
}

VCSBasePluginState::VCSBasePluginState(const VCSBasePluginState &rhs) : data(rhs.data)
{
}

VCSBasePluginState &VCSBasePluginState::operator=(const VCSBasePluginState &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

VCSBasePluginState::~VCSBasePluginState()
{
}

QString VCSBasePluginState::currentFile() const
{
    return data->m_state.currentFile;
}

QString VCSBasePluginState::currentFileName() const
{
    return data->m_state.currentFileName;
}

QString VCSBasePluginState::currentFileTopLevel() const
{
    return data->m_state.currentFileTopLevel;
}

QString VCSBasePluginState::currentFileDirectory() const
{
    return data->m_state.currentFileDirectory;
}

QString VCSBasePluginState::relativeCurrentFile() const
{
    QTC_ASSERT(hasFile(), return QString())
    return QDir(data->m_state.currentFileTopLevel).relativeFilePath(data->m_state.currentFile);
}

QString VCSBasePluginState::currentProjectPath() const
{
    return data->m_state.currentProjectPath;
}

QString VCSBasePluginState::currentProjectName() const
{
    return data->m_state.currentProjectName;
}

QString VCSBasePluginState::currentProjectTopLevel() const
{
    return data->m_state.currentProjectTopLevel;
}

QStringList VCSBasePluginState::relativeCurrentProject() const
{
    QStringList rc;
    QTC_ASSERT(hasProject(), return rc)
    if (data->m_state.currentProjectTopLevel != data->m_state.currentProjectPath)
        rc.append(QDir(data->m_state.currentProjectTopLevel).relativeFilePath(data->m_state.currentProjectPath));
    return rc;
}

bool VCSBasePluginState::hasTopLevel() const
{
    return data->m_state.hasFile() || data->m_state.hasProject();
}

QString VCSBasePluginState::topLevel() const
{
    return hasFile() ? data->m_state.currentFileTopLevel : data->m_state.currentProjectTopLevel;
}

bool VCSBasePluginState::equals(const Internal::State &rhs) const
{
    return data->m_state.equals(rhs);
}

bool VCSBasePluginState::equals(const VCSBasePluginState &rhs) const
{
    return equals(rhs.data->m_state);
}

void VCSBasePluginState::clear()
{
    data->m_state.clear();
}

void VCSBasePluginState::setState(const Internal::State &s)
{
    data->m_state = s;
}

bool VCSBasePluginState::isEmpty() const
{
    return data->m_state.isEmpty();
}

bool VCSBasePluginState::hasFile() const
{
    return data->m_state.hasFile();
}

bool VCSBasePluginState::hasProject() const
{
    return data->m_state.hasProject();
}

VCSBASE_EXPORT QDebug operator<<(QDebug in, const VCSBasePluginState &state)
{
    in << state.data->m_state;
    return in;
}

//  VCSBasePlugin
struct VCSBasePluginPrivate {
    explicit VCSBasePluginPrivate(const QString &submitEditorKind);

    const QString m_submitEditorKind;
    Core::IVersionControl *m_versionControl;
    VCSBasePluginState m_state;
    int m_actionState;

    static Internal::StateListener *m_listener;
};

VCSBasePluginPrivate::VCSBasePluginPrivate(const QString &submitEditorKind) :
    m_submitEditorKind(submitEditorKind),
    m_versionControl(0),
    m_actionState(-1)
{
}

Internal::StateListener *VCSBasePluginPrivate::m_listener = 0;

VCSBasePlugin::VCSBasePlugin(const QString &submitEditorKind) :
    d(new VCSBasePluginPrivate(submitEditorKind))
{
}

VCSBasePlugin::~VCSBasePlugin()
{
    delete d;
}

void VCSBasePlugin::initialize(Core::IVersionControl *vc)
{
    d->m_versionControl = vc;
    addAutoReleasedObject(vc);

    Internal::VCSPlugin *plugin = Internal::VCSPlugin::instance();
    connect(plugin->coreListener(), SIGNAL(submitEditorAboutToClose(VCSBaseSubmitEditor*,bool*)),
            this, SLOT(slotSubmitEditorAboutToClose(VCSBaseSubmitEditor*,bool*)));
    // First time: create new listener
    if (!VCSBasePluginPrivate::m_listener)
        VCSBasePluginPrivate::m_listener = new Internal::StateListener(plugin);
    connect(VCSBasePluginPrivate::m_listener,
            SIGNAL(stateChanged(VCSBase::Internal::State, Core::IVersionControl*)),
            this,
            SLOT(slotStateChanged(VCSBase::Internal::State,Core::IVersionControl*)));
}

void VCSBasePlugin::slotSubmitEditorAboutToClose(VCSBaseSubmitEditor *submitEditor, bool *result)
{
    if (debug)
        qDebug() << this << d->m_submitEditorKind << "Closing submit editor" << submitEditor << submitEditor->kind();
    if (submitEditor->kind() == d->m_submitEditorKind)
        *result = submitEditorAboutToClose(submitEditor);
}

Core::IVersionControl *VCSBasePlugin::versionControl() const
{
    return d->m_versionControl;
}

void VCSBasePlugin::slotStateChanged(const VCSBase::Internal::State &newInternalState, Core::IVersionControl *vc)
{
    if (vc == d->m_versionControl) {
        // We are directly affected: Change state
        if (!d->m_state.equals(newInternalState)) {
            d->m_state.setState(newInternalState);
            updateActions(VCSEnabled);
        }
    } else {
        // Some other VCS plugin or state changed: Reset us to empty state.
        const ActionState newActionState = vc ? OtherVCSEnabled : NoVCSEnabled;
        if (d->m_actionState != newActionState || !d->m_state.isEmpty()) {
            d->m_actionState = newActionState;
            const VCSBasePluginState emptyState;
            d->m_state = emptyState;
            updateActions(newActionState);
        }
    }
}

const VCSBasePluginState &VCSBasePlugin::currentState() const
{
    return d->m_state;
}

bool VCSBasePlugin::enableMenuAction(ActionState as, QAction *menuAction)
{
    if (debug)
        qDebug() << "enableMenuAction" << menuAction->text() << as;
    switch (as) {
    case VCSBase::VCSBasePlugin::NoVCSEnabled:
        menuAction->setVisible(true);
        menuAction->setEnabled(false);
        return false;
    case VCSBase::VCSBasePlugin::OtherVCSEnabled:
        menuAction->setVisible(false);
        return false;
    case VCSBase::VCSBasePlugin::VCSEnabled:
        menuAction->setVisible(true);
        menuAction->setEnabled(true);
        break;
    }
    return true;
}

} // namespace VCSBase

#include "vcsbaseplugin.moc"
