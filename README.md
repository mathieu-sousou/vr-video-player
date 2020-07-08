A virtual reality video player for Linux running X11, based on Valve's openvr `hellovr_opengl` sample code: https://github.com/ValveSoftware/openvr/tree/master/samples
Still early in development, not very user friendly.

Currently only works with stereo video when used for vr videos, but if the vr video player is launched with the `--plane` option then you can view
the video as a regular video in vr without depth (like a cinema).

# Building
vr video player can be built by running `./build.sh` or by using [sibs](https://github.com/DEC05EBA/sibs) or if you are running Arch Linux, then you can find it on aur under the name vr-video-player-git (`yay -S vr-video-player-git`).
Dependencies required: `glm, glew, sdl2, openvr and x11`.

# How to use
Install xdotool and launch a video in a video player (I recommend mpv) and fullscreen it or resize it to fit your monitor for best quality and then,

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

The video might not be in front of you, so to move the video in front of you, you can pull the trigger on the vr controller or press the "W" key while the vr-video-player is focused or press the select/back button on an xbox controller while the application is focused. You can also send the SIGUSR1 signal to the application, using the following command:
```
killall -USR1 vr-video-player`
```

You can launch vr-video-player without any arguments to show a list of all arguments:
```
usage: vr-video-player [--flat] [--left-right|--right-left|--plane] [--stretch|--no-stretch] [--zoom zoom-level] [--cursor-scale scale] <window_id>
```

# Games
This vr video player can also be used to play games in VR to to get a 3D effect, and even for games that don't support VR.\
For games such as Trine that have built-in side-by-side view, you can launch it with proton and there is a launch option for side-by-side view. Select this and when the game launches, get the X11 window id of the game
and launch vr video player with the `--flat` option.\
For games that do not have built-in side-by-side view, you can use [ReShade](https://reshade.me/) (or [vkBasalt](https://github.com/DadSchoorse/vkBasalt) for linux native games) and [SuperDepth3D_VR.fx](https://github.com/BlueSkyDefender/Depth3D) effect with proton. This will make the game render with side-by-side view and you can then get the X11 window id of the game and launch vr video player with the `--flat` option. The game you are playing might require settings to be changed manually in ReShade for SuperDepth3D_VR to make it look better.

# Images
`images/arrow.png` is from the adwaita project and it's licensed under Creative Commons Attribution-Share Alike 3.0 United States License. To view a copy of the license, visit http://creativecommons.org/licenses/by-sa/3.0/.
