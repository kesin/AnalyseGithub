
#define NETWORK_DEBUG
#include <client_network/network_module.h>
#include <simple_uv/UdpPacket.h>
#include <iostream>
#include <Windows.h>
using namespace std;
UdpPacket testmsg;
#pragma comment(lib,"client_network.lib")

#define WITHOUT_OPENCV

#ifdef WITHOUT_OPENCV
int main() {
		int you, him;
		PRINT_DEBUG("cin you:");
		cin >> you;
		ClientNetwork* network = ClientNetwork::GetInstance();
		while (1) {
			network->Login("192.168.1.192", 12345, you, 0);
		}
		//ClientNetwork::DestroyInstance();
		PRINT_DEBUG("cin him:");
		cin >> him;
			network->AddGroupClientNode(him);
			//Sleep(1000);
			//network->RemoveGroupClientNode(him);
			//Sleep(1000);
			//network->AddGroupClientNode(him);
		testmsg.type = 0x40;
		while (1) {
			int a;
			cin >> a;
			network->sendVoipData(((char*)&testmsg) + 4, 12 - 4);
		}

}
#else

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "./voip/voip_context.h"
#include "src\media_datasource_manager.h"
#include <Windows.h>
#include <atomic>
//#include "vld.h"
using namespace std;
using namespace cv;

atomic<int> g_nCount(0);

FILE *fp = fopen("test.h264", "wb+");
int nTime = 0;

class CVoipCallBack : public VOIP::Voip::Callback {
public:

	virtual void onRemoteAudioChanged(bool enable) {};
	virtual void onRemoteVideoChanged(bool enable) {};
	virtual void onLocalVideoChanged(bool enable) {};
	virtual void onAudioMsgRecorderNotify(int code) {};
	virtual void onAudioMsgData(SharedPtr<Buffer> buf, bool isHeader) {};
	virtual void onAudioMsgPlayerNotify(int code) {};
	virtual void onVideoFrame(bool isLocal, SharedPtr<VOIP::MediaData> image, uint32_t userId)
	{
		if (isLocal)
		{
			return;
		}

		VOIP::VideoRawDataParameter param = image->parameter.getValue<VOIP::VideoRawDataParameter>();
		uint32_t size = param.width*param.height * 3 / 2;

		cv::Mat yuvImg;
		yuvImg.create(param.height * 3 / 2, param.width, CV_8UC1);
		memcpy(yuvImg.data, &image->buffer.byte(0), param.width*param.height * 3 / 2);

		fwrite(&image->buffer.byte(0), 1, size, fp);

		if (nTime++ > 2000)
		{
			fclose(fp);
			exit(1);
		}

		cv::Mat rgbImg;
		cv::cvtColor(yuvImg, rgbImg, CV_YUV2BGR_I420);

		cv::imshow("rgbImg", rgbImg);
		char c = cvWaitKey(25);
		g_nCount++;
	};
	virtual void onAudioAnalysis(bool isWind, int dB) {};
public:
	int m_nWidth;
	int m_nHeight;
	int m_nFmt;
	// VideoInfo m_vi;
private:
	 // CUsrManager * m_usrManager;
};


class CConnectListener : public IConnectListener {
public:
	virtual void MessageServerConnectStart(int status, char *msg) {};
	virtual void MessageServerConnectSuccess(int status, char *msg) {};
	virtual void MessageServerConnectError(int status, char *msg) {};
	virtual void MessageServerConnectTimeout(int status, char *msg) {};

	virtual void MessageServerDisconnectStart(int status, char *msg) {};
	virtual void MessageServerDisconnectSuccess(int status, char *msg) {};
	virtual void MessageServerDisconnectError(int status, char *msg) {};

	virtual void MessageServerLoginStart(int status, char *msg) {};
	virtual void MessageServerLoginSuccess(int status, char *msg) {};
	virtual void MessageServerLoginError(int status, char *msg) {};
	virtual void MessageServerKickoff(int status, char *msg) {};

	virtual void UdpServerConnectSuccess(int status, char *msg) {};
	virtual void UdpServerConnectError(int status, char *msg) {
		return;
	};
	virtual void UdpServerDisconnectSuccess(int status, char *msg) {};
	virtual void UdpServerDisconnectError(int status, char *msg) {};


	virtual void TcpServerHeartbeating(int status, char *msg) {};
	virtual void TcpServerHeartbeatSuccess(int status, char *msg) {};
	virtual void TcpServerHeartbeatError(int status, char *msg) {};

	virtual void P2pConnectSuccess(int status, char *msg) {};
	virtual void P2pConnectError(int status, char *msg) {};
	virtual void P2pDisconnectSuccess(int status, char *msg) {};
};

int nSelfID = 0;
int nDstID = 0;

int TEST2(int argc, char *argv[])
{
	nSelfID = atoi(argv[1]);
	nDstID = atoi(argv[2]);
	cv::VideoCapture cap(0);
	cv::Mat rgbImg;
	cv::Mat yuvImg;
	uint16_t width = 512;
	uint16_t height = 256;
	yuvImg.create(height * 3 / 2, width, CV_8UC1);

	ClientNetwork *client = ClientNetwork::GetInstance();

	VOIP::Voip *m_voip;
	CVoipCallBack *m_voipCallBack = new CVoipCallBack;
	CConnectListener connectListener;
	VOIP::Voip::Parameter para{ ".", CPU_IPHONE6S, 0, 1.0, 1.0, 1.0, 1.0, 0, 0, 10, 0 };
	m_voip = VOIP::Voip::createInstance(para, m_voipCallBack);


	client->Login("103.202.141.87", 12345, 1, 0);
	
	for (int i = 2; i < 9999; i++)
	{

		//m_voip->startGroupCallByAgent("", 1, client);

		m_voip->startAudioPipeline(VOIP::Voip::AudioParameter(16000, false, false));
		m_voip->startVideoPipeline(VOIP::Voip::VideoParameter(512, 1024 * 16 / 9, false));

		// Sleep(1000);
		client->AddGroupClientNode(1);

		cout << "i = " << i << endl;
		
		if (!cap.isOpened()) {
			cout << "不能打开摄像头设备！" << endl;
			return 0;
		}
		
		while (true)
		{
			if (!cap.read(rgbImg))
			{
				assert(false);
				Sleep(50);
				continue;
			}

			imshow("rgbImg2", rgbImg);

			cv::resize(rgbImg, rgbImg, cv::Size(512, 256), 0.0, 0.0, cv::INTER_LINEAR);
			cv::cvtColor(rgbImg, yuvImg, CV_BGR2YUV_I420);
			static const uint8_t VIDEO_DATA[320 * 240 * 3 / 2] = { 0 };

			VOIP::MediaDatasourceManager::getInstance().setVideoData(
				string(VOIP::DEFAULT_VIDEO_INPUT_SOURCE_ID),
				yuvImg.data,
				width * height * 3 / 2,
				width,
				height,
				VOIP::VIDEO_FORMAT_I420,
				VIDEO_ROTATION_0,
				0,
				0);

			char c = cvWaitKey(50);
			// if (c == 27)break;

			if (g_nCount > 20)
			{
				client->RemoveGroupClientNode(i);
				// client->ClearGroupClientList();
				m_voip->removeGroupClientNode(i);
				m_voip->stopCall();
				g_nCount = 0;
				break;
			}
		}
	}
	

	return 0;
}
#endif