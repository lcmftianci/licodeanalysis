#pragma once
#include <webrtc/modules/video_capture/video_capture_defines.h>
#include "VosVideo.Data/CameraConfMsg.h"
#include "VosVideo.Data/SendData.h"

namespace vosvideo
{
	namespace cameraplayer
	{
		enum class PlayerState	
		{
			Closed,         // No session.
			Ready,          // Session was created, ready to open a file.
			OpenPending,    // Session is opening a file.
			Started,        // Session is playing a file.
			Paused,         // Session is paused.
			Stopped,        // Session is stopped (ready to play).
			Closing         // Application is waiting for MESessionClosed.
		};


		class CameraPlayerBase
		{
		public:
			virtual ~CameraPlayerBase();

			// Playback control
			virtual int32_t OpenURL(vosvideo::data::CameraConfMsg&) = 0;
			virtual void GetWebRtcCapability(webrtc::VideoCaptureCapability& webRtcCapability) = 0;
			virtual int32_t Play() = 0;
			virtual int32_t Pause() = 0;
			virtual int32_t Stop() = 0;
			virtual int32_t Shutdown() = 0;

			virtual PlayerState GetState(std::shared_ptr<vosvideo::data::SendData>& lastErrMsg) const = 0;
			virtual PlayerState GetState() const = 0;

			// Probably most important method, through it camera communicates to WebRTC
			virtual void SetExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) = 0;
			virtual void RemoveExternalCapturers() = 0;
			virtual void RemoveExternalCapturer(webrtc::VideoCaptureExternal* captureObserver) = 0;

			virtual uint32_t GetDeviceId() const = 0;
			virtual vosvideo::data::CameraType GetCameraType(){ return cameraType_; }

		protected:
			vosvideo::data::CameraType cameraType_ = vosvideo::data::CameraType::UNKNOWN;
		};
	}
}

