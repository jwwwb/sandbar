# Sandbar Code Documentation

Trying to keep an overview of all the classes, members and their interactions
in one place, to avoid redundant code and adhere to MVC programming style.


## Files (and contained classes)

* **MainWindow:** the main controller for the gui (MainWindow)
* **Model:** the brain of the whole thing (Model, Playlist)
* **MediaFile:** location and metadata of a song (MediaFile, some enums)
* **PlaybackController:** bridges between the ui and audio (PlaybackController)
* **AudioPlayback:** decodes and outputs media files (AudioPlayback)
* **mainwindow.ui:** the view (UI)


## Classes

* **MainWindow:** the main controller for the gui.
* **Model:** the brain of the whole thing.
* **Playlist:** an object that contains a list of MediaFiles, and some state 
    info.
* **MediaFile:** all media, meta and file data of a song.
* **PlaybackController:** controls and monitors AudioPlayback.
* **AudioPlayback:** reads a media file from disk and outputs it.
* **UI:** provides the main input for the user, and some visualization.


## Interactions

### Interactions initiated by UI with...
#### MainWindow:
* playButton
* pauseButton
* stopButton
* nextButton
* previousButton
* randomButton
* volumeSlider
* progressSlider
* openDialogue
* addDialogue
* newPlaylist
* playlistTabDoubleclick
* closePlaylist
* clearPlaylist
* switchPlaylist
* openPlaylist
* savePlaylist
* mediaFileDoubleclick
* preferences
* randomMenuEntry
* changePlaybackOrder
* receiveGlobalHotkey? // TODO

### Interactions initiated by MainWindow with...
#### Model:
* newPlaylist
* switchPlaylist
* clearPlaylist
* addToPlaylist
* renamePlaylist
* changePlaylistCurrentEntry
* changePlaybackOrder
#### PlaybackController:
* seekPlayback
* play
* pause
* stop
* changeMediaFile
* changeVolume

### Interactions initiated by PlaybackController with...
#### Model:
* getNextMediaFile?


## Member attributes
### MainWindow
* listOfTableViewPointers // TODO
* model

### Model
* listOfPlaylists
* playlistSettings
* playlistInFocus

### Playlist
* currentEntry

### PlaybackController
* currentVolume
* currentPlayProgres
* isPlaying
* currentMediaFile