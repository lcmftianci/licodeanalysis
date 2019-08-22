#pragma once
#include <streams.h>
#include <functional>


class VideoPlayer : private CBaseVideoRenderer
{
public:
	VideoPlayer(LPCWSTR fileName, HRESULT* phr);
	virtual ~VideoPlayer();

	virtual void RunVideo();
	virtual void PauseVideo();
	virtual void StopVideo();
	virtual void SeekVideo(LONGLONG pos);

	virtual void GetVideoSize(SIZE& size);
	virtual int GetVolume(); // -100~0，分贝
	virtual void SetVolume(int volume); // -100~0，分贝

	// 设置需要呈现时的回调函数
	virtual void SetOnPresent(std::function<void(IMediaSample*)> onPresent);
	// 设置接收事件的窗口和消息，lParam是IMediaEventEx*
	virtual void SetNotifyWindow(HWND hwnd, UINT messageID);

protected:
	CComPtr<IGraphBuilder> m_graph;
	CComPtr<IMediaControl> m_control;
	CComPtr<IMediaSeeking> m_seeking;
	CComPtr<IMediaEventEx> m_event;
	CComPtr<IBasicAudio> m_basicAudio;

	CComPtr<IBaseFilter> m_source;
	CComPtr<IBaseFilter> m_audioRenderer;

	SIZE m_videoSize;
	// 需要呈现时被调用
	/*
	类模板std::function是一种通用的多态函数包装器。std::function可以存储，复制和调用任何Callable 目标的实例- 函数，
	lambda表达式，绑定表达式或其他函数对象，以及指向成员函数和指向数据成员的指针。
	所存储的可调用对象被称为目标的std::function。如果a不std::function包含目标，则将其称为空。调用目标的的空std::function的结果的std::bad_function_call抛出异常。
	std::function满足CopyConstructible和CopyAssignable的要求
	*/
//public:
	std::function<void(IMediaSample*)> m_onPresent;


	// CBaseVideoRenderer
private:
	HRESULT CheckMediaType(const CMediaType *);
	HRESULT DoRenderSample(IMediaSample *pMediaSample);
};
