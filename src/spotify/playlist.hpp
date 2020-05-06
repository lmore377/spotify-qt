#pragma once

namespace spt { class Playlist; }

#include "track.hpp"
#include "spotify.hpp"
#include <utility>

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

namespace spt
{
	class Playlist
	{
	public:
		Playlist(const QJsonObject &json);
		bool collaborative;
		QString description, id, image, name, snapshot, ownerId, ownerName;
		bool isPublic;
		QVector<Track> loadTracks(Spotify &spotify);
		QJsonObject toJson(Spotify &spotify);
	private:
		QJsonObject tracks;
		static bool loadTracksFromUrl(QVector<Track> &trackList, QString &url, int offset, Spotify &spotify);
	};
}