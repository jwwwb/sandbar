# Sandbar Code Documentation

Trying to keep an overview of all the classes, members and their interactions
in one place, to avoid redundant code and adhere to MVC programming style.


## Classes

* **MainWindow:** the main controller for the gui.
* **Model:** the brain of the whole thing.
* **Playlist:** an object that contains a list of MediaFiles.
* **MediaFile:** all media, meta and file data of a song.
* **PlaybackController:** controls and monitors AudioDecoder and PortAudio.
* **AudioDecoder:** reads a media file from disk and monitors output progress.
* **PortAudioInterface:** PA stream variables and some functions to control it.
* **UI:** provides the main input for the user, and some visualization.

## Threads

* **GUIthread:** Qt handles this for me, keeps the interface responsive.
* **Main:** Should be the main event loop, everything goes here by default.
* **decoderThread:** Uses a QTimer to periodically track the progress of PA.
* **paInterfaceThread:** Should the PA interface also be its own thread? // TODO
* **(paCallback):** Not actually something I have control over, run at hi-prio.

## Interactions

### Interactions initiated by MainWindow with...
#### Model:
* addToPlaylist
* changePlaybackOrder   // TODO
* clearPlaylist
* getInFocusPlaylist
* getInFocusPlaylistIndex
* getPlayingPlaylist
* getPlayingPlaylistIndex
* getCurrentPlaylistIndex
* getPlaylistSettings
* newPlaylist
* removePlaylist
* renamePlaylist    // TODO
* requestNextFile
* requestPreviousFile
* requestRandomFile
* switchPlaylist

#### PlaybackController:
* pausePushed
* playPushed
* stopPushed
* seekFile
* seekDifferential
* setVolume
* setVolumeDeciBel

#### UI:
* updatePlaybackInformation


### Interactions initiated by Model with...
#### MainWindow:
* signalNoMoreFiles

#### AudioDecoder:
* signalNoMoreFiles
* signalCurrentFile/slotFile


### Interactions initiated by PlaybackController with...
#### AudioDecoder:
* play
* stop
* seek
* setVolume     // TODO maybe have controller do it directly.
* that's it. everything with file loads/playback is model or PAint

#### PortAudioInterface:
* initialize
* open stream
* pause
* resume


### Interactions initiated by AudioDecoder with...
#### MainWindow:
* signalDuration
* signalPlaybackProgress
* signalFileEnded

#### Model:
* signalRequestFile
* signalFileEnded

#### PlaybackController:
* signalDuration
* signalPlaybackProgress

#### PortAudioInterface:
* bufferReadyForPlayback
* finishedPlaying


### Interactions initiated by UI with...
#### MainWindow:
* actionAdd_File // TODO folders
* actionNew_Playlist
* actionOpen // TODO folders
* openPlaylist // TODO
* preferences // TODO
* randomMenuEntry // TODO
* savePlaylist // TODO
* progressSlider
* playlistTabs_currentChanged
* playlistTabs_tabBarDoubleClicked
* playlistTabs_tabClose
* pushButtonClear
* pushButtonNext
* pushButtonPause
* pushButtonPlay
* pushButtonPrevious
* pushButtonRandom
* pushButtonStop
* volumeSlider
* changePlaybackOrder // TODO
* mediaFileDoubleclick // see below
* receiveGlobalHotkey? // TODO

#### Model:
* jumpToFile    // TODO this should be more intermediated by the controller


## Member attributes
### MainWindow
* playlistInFocus
* listOfPlaylistIDs
* listOfPlaylistNames
* listOfTableViewPointers
* model
* playback
* UI
* freezeUpdates

### Model
* listOfPlaylists   //  TODO Q-ify
* playlistInFocus
* entryNumSelected
* playlistPlaying
* entryNumPlaying
* playlistSettings
* settings  // TODO: actually this will be managed by QSettings, so not directly in the model 

### MediaFile
* static formatContext
* static tag
* static codecContext
* public location   // TODO does it make any sense to encapsulate these?
* public allData    //

### Playlist
* mediaFiles
* identifier    // TODO unify playlist numbers/identifiers globally.
* playlistName

### PlaybackController
* dataPointer
* paused
* currentVolume
* currentProgress
* fileDuration
* isPlaying     // TODO re-implement
* currentMediaFile  // TODO

### AudioDecoder
* all the AV Contexts
* bufferData
* audioStream
* fileOpen
* initialized
* duration
* currentTime
* timer

## Private Methods
### MainWindow
* connectSignals
* deletePlaylist
* getSelectedInPlaylist
* initializePlaylistTable
* newPlaylist
* updatePlaylist

### Model
* playlistNames     // TODO not sure if I really need these...
* playlistIDs   //

### AudioDecoder
* fillUpBuffer
* finishFile
* getTime
* loadFile
* readNextPacket

# Other Brainstorming for now

Not quite sure what to use threads for, and how many. Obviously GUI and main loop
need to be in their own threads, and the decoder loop as well, however now that
a QTimer is managing the decoder loop, I'm unsure if the AudioDecoder has to be
in its own thread as well. The PortAudio callback obviously runs at its own level,
so the PortAudioInterface doesn't really justify an own thread either. Potentially
the extracting data method for MediaFiles could even be in its own thread though,
when adding large playlists - will have to see how long it takes in practice.