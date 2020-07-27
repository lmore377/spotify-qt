import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

ApplicationWindow {
	id: root
	title: "spotify-qtquick"
	visible: true
	width: 540
	height: 960
	
	readonly property bool inPortrait: root.width < root.height

	function icSrc(name) {
		return "qrc:/res/ic/dark/%1.svg".arg(name)
	}

	ToolBar {
		id: toolBar
		z: inPortrait ? 0 : 1
		parent: root.overlay
		width: root.width
		RowLayout {
			anchors.fill: parent
			ToolButton {
				icon.name: "application-menu"
				icon.source: icSrc("application-menu")
				visible: inPortrait
				onClicked: function () {
					drawer.visible = !drawer.visible
				}
			}
			Label {
				Layout.fillWidth: true
				Layout.leftMargin: inPortrait ? 0 : 12
				text: "No media playing"
				font.pixelSize: 20
			}
			ToolButton {
				icon.name: "media-optical-audio"
				icon.source: icSrc("media-optical-audio")
			}
			ToolButton {
				icon.name: "speaker"
				icon.source: icSrc("speaker")
				text: "Device"
			}
			ToolButton {
				icon.name: "configure"
				icon.source: icSrc("configure")
				text: "Settings"
			}
		}
	}

	footer: ColumnLayout {
		id: footer
		anchors {
			left: parent.left
			right: parent.right
			leftMargin: !inPortrait ? drawer.width : 12
			rightMargin: !inPortrait ? 0 : 12
		}
		RowLayout {
			Layout.topMargin: 16
			Slider {
				Layout.fillWidth: true
				value: 0
			}
			Label {
				text: "0:00/0:00"
			}
		}
		RowLayout {
			Layout.bottomMargin: 6
			Item {
				Layout.fillWidth: true
			}
			ToolButton {
				icon.name: "media-playlist-shuffle"
				icon.source: icSrc("media-playlist-shuffle")
				checkable: true
			}
			ToolButton {
				icon.name: "media-playlist-repeat"
				icon.source: icSrc("media-playlist-repeat")
				checkable: true
			}
			ToolSeparator {
			}
			ToolButton {
				icon.name: "media-skip-backward"
				icon.source: icSrc("media-skip-backward")
			}
			ToolButton {
				icon.name: "media-playback-start"
				icon.source: icSrc("media-playback-start")
			}
			ToolButton {
				icon.name: "media-skip-forward"
				icon.source: icSrc("media-skip-forward")
			}
			ToolSeparator {
			}
			ToolButton {
				icon.name: "audio-volume-high"
				icon.source: icSrc("audio-volume-high")
			}
			ToolButton {
				icon.name: "overflow-menu"
				icon.source: icSrc("overflow-menu")
			}
			Item {
				Layout.fillWidth: true
			}
		}
	}

	Drawer {
		id: drawer
		width: inPortrait ? root.width * 0.66 : 400
		height: root.height - (inPortrait ? 0 : toolBar.height)
		y: inPortrait ? 0 : toolBar.height

		modal: inPortrait
		interactive: inPortrait
		position: inPortrait ? 0 : 1
		visible: !inPortrait

		ColumnLayout {
			anchors.fill: parent
			StackLayout {
				currentIndex: drawerTabs.currentIndex
				// Search
				Item {
					Layout.fillHeight: true
					RowLayout {
						anchors {
							bottom: parent.bottom
							left: parent.left
							right: parent.right
							margins: 16
						}
						TextField {
							Layout.fillWidth: true
							placeholderText: "Search query"
						}
						ToolButton {
							id: menuSearchSelectedType
							icon.name: "view-media-track"
							onClicked: menuSearchType.open()
							Menu {
								id: menuSearchType
								onClosed: console.log(menuSearchSelectedType.icon.name)
								MenuItem {
									text: "Tracks"
									icon.name: "view-media-track"
									onClicked: menuSearchSelectedType.icon.name = "view-media-track"
								}
								MenuItem {
									text: "Artists"
									icon.name: "view-media-artist"
									onClicked: menuSearchSelectedType.icon.name = "view-media-artist"
								}
								MenuItem {
									text: "Albums"
									icon.name: "view-media-album-cover"
									onClicked: menuSearchSelectedType.icon.name = "view-media-album-cover"
								}
								MenuItem {
									text: "Playlists"
									icon.name: "view-media-playlist"
									onClicked: menuSearchSelectedType.icon.name = "view-media-playlist"
								}
							}
						}
					}
				}
				// Library
				Item {
					Label {
						text: "library"
					}
				}
				// Playlists
				Item {
					Label {
						text: "playlists"
					}
				}
			}
			TabBar {
				id: drawerTabs
				Layout.alignment: Qt.AlignBottom
				Layout.fillWidth: true
				Layout.leftMargin: 8
				Layout.rightMargin: 8
				currentIndex: 2
				TabButton {
					icon.name: "search"
					text: "Search"
				}
				TabButton {
					icon.name: "bookmarks-toolbar"
					text: "Library"
				}
				TabButton {
					icon.name: "view-media-playlist"
					text: "Playlists"
				}
			}
			RowLayout {
				Layout.margins: 8
				ToolButton {
					Layout.fillWidth: true
					icon.name: "help-about"
					text: "spotify-qtquick v3.0-alpha.1"
					enabled: false
				}
				ToolButton {
					icon.name: "im-user-away"
				}
			}
		}
	}

	Component {
		id: listDelegate
		RowLayout {
			id: listRow
			height: trackList.height / 20
			width: trackList.width - 32
			x: 16
			Label {
				text: model.track
				Layout.preferredWidth: trackList.width / 3
			}
			Label {
				text: model.artist
			}
			Item {
				Layout.fillWidth: true
			}
			ToolButton {
				icon.name: "overflow-menu"
				icon.source: icSrc("overflow-menu")
				flat: true
			}
		}
	}

	Component {
		id: listSeparator
		Rectangle {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.leftMargin: 16
			anchors.rightMargin: anchors.leftMargin
			height: 1
			color: "#9e9e9e"
			opacity: 0
		}
	}

	ListView {
		id: trackList
		anchors.fill: parent
		anchors.leftMargin: !inPortrait ? drawer.width : 0
		anchors.topMargin: toolBar.height
		section.property: "track"
		section.criteria: ViewSection.FullString
		section.delegate: listSeparator
		model: ListModel {
			id: listModel
			Component.onCompleted: {
				for (var i = 0; i < 200; i++) {
					listModel.append({
						"artist": Math.round(Math.random() * 1024 * 1024).toString(16),
						"track": Math.round(Math.random() * 1024 * 1024).toString(16)
					})
				}
			}
		}
		delegate: listDelegate
	}
}
