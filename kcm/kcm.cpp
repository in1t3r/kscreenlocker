/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2014 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "kcm.h"
#include "lockermodel.h"
#include "kscreensaversettings.h"
#include "ui_kcm.h"
#include "screenlocker_interface.h"
#include <config-ksmserver.h>
#include <KCModule>
#include <KPluginFactory>
#include <QVBoxLayout>
#include <QQuickWidget>
#include <QtQml>
#include <QQmlEngine>
#include <QQmlContext>
#include <QMessageBox>

#include <Plasma/Package>
#include <Plasma/PluginLoader>

class ScreenLockerKcmForm : public QWidget, public Ui::ScreenLockerKcmForm
{
    Q_OBJECT
public:
    explicit ScreenLockerKcmForm(QWidget *parent);
};

ScreenLockerKcmForm::ScreenLockerKcmForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}



ScreenLockerKcm::ScreenLockerKcm(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    qmlRegisterType<LockerModel>();
    ScreenLockerKcmForm *ui = new ScreenLockerKcmForm(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(ui);

    addConfig(KScreenSaverSettings::self(), ui);

    m_model = new LockerModel(this);

    m_quickWidget = new QQuickWidget(this);
    m_quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    Plasma::Package package = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    package.setDefaultPackageRoot("plasma/kcms");
    package.setPath("screenlocker_kcm");
    m_quickWidget->rootContext()->setContextProperty("kcm", this);
    m_quickWidget->setSource(QUrl::fromLocalFile(package.filePath("mainscript")));
    setMinimumHeight(m_quickWidget->initialSize().height());

    layout->addWidget(m_quickWidget);
}

void ScreenLockerKcm::load()
{
    QString currentPlugin = KScreenSaverSettings::theme();
    if (currentPlugin.isEmpty()) {
        currentPlugin = m_access.metadata().pluginName();
    }
    setSelectedPlugin(currentPlugin);

    const QList<Plasma::Package> pkgs = LookAndFeelAccess::availablePackages("lockscreenmainscript");
    for (const Plasma::Package &pkg : pkgs) {
        QStandardItem* row = new QStandardItem(pkg.metadata().name());
        row->setData(pkg.metadata().pluginName(), LockerModel::PluginNameRole);
        row->setData(pkg.filePath("lockscreen", "screenshot.png"), LockerModel::ScreenhotRole);
        m_model->appendRow(row);
    }
}

LockerModel *ScreenLockerKcm::lockerModel()
{
    return m_model;
}

QString ScreenLockerKcm::selectedPlugin() const
{
    return m_selectedPlugin;
}

void ScreenLockerKcm::setSelectedPlugin(const QString &plugin)
{
    if (m_selectedPlugin == plugin) {
        return;
    }

    m_selectedPlugin = plugin;
    emit selectedPluginChanged();
    changed();
}

void ScreenLockerKcm::test(const QString &plugin)
{
    if (plugin.isEmpty() || plugin == "none") {
        return;
    }

    QProcess proc;
    QStringList arguments;
    arguments << plugin << "--testing";
    if (proc.execute(KSCREENLOCKER_GREET_BIN, arguments)) {
        QMessageBox::critical(this, i18n("Error"), i18n("Failed to successfully test the screen locker."));
    }
}

void ScreenLockerKcm::save()
{
    KCModule::save();

    KScreenSaverSettings::setTheme(m_selectedPlugin);

    // reconfigure through DBus
    OrgKdeScreensaverInterface interface(QStringLiteral("org.kde.screensaver"),
                                         QStringLiteral("/ScreenSaver"),
                                         QDBusConnection::sessionBus());
    if (interface.isValid()) {
        interface.configure();
    }
}

K_PLUGIN_FACTORY_WITH_JSON(ScreenLockerKcmFactory,
                           "screenlocker.json",
                           registerPlugin<ScreenLockerKcm>();)

#include "kcm.moc"