//========= Copyright Valve Corporation ============//
//Original BSD 3 License by Valve Corporation:
//Copyright (c) 2015, Valve Corporation
//All rights reserved.

//Redistribution and use in source and binary forms, with or without modification,
//are permitted provided that the following conditions are met:

//1. Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.

//2. Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation and/or
//other materials provided with the distribution.

//3. Neither the name of the copyright holder nor the names of its contributors
//may be used to endorse or promote products derived from this software without
//specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
//ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


// Modified by: DEC05EBA

#include <GL/glew.h>
#include "../include/window_texture.h"
#include "../include/mpv.hpp"
#include "../include/config.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <openvr.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>

#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>

#include <unistd.h>
#include <signal.h>
#include <libgen.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <thread>
#include <mutex>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

static bool g_bPrintf = true;

enum class ViewMode {
	LEFT_RIGHT,
	RIGHT_LEFT,
	PLANE,
	SPHERE360
};

enum class ProjectionMode {
	SPHERE,
	FLAT,
	CYLINDER, /* aka plane */
	SPHERE360
};

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication( int argc, char *argv[] );
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
    void zoom_in();
    void zoom_out();
	void ProcessVREvent( const vr::VREvent_t & event );
	void RenderFrame();

	void ResetRotation();
	void MoveCursor(float x, float y);
	void MouseButton(int button, bool down);

	void SetupScene();
	void AddCubeToScene( const glm::mat4 &mat, std::vector<float> &vertdata );

	bool SetupStereoRenderTargets();
	void SetupCompanionWindow();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderCompanionWindow();
	void RenderScene( vr::Hmd_Eye nEye );

	glm::mat4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
	glm::mat4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
	glm::mat4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
	void UpdateHMDMatrixPose();

	glm::mat4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );

	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	bool CreateAllShaders();

	bool SetCursorFromX11CursorImage(XFixesCursorImage *x11_cursor_image);
	// Get focused window or None
	Window get_focused_window();

	void save_config();

	int exit_code = 0;
	bool bQuit = false;
private: 
	bool m_bDebugOpenGL;
	bool m_bVblank;
	bool m_bGlFinishHack;

	vr::IVRSystem *m_pHMD;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	glm::mat4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];

private: // SDL bookkeeping
	SDL_Window *m_pCompanionWindow;
	uint32_t m_nCompanionWindowWidth;
	uint32_t m_nCompanionWindowHeight;

	SDL_GLContext m_pContext;
	SDL_GLContext m_pMpvContext;

private: // OpenGL bookkeeping
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	bool m_bResetRotation;

	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class

	int m_iSceneVolumeWidth;
	int m_iSceneVolumeHeight;
	int m_iSceneVolumeDepth;
	float m_fScaleSpacing;
	float m_fScale;
	
	int m_iSceneVolumeInit;                                  // if you want something other than the default 20x20x20
	
	float m_fNearClip;
	float m_fFarClip;

	unsigned int m_uiVertcount;

	GLuint m_glSceneVertBuffer;
	GLuint m_unSceneVAO;
	GLuint m_unCompanionWindowVAO;
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	glm::mat4 m_mat4HMDPose;
	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;

	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;

	glm::vec3 current_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 hmd_pos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat hmd_rot = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::quat m_reset_rotation = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);

	Config config;

	struct VertexDataScene
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow( const glm::vec2 & pos, const glm::vec2 tex ) :  position(pos), texCoord(tex) {	}
	};

	GLuint m_unSceneProgramID;
	GLuint m_unCompanionWindowProgramID;

	GLint m_nSceneMatrixLocation;
	GLint m_nSceneTextureOffsetXLocation;
	GLint m_nSceneTextureScaleXLocation;
	GLint m_nCursorLocation;
	GLint m_nArrowSizeLocation = -1;
	GLint m_myTextureLocation = -1;
	GLint m_arrowTextureLocation = -1;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	FramebufferDesc mpvDesc;

	bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
	void set_current_context(SDL_GLContext context);
	bool take_render_update();
	void set_render_update();
	
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	vr::VRActionHandle_t m_actionHideCubes = vr::k_ulInvalidActionHandle;
	vr::VRActionSetHandle_t m_actionsetDemo = vr::k_ulInvalidActionSetHandle;

private: // X compositor
	Display *x_display = nullptr;
	Atom net_active_window_atom;
	Window src_window_id = None;
	WindowTexture window_texture;
	bool follow_focused = false;
	bool focused_window_changed = true;
	bool focused_window_set = false;
	const char *mpv_file = nullptr;
	Mpv mpv;
	std::mutex mpv_render_update_mutex;
	bool mpv_render_update = false;
	int64_t mpv_video_width = 0;
	int64_t mpv_video_height = 0;
	bool mpv_video_loaded = false;
	bool mpv_loaded_in_thread = false;
	bool running = true;
	std::mutex context_mutex;

	std::thread mpv_thread;

	int mouse_x = 0;
	int mouse_y = 0;
	int window_width = 1;
	int window_height = 1;
	Uint32 window_resize_time;
	bool window_resized = false;
	
    bool zoom_resize = false;

	int x_fixes_event_base;
	int x_fixes_error_base;
	int prev_visibility_state = VisibilityFullyObscured;

	GLint pixmap_texture_width = 1;
	GLint pixmap_texture_height = 1;

	ProjectionMode projection_mode = ProjectionMode::SPHERE;
	double zoom = 0.0;
	float cursor_scale = 2.0f;
	ViewMode view_mode = ViewMode::LEFT_RIGHT;
	bool stretch = true;
	bool cursor_wrap = true;
	bool free_camera = false;
	bool reduce_flicker = false;
	bool use_system_mpv_config = false;
	double reduce_flicker_counter = 0.0;

	GLuint arrow_image_texture_id = 0;
	int arrow_image_width = 1;
	int arrow_image_height = 1;
	int cursor_offset_x = 0;
	int cursor_offset_y = 0;

	float cursor_scale_uniform[2];
	double arrow_ratio;
	bool cursor_image_set = false;

	bool config_exists = false;
};


//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and had a rising edge
//---------------------------------------------------------------------------------------------------------------------
bool GetDigitalActionRisingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr )
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bChanged && actionData.bState;
}


//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and had a falling edge
//---------------------------------------------------------------------------------------------------------------------
bool GetDigitalActionFallingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr )
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bChanged && !actionData.bState;
}


//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and its state is true
//---------------------------------------------------------------------------------------------------------------------
bool GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr )
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bState;
}

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsnprintf( buffer, sizeof(buffer), fmt, args );
	va_end( args );

	if ( g_bPrintf )
		printf( "%s", buffer );
}

static void usage() {
	fprintf(stderr, "usage: vr-video-player [--sphere|--sphere360|--flat|--plane] [--left-right|--right-left] [--stretch|--no-stretch] [--zoom zoom-level] [--cursor-scale scale] [--cursor-wrap|--no-cursor-wrap] [--follow-focused|--video video|<window_id>] [--use-system-mpv-config] [--free-camera] [--reduce-flicker]\n");
    fprintf(stderr, "\n");
	fprintf(stderr, "OPTIONS\n");
    fprintf(stderr, "  --sphere                  View the window as a stereoscopic 180 degrees screen (half sphere). The view will be attached to your head in vr. This is recommended for 180 degrees videos. This is the default value\n");
	fprintf(stderr, "  --sphere360               View the window as an equirectangular cube map. This is what is mostly used on youtube, where the video is split into top and bottom as a cubemap. The view will be attached to your head in vr\n");
	fprintf(stderr, "  --flat                    View the window as a stereoscopic flat screen. This is recommended for stereoscopic videos and games\n");
    fprintf(stderr, "  --left-right              This option is used together with --flat, to specify if the left side of the window is meant to be viewed with the left eye and the right side is meant to be viewed by the right eye. This is the default value\n");
    fprintf(stderr, "  --right-left              This option is used together with --flat, to specify if the left side of the window is meant to be viewed with the right eye and the right side is meant to be viewed by the left eye\n");
    fprintf(stderr, "  --plane                   View the window as a slightly curved screen. This is recommended for non-stereoscopic content\n");
    fprintf(stderr, "  --stretch                 This option is used together with --flat, To specify if the size of both sides of the window should be combined and stretch to that size when viewed in vr. This is the default value\n");
    fprintf(stderr, "  --no-stretch              This option is used together with --flat, To specify if the size of one side of the window should be the size of the whole window when viewed in vr. This is the option you want if the window looks too wide\n");
    fprintf(stderr, "  --zoom <zoom>             Change the distance to the window. This should be a positive value. In flat and plane modes, this is the distance to the window when the window is reset (with W key or controller trigger button). The default value is 0 for all modes except sphere mode, where the default value is 1. This value is unused for sphere360 mode\n");
    fprintf(stderr, "  --cursor-scale <scale>    Change the size of the cursor. This should be a positive value. If set to 0, then the cursor is hidden. The default value is 1 for all modes except sphere mode, where the default value is 0. The cursor is always hidden in sphere360 mode\n");
    fprintf(stderr, "  --cursor-wrap             If this option is set, then the cursor position in the vr view will wrap around when it reached the center of the window (i.e when it reaches the edge of one side of the stereoscopic view). This option is only valid for stereoscopic view (flat and sphere modes)\n");
    fprintf(stderr, "  --no-cursor-wrap          If this option is set, then the cursor position in the vr view will match the the real cursor position inside the window\n");
	fprintf(stderr, "  --reduce-flicker          A hack to reduce flickering in low resolution text when the headset is not moving by moving the window around quickly by a few pixels\n");
	fprintf(stderr, "  --free-camera             If this option is set, then the camera wont follow your position\n");
    fprintf(stderr, "  --follow-focused          If this option is set, then the selected window will be the focused window. vr-video-player will automatically update when the focused window changes. Either this option, --video or window_id should be used\n");
	fprintf(stderr, "  --video <video>           Select the video to play (using mpv). Either this option, --follow-focused or window_id should be used\n");
	fprintf(stderr, "  --use-system-mpv-config   Use system (~/.config/mpv/mpv.conf) mpv config. Disabled by default\n");
    fprintf(stderr, "  window_id                 The X11 window id of the window to view in vr. Either this option, --follow-focused or --video should be used\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "EXAMPLES\n");
    fprintf(stderr, "  vr-video-player 1830423\n");
    fprintf(stderr, "  vr-video-player --flat 1830423\n");
    fprintf(stderr, "  vr-video-player --flat --right-left 1830423\n");
    fprintf(stderr, "  vr-video-player --plane --zoom 2.0 1830423\n");
    fprintf(stderr, "  vr-video-player --flat $(xdotool selectwindow)\n");
	fprintf(stderr, "  vr-video-player --sphere --video $HOME/Videos/cool-vr-video.mp4\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Note: All options except window_id are optional\n");
	exit(1);
}

static void get_config_values(const Config &config, ProjectionMode projection_mode, glm::vec3 &pos, glm::quat &rot, float &zoom) {
	switch(projection_mode) {
		case ProjectionMode::SPHERE: {
			pos = config.sphere.position;
			rot = config.sphere.rotation;
			zoom = config.sphere.zoom;
			break;
		}
		case ProjectionMode::FLAT: {
			pos = config.flat.position;
			rot = config.flat.rotation;
			zoom = config.flat.zoom;
			break;
		}
		case ProjectionMode::CYLINDER: {
			pos = config.plane.position;
			rot = config.plane.rotation;
			zoom = config.plane.zoom;
			break;
		}
		case ProjectionMode::SPHERE360: {
			pos = config.sphere360.position;
			rot = config.sphere360.rotation;
			zoom = config.sphere360.zoom;
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication( int argc, char *argv[] )
	: m_pCompanionWindow(NULL)
	, m_pContext(NULL)
	, m_pMpvContext(NULL)
	, m_nCompanionWindowWidth( 800 )
	, m_nCompanionWindowHeight( 600 )
	, m_unSceneProgramID( 0 )
	, m_unCompanionWindowProgramID( 0 )
	, m_pHMD( NULL )
	, m_bDebugOpenGL( false )
	, m_bVblank( false )
	, m_bGlFinishHack( false )
	, m_unSceneVAO( 0 )
	, m_nSceneMatrixLocation( -1 )
	, m_nSceneTextureOffsetXLocation( -1 )
	, m_nSceneTextureScaleXLocation( -1 )
	, m_nCursorLocation( -1 )
	, m_iTrackedControllerCount( 0 )
	, m_iTrackedControllerCount_Last( -1 )
	, m_iValidPoseCount( 0 )
	, m_iValidPoseCount_Last( -1 )
	, m_iSceneVolumeInit( 10 )
	, m_strPoseClasses("")
	, m_bResetRotation( false )
{
	const char *projection_arg = nullptr;
	const char *view_mode_arg = nullptr;
	bool zoom_set = false;
	bool cursor_scale_set = false;
	bool cursor_wrap_set = false;

	for(int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "--sphere") == 0) {
			if(projection_arg) {
				fprintf(stderr, "Error: --sphere option can't be used together with the %s option\n", projection_arg);
				exit(1);
			}
			projection_mode = ProjectionMode::SPHERE;
			projection_arg = argv[i];
		} else if(strcmp(argv[i], "--flat") == 0) {
			if(projection_arg) {
				fprintf(stderr, "Error: --flat option can't be used together with the %s option\n", projection_arg);
				exit(1);
			}
			projection_mode = ProjectionMode::FLAT;
			projection_arg = argv[i];
		} else if(strcmp(argv[i], "--zoom") == 0 && i < argc - 1) {
			zoom = atof(argv[i + 1]);
			++i;
			zoom_set = true;
		} else if(strcmp(argv[i], "--cursor-scale") == 0 && i < argc - 1) {
			cursor_scale = atof(argv[i + 1]);
			++i;
			cursor_scale_set = true;
		} else if(strcmp(argv[i], "--left-right") == 0) {
			if(view_mode_arg) {
				fprintf(stderr, "Error: --left-right option can't be used together with the %s option\n", view_mode_arg);
				exit(1);
			}
			view_mode = ViewMode::LEFT_RIGHT;
			view_mode_arg = argv[i];
		} else if(strcmp(argv[i], "--right-left") == 0) {
			if(view_mode_arg) {
				fprintf(stderr, "Error: --right-left option can't be used together with the %s option\n", view_mode_arg);
				exit(1);
			}
			view_mode = ViewMode::RIGHT_LEFT;
			view_mode_arg = argv[i];
		} else if(strcmp(argv[i], "--plane") == 0) {
			if(projection_arg) {
				fprintf(stderr, "Error: --plane option can't be used together with the %s option\n", projection_arg);
				exit(1);
			}
			if(view_mode_arg) {
				fprintf(stderr, "Error: --plane option can't be used together with the %s option\n", view_mode_arg);
				exit(1);
			}
			view_mode = ViewMode::PLANE;
			projection_mode = ProjectionMode::CYLINDER;
			projection_arg = argv[i];
			view_mode_arg = argv[i];
		} else if(strcmp(argv[i], "--sphere360") == 0) {
			if(projection_arg) {
				fprintf(stderr, "Error: --sphere360 option can't be used together with the %s option\n", projection_arg);
				exit(1);
			}
			if(view_mode_arg) {
				fprintf(stderr, "Error: --sphere360 option can't be used together with the %s option\n", view_mode_arg);
				exit(1);
			}
			view_mode = ViewMode::SPHERE360;
			projection_mode = ProjectionMode::SPHERE360;
			projection_arg = argv[i];
			view_mode_arg = argv[i];
		} else if(strcmp(argv[i], "--stretch") == 0) {
			stretch = true;
		} else if(strcmp(argv[i], "--no-stretch") == 0) {
			stretch = false;
		} else if(strcmp(argv[i], "--cursor-wrap") == 0) {
			cursor_wrap = true;
			cursor_wrap_set = true;
		} else if(strcmp(argv[i], "--no-cursor-wrap") == 0) {
			cursor_wrap = false;
			cursor_wrap_set = true;
		} else if(strcmp(argv[i], "--follow-focused") == 0) {
			if(src_window_id) {
				fprintf(stderr, "Error: window_id option can't be used together with the --follow-focused option\n");
				exit(1);
			}
			if(mpv_file) {
				fprintf(stderr, "Error: --video option can't be used together with the --follow-focused option\n");
				exit(1);
			}
			follow_focused = true;
		} else if(strcmp(argv[i], "--video") == 0 && i < argc - 1) {
			if(src_window_id) {
				fprintf(stderr, "Error: --follow-focused option can't be used together with the --video option\n");
				exit(1);
			}
			if(follow_focused) {
				fprintf(stderr, "Error: window_id option can't be used together with the --video option\n");
				exit(1);
			}
			mpv_file = argv[i + 1];
			++i;
		} else if(strcmp(argv[i], "--use-system-mpv-config") == 0) {
			use_system_mpv_config = true;
		} else if(strcmp(argv[i], "--free-camera") == 0) {
			free_camera = true;
		} else if(strcmp(argv[i], "--reduce-flicker") == 0) {
			reduce_flicker = true;
		} else if(argv[i][0] == '-') {
			fprintf(stderr, "Invalid flag: %s\n", argv[i]);
			usage();
		} else {
			if(follow_focused) {
				fprintf(stderr, "Error: --follow-focused option can't be used together with the window_id option\n");
				exit(1);
			}
			if(mpv_file) {
				fprintf(stderr, "Error: --video option can't be used together with the window_id option\n");
				exit(1);
			}
			if (strncmp(argv[i], "window:", 7) == 0) {
				argv[i] += 7; // "window:".length
			}
			src_window_id = strtol(argv[i], nullptr, 0);
		}
	}

	if(src_window_id == None && !follow_focused && !mpv_file) {
		fprintf(stderr, "Missing required window_id, --follow-focused or --video option\n");
		usage();
	}

	if(!zoom_set && projection_mode != ProjectionMode::SPHERE) {
		zoom = 1.0;
	}

	if(cursor_scale < 0.001f || (!cursor_scale_set && projection_mode == ProjectionMode::SPHERE)) {
		cursor_scale = 0.001f;
	}

	if(!cursor_wrap_set && projection_mode == ProjectionMode::FLAT) {
		cursor_wrap = false;
	}

	if(projection_mode == ProjectionMode::SPHERE360) {
		zoom = 0.0f;
		cursor_scale = 0.001f;
	}

	// other initialization tasks are done in BInit
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));

	cursor_scale_uniform[0] = 0.0f;
	cursor_scale_uniform[1] = 0.0f;

#ifdef _DEBUG
	m_bDebugOpenGL = true;
#endif

	config = read_config(config_exists);
	if(!config_exists)
		return;

	float saved_config = 0.0f;
	get_config_values(config, projection_mode, hmd_pos, m_reset_rotation, saved_config);
	if(!zoom_set)
		zoom = saved_config;
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf( "Shutdown" );
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}

static int xerror(Display *dpy, XErrorEvent *ee) {
    //if (ee->error_code == BadWindow
    //|| (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    //|| (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    //|| (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    //|| (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    //|| (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    //|| (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    //|| (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    //|| (ee->request_code == X_CopyArea && ee->error_code == BadDrawable)
    //|| (ee->request_code == X_GLXCreatePixmap && ee->error_code == BadMatch))
    //    return 0;
    //fprintf(stderr, "vr-video-player: fatal error: request code=%d, error code=%d\n",
    //    ee->request_code, ee->error_code);
    return 0; /* may call exit */ /* TODO: xerrorxlib(dpy, ee); */
}

static void grabkeys(Display *display) {
	unsigned int numlockmask = 0;
    KeyCode numlock_keycode = XKeysymToKeycode(display, XK_Num_Lock);
    XModifierKeymap *modmap = XGetModifierMapping(display);
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < modmap->max_keypermod; ++j) {
            if(modmap->modifiermap[i * modmap->max_keypermod + j] == numlock_keycode)
                numlockmask = (1 << i); 
        }
    }
	XFreeModifiermap(modmap);

    const int num_keys = 3;
    int keys[num_keys] = { XK_F1, XK_q, XK_e };
    
	Window root_window = DefaultRootWindow(display);
    unsigned int modifiers[] = { 0, LockMask, numlockmask, numlockmask|LockMask };
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < num_keys; ++j) {
            XGrabKey(display, XKeysymToKeycode(display, keys[j]), Mod1Mask|modifiers[i], root_window, False, GrabModeAsync, GrabModeAsync);
        }
    }
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{
	x_display = XOpenDisplay(nullptr);
	if (!x_display)
	{
		printf("Failed to open x display\n");
		return false;
	}

	XSetErrorHandler(xerror);

	net_active_window_atom = XInternAtom(x_display, "_NET_ACTIVE_WINDOW", False);
	if(!net_active_window_atom) {
		fprintf(stderr, "Failed to get _NET_ACTIVE_WINDOW atom\n");
		return false;
	}

	if(!XFixesQueryExtension(x_display, &x_fixes_event_base, &x_fixes_error_base)) {
		fprintf(stderr, "Your x11 server is missing the xfixes extension\n");
		return false;
	}

	grabkeys(x_display);

	if(follow_focused)
		XSelectInput(x_display, DefaultRootWindow(x_display), PropertyChangeMask);

	Bool sup = False;
	XkbSetDetectableAutoRepeat(x_display, True, &sup);

	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK ) < 0 )
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		char buf[1024];
		snprintf( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}

	auto standing_pos = m_pHMD->GetSeatedZeroPoseToStandingAbsoluteTrackingPose();
	if(!config_exists)
		hmd_pos += glm::vec3(standing_pos.m[0][3], standing_pos.m[1][3], standing_pos.m[2][3]);

	int nWindowPosX = 700;
	int nWindowPosY = 100;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
	SDL_GL_SetAttribute( SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1 );
	if( m_bDebugOpenGL )
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

	// Needed for mpv
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "no");

	m_pCompanionWindow = SDL_CreateWindow( "vr-video-player", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
	if (m_pCompanionWindow == NULL)
	{
		printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
	if (m_pContext == NULL)
	{
		printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	if(mpv_file) {
		m_pMpvContext = SDL_GL_CreateContext(m_pCompanionWindow);
		if (m_pMpvContext == NULL)
		{
			printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
			return false;
		}
	}

	if(SDL_GL_MakeCurrent(m_pCompanionWindow, m_pContext) < 0) {
		fprintf(stderr, "Failed to make opengl context current, error: %s\n", SDL_GetError());
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
	{
		printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	// cube array
 	m_iSceneVolumeWidth = m_iSceneVolumeInit;
 	m_iSceneVolumeHeight = m_iSceneVolumeInit;
 	m_iSceneVolumeDepth = m_iSceneVolumeInit;
 		
 	m_fScale = 1.0f;
 	m_fScaleSpacing = 2.0f;
 
 	m_fNearClip = 0.01f;
 	m_fFarClip = 30.0f;
 
 	m_uiVertcount = 0;
 
// 		m_MillisecondsTimer.start(1, this);
// 		m_SecondsTimer.start(1000, this);
	
	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	if(mpv_file) {
		mpv_thread = std::thread([&]{
			set_current_context(m_pMpvContext);
			if(!mpv.create(use_system_mpv_config))
				return;

			mpv.load_file(mpv_file);
			set_current_context(NULL);

			while(running) {
				set_current_context(m_pMpvContext);

				if(mpv_video_loaded && !mpv_loaded_in_thread) {
					mpv_loaded_in_thread = true;
					// TODO: Do not create depth buffer and extra framebuffers
					CreateFrameBuffer(mpv_video_width, mpv_video_height, mpvDesc);
				}

				if(mpv_video_loaded) {
					glBindFramebuffer( GL_FRAMEBUFFER, mpvDesc.m_nRenderFramebufferId );
					glViewport(0, 0, mpv_video_width, mpv_video_height);
					if(take_render_update()) {
						glDisable(GL_DEPTH_TEST);

						glBindVertexArray( m_unCompanionWindowVAO );
						glUseProgram( m_unCompanionWindowProgramID );

						mpv.draw(mpvDesc.m_nRenderFramebufferId, mpv_video_width, mpv_video_height);

						glBindVertexArray( 0 );
						glUseProgram( 0 );
					}
					glBindFramebuffer( GL_FRAMEBUFFER, 0 );
					
					glDisable( GL_MULTISAMPLE );

					glBindFramebuffer(GL_READ_FRAMEBUFFER, mpvDesc.m_nRenderFramebufferId );
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mpvDesc.m_nResolveFramebufferId );
					
					glBlitFramebuffer( 0, 0, mpv_video_width, mpv_video_height, 0, 0, mpv_video_width, mpv_video_height, 
						GL_COLOR_BUFFER_BIT,
						GL_LINEAR  );

					glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
					glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );

					glEnable( GL_MULTISAMPLE );
				} else {
					usleep(1000);
				}

				set_current_context(NULL);
			}
		});
	}

	char action_manifest_path[PATH_MAX];
	realpath("config/hellovr_actions.json", action_manifest_path);
	if(access(action_manifest_path, F_OK) == -1) {
		strcpy(action_manifest_path, "/usr/share/vr-video-player/hellovr_actions.json");
		if(access(action_manifest_path, F_OK) == -1) {
			fprintf(stderr, "Unable to find hellovr_action.json!\n");
			return false;
		}
	}

	fprintf(stderr, "Using openvr config file: %s\n", action_manifest_path);

	vr::VRInput()->SetActionManifestPath(action_manifest_path);
	vr::VRInput()->GetActionHandle( "/actions/demo/in/HideCubes", &m_actionHideCubes );
	vr::VRInput()->GetActionSetHandle( "/actions/demo", &m_actionsetDemo );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL()
{
	if( m_bDebugOpenGL )
	{
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
	}

	if( !CreateAllShaders() )
		return false;

	//glEnable(GL_CULL_FACE);

	glUseProgram( m_unSceneProgramID );

	//glActiveTexture(GL_TEXTURE0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &arrow_image_texture_id);
    if(arrow_image_texture_id == 0)
        return false;

    glBindTexture(GL_TEXTURE_2D, 0);

	glUniform1i(m_myTextureLocation, 0);
	glUniform1i(m_arrowTextureLocation, 1);
	glActiveTexture(GL_TEXTURE0);
	glUseProgram( 0);

	glGenVertexArrays( 1, &m_unSceneVAO );
	glGenBuffers( 1, &m_glSceneVertBuffer );

	SetupScene();
	SetupCameras();
	if(!SetupStereoRenderTargets())
		return false;
	SetupCompanionWindow();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()
{
	if ( !vr::VRCompositor() )
	{
		printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if( m_pHMD )
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}
	
	if( m_pContext )
	{
		if(mpv_thread.joinable())
			mpv_thread.join();

		if(mpv_file)
			mpv.destroy();

		if( m_bDebugOpenGL )
		{
			glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
			glDebugMessageCallback(nullptr, nullptr);
		}
		glDeleteBuffers(1, &m_glSceneVertBuffer);

		if ( m_unSceneProgramID )
		{
			glDeleteProgram( m_unSceneProgramID );
		}
		if ( m_unCompanionWindowProgramID )
		{
			glDeleteProgram( m_unCompanionWindowProgramID );
		}

		glDeleteTextures(1, &arrow_image_texture_id);

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &mpvDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &mpvDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &mpvDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &mpvDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &mpvDesc.m_nResolveFramebufferId );

		if( m_unCompanionWindowVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
		}
		if( m_unSceneVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unSceneVAO );
		}
	}

	window_texture_deinit(&window_texture);

	if( m_pCompanionWindow )
	{
		SDL_DestroyWindow(m_pCompanionWindow);
		m_pCompanionWindow = NULL;
	}

	SDL_Quit();

	if (x_display)
		XCloseDisplay(x_display);
}

void CMainApplication::zoom_in() {
    if(projection_mode == ProjectionMode::SPHERE360)
        zoom -= 1.0f;
    else
        zoom -= 0.01f;
    zoom_resize = true;

    std::stringstream strstr;
    if(follow_focused)
        strstr << "/tmp/vr-video-player_focused";
    else
        strstr << "/tmp/vr-video-player_" << src_window_id;
    std::ofstream zoomstate(strstr.str());
    zoomstate << zoom; 
}

void CMainApplication::zoom_out() {
    if(projection_mode == ProjectionMode::SPHERE360)
        zoom += 1.0f;
    else
        zoom += 0.01f;
    zoom_resize = true;

    std::stringstream strstr;
    if(follow_focused)
        strstr << "/tmp/vr-video-player_focused";
    else
        strstr << "/tmp/vr-video-player_" << src_window_id;
    std::ofstream zoomstate(strstr.str());
    zoomstate << zoom;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;
    zoom_resize = false;
	int64_t video_width = 0;
	int64_t video_height = 0;
	bool mpv_quit = false;

	while ( SDL_PollEvent( &sdlEvent ) != 0 )
	{
		if ( sdlEvent.type == SDL_QUIT )
		{
			bRet = true;
		}
		else if ( sdlEvent.type == SDL_KEYDOWN )
		{
			if( sdlEvent.key.keysym.sym == SDLK_w )
			{
				m_bResetRotation = true;
			}
			if( sdlEvent.key.keysym.sym == SDLK_ESCAPE )
			{
				bRet = true;
			}
			if( sdlEvent.key.keysym.sym == SDLK_q )
			{
                zoom_in();
			}
			if( sdlEvent.key.keysym.sym == SDLK_e )
			{
                zoom_out();
			}
			if(mpv_file && sdlEvent.key.keysym.sym == SDLK_LEFT)
			{
				mpv.seek(-5.0); // Seek backwards 5 seconds
			}
			if(mpv_file && sdlEvent.key.keysym.sym == SDLK_RIGHT)
			{
				mpv.seek(5.0); // Seek forwards 5 seconds
			}
			if(mpv_file && sdlEvent.key.keysym.sym == SDLK_SPACE)
			{
				mpv.toggle_pause();
			}
		}

		bool opdoot = false;
		if(mpv_file) {
			int error = 0;
			mpv.on_event(sdlEvent, &opdoot, &video_width, &video_height, &mpv_quit, &error);
			if(mpv_quit && error != 0)
				exit_code = 2;
		}

		if(opdoot)
			set_render_update();

		if(mpv_quit)
			bRet = true;

		// TODO: Allow video resize to update texture size
		if(video_width > 0 && video_height > 0 && video_width != mpv_video_width && video_height != mpv_video_height && !mpv_video_loaded && !mpv_loaded_in_thread) {
			mpv_video_width = video_width;
			mpv_video_height = video_height;
			pixmap_texture_width = mpv_video_width;
			pixmap_texture_height = mpv_video_height;
			mpv_video_loaded = true;
			SetupScene();
		}
	}

	XEvent xev;
	
    if(XCheckTypedEvent(x_display, MappingNotify, &xev)) {
		XMappingEvent *mapping_ev = &xev.xmapping;
		XRefreshKeyboardMapping(mapping_ev);
		if(mapping_ev->request == MappingKeyboard) {
			fprintf(stderr, "Update keyboard mapping!\n");
			grabkeys(x_display);
		}
	}
	
    if (XCheckTypedEvent(x_display, KeyPress, &xev) && (xev.xkey.state & Mod1Mask)) {
	    KeySym keysym = XLookupKeysym(&xev.xkey, 0);
        if(keysym == XK_F1)
    		m_bResetRotation = true;
        else if(keysym == XK_q)
            zoom_in();
        else if(keysym == XK_e)
            zoom_out();
	}

	if(follow_focused && ((XCheckTypedWindowEvent(x_display, DefaultRootWindow(x_display), PropertyNotify, &xev) && xev.xproperty.atom == net_active_window_atom) || !focused_window_set)) {
		focused_window_set = true;
		Window focused_window = get_focused_window();
		if(focused_window && focused_window != src_window_id) {
			fprintf(stderr, "Window focus changed to window %ld\n", focused_window);
			src_window_id = focused_window;
			focused_window_changed = true;
		}
	}

	if(src_window_id) {
		if (XCheckTypedWindowEvent(x_display, src_window_id, VisibilityNotify, &xev)) {
			if((prev_visibility_state == VisibilityFullyObscured && xev.xvisibility.state != VisibilityFullyObscured) || (xev.xvisibility.state == prev_visibility_state)) {
				window_resize_time = SDL_GetTicks();
				window_resized = true;
			}
			prev_visibility_state = xev.xvisibility.state;
		}

		if (XCheckTypedWindowEvent(x_display, src_window_id, ConfigureNotify, &xev) && xev.xconfigure.window == src_window_id) {
			// Window resize
			if(xev.xconfigure.width != window_width || xev.xconfigure.height != window_height) {
				window_width = xev.xconfigure.width;
				window_height = xev.xconfigure.height;
				window_resize_time = SDL_GetTicks();
				window_resized = true;
			}
		}

		if(XCheckTypedWindowEvent(x_display, src_window_id, x_fixes_event_base + XFixesCursorNotify, &xev)) {
			XFixesCursorNotifyEvent *cursor_notify_event = (XFixesCursorNotifyEvent*)&xev;
			if(cursor_notify_event->subtype == XFixesDisplayCursorNotify && cursor_notify_event->window == src_window_id) {
				cursor_image_set = true;
				SetCursorFromX11CursorImage(XFixesGetCursorImage(x_display));
			}
		}
	}

	if(!cursor_image_set) {
		cursor_image_set = true;
		SetCursorFromX11CursorImage(XFixesGetCursorImage(x_display));
	}

	Uint32 time_now = SDL_GetTicks();
	const int window_resize_timeout = 1000; /* 1.0 second */
	if((focused_window_changed && src_window_id) || (window_resized && time_now - window_resize_time >= window_resize_timeout)) {
		XWindowAttributes xwa;
		if(!XGetWindowAttributes(x_display, src_window_id, &xwa)) {
			fprintf(stderr, "Error: Invalid window id: %lud\n", src_window_id);
		}
		window_width = xwa.width;
		window_height = xwa.height;
		window_resize_time = SDL_GetTicks();
		window_resized = false;

		if(focused_window_changed) {
			XSelectInput(x_display, src_window_id, StructureNotifyMask|VisibilityChangeMask|KeyPressMask|KeyReleaseMask);
			XFixesSelectCursorInput(x_display, src_window_id, XFixesDisplayCursorNotifyMask);
		}

		focused_window_changed = false;
		window_resized = false;
		window_texture_deinit(&window_texture);
		if(window_texture_init(&window_texture, x_display, src_window_id) != 0) {
			fprintf(stderr, "Failed to init texture\n");
			//return false;
		}
		glBindTexture(GL_TEXTURE_2D, window_texture_get_opengl_texture_id(&window_texture));
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &pixmap_texture_width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &pixmap_texture_height);
		if(pixmap_texture_width == 0)
			pixmap_texture_width = 1;
		if(pixmap_texture_height == 0)
			pixmap_texture_height = 1;
		glBindTexture(GL_TEXTURE_2D, 0);
		SetupScene();
	} else if(!window_resized && zoom_resize) {
		SetupScene();
	}

	if(src_window_id) {
		Window dummyW;
		int dummyI;
		unsigned int dummyU;
		XQueryPointer(x_display, src_window_id, &dummyW, &dummyW,
					&dummyI, &dummyI, &mouse_x, &mouse_y, &dummyU);
	}

	// Process SteamVR events
	vr::VREvent_t event;
	while( m_pHMD->PollNextEvent( &event, sizeof( event ) ) )
	{
		ProcessVREvent( event );
	}

	// Process SteamVR action state
	// UpdateActionState is called each frame to update the state of the actions themselves. The application
	// controls which action sets are active with the provided array of VRActiveActionSet_t structs.
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionsetDemo;
	vr::VRInput()->UpdateActionState( &actionSet, sizeof(actionSet), 1 );

	if(GetDigitalActionState( m_actionHideCubes ) || m_bResetRotation) {
		printf("reset rotation!\n");
		//printf("pos, %f, %f, %f\n", m_mat4HMDPose[0][2], m_mat4HMDPose[1][2], m_mat4HMDPose[2][2]);
		// m_resetPos = m_mat4HMDPose;
		hmd_pos = current_pos;
		m_bResetRotation = false;
		m_reset_rotation = glm::inverse(hmd_rot);
	}

	if(!free_camera)
		hmd_pos = current_pos;

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	SDL_StartTextInput();

	SDL_Joystick *controller = SDL_JoystickOpen(0);
	if (!controller)
		fprintf(stderr, "Could not open gamecontroller: %s\n", SDL_GetError());


	while ( !bQuit )
	{
		set_current_context(m_pContext);
		bQuit = HandleInput();
		if(bQuit)
			running = false;

		RenderFrame();
		set_current_context(NULL);
	}

	if(mpv_thread.joinable())
		mpv_thread.join();

	set_current_context(m_pContext);

	if (controller)
		SDL_JoystickClose(controller);

	SDL_StopTextInput();
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent( const vr::VREvent_t & event )
{
	switch( event.eventType )
	{
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			dprintf( "Device %u detached.\n", event.trackedDeviceIndex );
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			dprintf( "Device %u updated.\n", event.trackedDeviceIndex );
		}
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
	// for now as fast as possible
	if ( m_pHMD )
	{
		RenderStereoTargets();
		RenderCompanionWindow();

		vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
	}

	if ( m_bVblank && m_bGlFinishHack )
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow( m_pCompanionWindow );
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	// Flush and wait for swap.
	if ( m_bVblank )
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
		
		dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}

	UpdateHMDMatrixPose();
}

//-----------------------------------------------------------------------------
// Purpose: resets rotation & position of the screen
//-----------------------------------------------------------------------------
void CMainApplication::ResetRotation()
{
	m_bResetRotation = true;
}

//-----------------------------------------------------------------------------
// Purpose: simulates user mouse movement. Called when pointing a controller at
// the window for example.
//-----------------------------------------------------------------------------
void CMainApplication::MoveCursor(float x, float y)
{
	if(!src_window_id)
		return;

	int xi = (int)(x * window_width);
	int yi = (int)(y * window_height);
	XWarpPointer(x_display, None, src_window_id, 0, 0, 0, 0, xi, yi);
}

//-----------------------------------------------------------------------------
// Purpose: simulates user mouse clicking. Called when clicking a button on the
// controller for example.
//-----------------------------------------------------------------------------
void CMainApplication::MouseButton(int button, bool down)
{
	if(!src_window_id)
		return;

	Window root = DefaultRootWindow(x_display);

	Window dummyW;
	int dummyI;

	XButtonEvent xbpe;
	xbpe.window = src_window_id;
	xbpe.button = button;
	xbpe.display = x_display;
	xbpe.root = root;
	xbpe.same_screen = True;
	xbpe.subwindow = None;
	xbpe.time = CurrentTime;
	xbpe.type = (down ? ButtonPress : ButtonRelease);

	XQueryPointer(x_display, root, &dummyW, &dummyW,
			&dummyI, &dummyI, &dummyI, &dummyI, &xbpe.state);

	XSendEvent(x_display, src_window_id, True, ButtonPressMask, (XEvent *)&xbpe);
	XFlush(x_display);
}


//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------
GLuint CMainApplication::CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		dprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		dprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}


//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool CMainApplication::CreateAllShaders()
{
	m_unSceneProgramID = CompileGLShader( 
		"Scene",

		// Vertex Shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"uniform float texture_offset_x;\n"
		"uniform float texture_scale_x;\n"
		"uniform vec2 cursor_location;\n"
		"uniform vec2 arrow_size;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVcoordsIn;\n"
		"layout(location = 2) in vec3 v3NormalIn;\n"
		"out vec2 v2CursorLocation;\n"
		"out vec2 arrow_size_frag;\n"
		"out vec2 v2UVcoords;\n"
		"void main()\n"
		"{\n"
		"	v2UVcoords = vec2(1.0 - v2UVcoordsIn.x, v2UVcoordsIn.y) * vec2(texture_scale_x, 1.0) + vec2(texture_offset_x, 0.0);\n"
		"   vec4 inverse_pos = vec4(position.x, position.y, -position.z, position.w);\n"
		"	v2CursorLocation = cursor_location;\n"
		"	arrow_size_frag = arrow_size;\n"
		"	gl_Position = matrix * inverse_pos;\n"
		"}\n",

		// Fragment Shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"uniform sampler2D arrow_texture;\n"
		"in vec2 v2UVcoords;\n"
		"in vec2 v2CursorLocation;\n"
		"in vec2 arrow_size_frag;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"	vec2 cursor_diff = (v2CursorLocation + arrow_size_frag) - v2UVcoords;\n"
		"	vec2 arrow_coord = (arrow_size_frag - cursor_diff) / arrow_size_frag;\n"
		"	vec4 arrow_col = texture(arrow_texture, arrow_coord);\n"
		"	vec4 col = texture(mytexture, v2UVcoords);\n"
		"	if(arrow_size_frag.x < 0.01 || arrow_size_frag.y < 0.01 || arrow_coord.x < 0.0 || arrow_coord.x > 1.0 || arrow_coord.y < 0.0 || arrow_coord.y > 1.0) arrow_col.a = 0.0;\n"
		"	outputColor = mix(col, arrow_col.bgra, arrow_col.a);\n"
		"}\n"
		);
	m_nSceneMatrixLocation = glGetUniformLocation( m_unSceneProgramID, "matrix" );
	if( m_nSceneMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in scene shader\n" );
		return false;
	}
	m_nSceneTextureOffsetXLocation = glGetUniformLocation( m_unSceneProgramID, "texture_offset_x" );
	if( m_nSceneTextureOffsetXLocation == -1 )
	{
		dprintf( "Unable to find texture_offset_x uniform in scene shader\n" );
		return false;
	}
	m_nSceneTextureScaleXLocation = glGetUniformLocation( m_unSceneProgramID, "texture_scale_x" );
	if( m_nSceneTextureScaleXLocation == -1 )
	{
		dprintf( "Unable to find texture_scale_x uniform in scene shader\n" );
		return false;
	}
	m_nCursorLocation = glGetUniformLocation( m_unSceneProgramID, "cursor_location" );
	if( m_nCursorLocation == -1 )
	{
		dprintf( "Unable to find cursor_location uniform in scene shader\n" );
		return false;
	}
	m_nArrowSizeLocation = glGetUniformLocation( m_unSceneProgramID, "arrow_size" );
	if( m_nArrowSizeLocation == -1 )
	{
		dprintf( "Unable to find arrow_size uniform in scene shader\n" );
		return false;
	}
	m_myTextureLocation = glGetUniformLocation(m_unSceneProgramID, "mytexture");
	if(m_myTextureLocation == -1) {
		dprintf( "Unable to find mytexture uniform in scene shader\n" );
		return false;
	}
	m_arrowTextureLocation = glGetUniformLocation(m_unSceneProgramID, "arrow_texture");
	if(m_arrowTextureLocation == -1) {
		dprintf( "Unable to find arrow_texture uniform in scene shader\n" );
		return false;
	}

	m_unCompanionWindowProgramID = CompileGLShader(
		"CompanionWindow",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVIn;\n"
		"noperspective out vec2 v2UV;\n"
		"void main()\n"
		"{\n"
		"	v2UV = vec2(v2UVIn.x, 1.0 - v2UVIn.y);\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"
		"noperspective in vec2 v2UV;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"	vec4 col = texture(mytexture, v2UV);\n"
		"	outputColor = col.rgba;\n"
		"}\n"
		);

	return m_unSceneProgramID != 0 
		&& m_unCompanionWindowProgramID != 0;
}

bool CMainApplication::SetCursorFromX11CursorImage(XFixesCursorImage *x11_cursor_image) {
	if(!x11_cursor_image)
		return false;
		
	if(!x11_cursor_image->pixels) {
		XFree(x11_cursor_image);
		return false;
	}

	cursor_offset_x = x11_cursor_image->xhot;
	cursor_offset_y = x11_cursor_image->yhot;
	glBindTexture(GL_TEXTURE_2D, arrow_image_texture_id);

	arrow_image_width = x11_cursor_image->width;
	arrow_image_height = x11_cursor_image->height;
	const unsigned long *pixels = x11_cursor_image->pixels;
	uint8_t *cursor_data = new uint8_t[arrow_image_width * arrow_image_height * 4];
	uint8_t *out = cursor_data;
	/* Un-premultiply alpha */
	for(int y = 0; y < arrow_image_height; ++y) {
		for(int x = 0; x < arrow_image_width; ++x) {
			uint32_t pixel = *pixels++;
			uint8_t *in = (uint8_t*)&pixel;
			uint8_t alpha = in[3];
			if(alpha == 0)
				alpha = 1;

			*out++ = (unsigned)*in++ * 255/alpha;
			*out++ = (unsigned)*in++ * 255/alpha;
			*out++ = (unsigned)*in++ * 255/alpha;
			*out++ = *in++;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, arrow_image_width, arrow_image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, cursor_data);
	delete []cursor_data;
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	float fLargest = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	cursor_scale_uniform[0] = 0.01 * cursor_scale;
	cursor_scale_uniform[1] = cursor_scale_uniform[0] * arrow_ratio * ((float)arrow_image_height / (float)(arrow_image_width == 0 ? 1 : arrow_image_width));

	glUseProgram( m_unSceneProgramID );
	glUniform2fv(m_nArrowSizeLocation, 1, &cursor_scale_uniform[0]);
	glUseProgram( 0 );
	XFree(x11_cursor_image);
	return true;
}

Window CMainApplication::get_focused_window() {
	Atom type;
	int format = 0;
	unsigned long num_items = 0;
	unsigned long bytes_after = 0;
	unsigned char *properties = nullptr;
	if(XGetWindowProperty(x_display, DefaultRootWindow(x_display), net_active_window_atom, 0, 1024, False, AnyPropertyType, &type, &format, &num_items, &bytes_after, &properties) == Success && properties) {
		Window focused_window = *(unsigned long*)properties;
		XFree(properties);
		return focused_window;
	}
	return None;
}

void CMainApplication::save_config() {
	if(free_camera) {
		switch(projection_mode) {
			case ProjectionMode::SPHERE: {
				config.sphere.position = hmd_pos;
				break;
			}
			case ProjectionMode::FLAT: {
				config.flat.position = hmd_pos;
				break;
			}
			case ProjectionMode::CYLINDER: {
				config.plane.position = hmd_pos;
				break;
			}
			case ProjectionMode::SPHERE360: {
				config.sphere360.position = hmd_pos;
				break;
			}
		}
	}
	switch(projection_mode) {
		case ProjectionMode::SPHERE: {
			config.sphere.rotation = m_reset_rotation;
			config.sphere.zoom = zoom;
			break;
		}
		case ProjectionMode::FLAT: {
			config.flat.rotation = m_reset_rotation;
			config.flat.zoom = zoom;
			break;
		}
		case ProjectionMode::CYLINDER: {
			config.plane.rotation = m_reset_rotation;
			config.plane.zoom = zoom;
			break;
		}
		case ProjectionMode::SPHERE360: {
			config.sphere360.rotation = m_reset_rotation;
			config.sphere360.zoom = zoom;
			break;
		}
	}
	::save_config(config);
}


//-----------------------------------------------------------------------------
// Purpose: create a sea of cubes
//-----------------------------------------------------------------------------
void CMainApplication::SetupScene()
{
	if ( !m_pHMD )
		return;

	std::vector<float> vertdataarray;
#if 0
	glm::mat4 matScale =glm::scale(glm::mat4(1.0f), glm::vec3(m_fScale, m_fScale, m_fScale));
	glm::mat4 matTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(
			-( (float)m_iSceneVolumeWidth * m_fScaleSpacing ) / 2.f,
			-( (float)m_iSceneVolumeHeight * m_fScaleSpacing ) / 2.f,
			-( (float)m_iSceneVolumeDepth * m_fScaleSpacing ) / 2.f)
	);
	
	glm::mat4 mat = matScale * matTransform;

	for( int z = 0; z< m_iSceneVolumeDepth; z++ )
	{
		for( int y = 0; y< m_iSceneVolumeHeight; y++ )
		{
			for( int x = 0; x< m_iSceneVolumeWidth; x++ )
			{
				AddCubeToScene( mat, vertdataarray );
				mat = mat * glm::translate(glm::mat4(1.0f), glm::vec3(m_fScaleSpacing, 0, 0 ));
			}
			mat = mat * glm::translate(glm::mat4(1.0f), glm::vec3(-((float)m_iSceneVolumeWidth) * m_fScaleSpacing, m_fScaleSpacing, 0 ));
		}
		mat = mat * glm::translate(glm::mat4(1.0f), glm::vec3(0, -((float)m_iSceneVolumeHeight) * m_fScaleSpacing, m_fScaleSpacing ));
	}

#else
	glm::mat4 matScale = glm::mat4(1.0f);
	matScale = glm::scale(matScale, glm::vec3(m_fScale, m_fScale, m_fScale));
	glm::mat4 matTransform = glm::mat4(1.0f);
	/*
	matTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(-m_fScale*0.5f, -m_fScale*0.5f, 0.5f)
	);
	*/
	
	glm::mat4 mat = matScale * matTransform;
	AddCubeToScene( mat, vertdataarray );
#endif
	m_uiVertcount = vertdataarray.size()/5;
	
	glBindVertexArray( m_unSceneVAO );
	glBindBuffer( GL_ARRAY_BUFFER, m_glSceneVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);

	GLsizei stride = sizeof(VertexDataScene);
	uintptr_t offset = 0;

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride , (const void *)offset);

	offset += sizeof(glm::vec3);
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

	glBindVertexArray( 0 );
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


static void AddCubeVertex( float x, float y, float z, float u, float v, std::vector<float> &vertdata )
{
	vertdata.push_back( x );
	vertdata.push_back( y );
	vertdata.push_back( z );
	vertdata.push_back( u );
	vertdata.push_back( v );
}

static void CreateSegmentedPlane(std::vector<float> &vertdata, float width, float height, float depth, float texture_width, float texture_height, float texture_offset_x, float texture_offset_y, int num_columns, int num_rows) {
	#if 0
	float segment_width = width / (float)num_columns;
	float segment_height = height / (float)num_rows;
	float segment_texture_width = texture_width / (float)num_columns;
	float segment_texture_height = texture_height / (float)num_rows;
	for(int y = 0; y < num_rows; ++y) {
		float segment_height_offset = segment_height * (float)y;
		float segment_texture_height_offset = segment_texture_height * (float)y;
		for(int x = 0; x < num_columns; ++x) {
			float segment_width_offset = segment_width * (float)x;
			float segment_texture_width_offset = segment_texture_width * (float)x;
			//segment_texture_width_offset = 0.0f;
			//segment_texture_height_offset = 0.0f;
			//segment_texture_width = texture_width;
			//segment_texture_height = texture_height;
			// AddCubeVertex(segment_width_offset + -segment_width, 	segment_height_offset +  segment_height, zoom, segment_texture_width_offset + texture_offset_x + segment_texture_width, 	segment_texture_height_offset + texture_offset_y, 							vertdata);
			// AddCubeVertex(segment_width_offset + segment_width, 	segment_height_offset +  segment_height, zoom, segment_texture_width_offset + texture_offset_x, 							segment_texture_height_offset + texture_offset_y, 							vertdata);
			// AddCubeVertex(segment_width_offset + -segment_width, 	segment_height_offset + -segment_height, zoom, segment_texture_width_offset + texture_offset_x + segment_texture_width, 	segment_texture_height_offset + texture_offset_y + segment_texture_height, 	vertdata);

			// AddCubeVertex(segment_width_offset + -segment_width, 	segment_height_offset + -segment_height, zoom, segment_texture_width_offset + texture_offset_x + segment_texture_width, 	segment_texture_height_offset + texture_offset_y + segment_texture_height, 	vertdata);
			// AddCubeVertex(segment_width_offset + segment_width, 	segment_height_offset + -segment_height, zoom, segment_texture_width_offset + texture_offset_x, 							segment_texture_height_offset + texture_offset_y + segment_texture_height, 	vertdata);
			// AddCubeVertex(segment_width_offset + segment_width, 	segment_height_offset +  segment_height, zoom, segment_texture_width_offset + texture_offset_x, 							segment_texture_height_offset + texture_offset_y, 							vertdata);
		}
	}
	#endif
	float segment_width = width / (float)num_columns;
	float segment_height = height / (float)num_rows;
	float segment_texture_width = texture_width / (float)num_columns;
	float segment_texture_height = texture_height / (float)num_rows;

	for(int y = 0; y < num_rows; ++y) {
		float segment_height_offset = height - segment_height * 2.0f * (float)y;
		float segment_texture_height_offset = segment_texture_height * (float)y;
		for(int x = 0; x < num_columns; ++x) {
			float segment_width_offset = width - segment_width * 2.0f * (float)x;
			float segment_texture_width_offset = segment_texture_width * (float)x;

			AddCubeVertex(segment_width_offset, segment_height_offset, depth, segment_texture_width_offset + texture_offset_x, segment_texture_height_offset + texture_offset_y, vertdata);
			AddCubeVertex(segment_width_offset - segment_width*2.0f, segment_height_offset, depth, segment_texture_width_offset + texture_offset_x + segment_texture_width, segment_texture_height_offset + texture_offset_y, vertdata);
			AddCubeVertex(segment_width_offset - segment_width*2.0f, segment_height_offset - segment_height*2.0f, depth, segment_texture_width_offset + texture_offset_x + segment_texture_width, segment_texture_height_offset + texture_offset_y + segment_texture_height, vertdata);

			AddCubeVertex(segment_width_offset - segment_width*2.0f, segment_height_offset - segment_height*2.0f, depth, segment_texture_width_offset + texture_offset_x + segment_texture_width, segment_texture_height_offset + texture_offset_y + segment_texture_height, vertdata);
			AddCubeVertex(segment_width_offset, segment_height_offset - segment_height*2.0f, depth, segment_texture_width_offset + texture_offset_x, segment_texture_height_offset + texture_offset_y + segment_texture_height, vertdata);
			AddCubeVertex(segment_width_offset, segment_height_offset, depth, segment_texture_width_offset + texture_offset_x, segment_texture_height_offset + texture_offset_y, vertdata);
		}
	}
}

static glm::vec3 vertex_get_center(glm::mat3 vertex) {
	return glm::vec3(
		(vertex[0].x + vertex[1].x + vertex[2].x) / 3.0f,
		(vertex[0].y + vertex[1].y + vertex[2].y) / 3.0f,
		(vertex[0].z + vertex[1].z + vertex[2].z) / 3.0f
	);
}

static void plane_normalize_depth(float *vertices, size_t num_vertices, float depth) {
	for(size_t i = 0; i < num_vertices; ++i) {
		float *vertex_data = &vertices[i * 5];
		float dist = sqrtf(vertex_data[0]*vertex_data[0] + vertex_data[1]*vertex_data[1] + vertex_data[2]*vertex_data[2]);
		vertex_data[0] = vertex_data[0]/dist * depth;
		vertex_data[1] = vertex_data[1]/dist * depth;
		vertex_data[2] = vertex_data[2]/dist * depth;
	}
}

static void vertices_rotate(float *vertices, size_t num_vertices, float angle, glm::vec3 rotation_axis) {
	for(size_t i = 0; i < num_vertices - 2; i += 3) {
		float *vertex_data1 = &vertices[(i + 0) * 5];
		float *vertex_data2 = &vertices[(i + 1) * 5];
		float *vertex_data3 = &vertices[(i + 2) * 5];
		glm::quat quatRot = glm::angleAxis(angle, rotation_axis);
		glm::mat4x4 matRot = glm::mat4_cast(quatRot);
		glm::vec3 &vec1 = *(glm::vec3*)vertex_data1;
		glm::vec3 &vec2 = *(glm::vec3*)vertex_data2;
		glm::vec3 &vec3 = *(glm::vec3*)vertex_data3;
		glm::vec3 center = vertex_get_center(glm::mat3(vec1, vec2, vec3));

		vec1 -= center;
		vec2 -= center;
		vec3 -= center;

		glm::mat4 tran = glm::translate(matRot, center);

		glm::vec4 out1 = tran * glm::vec4(vec1.x, vec1.y, vec1.z, 1.0f);
		glm::vec4 out2 = tran * glm::vec4(vec2.x, vec2.y, vec2.z, 1.0f);
		glm::vec4 out3 = tran * glm::vec4(vec3.x, vec3.y, vec3.z, 1.0f);
		vec1 = glm::vec3(out1.x, out1.y, out1.z);
		vec2 = glm::vec3(out2.x, out2.y, out2.z);
		vec3 = glm::vec3(out3.x, out3.y, out3.z);
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::AddCubeToScene( const glm::mat4 &mat, std::vector<float> &vertdata )
{
	double width_ratio = (double)pixmap_texture_width / (double)pixmap_texture_height;
	arrow_ratio = width_ratio;

	Window root_window;
	int x_return, y_return;
	unsigned int width_return, height_return, border_width_return = 0, depth_return;
	if(src_window_id)
		XGetGeometry(x_display, src_window_id, &root_window, &x_return, &y_return, &width_return, &height_return, &border_width_return, &depth_return);

	if(projection_mode == ProjectionMode::SPHERE)
	{
		long columns = 32;
		long rows = 32;
		double angle_x = 3.14;
		double radius_height = 1.0;
		double radius = radius_height * width_ratio * 0.5;

		for(long row = 0; row < rows; ++row) {
			for(long column = 0; column < columns; ++column) {
				double offset_angle = 0.0;//angle_x*0.5;

				double y_sin1 = sin((double)row / (double)rows * 3.14);
				double y_sin2 = sin((double)(row + 1) / (double)rows * 3.14);

				double z1 = sin(offset_angle + (double)column / (double)columns * angle_x) * radius;
				double z2 = sin(offset_angle + (double)(column + 1) / (double)columns * angle_x) * radius;
				double z3 = z1;

				double z4 = z3;
				double z5 = z2;
				double z6 = z2;

				z1 *= y_sin1;
				z2 *= y_sin1;
				z3 *= y_sin2;
				z4 *= y_sin2;
				z5 *= y_sin2;
				z6 *= y_sin1;

				double x1 = -cos(offset_angle + (double)column / (double)columns * angle_x) * radius;
				double x2 = -cos(offset_angle + (double)(column + 1) / (double)columns * angle_x) * radius;
				double x3 = x1;

				double x4 = x3;
				double x5 = x2;
				double x6 = x2;

				x1 *= y_sin1;
				x2 *= y_sin1;
				x3 *= y_sin2;
				x4 *= y_sin2;
				x5 *= y_sin2;
				x6 *= y_sin1;
	#if 0
				double y1 = cos((double)row / (double)rows * angle_y) * radius;
				double y2 = y1;
				double y3 = cos((double)(row + 1) / (double)rows * angle_y) * radius;

				double y4 = y3;
				double y5 = y3;
				double y6 = y1;

				z1 *= sin((double)row / (double)rows * angle_y) * radius_depth;
				z2 *= sin((double)row / (double)rows * angle_y) * radius_depth;
				z3 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				z4 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				z5 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				z6 *= sin((double)row / (double)rows * angle_y) * radius_depth;

				x1 *= sin((double)row / (double)rows * angle_y) * radius_depth;
				x2 *= sin((double)row / (double)rows * angle_y) * radius_depth;
				x3 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				x4 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				x5 *= sin((double)(row + 1) / (double)rows * angle_y) * radius_depth;
				x6 *= sin((double)row / (double)rows * angle_y) * radius_depth;
	#else
				double y1 = cos((double)row / (double)rows * 3.14) * radius_height;
				double y2 = y1;
				double y3 = cos((double)(row + 1) / (double)rows * 3.14) * radius_height;

				double y4 = y3;
				double y5 = y3;
				double y6 = y1;
	#endif

				glm::vec4 v1 = mat * glm::vec4(x1, y1, z1 + zoom, 1.0);
				glm::vec4 v2 = mat * glm::vec4(x2, y2, z2 + zoom, 1.0);
				glm::vec4 v3 = mat * glm::vec4(x3, y3, z3 + zoom, 1.0);
				glm::vec4 v4 = mat * glm::vec4(x4, y4, z4 + zoom, 1.0);
				glm::vec4 v5 = mat * glm::vec4(x5, y5, z5 + zoom, 1.0);
				glm::vec4 v6 = mat * glm::vec4(x6, y6, z6 + zoom, 1.0);

				AddCubeVertex(v1.x, v1.y, v1.z, 1.0 - (double)column / (double)columns,                 (double)row / (double)rows, vertdata);
				AddCubeVertex(v2.x, v2.y, v2.z, 1.0 - (double)(column + 1) / (double)columns,   (double)row / (double)rows, vertdata);
				AddCubeVertex(v3.x, v3.y, v3.z, 1.0 - (double)column / (double)columns,                 (double)(row + 1) / (double)rows, vertdata);

				AddCubeVertex(v4.x, v4.y, v4.z, 1.0 - (double)column / (double)columns,                 (double)(row + 1) / (double)rows, vertdata);
				AddCubeVertex(v5.x, v5.y, v5.z, 1.0 - (double)(column + 1) / (double)columns,   (double)(row + 1) / (double)rows, vertdata);
				AddCubeVertex(v6.x, v6.y, v6.z, 1.0 - (double)(column + 1) / (double)columns,   (double)row / (double)rows, vertdata);
			}
		}
	}
	else if (projection_mode == ProjectionMode::CYLINDER)
	{
		long columns = 64;
		double angle_start = -0.8;
		double angle_end = 0.8;
		double height = 1.5;
		double angle_len = angle_end - angle_start;

		double width_start = sin(angle_start);
		double width_end = sin(angle_start + angle_len);
		double target_radius = height * width_ratio;
		double radius = 2.0 * (target_radius / (width_end - width_start));

		for(long column = 0; column < columns; ++column) {
			double t1 = ((double)column / (double)columns);
			double t2 = (((double)column + 1) / (double)columns);

			double x1 = sin(angle_start + t1 * angle_len) * radius;
			double y1 = cos(angle_start + t1 * angle_len) * radius * 0.6;
			double x2 = sin(angle_start + t2 * angle_len) * radius;
			double y2 = cos(angle_start + t2 * angle_len) * radius * 0.6;

			//     2     n
			// 1  /|   / |    m
			// | / | /   |  / |
			// |/  2     n/   |
			// 1              m

			AddCubeVertex(x1, height, zoom + y1, 1 - t1, 0, vertdata);
			AddCubeVertex(x2, height, zoom + y2, 1 - t2, 0, vertdata);
			AddCubeVertex(x1, -height, zoom + y1, 1 - t1, 1, vertdata);

			AddCubeVertex(x1, -height, zoom + y1, 1 - t1, 1, vertdata);
			AddCubeVertex(x2, height, zoom + y2, 1 - t2, 0, vertdata);
			AddCubeVertex(x2, -height, zoom + y2, 1 - t2, 1, vertdata);
		}
	} else if (projection_mode == ProjectionMode::FLAT) {
		double height = 0.5;
		double width = height * (stretch ? 1.0 : 0.5) * width_ratio;
		AddCubeVertex(-width, 	 height, zoom, 1.0, 0.0, vertdata);
		AddCubeVertex(width, 	 height, zoom, 0.0, 0.0, vertdata);
		AddCubeVertex(-width, 	-height, zoom, 1.0, 1.0, vertdata);

		AddCubeVertex(-width, 	-height, zoom, 1.0, 1.0, vertdata);
		AddCubeVertex(width, 	-height, zoom, 0.0, 1.0, vertdata);
		AddCubeVertex(width, 	 height, zoom, 0.0, 0.0, vertdata);

		if(stretch)
			arrow_ratio = width_ratio * 2.0;
	} else if (projection_mode == ProjectionMode::SPHERE360) {
		if(!mpv_file)
			border_width_return += 2; // Meh, hac k to deal with seams a bit
		double px = (double)border_width_return / (double)pixmap_texture_width;
		double py = (double)border_width_return / (double)pixmap_texture_height;

		double width = 1.0 - px * 2.0;
		double height = 1.0 - py * 2.0;

		double hz = zoom / (double)pixmap_texture_height;

		double texture_width = width / 3.0;
		double texture_height = height * 0.5;

		for(int i = 0; i < 3; ++i) {
			size_t plane_vertices_start = vertdata.size();
			CreateSegmentedPlane(vertdata, 1.0f, 1.0f, 1.0f, texture_width, texture_height - hz, texture_width * (2 - i) + px, py + hz, 32, 32);
			size_t plane_vertices_end = vertdata.size();
			size_t num_vertex_data = (plane_vertices_end - plane_vertices_start) / 5;

			plane_normalize_depth(&vertdata[plane_vertices_start], num_vertex_data, 1.0f);
			vertices_rotate(&vertdata[plane_vertices_start], num_vertex_data, -glm::half_pi<float>() + i * glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		for(int i = 0; i < 3; ++i) {
			size_t plane_vertices_start = vertdata.size();
			CreateSegmentedPlane(vertdata, 1.0f, 1.0f, 1.0f, texture_width, texture_height - hz, px + texture_width * i, 0.5f, 32, 32);
			size_t plane_vertices_end = vertdata.size();
			size_t num_vertex_data = (plane_vertices_end - plane_vertices_start) / 5;

			plane_normalize_depth(&vertdata[plane_vertices_start], num_vertex_data, 1.0f);
			vertices_rotate(&vertdata[plane_vertices_start], num_vertex_data, -glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f));
			vertices_rotate(&vertdata[plane_vertices_start], num_vertex_data, -glm::half_pi<float>() - i * glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}

	cursor_scale_uniform[0] = 0.01 * cursor_scale;
	cursor_scale_uniform[1] = cursor_scale_uniform[0] * arrow_ratio * ((float)arrow_image_height / (float)(arrow_image_width == 0 ? 1 : arrow_image_width));

	glUseProgram( m_unSceneProgramID );
	glUniform2fv(m_nArrowSizeLocation, 1, &cursor_scale_uniform[0]);
	glUseProgram( 0 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}


//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc )
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}

void CMainApplication::set_current_context(SDL_GLContext context) {
	std::lock_guard<std::mutex> lock(context_mutex);
	SDL_GL_MakeCurrent(m_pCompanionWindow, context);
}

bool CMainApplication::take_render_update() {
	std::lock_guard<std::mutex> lock(mpv_render_update_mutex);
	bool should_update = mpv_render_update;
	mpv_render_update = false;
	return should_update;
}

void CMainApplication::set_render_update() {
	std::lock_guard<std::mutex> lock(mpv_render_update_mutex);
	mpv_render_update = true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
	if ( !m_pHMD )
		return false;

	m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, leftEyeDesc );
	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, rightEyeDesc );
	
	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow()
{
	if ( !m_pHMD )
		return;

	std::vector<VertexDataWindow> vVerts;

	// left eye verts
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, -1), glm::vec2(0, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(0, -1), glm::vec2(1, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(-1, 1), glm::vec2(0, 0)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(0, 1), glm::vec2(1, 0)) );

	// right eye verts
	vVerts.push_back( VertexDataWindow( glm::vec2(0, -1), glm::vec2(0, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, -1), glm::vec2(1, 1)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(0, 1), glm::vec2(0, 0)) );
	vVerts.push_back( VertexDataWindow( glm::vec2(1, 1), glm::vec2(1, 0)) );

	GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6 };
	m_uiCompanionWindowIndexSize = _countof(vIndices);

	glGenVertexArrays( 1, &m_unCompanionWindowVAO );
	glBindVertexArray( m_unCompanionWindowVAO );

	glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glEnable( GL_MULTISAMPLE );

	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Left );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId );

    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );	

	glEnable( GL_MULTISAMPLE );

	// Right Eye
	glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Right );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
 	
	glDisable( GL_MULTISAMPLE );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId );
	
    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR  );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );

	//glEnable( GL_MULTISAMPLE );

	//glBindTexture(GL_TEXTURE_2D, 0);
}


//-----------------------------------------------------------------------------
// Purpose: Renders a scene with respect to nEye.
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene( vr::Hmd_Eye nEye )
{
	if(!src_window_id && !mpv_file)
		return;
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glUseProgram( m_unSceneProgramID );
	glUniformMatrix4fv( m_nSceneMatrixLocation, 1, GL_FALSE, glm::value_ptr(GetCurrentViewProjectionMatrix( nEye )));

	float m[2];
	m[0] = mouse_x / (float)window_width;
	m[1] = mouse_y / (float)window_height;

	if(view_mode != ViewMode::PLANE) {
		if(cursor_wrap && m[0] >= 0.5f)
			m[0] -= 0.5f;
		else if(!cursor_wrap)
			m[0] *= 0.5f;
	}

	if( nEye == vr::Eye_Left )
	{
		float offset = 0.0f;
		float scale = 0.5f;
		if(view_mode == ViewMode::RIGHT_LEFT) {
			offset = 0.5f;
		} else if(view_mode == ViewMode::PLANE || view_mode == ViewMode::SPHERE360) {
			offset = 0.0f;
			scale = 1.0f;
		}
		glUniform1fv(m_nSceneTextureOffsetXLocation, 1, &offset);
		glUniform1fv(m_nSceneTextureScaleXLocation, 1, &scale);

		if(view_mode == ViewMode::RIGHT_LEFT)
			m[0] += offset;
	}
	else if( nEye == vr::Eye_Right )
	{
		float offset = 0.5f;
		float scale = 0.5f;
		if (view_mode == ViewMode::RIGHT_LEFT) {
			offset = 0.0f;
		} else if (view_mode == ViewMode::PLANE || view_mode == ViewMode::SPHERE360) {
			offset = 0.0f;
			scale = 1.0f;
		}
		glUniform1fv(m_nSceneTextureOffsetXLocation, 1, &offset);
		glUniform1fv(m_nSceneTextureScaleXLocation, 1, &scale);

		if(view_mode == ViewMode::LEFT_RIGHT)
			m[0] += offset;
	}


	float drawn_arrow_width = cursor_scale_uniform[0] * window_width;
	float drawn_arrow_height = cursor_scale_uniform[1] * window_height;
	float arrow_drawn_scale_x = drawn_arrow_width / (float)(arrow_image_width == 0 ? 1 : arrow_image_width);
	float arrow_drawn_scale_y = drawn_arrow_height / (float)(arrow_image_height == 0 ? 1 : arrow_image_height);

	m[0] += (-cursor_offset_x * arrow_drawn_scale_x) / (float)window_width;
	m[1] += (-cursor_offset_y * arrow_drawn_scale_y) / (float)window_height;

	glUniform2fv(m_nCursorLocation, 1, &m[0]);

	glBindVertexArray( m_unSceneVAO );
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mpv_file ? mpvDesc.m_nResolveTextureId :  window_texture_get_opengl_texture_id(&window_texture));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, mpv_file ? 0 : arrow_image_texture_id);
	glDrawArrays( GL_TRIANGLES, 0, m_uiVertcount );

	glBindVertexArray( 0 );
	glActiveTexture(GL_TEXTURE0);
	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderCompanionWindow()
{
	glDisable(GL_DEPTH_TEST);
	glViewport( 0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight );

	glBindVertexArray( m_unCompanionWindowVAO );
	glUseProgram( m_unCompanionWindowProgramID );

	// render left eye (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0 );

	// render right eye (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize) );

	glBindVertexArray( 0 );
	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return glm::mat4(1.0f);

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return glm::mat4(1.0f);

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
	glm::mat4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return glm::inverse(matrixObj);
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
	glm::mat4 matMVP;
	//glm::mat4 pp;
	//memcpy(&pp[0], m_mat4HMDPose.get(), sizeof(m_mat4HMDPose));
	//memcpy(&m_mat4HMDPose[0], &pp[0], sizeof(pp));
	glm::mat4 hmd_pose = m_mat4HMDPose;
	hmd_pose = glm::translate(hmd_pose, hmd_pos);
	if(reduce_flicker) {
		hmd_pose = glm::rotate(hmd_pose, (float)(cos(reduce_flicker_counter)*0.0005), glm::vec3(0.0f, 1.0f, 0.0f));
		hmd_pose = glm::rotate(hmd_pose, (float)(sin(reduce_flicker_counter)*0.0005), glm::vec3(1.0f, 0.0f, 0.0f));
		reduce_flicker_counter += 1.0;
	}
	hmd_pose = hmd_pose * mat4_cast(m_reset_rotation);
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * hmd_pose;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * hmd_pose;
	}

	return matMVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose()
{
	if ( !m_pHMD )
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < (int)vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			switch (m_pHMD->GetTrackedDeviceClass(nDevice))
			{
			case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
			case vr::TrackedDeviceClass_HMD: {
				//printf("pos: %f, %f, %f\n", m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[0][3], m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[1][3], m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[2][3]);
				current_pos.x = m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[0][3];
				current_pos.y = m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[1][3];
				current_pos.z = m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[2][3];

				glm::mat4 *mat = (glm::mat4*)&m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking;
				hmd_rot = glm::quat_cast(*mat);
			
				m_rDevClassChar[nDevice] = 'H';
				break;
			}
			case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
			case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
			case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
			default:                                       m_rDevClassChar[nDevice] = '?'; break;
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		m_mat4HMDPose = glm::inverse(m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd]);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
glm::mat4 CMainApplication::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose )
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}

CMainApplication *pMainApplication;

void reset_position(int signum)
{
	printf("ok\n");
	if(pMainApplication)
		pMainApplication->ResetRotation();
}

void quit(int signum) {
	if(pMainApplication)
		pMainApplication->bQuit = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	pMainApplication = new CMainApplication( argc, argv );

	signal(SIGUSR1, reset_position);
	signal(SIGUSR2, reset_position);

	signal(SIGINT, quit);
	signal(SIGTERM, quit);

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		return 1;
	}

	pMainApplication->RunMainLoop();
	pMainApplication->Shutdown();

	if(pMainApplication->exit_code == 0)
		pMainApplication->save_config();

	return pMainApplication->exit_code;
}
