A virtual reality video player for Linux, based on Valve's openvr `hellovr_opengl` sample code: https://github.com/ValveSoftware/openvr/tree/master/samples

Currently only works with stereo video and equirectangular cube maps (youtube 360 videos) when used for vr videos, but if the vr video player is launched with the `--plane` option then you can view
the video as a regular video in vr without depth (like a cinema).

You can view videos directly in the VR headset or as an overlay in the SteamVR menu if you use the --overlay option.

## Note
Might not work when using a compositor such as picom when using the glx backend (when capturing a window).

# Building
Run `./build.sh` or if you are running Arch Linux, then you can find it on aur under the name vr-video-player-git (`yay -S vr-video-player-git`).\
Dependencies needed when building using `build.sh`: `glm, glew, sdl2, openvr, libx11, libxcomposite, libxfixes, libmpv, libxdo (xdotool)`.

# How to use
vr-video-player has two options. Either capture a window and view it in vr (works only on x11) or a work-in-progress built-in mpv option.
# Using the built-in video player
To play a video with the built-in mpv player, run vr-video-player like so:
```
./vr-video-player --sphere --video <file-path>
```
for example:
```
./vr-video-player --sphere --video /home/adam/Videos/my-cool-vr-video.mp4
```
`--sphere` can be replaced with `--flat`, `plane` or `--sphere360` for different display modes.

# Capturing a window
Install xdotool and launch a video in a video player (I recommend mpv, because browsers, smplayer and vlc player remove the vr for 360 videos) and resize it to fit your monitor or larger for best quality and then,

if you want to watch 180 degree stereoscopic videos then run:
```
./vr-video-player $(xdotool selectwindow)
```
and click on your video player.

if you want to watch side-by-side stereoscopic videos or games (flat) then run:
```
./vr-video-player --flat $(xdotool selectwindow)`
```
and click on your video player.

if the side-by-side video is mirrored so that the left side is on the right side of the video, then run:
```
./vr-video-player --flat --right-left $(xdotool selectwindow)
```
and click on your video player.

if you want to watch a regular non-stereoscopic video, then run:
```
./vr-video-player --plane $(xdotool selectwindow)
```
and click on your video player.

if you want to watch a 360 video (for example a youtube video), then run:
```
./vr-video-player --sphere360 $(xdotool selectwindow)
```
and click on your video player.

Alternatively, you can run:
```
./vr-video-player --follow-focused
```
and vr-video-player will automatically select the focused window (and update when the focused window changes).

To display a window as an overlay in SteamVR you can run:
```
./vr-video-player --plane --overlay $(xdotool selectwindow)
```
and click on the window you want to view in your VR headset.

# Input options
The video might not be in front of you, so to move the video in front of you, you can do any of the following:
* Pull the trigger on the vr controller
* Press "Alt + F1"
* Press the "W" key while the vr-video-player is focused
* Press the select/back button on an xbox controller while the application is focused
* Send a SIGUSR1 signal to the application, using the following command: `killall -USR1 vr-video-player`

You can close the application by pressing Esc when the application is focused.

You can zoom the view with Alt + Q/E.

When using the built-in video player and the vr window is focused you can then use left/right arrow keys to move back/forward in the video and space to pause. In the future vr-video-player will show a graphical interface inside vr to manipulate the video.

You can launch vr-video-player without any arguments to show a list of all arguments.

Note: If the cursor position is weird and does not match what you are seeing in stereoscopic vr mode, then try running the vr video player with the --cursor-wrap option:

--cursor-wrap and --no-cursor-wrap changes the behavior of the cursor in steroscopic mode. Usually in games the game view is mirrored but the cursor is not and the center of the
game which is normally at the center moves to 1/4 and 3/4 of the window. With --cursor-wrap, the cursor position in VR will match the real position of the
cursor relative to the window and with --no-cursor-wrap the cursor will match the position of the cursor as the game sees it.

Note: --cursor-scale is set to 0 by default in 180 degrees stereoscopic mode and 2 in other modes. Also --no-cursor-wrap is set by default in --flat mode.\
Note: When fullscreening a video, the video player can add black bars to the top and bottom. If you do not want black blacks then you either have to view the video in windowed mode or try launching mpv with the `--panscan=1.0` option.

# Games
This vr video player can also be used to play games in VR to to get a 3D effect, and even for games that don't support VR.\
For games such as Trine that have built-in side-by-side view, you can launch it with proton and there is a launch option for side-by-side view. Select this and when the game launches, get the X11 window id of the game
and launch vr video player with the `--flat` option.\
For games that do not have built-in side-by-side view, you can use [ReShade](https://reshade.me/) (or [vkBasalt](https://github.com/DadSchoorse/vkBasalt) for linux native games) and [SuperDepth3D_VR.fx](https://github.com/BlueSkyDefender/Depth3D) effect with proton. This will make the game render with side-by-side view and you can then get the X11 window id of the game and launch vr video player with the `--flat` option. The game you are playing might require settings to be changed manually in ReShade for SuperDepth3D_VR to make it look better.

# SteamVR issues
SteamVR on linux has several issues. For example if you launch vr-video-player it may get stuck with a "Next up" window inside vr. If that is the case, then close SteamVR and make sure all SteamVR are dead (kill them if they aren't) and launch vr-video-player and it should launch SteamVR (this is different than launching the SteamVR application in steam).

# Screenshots
![](https://www.dec05eba.com/images/vr-video-player.jpg)
