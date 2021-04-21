#include "spotify.hpp"

using namespace spt;

Spotify::Spotify(lib::settings &settings, QObject *parent)
	: networkManager(new QNetworkAccessManager(this)),
	lib::spt::api(settings),
	QObject(parent)
{
}

auto Spotify::request(const QString &url) -> QNetworkRequest
{
	constexpr int secsInHour = 3600;

	// See when last refresh was
	auto lastRefresh = seconds_since_epoch() - last_auth;
	if (lastRefresh > secsInHour)
	{
		lib::log::info("Access token probably expired, refreshing");
		try
		{
			lib::spt::api::refresh();
		}
		catch (const std::exception &e)
		{
			lib::log::error("Refresh failed: {}", e.what());
		}
	}

	// Prepare request
	QNetworkRequest request((QUrl("https://api.spotify.com/v1/" + url)));

	// Set header
	request.setRawHeader("Authorization",
		QString("Bearer %1")
			.arg(QString::fromStdString(settings.account.access_token))
			.toUtf8());

	// Return prepared header
	return request;
}

void Spotify::await(QNetworkReply *reply, lib::callback<QByteArray> &callback)
{
	QNetworkReply::connect(reply, &QNetworkReply::finished, this,
		[reply, callback]()
		{
			callback(reply->readAll());
			reply->deleteLater();
		});
}

auto Spotify::error_message(const std::string &url, const std::string &data) -> std::string
{
	nlohmann::json json;
	try
	{
		if (!data.empty())
		{
			json = nlohmann::json::parse(data);
		}
	}
	catch (const std::exception &e)
	{
		lib::log::warn("{} failed: {}", url, e.what());
		return std::string();
	}

	if (json.is_null() || !json.is_object() || !json.contains("error"))
	{
		return std::string();
	}

	auto message = json.at("error").at("message").get<std::string>();
	if (!message.empty())
	{
		lib::log::error("{} failed: {}", url, message);
	}
	return message;
}

void Spotify::get(const std::string &url, lib::callback<nlohmann::json> &callback)
{
	await(networkManager->get(request(QString::fromStdString(url))),
		[url, callback](const QByteArray &data)
		{
			try
			{
				// Parse reply as json
				callback(data.isEmpty()
					? nlohmann::json()
					: nlohmann::json::parse(data.toStdString()));
			}
			catch (const std::exception &e)
			{
				lib::log::error("{} failed: {}", url, e.what());
			}
		});
}

void Spotify::select_device(const std::vector<lib::spt::device> &devices,
	lib::callback<lib::spt::device> &callback)
{
	DeviceSelectDialog dialog(devices, dynamic_cast<QWidget *>(parent()));

	if (dialog.exec() == QDialog::Accepted)
	{
		auto selected = dialog.selectedDevice();
		if (!selected.id.empty())
		{
			callback(selected);
		}
	}
}

void Spotify::put(const std::string &url, const nlohmann::json &body,
	lib::callback<std::string> &callback)
{
	// Set in header we're sending json data
	auto req = request(QString::fromStdString(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader,
		QString("application/json"));

	auto data = body.is_null()
		? QByteArray()
		: QByteArray::fromStdString(body.dump());

	await(networkManager->put(req, data),
		[this, url, body, callback](const nlohmann::json &data)
		{
			auto error = error_message(url, data);

			if (lib::strings::contains(error, "No active device found")
				|| lib::strings::contains(error, "Device not found"))
			{
				devices([this, url, body, error, callback]
					(const std::vector<lib::spt::device> &devices)
				{
					if (devices.empty())
					{
						if (callback)
						{
							callback(error);
						}
					}
					else
					{
						select_device(devices, [this, url, body, callback, error]
							(const lib::spt::device &device)
						{
							if (device.id.empty())
							{
								callback(error);
								return;
							}

							this->set_device(device, [this, url, body, callback]
								(const std::string &status)
							{
								if (status.empty())
								{
									this->put(url, body, callback);
								}
							});
						});
					}
				});
			}
			else if (callback)
			{
				callback(error);
			}
		});
}

void Spotify::post(const std::string &url,
	lib::callback<std::string> &callback)
{
	auto req = request(QString::fromStdString(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader,
		"application/x-www-form-urlencoded");

	await(networkManager->post(req, QByteArray()),
		[url, callback](const nlohmann::json &data)
		{
			callback(error_message(url, data));
		});
}

void Spotify::del(const std::string &url, const nlohmann::json &json,
	lib::callback<std::string> &callback)
{
	auto req = request(QString::fromStdString(url));
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	auto data = json.is_null()
		? QByteArray()
		: QByteArray::fromStdString(json.dump());

	await(networkManager->sendCustomRequest(req, "DELETE", data),
		[url, callback](const QByteArray &data)
		{
			callback(error_message(url, data.toStdString()));
		});
}

auto Spotify::request_refresh(const std::string &post_data,
	const std::string &authorization) -> std::string
{
	// Create request
	QNetworkRequest request(QUrl("https://accounts.spotify.com/api/token"));
	request.setHeader(QNetworkRequest::ContentTypeHeader,
		"application/x-www-form-urlencoded");
	request.setRawHeader("Authorization",
		QString::fromStdString(authorization).toUtf8());

	// Send request
	auto *reply = networkManager->post(request, QByteArray::fromStdString(post_data));
	while (!reply->isFinished())
	{
		QCoreApplication::processEvents();
	}

	return reply->readAll().toStdString();
}

auto Spotify::tryRefresh() -> bool
{
	auto *parentWidget = dynamic_cast<QWidget *>(parent());

	try
	{
		refresh();
	}
	catch (const nlohmann::json::exception &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Failed to parse response from Spotify:\n%1").arg(e.what()));
		return false;
	}
	catch (const lib::spotify_error &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Unexpected response:\n%1").arg(e.what()));
		return false;
	}
	catch (const std::exception &e)
	{
		QMessageBox::warning(parentWidget, "Connection failed",
			QString("Failed to connect to Spotify, check your connection and try again:\n%1")
				.arg(e.what()));
		return false;
	}

	return true;
}
