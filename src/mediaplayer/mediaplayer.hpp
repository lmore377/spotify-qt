#pragma once

#ifdef USE_DBUS

#include "lib/spotify/api.hpp"
#include "util/parent.hpp"
#include "mediaplayerplayer.hpp"

#include <QCoreApplication>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QStringList>
#include <QMainWindow>

class MediaPlayer: public QDBusAbstractAdaptor
{
Q_OBJECT

	Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

	Q_PROPERTY(bool CanQuit READ canQuit)
	Q_PROPERTY(bool CanRaise READ canQuit)
	Q_PROPERTY(QString Identity READ identity)
	Q_PROPERTY(QString DesktopEntry READ identity)
	Q_PROPERTY(QStringList SupportedUriSchemas READ supportedUriSchemas)
	Q_PROPERTY(QStringList SupportedMimeTypes READ supportedMimeTypes)
	Q_PROPERTY(bool HasTrackList READ hasTrackList)

public:
	MediaPlayer(lib::spt::api &spotify, QObject *parent);

	bool canQuit() const;
	bool hasTrackList() const;
	QString identity() const;
	QStringList supportedUriSchemas() const;
	QStringList supportedMimeTypes() const;

public slots:
	Q_NOREPLY void Quit() const;
	Q_NOREPLY void Raise() const;

private:
	QDBusConnection dBus;
	lib::spt::api &spotify;
};

#endif